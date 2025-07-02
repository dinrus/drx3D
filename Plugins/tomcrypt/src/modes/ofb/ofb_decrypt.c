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
  @file ofb_decrypt.c
  OFB implementation, decrypt data, Tom St Denis
*/

#ifdef LTC_OFB_MODE

/**
   OFB decrypt
   @param ct      Ciphertext
   @param pt      [out] Plaintext
   @param len     Length of ciphertext (octets)
   @param ofb     OFB state
   @return CRYPT_OK if successful
*/
i32 ofb_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_OFB *ofb)
{
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(ofb != NULL);
   return ofb_encrypt(ct, pt, len, ofb);
}


#endif



/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
