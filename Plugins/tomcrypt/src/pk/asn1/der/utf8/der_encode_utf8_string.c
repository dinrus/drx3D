/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file der_encode_utf8_string.c
  ASN.1 DER, encode a UTF8 STRING, Tom St Denis
*/


#ifdef LTC_DER

/**
  Store an UTF8 STRING
  @param in       The array of UTF8 to store (one per wchar_t)
  @param inlen    The number of UTF8 to store
  @param out      [out] The destination for the DER encoded UTF8 STRING
  @param outlen   [in/out] The max size and resulting size of the DER UTF8 STRING
  @return CRYPT_OK if successful
*/
i32 der_encode_utf8_string(const wchar_t *in,  u64 inlen,
                           u8 *out, u64 *outlen)
{
   u64 x, y, len;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* get the size */
   for (x = len = 0; x < inlen; x++) {
       if (!der_utf8_valid_char(in[x])) return CRYPT_INVALID_ARG;
       len += der_utf8_charsize(in[x]);
   }

   if (len < 128) {
      y = 2 + len;
   } else if (len < 256) {
      y = 3 + len;
   } else if (len < 65536UL) {
      y = 4 + len;
   } else if (len < 16777216UL) {
      y = 5 + len;
   } else {
      return CRYPT_INVALID_ARG;
   }

   /* too big? */
   if (y > *outlen) {
      *outlen = y;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* encode the header+len */
   x = 0;
   out[x++] = 0x0C;
   if (len < 128) {
      out[x++] = (u8)len;
   } else if (len < 256) {
      out[x++] = 0x81;
      out[x++] = (u8)len;
   } else if (len < 65536UL) {
      out[x++] = 0x82;
      out[x++] = (u8)((len>>8)&255);
      out[x++] = (u8)(len&255);
   } else if (len < 16777216UL) {
      out[x++] = 0x83;
      out[x++] = (u8)((len>>16)&255);
      out[x++] = (u8)((len>>8)&255);
      out[x++] = (u8)(len&255);
   } else {
       /* coverity[dead_error_line] */
      return CRYPT_INVALID_ARG;
   }

   /* store UTF8 */
   for (y = 0; y < inlen; y++) {
       switch (der_utf8_charsize(in[y])) {
          case 1: out[x++] = (u8)in[y]; break;
          case 2: out[x++] = 0xC0 | ((in[y] >> 6) & 0x1F);  out[x++] = 0x80 | (in[y] & 0x3F); break;
          case 3: out[x++] = 0xE0 | ((in[y] >> 12) & 0x0F); out[x++] = 0x80 | ((in[y] >> 6) & 0x3F); out[x++] = 0x80 | (in[y] & 0x3F); break;
#if !defined(LTC_WCHAR_MAX) || LTC_WCHAR_MAX > 0xFFFF
          case 4: out[x++] = 0xF0 | ((in[y] >> 18) & 0x07); out[x++] = 0x80 | ((in[y] >> 12) & 0x3F); out[x++] = 0x80 | ((in[y] >> 6) & 0x3F); out[x++] = 0x80 | (in[y] & 0x3F); break;
#endif
       }
   }

   /* retun length */
   *outlen = x;

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
