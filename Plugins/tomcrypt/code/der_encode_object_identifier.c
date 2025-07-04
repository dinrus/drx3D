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
  @file der_encode_object_identifier.c
  ASN.1 DER, Encode Object Identifier, Tom St Denis
*/

#ifdef LTC_DER
/**
  Encode an OID
  @param words   The words to encode  (upto 32-bits each)
  @param nwords  The number of words in the OID
  @param out     [out] Destination of OID data
  @param outlen  [in/out] The max and resulting size of the OID
  @return CRYPT_OK if successful
*/
i32 der_encode_object_identifier(u64 *words, u64  nwords,
                                 u8 *out,   u64 *outlen)
{
   u64 i, x, y, z, t, mask, wordbuf;
   i32           err;

   LTC_ARGCHK(words  != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* check length */
   if ((err = der_length_object_identifier(words, nwords, &x)) != CRYPT_OK) {
      return err;
   }
   if (x > *outlen) {
      *outlen = x;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* compute length to store OID data */
   z = 0;
   wordbuf = words[0] * 40 + words[1];
   for (y = 1; y < nwords; y++) {
       t = der_object_identifier_bits(wordbuf);
       z += t/7 + ((t%7) ? 1 : 0) + (wordbuf == 0 ? 1 : 0);
       if (y < nwords - 1) {
          wordbuf = words[y + 1];
       }
   }

   /* store header + length */
   x = 0;
   out[x++] = 0x06;
   if (z < 128) {
      out[x++] = (u8)z;
   } else if (z < 256) {
      out[x++] = 0x81;
      out[x++] = (u8)z;
   } else if (z < 65536UL) {
      out[x++] = 0x82;
      out[x++] = (u8)((z>>8)&255);
      out[x++] = (u8)(z&255);
   } else {
      return CRYPT_INVALID_ARG;
   }

   /* store first byte */
   wordbuf = words[0] * 40 + words[1];
   for (i = 1; i < nwords; i++) {
      /* store 7 bit words in little endian */
      t    = wordbuf & 0xFFFFFFFF;
      if (t) {
         y    = x;
         mask = 0;
         while (t) {
            out[x++] = (u8)((t & 0x7F) | mask);
            t    >>= 7;
            mask  |= 0x80;  /* upper bit is set on all but the last byte */
         }
         /* now swap bytes y...x-1 */
         z = x - 1;
         while (y < z) {
            t = out[y]; out[y] = out[z]; out[z] = (u8)t;
            ++y;
            --z;
         }
      } else {
         /* zero word */
         out[x++] = 0x00;
      }

      if (i < nwords - 1) {
         wordbuf = words[i + 1];
      }
   }

   *outlen = x;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
