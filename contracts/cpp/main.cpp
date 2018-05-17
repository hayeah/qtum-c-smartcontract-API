#include <cstdio>
#include <iostream>
#include <memory>

#include "qtum.hpp"

const auto kOwner = std::string("owner");

void init(qtum::Context& ctx) {
  // Set address owner;
  auto err = ctx.put(kOwner, ctx.sender);
  if (err) {
    qtum::error_exit(*err);
  }
}
void handle(qtum::Context& ctx) {
  auto err = std::unique_ptr<qtum::Error>{};
  auto owner = std::unique_ptr<qtum::Address>{};

  std::tie(owner, err) = ctx.getAddress(kOwner);
  if (err) {
    qtum::error_exit(*err);
  }

  std::cout << "owner:" << owner->hexstr() << std::endl;
}

int main(int argc, char const* argv[]) {
  auto err = std::unique_ptr<qtum::Error>{};
  auto ctx = std::unique_ptr<qtum::Context>{};

  std::tie(ctx, err) = qtum::Context::open(argc, argv);
  if (err) {
    qtum::error_exit(*err);
  }

  auto& sender = ctx->sender;
  auto& address = ctx->address;

  std::cout << "contract:" << address.hexstr() << std::endl;
  std::cout << "sender:" << sender.hexstr() << std::endl;

  if (ctx->isInit()) {
    init(*ctx);
  } else {
    handle(*ctx);
  }

  return 0;
}
