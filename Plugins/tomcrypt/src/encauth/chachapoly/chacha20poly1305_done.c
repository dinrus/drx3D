/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_CHACHA20POLY1305_MODE

/**
  Terminate a ChaCha20Poly1305 stream
  @param st      The ChaCha20Poly1305 state
  @param tag     [out] The destination for the MAC tag
  @param taglen  [in/out]  The length of the MAC tag
  @return CRYPT_OK on success
 */
i32 chacha20poly1305_done(chacha20poly1305_state *st, u8 *tag, u64 *taglen)
{
   u8 padzero[16] = { 0 };
   u64 padlen;
   u8 buf[16];
   i32 err;

   LTC_ARGCHK(st != NULL);

   padlen = 16 - (u64)(st->ctlen % 16);
   if (padlen < 16) {
     if ((err = poly1305_process(&st->poly, padzero, padlen)) != CRYPT_OK) return err;
   }
   STORE64L(st->aadlen, buf);
   STORE64L(st->ctlen, buf + 8);
   if ((err = poly1305_process(&st->poly, buf, 16)) != CRYPT_OK)           return err;
   if ((err = poly1305_done(&st->poly, tag, taglen)) != CRYPT_OK)          return err;
   if ((err = chacha_done(&st->chacha)) != CRYPT_OK)                       return err;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
