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
  @file f8_decrypt.c
  F8 implementation, decrypt data, Tom St Denis
*/

#ifdef LTC_F8_MODE

/**
   F8 decrypt
   @param ct      Ciphertext
   @param pt      [out] Plaintext
   @param len     Length of ciphertext (octets)
   @param f8      F8 state
   @return CRYPT_OK if successful
*/
i32 f8_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_F8 *f8)
{
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(f8 != NULL);
   return f8_encrypt(ct, pt, len, f8);
}


#endif



/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
