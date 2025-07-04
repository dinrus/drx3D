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
  @file der_encode_boolean.c
  ASN.1 DER, encode a BOOLEAN, Tom St Denis
*/


#ifdef LTC_DER

/**
  Store a BOOLEAN
  @param in       The boolean to encode
  @param out      [out] The destination for the DER encoded BOOLEAN
  @param outlen   [in/out] The max size and resulting size of the DER BOOLEAN
  @return CRYPT_OK if successful
*/
i32 der_encode_boolean(i32 in,
                       u8 *out, u64 *outlen)
{
   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(out    != NULL);

   if (*outlen < 3) {
       *outlen = 3;
       return CRYPT_BUFFER_OVERFLOW;
   }

   *outlen = 3;
   out[0] = 0x01;
   out[1] = 0x01;
   out[2] = in ? 0xFF : 0x00;

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
