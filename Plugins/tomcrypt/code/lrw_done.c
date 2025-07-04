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
   @file lrw_done.c
   LRW_MODE implementation, Free resources, Tom St Denis
*/

#ifdef LTC_LRW_MODE

/**
  Terminate a LRW state
  @param lrw   The state to terminate
  @return CRYPT_OK if successful
*/
i32 lrw_done(symmetric_LRW *lrw)
{
   i32 err;

   LTC_ARGCHK(lrw != NULL);

   if ((err = cipher_is_valid(lrw->cipher)) != CRYPT_OK) {
      return err;
   }
   cipher_descriptor[lrw->cipher].done(&lrw->key);

   return CRYPT_OK;
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
