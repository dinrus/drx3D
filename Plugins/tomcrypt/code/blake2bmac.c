/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_BLAKE2BMAC

/**
   Initialize an BLAKE2B MAC context.
   @param st       The BLAKE2B MAC state
   @param outlen   The size of the MAC output (octets)
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
i32 blake2bmac_init(blake2bmac_state *st, u64 outlen, u8k *key, u64 keylen)
{
   LTC_ARGCHK(st  != NULL);
   LTC_ARGCHK(key != NULL);
   return blake2b_init(st, outlen, key, keylen);
}

/**
  Process data through BLAKE2B MAC
  @param st      The BLAKE2B MAC state
  @param in      The data to send through HMAC
  @param inlen   The length of the data to HMAC (octets)
  @return CRYPT_OK if successful
*/
i32 blake2bmac_process(blake2bmac_state *st, u8k *in, u64 inlen)
{
   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(in != NULL);
   return blake2b_process(st, in, inlen);
}

/**
   Terminate a BLAKE2B MAC session
   @param st      The BLAKE2B MAC state
   @param mac     [out] The destination of the BLAKE2B MAC authentication tag
   @param maclen  [in/out]  The max size and resulting size of the BLAKE2B MAC authentication tag
   @return CRYPT_OK if successful
*/
i32 blake2bmac_done(blake2bmac_state *st, u8 *mac, u64 *maclen)
{
   LTC_ARGCHK(st     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);
   LTC_ARGCHK(*maclen >= st->blake2b.outlen);

   *maclen = st->blake2b.outlen;
   return blake2b_done(st, mac);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
