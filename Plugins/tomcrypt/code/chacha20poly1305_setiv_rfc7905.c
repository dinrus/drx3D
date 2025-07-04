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
  Set IV + counter data (with RFC7905-magic) to the ChaCha20Poly1305 state and reset the context
  @param st     The ChaCha20Poly1305 state
  @param iv     The IV data to add
  @param ivlen  The length of the IV (must be 12 or 8)
  @param sequence_number   64bit sequence number which is incorporated into IV as described in RFC7905
  @return CRYPT_OK on success
 */
i32 chacha20poly1305_setiv_rfc7905(chacha20poly1305_state *st, u8k *iv, u64 ivlen, ulong64 sequence_number)
{
   i32 i;
   u8 combined_iv[12] = { 0 };

   LTC_ARGCHK(st != NULL);
   LTC_ARGCHK(iv != NULL);
   LTC_ARGCHK(ivlen == 12);

   STORE64L(sequence_number, combined_iv + 4);
   for (i = 0; i < 12; i++) combined_iv[i] = iv[i] ^ combined_iv[i];
   return chacha20poly1305_setiv(st, combined_iv, 12);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
