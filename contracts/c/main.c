#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "qtum.h"

enum errcode { ERR_ABNORMAL = 1, ERR_FORBIDDEN = 2 };

const uint8_t* keyOwner = (uint8_t*)"owner";

void init(qtum_context* ctx) {
  qtum_err* err = NULL;
  qtum_put(ctx, keyOwner, sizeof(keyOwner), ctx->sender, ADDRESS_SIZE, &err);
  if (err) qtum_exit_error(err);

  printf("put owner: %s\n", bytesToHexString(ctx->sender, ADDRESS_SIZE));
}

void handle(qtum_context* ctx) {
  qtum_err* err = NULL;

  size_t datalen;
  uint8_t* owner = qtum_get(ctx, keyOwner, sizeof(keyOwner), &datalen, &err);
  if (err) qtum_exit_error(err);

  printf("get owner: %s\n", bytesToHexString(owner, ADDRESS_SIZE));
}

int main(int argc, char** argv) {
  qtum_err* err = NULL;
  qtum_context* ctx = qtum_context_open(argc, argv, &err);
  if (err) qtum_exit_error(err);

  printf("contract: %s\n", bytesToHexString(ctx->address, 20));
  printf("sender: %s\n", bytesToHexString(ctx->sender, ADDRESS_SIZE));

  switch (ctx->action) {
    case QTUM_ACTION_INIT:
      init(ctx);
      break;
    case QTUM_ACTION_CALL:
      handle(ctx);
      break;
  }

  return 0;
}
