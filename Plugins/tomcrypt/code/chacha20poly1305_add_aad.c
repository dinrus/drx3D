/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_CHACHA20POLY1305_MODE

/**
  Add AAD to the ChaCha20Poly1305 state
  @param st     The ChaCha20Poly1305 state
  @param in     The additional authentication data to add to the ChaCha20Poly1305 state
  @param inlen  The length of the ChaCha20Poly1305 data.
  @return CRYPT_OK on success
 */
i32 chacha20poly1305_add_aad(chacha20poly1305_state *st, u8k *in, u64 inlen)
{
   i32 err;

   if (inlen == 0) return CRYPT_OK; /* nothing to do */
   LTC_ARGCHK(st != NULL);

   if (st->aadflg == 0) return CRYPT_ERROR;
   if ((err = poly1305_process(&st->poly, in, inlen)) != CRYPT_OK) return err;
   st->aadlen += (ulong64)inlen;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
