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
  Old-style creation of a DSA key
  @param prng          An active PRNG state
  @param wprng         The index of the PRNG desired
  @param group_size    Size of the multiplicative group (octets)
  @param modulus_size  Size of the modulus (octets)
  @param key           [out] Where to store the created key
  @return CRYPT_OK if successful.
*/
i32 dsa_make_key(prng_state *prng, i32 wprng, i32 group_size, i32 modulus_size, dsa_key *key)
{
  i32 err;

  if ((err = dsa_generate_pqg(prng, wprng, group_size, modulus_size, key)) != CRYPT_OK) { return err; }
  if ((err = dsa_generate_key(prng, wprng, key)) != CRYPT_OK) { return err; }

  return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
