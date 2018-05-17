#ifndef QTUM_SMARTCONTRACT_H
#define QTUM_SMARTCONTRACT_H

#include <leveldb/c.h>
#include <stdarg.h>
#include <stdint.h>

typedef enum qtum_action { QTUM_ACTION_INIT, QTUM_ACTION_CALL } qtum_action;

typedef enum {
  QERR_INVALID_CONTEXT = 1,
  QERR_STORAGE,
  QERR_OOM = 255,
} qtum_errcode;

typedef struct qtum_err {
  qtum_errcode code;
  char* message;
} qtum_err;

typedef struct qtum_context {
  qtum_action action;
  char address[20];
  char sender[20];
  uint64_t value;
  uint8_t* data;
  size_t datasize;

  leveldb_t* db;
} qtum_context;

extern void qtum_put(qtum_context* ctx, const uint8_t* key, size_t keylen,
                     const uint8_t* data, size_t datalen, char** err);

extern uint8_t* qtum_get(qtum_context* ctx, const uint8_t* key, size_t keylen,
                         size_t* retlen, char** err);

extern int qtum_error(int errcode, const char* err);

extern qtum_context* qtum_context_open(int argc, char** argv, qtum_err** err);
extern void qtum_context_close(qtum_context* ctx);

extern qtum_err* qtum_err_fmt(qtum_errcode code, const char* fmt, ...);
// copies message string to an allocated struct
extern qtum_err* qtum_err_new(qtum_errcode code, const char* msg);
extern void qtum_err_free(qtum_err* err);

#endif
