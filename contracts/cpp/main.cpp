#include <cstdio>
#include <iostream>
#include <memory>

#include "qtum.hpp"

// use qtum;

void init(qtum::Context& ctx) {}
void handle(qtum::Context& ctx) {}

int main(int argc, char const* argv[]) {
  auto err = std::unique_ptr<qtum::Error>{};
  auto ctx = std::unique_ptr<qtum::Context>{};

  std::tie(ctx, err) = qtum::Context::open(argc, argv);
  if (err) {
    qtum::error_exit(*err);
  }

  if (ctx->isInit()) {
    init(*ctx);
  } else {
    handle(*ctx);
  }

  return 0;
}
