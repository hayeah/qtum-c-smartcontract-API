// adapted from
// https://opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/LocalTests/XTSTest/hexString.c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEX(c)                                 \
  ((c >= '0' && c <= '9')                      \
       ? c - '0'                               \
       : (c >= 'a' && c <= 'f') ? c - 'a' + 10 \
                                : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1)

/* Convert a string of characters representing a hex buffer into a series of
 * bytes of that real value */
int decodeHexString(char *inhex, uint8_t *out, size_t len) {
  uint8_t *p;
  int i;

  for (i = 0, p = (uint8_t *)inhex; i < len; i++, p += 2) {
    int c1 = HEX(*p);
    int c2 = HEX(*(p + 1));

    if (c1 == -1 || c2 == -1) {
      return 0;
    }

    out[i] = (uint8_t)c1 << 4 | c2;
    // ;
  }

  return 1;
}

static char byteMap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static int byteMapLen = sizeof(byteMap);

/* Utility function to convert nibbles (4 bit values) into a hex character
 * representation */
static char nibbleToChar(uint8_t nibble) {
  if (nibble < byteMapLen) return byteMap[nibble];
  return '*';
}

/* Convert a buffer of binary values into a hex string representation */
char *bytesToHexString(const uint8_t *bytes, size_t buflen) {
  char *retval;
  int i;

  retval = malloc(buflen * 2 + 1);
  for (i = 0; i < buflen; i++) {
    retval[i * 2] = nibbleToChar(bytes[i] >> 4);
    retval[i * 2 + 1] = nibbleToChar(bytes[i] & 0x0f);
  }
  retval[buflen * 2] = '\0';
  return retval;
}
