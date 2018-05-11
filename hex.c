// adapted from
// https://opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/LocalTests/XTSTest/hexString.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HEX(c)                                 \
  ((c >= '0' && c <= '9')                      \
       ? c - '0'                               \
       : (c >= 'a' && c <= 'f') ? c - 'a' + 10 \
                                : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1)

/* Convert a string of characters representing a hex buffer into a series of
 * bytes of that real value */
int decodeHexString(char *inhex, uint8_t *out, size_t len) {
  uint8_t* p;
  int i;

  for (i = 0,  p = (uint8_t *)inhex; i < len; i++, p += 2) {
    int c1 = HEX(*p);
    int c2 = HEX(*(p + 1));

    if (c1 == -1 || c2 == -1) {
      return 0;
    }

    out[i] = (uint8_t) c1 << 4 | c2;
    // ;
  }

  return 1;
}

static char byteMap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void printHex(uint8_t* bytes, size_t buflen) {
  for (int i = 0; i < buflen; i++) {
    char c1 = byteMap[bytes[i] >> 4];
    char c2 = byteMap[bytes[i] & 0x0f];

    putchar(c1);
    putchar(c2);
  }
  putchar('\n');
}
