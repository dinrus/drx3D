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
  @file katja_free.c
  Free an Katja key, Tom St Denis
*/

#ifdef LTC_MKAT

/**
  Free an Katja key from memory
  @param key   The RSA key to free
*/
void katja_free(katja_key *key)
{
   LTC_ARGCHK(key != NULL);
   mp_clear_multi( key->d,  key->N,  key->dQ,  key->dP,
                   key->qP,  key->p,  key->q, key->pq, NULL);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
