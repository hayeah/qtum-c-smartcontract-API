#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "hex.h"
#include "json.h"
#include "qtum.h"

enum errcode { ERR_ABNORMAL = 1, ERR_FORBIDDEN = 2 };

const char* keyOwner = "owner";

int init(qtum_context* ctx, char** err) {
  printHex((uint8_t*)&ctx->address, 20);
  printHex((uint8_t*)&ctx->sender, 20);
  printHex((uint8_t*)ctx->data, ctx->datasize);

  qtum_put(ctx, keyOwner, sizeof(keyOwner), ctx->sender, 20, err);

  if (*err != NULL) {
    return ERR_ABNORMAL;
  }

  return 0;
}

int isOwner(qtum_context* ctx, char** err) {
  size_t datalen;
  char* owner = qtum_get(ctx, keyOwner, sizeof(keyOwner), &datalen, err);
  int result = memcmp(owner, &ctx->sender, 20) == 0;
  free(owner);
  return result;
}

int handle(qtum_context* ctx, char** err) {
  int authok = isOwner(ctx, err);
  if (*err != NULL) {
    return ERR_ABNORMAL;
  }

  if (!authok) {
    *err = "Not contract owner";
    return ERR_FORBIDDEN;
  }

  printHex((uint8_t*)&ctx->address, 20);
  printHex((uint8_t*)&ctx->sender, 20);
  printHex((uint8_t*)ctx->data, ctx->datasize);

  return 0;
}

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
