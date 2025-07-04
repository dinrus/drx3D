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
  Process an entire GCM packet in one call.
  @param key               The secret key
  @param keylen            The length of the secret key
  @param iv                The initialization vector
  @param ivlen             The length of the initialization vector
  @param aad               The additional authentication data (header)
  @param aadlen            The length of the aad
  @param in                The plaintext
  @param inlen             The length of the plaintext (ciphertext length is the same)
  @param out               The ciphertext
  @param tag               [out] The MAC tag
  @param taglen            [in/out] The MAC tag length
  @param direction         Encrypt or Decrypt mode (CHACHA20POLY1305_ENCRYPT or CHACHA20POLY1305_DECRYPT)
  @return CRYPT_OK on success
 */
i32 chacha20poly1305_memory(u8k *key, u64 keylen,
                            u8k *iv,  u64 ivlen,
                            u8k *aad, u64 aadlen,
                            u8k *in,  u64 inlen,
                                  u8 *out,
                                  u8 *tag, u64 *taglen,
                            i32 direction)
{
   chacha20poly1305_state st;
   i32 err;

   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(iv  != NULL);
   LTC_ARGCHK(in  != NULL);
   LTC_ARGCHK(out != NULL);
   LTC_ARGCHK(tag != NULL);

   if ((err = chacha20poly1305_init(&st, key, keylen)) != CRYPT_OK)          { goto LBL_ERR; }
   if ((err = chacha20poly1305_setiv(&st, iv, ivlen)) != CRYPT_OK)           { goto LBL_ERR; }
   if (aad && aadlen > 0) {
      if ((err = chacha20poly1305_add_aad(&st, aad, aadlen)) != CRYPT_OK)    { goto LBL_ERR; }
   }
   if (direction == CHACHA20POLY1305_ENCRYPT) {
      if ((err = chacha20poly1305_encrypt(&st, in, inlen, out)) != CRYPT_OK) { goto LBL_ERR; }
   }
   else if (direction == CHACHA20POLY1305_DECRYPT) {
      if ((err = chacha20poly1305_decrypt(&st, in, inlen, out)) != CRYPT_OK) { goto LBL_ERR; }
   }
   else {
      err = CRYPT_INVALID_ARG;
      goto LBL_ERR;
   }
   err = chacha20poly1305_done(&st, tag, taglen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(chacha20poly1305_state));
#endif
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
