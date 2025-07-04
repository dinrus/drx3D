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
  @file crypt_unregister_prng.c
  Unregister a PRNG, Tom St Denis
*/

/**
  Unregister a PRNG from the descriptor table
  @param prng   The PRNG descriptor to remove
  @return CRYPT_OK on success
*/
i32 unregister_prng(const struct ltc_prng_descriptor *prng)
{
   i32 x;

   LTC_ARGCHK(prng != NULL);

   /* is it already registered? */
   LTC_MUTEX_LOCK(&ltc_prng_mutex);
   for (x = 0; x < TAB_SIZE; x++) {
       if (XMEMCMP(&prng_descriptor[x], prng, sizeof(struct ltc_prng_descriptor)) == 0) {
          prng_descriptor[x].name = NULL;
          LTC_MUTEX_UNLOCK(&ltc_prng_mutex);
          return CRYPT_OK;
       }
   }
   LTC_MUTEX_UNLOCK(&ltc_prng_mutex);
   return CRYPT_ERROR;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
