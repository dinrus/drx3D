/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file ocb3_encrypt_authenticate_memory.c
  OCB implementation, encrypt block of memory, by Tom St Denis
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_OCB3_MODE

/**
   Encrypt and generate an authentication code for a buffer of memory
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce (length of the block ciphers block size)
   @param noncelen   The length of the nonce (octets)
   @param adata      The AAD - additional associated data
   @param adatalen   The length of AAD (octets)
   @param pt         The plaintext
   @param ptlen      The length of the plaintext (octets)
   @param ct         [out] The ciphertext
   @param tag        [out] The authentication tag
   @param taglen     [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
i32 ocb3_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *adata,  u64 adatalen,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen)
{
   i32 err;
   ocb3_state *ocb;

   LTC_ARGCHK(taglen != NULL);

   /* allocate memory */
   ocb = XMALLOC(sizeof(ocb3_state));
   if (ocb == NULL) {
      return CRYPT_MEM;
   }

   if ((err = ocb3_init(ocb, cipher, key, keylen, nonce, noncelen, *taglen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if (adata != NULL || adatalen != 0) {
      if ((err = ocb3_add_aad(ocb, adata, adatalen)) != CRYPT_OK) {
         goto LBL_ERR;
      }
   }

   if ((err = ocb3_encrypt_last(ocb, pt, ptlen, ct)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = ocb3_done(ocb, tag, taglen);

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(ocb, sizeof(ocb3_state));
#endif

   XFREE(ocb);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
