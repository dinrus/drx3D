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
   @file ecb_done.c
   ECB implementation, finish chain, Tom St Denis
*/

#ifdef LTC_ECB_MODE

/** Terminate the chain
  @param ecb    The ECB chain to terminate
  @return CRYPT_OK on success
*/
i32 ecb_done(symmetric_ECB *ecb)
{
   i32 err;
   LTC_ARGCHK(ecb != NULL);

   if ((err = cipher_is_valid(ecb->cipher)) != CRYPT_OK) {
      return err;
   }
   cipher_descriptor[ecb->cipher].done(&ecb->key);
   return CRYPT_OK;
}



#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
