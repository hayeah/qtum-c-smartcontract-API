#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "hex.h"
#include "json.h"
#include "qtum.h"

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

  leveldb_t* db = leveldb_open(options, "contractdb", err);
  free(options);

  if (*err != NULL) {
    return;
  }

  ctx->db = db;
}

qtum_context* qtum_context_open(int argc, char** argv, char** err) {
  if (argc < 2) {
    *err = errContextUnknownAction;
    return NULL;
  }

  qtum_context* ctx = (qtum_context*)malloc(sizeof(qtum_context));
  memset(ctx, 0, sizeof(qtum_context));

  if (ctx == NULL) {
    *err = errContextMallocFail;
    return NULL;
  }

  char* action = argv[1];

  switch (action[0]) {
    case 'i':  // "init"
      ctx->action = QTUM_ACTION_INIT;
      break;
    case 'c':  // "call"
      ctx->action = QTUM_ACTION_CALL;
      break;
    default:
      *err = errContextUnknownAction;
      goto exit_err;
  }

  // open contract store
  qtum_context_configure_db(ctx, err);
  if (*err != NULL) {
    goto exit_err;
  }

  if (argc == 3) {
    char* filename = argv[2];

    qtum_context_configure_by_jsonfile(ctx, filename, err);

    if (*err != NULL) {
      goto exit_err;
    }
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
