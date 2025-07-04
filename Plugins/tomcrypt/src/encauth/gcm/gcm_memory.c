/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file gcm_memory.c
   GCM implementation, process a packet, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_GCM_MODE

/**
  Process an entire GCM packet in one call.
  @param cipher            Index of cipher to use
  @param key               The secret key
  @param keylen            The length of the secret key
  @param IV                The initialization vector
  @param IVlen             The length of the initialization vector
  @param adata             The additional authentication data (header)
  @param adatalen          The length of the adata
  @param pt                The plaintext
  @param ptlen             The length of the plaintext (ciphertext length is the same)
  @param ct                The ciphertext
  @param tag               [out] The MAC tag
  @param taglen            [in/out] The MAC tag length
  @param direction         Encrypt or Decrypt mode (GCM_ENCRYPT or GCM_DECRYPT)
  @return CRYPT_OK on success
 */
i32 gcm_memory(      i32           cipher,
               u8k *key,    u64 keylen,
               u8k *IV,     u64 IVlen,
               u8k *adata,  u64 adatalen,
                     u8 *pt,     u64 ptlen,
                     u8 *ct,
                     u8 *tag,    u64 *taglen,
                               i32 direction)
{
    void      *orig;
    gcm_state *gcm;
    i32        err;

    if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
       return err;
    }

    if (cipher_descriptor[cipher].accel_gcm_memory != NULL) {
       return cipher_descriptor[cipher].accel_gcm_memory
                                          (key,   keylen,
                                           IV,    IVlen,
                                           adata, adatalen,
                                           pt,    ptlen,
                                           ct,
                                           tag,   taglen,
                                           direction);
    }



#ifndef LTC_GCM_TABLES_SSE2
    orig = gcm = XMALLOC(sizeof(*gcm));
#else
    orig = gcm = XMALLOC(sizeof(*gcm) + 16);
#endif
    if (gcm == NULL) {
        return CRYPT_MEM;
    }

   /* Force GCM to be on a multiple of 16 so we can use 128-bit aligned operations
    * note that we only modify gcm and keep orig intact.  This code is not portable
    * but again it's only for SSE2 anyways, so who cares?
    */
#ifdef LTC_GCM_TABLES_SSE2
   if ((u64)gcm & 15) {
      gcm = (gcm_state *)((u64)gcm + (16 - ((u64)gcm & 15)));
   }
#endif

    if ((err = gcm_init(gcm, cipher, key, keylen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_add_iv(gcm, IV, IVlen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_add_aad(gcm, adata, adatalen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_process(gcm, pt, ptlen, ct, direction)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    err = gcm_done(gcm, tag, taglen);
LTC_ERR:
    XFREE(orig);
    return err;
}
#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
