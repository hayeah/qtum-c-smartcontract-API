#ifndef QTUM_SMARTCONTRACT_H
#define QTUM_SMARTCONTRACT_H

#include <stdint.h>

typedef enum qtum_action { QTUM_ACTION_INIT, QTUM_ACTION_CALL } qtum_action;

typedef struct qtum_context {
  qtum_action action;
  char address[20];
  char sender[20];
  uint64_t value;
  uint8_t* data;
  size_t datasize;
} qtum_context;

int qtum_error(int errcode, const char* err);

qtum_context* qtum_context_open(int argc, char** argv, char** err);
void qtum_context_close(qtum_context* ctx);

#endif
