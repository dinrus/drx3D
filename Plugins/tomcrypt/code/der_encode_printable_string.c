/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

/**
  @file der_encode_printable_string.c
  ASN.1 DER, encode a printable STRING, Tom St Denis
*/

#ifdef LTC_DER

/**
  Store an printable STRING
  @param in       The array of printable to store (one per char)
  @param inlen    The number of printable to store
  @param out      [out] The destination for the DER encoded printable STRING
  @param outlen   [in/out] The max size and resulting size of the DER printable STRING
  @return CRYPT_OK if successful
*/
i32 der_encode_printable_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen)
{
   u64 x, y, len;
   i32           err;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* get the size */
   if ((err = der_length_printable_string(in, inlen, &len)) != CRYPT_OK) {
      return err;
   }

   /* too big? */
   if (len > *outlen) {
      *outlen = len;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* encode the header+len */
   x = 0;
   out[x++] = 0x13;
   if (inlen < 128) {
      out[x++] = (u8)inlen;
   } else if (inlen < 256) {
      out[x++] = 0x81;
      out[x++] = (u8)inlen;
   } else if (inlen < 65536UL) {
      out[x++] = 0x82;
      out[x++] = (u8)((inlen>>8)&255);
      out[x++] = (u8)(inlen&255);
   } else if (inlen < 16777216UL) {
      out[x++] = 0x83;
      out[x++] = (u8)((inlen>>16)&255);
      out[x++] = (u8)((inlen>>8)&255);
      out[x++] = (u8)(inlen&255);
   } else {
      return CRYPT_INVALID_ARG;
   }

   /* store octets */
   for (y = 0; y < inlen; y++) {
       out[x++] = der_printable_char_encode(in[y]);
   }

   /* retun length */
   *outlen = x;

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
