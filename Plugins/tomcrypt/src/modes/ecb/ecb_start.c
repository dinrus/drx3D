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
   @file ecb_start.c
   ECB implementation, start chain, Tom St Denis
*/


#ifdef LTC_ECB_MODE

/**
   Initialize a ECB context
   @param cipher      The index of the cipher desired
   @param key         The secret key
   @param keylen      The length of the secret key (octets)
   @param num_rounds  Number of rounds in the cipher desired (0 for default)
   @param ecb         The ECB state to initialize
   @return CRYPT_OK if successful
*/
i32 ecb_start(i32 cipher, u8k *key, i32 keylen, i32 num_rounds, symmetric_ECB *ecb)
{
   i32 err;
   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(ecb != NULL);

   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }
   ecb->cipher = cipher;
   ecb->blocklen = cipher_descriptor[cipher].block_length;
   return cipher_descriptor[cipher].setup(key, keylen, num_rounds, &ecb->key);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
