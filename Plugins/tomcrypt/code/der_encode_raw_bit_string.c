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
  @file der_encode_bit_string.c
  ASN.1 DER, encode a BIT STRING, Tom St Denis
*/


#ifdef LTC_DER

#define getbit(n, k) (((n) & ( 1 << (k) )) >> (k))

/**
  Store a BIT STRING
  @param in       The array of bits to store (8 per char)
  @param inlen    The number of bits to store
  @param out      [out] The destination for the DER encoded BIT STRING
  @param outlen   [in/out] The max size and resulting size of the DER BIT STRING
  @return CRYPT_OK if successful
*/
i32 der_encode_raw_bit_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen)
{
   u64 len, x, y;
   u8 buf;
   i32           err;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* avoid overflows */
   if ((err = der_length_bit_string(inlen, &len)) != CRYPT_OK) {
      return err;
   }

   if (len > *outlen) {
      *outlen = len;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* store header (include bit padding count in length) */
   x = 0;
   y = (inlen >> 3) + ((inlen&7) ? 1 : 0) + 1;

   out[x++] = 0x03;
   if (y < 128) {
      out[x++] = (u8)y;
   } else if (y < 256) {
      out[x++] = 0x81;
      out[x++] = (u8)y;
   } else if (y < 65536) {
      out[x++] = 0x82;
      out[x++] = (u8)((y>>8)&255);
      out[x++] = (u8)(y&255);
   }

   /* store number of zero padding bits */
   out[x++] = (u8)((8 - inlen) & 7);

   /* store the bits in big endian format */
   for (y = buf = 0; y < inlen; y++) {
      buf |= (getbit(in[y/8],7-y%8)?1:0) << (7 - (y & 7));
      if ((y & 7) == 7) {
         out[x++] = buf;
         buf      = 0;
      }
   }
   /* store last byte */
   if (inlen & 7) {
      out[x++] = buf;
   }

   *outlen = x;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
