#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "hex.h"
#include "json.h"
#include "qtum.h"

const char* CONTRACT_DB_PATH = ".contractdb";

char* errContextMallocFail = "Failed to allocate context";
char* errContextUnknownAction = "Unknown context action";
char* errContextFileOpenError = "Failed to open context file";
char* errContextFileReadError = "Failed to read context file";
char* errContextFileJSONParseError = "Unable to parse context file as JSON";
char* errContextFileInvalid = "Invalid context file";
char* errContextInvalidAddress = "Invalid address";
char* errContextInvalidHexString = "Invalid hexstring";

void qtum_context_configure(json_value* val, qtum_context* ctx, char** err);
void qtum_context_configure_by_jsonfile(qtum_context* ctx, const char* filename,
                                        char** err);

int qtum_error(int errcode, const char* err) {
  printf("[%d] %s\n", errcode, err);
  return errcode;
}

void qtum_context_configure_by_jsonfile(qtum_context* ctx, const char* filename,
                                        char** err) {
  struct stat filestatus;

  if (stat(filename, &filestatus) != 0) {
    *err = errContextFileOpenError;
    return;
  }

  size_t fsize = filestatus.st_size;

  // load context
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    *err = errContextFileOpenError;
    return;
  }

  char* fileContent = (char*)malloc(fsize);
  if (fileContent == NULL) {
    *err = errContextFileReadError;
    goto exit_fp;
  }

  if (fread(fileContent, fsize, 1, fp) != 1) {
    *err = errContextFileReadError;
    goto exit_fileContent;
  }

  json_value* val = json_parse(fileContent, fsize);
  if (val == NULL) {
    *err = errContextFileJSONParseError;
    goto exit_fileContent;
  }

  qtum_context_configure(val, ctx, err);

  // printf("json val type: %d\n", val->type);

  json_value_free(val);
exit_fileContent:
  free(fileContent);
exit_fp:
  fclose(fp);
}

void decodeAddress(json_value* val, uint8_t* out, char** err) {
  if (val->type != json_string) {
    *err = errContextInvalidAddress;
    return;
  }

  size_t hexsize = val->u.string.length / 2;
  if (hexsize != 20) {
    *err = errContextInvalidAddress;
    return;
  }

  char* hexstr = val->u.string.ptr;

  if (!decodeHexString(hexstr, out, hexsize)) {
    *err = errContextInvalidAddress;
    return;
  }
}

void decodeHexData(json_value* val, uint8_t** ret, size_t* retsize,
                   char** err) {
  if (val->type != json_string) {
    goto exit_err;
  }

  size_t hexsize = val->u.string.length / 2;
  char* hexstr = val->u.string.ptr;

  uint8_t* out = (uint8_t*)malloc(hexsize);

  if (!decodeHexString(hexstr, out, hexsize)) {
    free(out);
    goto exit_err;
  }

  *ret = out;
  *retsize = hexsize;
  return;

exit_err:
  *err = errContextInvalidHexString;
  return;
}

void qtum_context_configure(json_value* val, qtum_context* ctx, char** err) {
  if (val->type != json_array) {
    *err = errContextFileInvalid;
    return;
  }

  int length = val->u.array.length;

  if (length < 3) {
    *err = errContextFileInvalid;
    return;
  }

  json_value** values = val->u.array.values;

  decodeAddress(values[0], (uint8_t*)ctx->address, err);
  if (*err != NULL) {
    *err = "Invalid contract address";
    return;
  }

  decodeAddress(values[1], (uint8_t*)ctx->sender, err);
  if (*err != NULL) {
    *err = "Invalid sender address";
    return;
  }

  decodeHexData(values[2], &ctx->data, &ctx->datasize, err);
  if (*err != NULL) {
    *err = "Invalid data";
    return;
  }
}

void qtum_context_configure_db(qtum_context* ctx, char** err) {
  leveldb_options_t* options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);

  leveldb_t* db = leveldb_open(options, CONTRACT_DB_PATH, err);
  free(options);

  if (*err != NULL) {
    return;
  }

  ctx->db = db;
}

qtum_context* qtum_context_open(int argc, char** argv, qtum_err** err) {
  if (argc < 2) {
    *err =
        qtum_err_new(QERR_INVALID_CONTEXT, "Context action is not specified");
    return NULL;
  }

  qtum_context* ctx = (qtum_context*)malloc(sizeof(qtum_context));
  if (ctx == NULL) {
    exit(QERR_OOM);
  }

  memset(ctx, 0, sizeof(qtum_context));

  char* action = argv[1];

  if (strcmp(action, "init") == 0) {
    ctx->action = QTUM_ACTION_INIT;
  } else if (strcmp(action, "call") == 0) {
    ctx->action = QTUM_ACTION_CALL;
  } else {
    *err = qtum_err_fmt(QERR_INVALID_CONTEXT, "Invalid action: %s", action);
    goto exit_err;
  }

  char* serr = NULL;
  if (argc >= 3) {
    char* filename = argv[2];

    qtum_context_configure_by_jsonfile(ctx, filename, &serr);

    if (serr != NULL) {
      *err = qtum_err_new(QERR_INVALID_CONTEXT, serr);
      goto exit_err;
    }
  }

  // open contract store
  qtum_context_configure_db(ctx, &serr);
  if (serr != NULL) {
    *err = qtum_err_new(QERR_STORAGE, serr);
    goto exit_err;
  }

  return ctx;

exit_err:
  qtum_context_close(ctx);
  return NULL;
}

void qtum_context_close(qtum_context* ctx) {
  if (ctx->data != NULL) {
    free(ctx->data);
  }

  if (ctx->db != NULL) {
    leveldb_close(ctx->db);
  }

  free(ctx);
}

void qtum_put(qtum_context* ctx, const char* key, size_t keylen,
              const char* data, size_t datalen, char** err) {
  leveldb_writeoptions_t* woptions = leveldb_writeoptions_create();
  leveldb_put(ctx->db, woptions, key, keylen, data, datalen, err);
  free(woptions);
}

char* qtum_get(qtum_context* ctx, const char* key, size_t keylen,
               size_t* retlen, char** err) {
  leveldb_readoptions_t* roptions = leveldb_readoptions_create();
  char* retval = leveldb_get(ctx->db, roptions, key, keylen, retlen, err);
  free(roptions);
  return retval;
}

qtum_err* qtum_err_fmt(qtum_errcode code, const char* fmt, ...) {
  qtum_err* err = malloc(sizeof(qtum_err));
  if (err == NULL) {
    exit(QERR_OOM);
  }

  err->code = code;
  err->message = NULL;

  va_list args;
  va_start(args, fmt);

  int msgsize = vsnprintf(NULL, 0, fmt, args);
  if (msgsize <= 0) {
    return err;
  }

  char* msg = malloc(msgsize);
  if (msg == NULL) {
    exit(QERR_OOM);
  }

  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);

  err->message = msg;

  return err;
}

qtum_err* qtum_err_new(qtum_errcode code, const char* msg) {
  qtum_err* err = malloc(sizeof(qtum_err));
  if (err == NULL) {
    exit(QERR_OOM);
  }

  err->code = code;
  err->message = NULL;

  if (msg == NULL) {
    return err;
  }

  size_t size = strlen(msg);
  char* errmsg = malloc(size);
  if (errmsg == NULL) {
    exit(QERR_OOM);
  }

  memcpy(errmsg, msg, size);

  err->message = errmsg;

  return err;
}

void qtum_err_free(qtum_err* err) {
  if (err->message != NULL) {
    free(err->message);
  }

  free(err);
}
