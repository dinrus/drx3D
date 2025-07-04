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
  @file der_decode_object_identifier.c
  ASN.1 DER, Decode Object Identifier, Tom St Denis
*/

#ifdef LTC_DER
/**
  Decode OID data and store the array of integers in words
  @param in      The OID DER encoded data
  @param inlen   The length of the OID data
  @param words   [out] The destination of the OID words
  @param outlen  [in/out] The number of OID words
  @return CRYPT_OK if successful
*/
i32 der_decode_object_identifier(u8k *in,    u64  inlen,
                                       u64 *words, u64 *outlen)
{
   u64 x, y, t, len;
   i32 err;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(words  != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* header is at least 3 bytes */
   if (inlen < 3) {
      return CRYPT_INVALID_PACKET;
   }

   /* must be room for at least two words */
   if (*outlen < 2) {
      *outlen = 2;
      return CRYPT_BUFFER_OVERFLOW;
   }

   /* decode the packet header */
   x = 0;
   if ((in[x++] & 0x1F) != 0x06) {
      return CRYPT_INVALID_PACKET;
   }

   /* get the length */
   if (in[x] < 128) {
      len = in[x++];
   } else {
      if (in[x] < 0x81 || in[x] > 0x82) {
         return CRYPT_INVALID_PACKET;
      }
      y   = in[x++] & 0x7F;
      len = 0;
      while (y--) {
         len = (len << 8) | (u64)in[x++];
      }
   }

   if (len < 1 || (len + x) > inlen) {
      return CRYPT_INVALID_PACKET;
   }

   /* decode words */
   y = 0;
   t = 0;
   while (len--) {
      t = (t << 7) | (in[x] & 0x7F);
      if (!(in[x++] & 0x80)) {
         /* store t */
         if (y >= *outlen) {
            y++;
         } else {
            if (y == 0) {
               words[0] = t / 40;
               words[1] = t % 40;
               y = 2;
            } else {
               words[y++] = t;
            }
         }
         t = 0;
      }
   }

   if (y > *outlen) {
      err =  CRYPT_BUFFER_OVERFLOW;
   } else {
      err =  CRYPT_OK;
   }

   *outlen = y;
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
