#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "hex.h"
#include "json.h"
#include "qtum.h"

void init(qtum_context* ctx, char** err) {}

void handle(qtum_context* ctx, char** err) {
  printHex((uint8_t*) &ctx->address, 20);
  printHex((uint8_t*) &ctx->sender, 20);
  printHex((uint8_t*) ctx->data, ctx->datasize);
}

enum errcode { ERR_ABNORMAL = 1 };

int main(int argc, char** argv) {
  char* err = NULL;
  qtum_context* ctx = qtum_context_open(argc, argv, &err);
  if (err != NULL) {
    goto exit_err;
  }

  switch (ctx->action) {
    case QTUM_ACTION_INIT:
      init(ctx, &err);
      break;
    case QTUM_ACTION_CALL:
      handle(ctx, &err);
      break;
    default:
      err = "Invalid action";
      goto exit_err;
  }

  if (err != NULL) {
    goto exit_err;
  }

exit_err:
  if (ctx != NULL) {
    qtum_context_close(ctx);
  }

  if (err != NULL) {
    return qtum_error(ERR_ABNORMAL, err);
  }

  return 0;
}
