#ifndef QTUM_SMARTCONTRACT_HPP
#define QTUM_SMARTCONTRACT_HPP

#include <memory>
#include <string>

extern "C" {
#include "qtum.h"
}

namespace qtum {

using std::string;

class Error {
 public:
  int code;
  string message;
  Error(int code, string message) : code(code), message(message){};
  Error(qtum_err* err) : code(err->code), message(err->message){};
};

void error_exit(const Error& err) {
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

class Context {
 private:
  qtum_context* ctx;

 public:
  Context(qtum_context* _ctx) { ctx = _ctx; };
  ~Context() { qtum_context_close(ctx); };

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static std::tuple<std::unique_ptr<qtum::Context>,
                    std::unique_ptr<qtum::Error>>
  open(int argc, char const* argv[]);

  bool isInit() const { return ctx->action == QTUM_ACTION_INIT; }
  bool isCall() const { return ctx->action == QTUM_ACTION_CALL; }
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
