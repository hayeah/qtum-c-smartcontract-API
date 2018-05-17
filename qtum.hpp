#ifndef QTUM_SMARTCONTRACT_HPP
#define QTUM_SMARTCONTRACT_HPP

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include "qtum.h"
}

namespace qtum {

using std::string;

constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

std::string hexStr(const char* data, int len) {
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

class Error {
 public:
  int code;
  string message;
  Error(int code, string message) : code(code), message(message){};
  Error(qtum_err* err) : code(err->code), message(err->message){};
};

void exit_error(const Error& err) {
  std::cout << "error: " << err.message << std::endl;
  exit(err.code);
}

std::unique_ptr<Error> error(qtum_err* err) {
  auto cpperr = std::make_unique<Error>(err);
  if (err != nullptr) {
    qtum_err_free(err);
  }
  return cpperr;
};

std::unique_ptr<Error> error(int code, string message) {
  return std::make_unique<qtum::Error>(code, message);
};

#define ADDRESS_SIZE 20

class Address {
 private:
  uint8_t bytes[ADDRESS_SIZE];

 public:
  Address(const uint8_t* _bytes) { memcpy(&bytes, _bytes, ADDRESS_SIZE); }

  string hexstr() const { return hexStr((char*)&bytes, ADDRESS_SIZE); }

  const uint8_t* getRawBytes() const { return (uint8_t*)&bytes; }

  // transfer
};

class Context {
 private:
  qtum_context* ctx;

 public:
  const Address address;
  const Address sender;
  Context(qtum_context* _ctx)
      : address((uint8_t*)&_ctx->address), sender((uint8_t*)&_ctx->sender) {
    ctx = _ctx;
  }

  ~Context() { qtum_context_close(ctx); };

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static std::tuple<std::unique_ptr<qtum::Context>,
                    std::unique_ptr<qtum::Error>>
  open(int argc, char const* argv[]);

  bool isInit() const { return ctx->action == QTUM_ACTION_INIT; }
  bool isCall() const { return ctx->action == QTUM_ACTION_CALL; }

  std::unique_ptr<Error> put(string key, const uint8_t* data, size_t len) {
    qtum_err* err = nullptr;

    qtum_put(ctx, (uint8_t*)key.data(), key.length(), data, len, &err);

    if (err != nullptr) {
      auto qerr = error(err);
      qtum_err_free(err);
      return qerr;
    }

    return nullptr;
  }

  std::unique_ptr<Error> put(string key, const Address& address) {
    return put(key, address.getRawBytes(), ADDRESS_SIZE);
  }

  std::tuple<uint8_t*, size_t, std::unique_ptr<Error>> get(string key) {
    qtum_err* err = nullptr;
    size_t retlen;
    uint8_t* data =
        qtum_get(ctx, (uint8_t*)key.data(), key.length(), &retlen, &err);

    if (err != nullptr) {
      auto qerr = error(err);
      qtum_err_free(err);
      return {nullptr, 0, std::move(qerr)};
    }

    return {data, retlen, nullptr};
  }

  std::tuple<std::unique_ptr<string>, std::unique_ptr<Error>> getString(
      string key) {
    uint8_t* data = nullptr;
    size_t len = 0;
    auto err = std::unique_ptr<Error>{};
    std::tie(data, len, err) = get(key);

    if (err) {
      return {nullptr, std::move(err)};
    }

    auto s = std::make_unique<string>((char*)data, len);
    free(data);

    return {std::move(s), nullptr};
  }

  std::tuple<std::unique_ptr<Address>, std::unique_ptr<Error>> getAddress(
      string key) {
    uint8_t* data = nullptr;
    size_t len = 0;
    auto err = std::unique_ptr<Error>{};
    std::tie(data, len, err) = get(key);

    if (err) {
      return {nullptr, std::move(err)};
    }

    if (len != ADDRESS_SIZE) {
      return {nullptr, error(1, "Invalid address. Length not 20 bytes.")};
    }

    auto address = std::make_unique<Address>(data);
    free(data);

    return {std::move(address), nullptr};
  }
};

std::tuple<std::unique_ptr<qtum::Context>, std::unique_ptr<qtum::Error>>
Context::open(int argc, char const* argv[]) {
  qtum_err* err;
  qtum_context* ctx = qtum_context_open(argc, (char**)argv, &err);

  if (err != nullptr) {
    return {nullptr, error(err)};
  }

  return {std::make_unique<Context>(ctx), nullptr};
}

}  // namespace qtum

#endif
