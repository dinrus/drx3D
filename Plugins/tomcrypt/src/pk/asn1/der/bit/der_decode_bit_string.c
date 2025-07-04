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
  @file der_decode_bit_string.c
  ASN.1 DER, encode a BIT STRING, Tom St Denis
*/


#ifdef LTC_DER

/**
  Store a BIT STRING
  @param in      The DER encoded BIT STRING
  @param inlen   The size of the DER BIT STRING
  @param out     [out] The array of bits stored (one per char)
  @param outlen  [in/out] The number of bits stored
  @return CRYPT_OK if successful
*/
i32 der_decode_bit_string(u8k *in,  u64 inlen,
                                u8 *out, u64 *outlen)
{
   u64 dlen, blen, x, y;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* packet must be at least 4 bytes */
   if (inlen < 4) {
       return CRYPT_INVALID_ARG;
   }

   /* check for 0x03 */
   if ((in[0]&0x1F) != 0x03) {
      return CRYPT_INVALID_PACKET;
   }

   /* offset in the data */
   x = 1;

   /* get the length of the data */
   if (in[x] & 0x80) {
      /* long format get number of length bytes */
      y = in[x++] & 0x7F;

      /* invalid if 0 or > 2 */
      if (y == 0 || y > 2) {
         return CRYPT_INVALID_PACKET;
      }

      /* read the data len */
      dlen = 0;
      while (y--) {
         dlen = (dlen << 8) | (u64)in[x++];
      }
   } else {
      /* short format */
      dlen = in[x++] & 0x7F;
   }

   /* is the data len too long or too short? */
   if ((dlen == 0) || (dlen + x > inlen)) {
       return CRYPT_INVALID_PACKET;
   }

   /* get padding count */
   blen = ((dlen - 1) << 3) - (in[x++] & 7);

   /* too many bits? */
   if (blen > *outlen) {
      *outlen = blen;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* decode/store the bits */
   for (y = 0; y < blen; y++) {
       out[y] = (in[x] & (1 << (7 - (y & 7)))) ? 1 : 0;
       if ((y & 7) == 7) {
          ++x;
       }
   }

   /* we done */
   *outlen = blen;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
