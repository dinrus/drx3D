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
   @file sprng.c
   Secure PRNG, Tom St Denis
*/

/* A secure PRNG using the RNG functions.  Basically this is a
 * wrapper that allows you to use a secure RNG as a PRNG
 * in the various other functions.
 */

#ifdef LTC_SPRNG

const struct ltc_prng_descriptor sprng_desc =
{
    "sprng", 0,
    &sprng_start,
    &sprng_add_entropy,
    &sprng_ready,
    &sprng_read,
    &sprng_done,
    &sprng_export,
    &sprng_import,
    &sprng_test
};

/**
  Start the PRNG
  @param prng     [out] The PRNG state to initialize
  @return CRYPT_OK if successful
*/
i32 sprng_start(prng_state *prng)
{
   LTC_UNUSED_PARAM(prng);
   return CRYPT_OK;
}

/**
  Add entropy to the PRNG state
  @param in       The data to add
  @param inlen    Length of the data to add
  @param prng     PRNG state to update
  @return CRYPT_OK if successful
*/
i32 sprng_add_entropy(u8k *in, u64 inlen, prng_state *prng)
{
   LTC_UNUSED_PARAM(in);
   LTC_UNUSED_PARAM(inlen);
   LTC_UNUSED_PARAM(prng);
   return CRYPT_OK;
}

/**
  Make the PRNG ready to read from
  @param prng   The PRNG to make active
  @return CRYPT_OK if successful
*/
i32 sprng_ready(prng_state *prng)
{
   LTC_UNUSED_PARAM(prng);
   return CRYPT_OK;
}

/**
  Read from the PRNG
  @param out      Destination
  @param outlen   Length of output
  @param prng     The active PRNG to read from
  @return Number of octets read
*/
u64 sprng_read(u8 *out, u64 outlen, prng_state *prng)
{
   LTC_ARGCHK(out != NULL);
   LTC_UNUSED_PARAM(prng);
   return rng_get_bytes(out, outlen, NULL);
}

/**
  Terminate the PRNG
  @param prng   The PRNG to terminate
  @return CRYPT_OK if successful
*/
i32 sprng_done(prng_state *prng)
{
   LTC_UNUSED_PARAM(prng);
   return CRYPT_OK;
}

/**
  Export the PRNG state
  @param out       [out] Destination
  @param outlen    [in/out] Max size and resulting size of the state
  @param prng      The PRNG to export
  @return CRYPT_OK if successful
*/
i32 sprng_export(u8 *out, u64 *outlen, prng_state *prng)
{
   LTC_ARGCHK(outlen != NULL);
   LTC_UNUSED_PARAM(out);
   LTC_UNUSED_PARAM(prng);

   *outlen = 0;
   return CRYPT_OK;
}

/**
  Import a PRNG state
  @param in       The PRNG state
  @param inlen    Size of the state
  @param prng     The PRNG to import
  @return CRYPT_OK if successful
*/
i32 sprng_import(u8k *in, u64 inlen, prng_state *prng)
{
  LTC_UNUSED_PARAM(in);
  LTC_UNUSED_PARAM(inlen);
  LTC_UNUSED_PARAM(prng);
   return CRYPT_OK;
}

/**
  PRNG self-test
  @return CRYPT_OK if successful, CRYPT_NOP if self-testing has been disabled
*/
i32 sprng_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   prng_state st;
   u8 en[] = { 0x01, 0x02, 0x03, 0x04 };
   u8 out[1000];
   i32 err;

   if ((err = sprng_start(&st)) != CRYPT_OK)                         return err;
   if ((err = sprng_add_entropy(en, sizeof(en), &st)) != CRYPT_OK)   return err;
   if ((err = sprng_ready(&st)) != CRYPT_OK)                         return err;
   if (sprng_read(out, 500, &st) != 500)                             return CRYPT_ERROR_READPRNG; /* skip 500 bytes */
   if ((err = sprng_done(&st)) != CRYPT_OK)                          return err;

   return CRYPT_OK;
#endif
}

#endif




/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
