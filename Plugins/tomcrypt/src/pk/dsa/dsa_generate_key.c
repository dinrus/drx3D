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
   @file dsa_make_key.c
   DSA implementation, generate a DSA key
*/

#ifdef LTC_MDSA

/**
  Create a DSA key
  @param prng          An active PRNG state
  @param wprng         The index of the PRNG desired
  @param key           [in/out] Where to store the created key
  @return CRYPT_OK if successful.
*/
i32 dsa_generate_key(prng_state *prng, i32 wprng, dsa_key *key)
{
  i32 err;

  LTC_ARGCHK(key         != NULL);
  LTC_ARGCHK(ltc_mp.name != NULL);

  /* so now we have our DH structure, generator g, order q, modulus p
     Now we need a random exponent [mod q] and it's power g^x mod p
   */
  /* private key x should be from range: 1 <= x <= q-1 (see FIPS 186-4 B.1.2) */
  if ((err = rand_bn_upto(key->x, key->q, prng, wprng)) != CRYPT_OK)          { return err; }
  if ((err = mp_exptmod(key->g, key->x, key->p, key->y)) != CRYPT_OK)            { return err; }
  key->type = PK_PRIVATE;

  return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
