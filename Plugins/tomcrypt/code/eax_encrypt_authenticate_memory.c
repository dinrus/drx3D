/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file eax_encrypt_authenticate_memory.c
  EAX implementation, encrypt a block of memory, by Tom St Denis
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_EAX_MODE

/**
   EAX encrypt and produce an authentication tag
   @param cipher     The index of the cipher desired
   @param key        The secret key to use
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce [use once]
   @param noncelen   The length of the nonce
   @param header     The header for the session
   @param headerlen  The length of the header (octets)
   @param pt         The plaintext
   @param ptlen      The length of the plaintext (octets)
   @param ct         [out] The ciphertext
   @param tag        [out] The destination tag
   @param taglen     [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
i32 eax_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *header, u64 headerlen,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen)
{
   i32 err;
   eax_state *eax;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(taglen != NULL);

   eax = XMALLOC(sizeof(*eax));

   if ((err = eax_init(eax, cipher, key, keylen, nonce, noncelen, header, headerlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if ((err = eax_encrypt(eax, pt, ct, ptlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if ((err = eax_done(eax, tag, taglen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(eax, sizeof(*eax));
#endif

   XFREE(eax);

   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
