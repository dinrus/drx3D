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
  @file ecb_encrypt.c
  ECB implementation, encrypt a block, Tom St Denis
*/

#ifdef LTC_ECB_MODE

/**
  ECB encrypt
  @param pt     Plaintext
  @param ct     [out] Ciphertext
  @param len    The number of octets to process (must be multiple of the cipher block size)
  @param ecb    ECB state
  @return CRYPT_OK if successful
*/
i32 ecb_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_ECB *ecb)
{
   i32 err;
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(ecb != NULL);
   if ((err = cipher_is_valid(ecb->cipher)) != CRYPT_OK) {
       return err;
   }
   if (len % cipher_descriptor[ecb->cipher].block_length) {
      return CRYPT_INVALID_ARG;
   }

   /* check for accel */
   if (cipher_descriptor[ecb->cipher].accel_ecb_encrypt != NULL) {
      return cipher_descriptor[ecb->cipher].accel_ecb_encrypt(pt, ct, len / cipher_descriptor[ecb->cipher].block_length, &ecb->key);
   } else {
      while (len) {
         if ((err = cipher_descriptor[ecb->cipher].ecb_encrypt(pt, ct, &ecb->key)) != CRYPT_OK) {
            return err;
         }
         pt  += cipher_descriptor[ecb->cipher].block_length;
         ct  += cipher_descriptor[ecb->cipher].block_length;
         len -= cipher_descriptor[ecb->cipher].block_length;
      }
   }
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
