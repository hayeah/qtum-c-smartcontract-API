#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qtum.h"

typedef struct storekey {
  uint8_t* data;
  size_t length;
} storekey;

const storekey kOwner = {(uint8_t*)"owner", 5};
const storekey kTotalSupply = {(uint8_t*)"totalSupply", 11};

const uint8_t* kBalanceOf = (uint8_t*)"balanceOf";
const size_t kBalanceOf_len = 9;
const size_t kBalanceOfWithAddress_len = 9 + ADDRESS_SIZE;

enum {
  METHOD_TOTAL_SUPPLY = 0,
  METHOD_BALANCE_OF,
  METHOD_MINT
} contract_methods;

// "balanceOf" + address. length is kBalanceOfWithAddress_len
uint8_t* keyBalanceOf(const uint8_t* address) {
  uint8_t* key = malloc(kBalanceOf_len + ADDRESS_SIZE);
  memcpy(key, kBalanceOf, kBalanceOf_len);
  memcpy(key + kBalanceOf_len, address, ADDRESS_SIZE);
  return key;
}

// init(uint64_t supply)
// mint the initial supply to msg.sender.
// calldata: [uint64 (8 bytes)]
void init(qtum_context* ctx) {
  if (ctx->datasize != 8) {
    qtum_exit_error(
        qtum_err_new(QERR_GENERAL, "call data must be 8 bytes uint_64"));
  }

  qtum_err* err = NULL;

  // owner = msg.sender
  qtum_put(ctx, kOwner.data, kOwner.length, ctx->sender, ADDRESS_SIZE, &err);
  if (err) qtum_exit_error(err);

  // initialSupply = supply
  uint8_t* initialSupply = ctx->data;
  qtum_put(ctx, kTotalSupply.data, kTotalSupply.length, initialSupply,
           sizeof(uint64_t), &err);
  if (err) qtum_exit_error(err);

  // store["balance@${address}"] = supply
  uint8_t* balanceKey = keyBalanceOf(ctx->sender);
  qtum_put(ctx, balanceKey, kBalanceOfWithAddress_len, initialSupply,
           sizeof(uint64_t), &err);
  free(balanceKey);
  if (err) qtum_exit_error(err);

  printf("owner: %s\n", bytesToHexString(ctx->sender, ADDRESS_SIZE));
  printf("initial supply: %s\n",
         bytesToHexString(initialSupply, sizeof(uint64_t)));
}

void mint(qtum_context* ctx) {
  // + check owner
  // + update supply
  // + update balance of beneificiary address
}

void totalSupply(qtum_context* ctx) {
  size_t len = 0;
  qtum_err* err = NULL;

  uint8_t* supply =
      qtum_get(ctx, kTotalSupply.data, kTotalSupply.length, &len, &err);
  if (err) qtum_exit_error(err);

  qtum_exit_return(supply, len);
}

// balanceOf [01][address (20 bytes)]
void balanceOf(qtum_context* ctx) {
  if (ctx->datasize != 21) {
    qtum_exit_error(
        qtum_err_new(QERR_GENERAL, "invalid calldata to balanceOf"));
  }

  size_t len = 0;
  qtum_err* err = NULL;

  uint8_t* address = ctx->data + 1;

  uint8_t* balanceKey = keyBalanceOf(address);
  uint8_t* balance =
      qtum_get(ctx, balanceKey, kBalanceOfWithAddress_len, &len, &err);
  free(balanceKey);

  if (err) qtum_exit_error(err);

  qtum_exit_return(balance, len);
}

void handle(qtum_context* ctx) {
  qtum_err* err = NULL;

  // calldata [method (1 byte)][payload (n bytes)]

  if (ctx->datasize < 1) {
    qtum_exit_error(qtum_err_new(QERR_GENERAL, "call data cannot be empty"));
  }

  // totalSupply [00]
  // balanceOf [01][address (20 bytes)]
  // mint address [02][address (20 bytes)]
  uint8_t method = ctx->data[0];
  switch (method) {
    case METHOD_TOTAL_SUPPLY:
      totalSupply(ctx);
      break;
    case METHOD_BALANCE_OF:
      balanceOf(ctx);
      break;
    default:
      qtum_exit_error(qtum_err_new(QERR_GENERAL, "unknown method"));
  }
}

int main(int argc, char** argv) {
  qtum_err* err = NULL;
  qtum_context* ctx = qtum_context_open(argc, argv, &err);
  if (err) qtum_exit_error(err);

  printf("contract: %s\n", bytesToHexString(ctx->address, 20));
  printf("sender: %s\n", bytesToHexString(ctx->sender, ADDRESS_SIZE));
  printf("call data: %s\n", bytesToHexString(ctx->data, ctx->datasize));

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
