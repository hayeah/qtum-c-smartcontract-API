#ifndef QTUM_SMARTCONTRACT_H
#define QTUM_SMARTCONTRACT_H

#include <leveldb/c.h>
#include <stdarg.h>
#include <stdint.h>
#include "hex.h"

#define ADDRESS_SIZE 20

typedef enum qtum_action { QTUM_ACTION_INIT, QTUM_ACTION_CALL } qtum_action;

typedef enum {
  QERR_GENERAL = 1,
  QERR_INVALID_CONTEXT = 100,
  QERR_STORAGE = 101,
  QERR_OOM = 255,

  QERR_MAX = INT32_MAX,
} qtum_errcode;

typedef struct qtum_err {
  qtum_errcode code;
  char* message;
} qtum_err;

typedef struct qtum_context {
  qtum_action action;
  uint8_t address[ADDRESS_SIZE];
  uint8_t sender[ADDRESS_SIZE];
  uint64_t value;
  uint8_t* data;
  size_t datasize;

  leveldb_t* db;
} qtum_context;

extern void qtum_put(qtum_context* ctx, const uint8_t* key, size_t keylen,
                     const uint8_t* data, size_t datalen, qtum_err** err);

extern uint8_t* qtum_get(qtum_context* ctx, const uint8_t* key, size_t keylen,
                         size_t* retlen, qtum_err** err);
// free a generic pointer allocated by storage
extern void qtum_free_get_data(void* ptr);

extern void qtum_exit_return(uint8_t* data, size_t datalen);
extern void qtum_exit_error(qtum_err* err);

extern qtum_context* qtum_context_open(int argc, char** argv, qtum_err** err);
extern void qtum_context_close(qtum_context* ctx);

extern qtum_err* qtum_err_fmt(qtum_errcode code, const char* fmt, ...);
// copies message string to an allocated struct
extern qtum_err* qtum_err_new(qtum_errcode code, const char* msg);
extern void qtum_err_free(qtum_err* err);
extern void qtum_print_version();

#endif
