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
  @file der_decode_utf8_string.c
  ASN.1 DER, encode a UTF8 STRING, Tom St Denis
*/


#ifdef LTC_DER

/**
  Store a UTF8 STRING
  @param in      The DER encoded UTF8 STRING
  @param inlen   The size of the DER UTF8 STRING
  @param out     [out] The array of utf8s stored (one per char)
  @param outlen  [in/out] The number of utf8s stored
  @return CRYPT_OK if successful
*/
i32 der_decode_utf8_string(u8k *in,  u64 inlen,
                                       wchar_t *out, u64 *outlen)
{
   wchar_t       tmp;
   u64 x, y, z, len;
   i32 err;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* must have header at least */
   if (inlen < 2) {
      return CRYPT_INVALID_PACKET;
   }

   /* check for 0x0C */
   if ((in[0] & 0x1F) != 0x0C) {
      return CRYPT_INVALID_PACKET;
   }
   x = 1;

   /* decode the length */
   if (in[x] & 0x80) {
      /* valid # of bytes in length are 1,2,3 */
      y = in[x] & 0x7F;
      if ((y == 0) || (y > 3) || ((x + y) > inlen)) {
         return CRYPT_INVALID_PACKET;
      }

      /* read the length in */
      len = 0;
      ++x;
      while (y--) {
         len = (len << 8) | in[x++];
      }
   } else {
      len = in[x++] & 0x7F;
   }

   if (len + x > inlen) {
      return CRYPT_INVALID_PACKET;
   }

   /* proceed to decode */
   for (y = 0; x < inlen; ) {
      /* get first byte */
      tmp = in[x++];

      /* count number of bytes */
      for (z = 0; (tmp & 0x80) && (z <= 4); z++, tmp = (tmp << 1) & 0xFF);

      if (z > 4 || (x + (z - 1) > inlen)) {
         return CRYPT_INVALID_PACKET;
      }

      /* decode, grab upper bits */
      tmp >>= z;

      /* grab remaining bytes */
      if (z > 1) { --z; }
      while (z-- != 0) {
         if ((in[x] & 0xC0) != 0x80) {
            return CRYPT_INVALID_PACKET;
         }
         tmp = (tmp << 6) | ((wchar_t)in[x++] & 0x3F);
      }

      if (y < *outlen) {
         out[y] = tmp;
      }
      y++;
   }
   if (y > *outlen) {
      err = CRYPT_BUFFER_OVERFLOW;
   } else {
      err = CRYPT_OK;
   }
   *outlen = y;

   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
