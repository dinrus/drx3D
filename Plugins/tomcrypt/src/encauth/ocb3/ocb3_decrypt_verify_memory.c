/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file ocb3_decrypt_verify_memory.c
  OCB implementation, helper to decrypt block of memory, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Decrypt and compare the tag with OCB
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce (length of the block size of the block cipher)
   @param noncelen   The length of the nonce (octets)
   @param adata      The AAD - additional associated data
   @param adatalen   The length of AAD (octets)
   @param ct         The ciphertext
   @param ctlen      The length of the ciphertext (octets)
   @param pt         [out] The plaintext
   @param tag        The tag to compare against
   @param taglen     The length of the tag (octets)
   @param stat       [out] The result of the tag comparison (1==valid, 0==invalid)
   @return CRYPT_OK if successful regardless of the tag comparison
*/
i32 ocb3_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *adata,  u64 adatalen,
    u8k *ct,     u64 ctlen,
          u8 *pt,
    u8k *tag,    u64 taglen,
          i32           *stat)
{
   i32            err;
   ocb3_state     *ocb;
   u8 *buf;
   u64  buflen;

   LTC_ARGCHK(stat    != NULL);

   /* default to zero */
   *stat = 0;

   /* limit taglen */
   taglen = MIN(taglen, MAXBLOCKSIZE);

   /* allocate memory */
   buf = XMALLOC(taglen);
   ocb = XMALLOC(sizeof(ocb3_state));
   if (ocb == NULL || buf == NULL) {
      if (ocb != NULL) {
         XFREE(ocb);
      }
      if (buf != NULL) {
         XFREE(buf);
      }
      return CRYPT_MEM;
   }

   if ((err = ocb3_init(ocb, cipher, key, keylen, nonce, noncelen, taglen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if (adata != NULL || adatalen != 0) {
      if ((err = ocb3_add_aad(ocb, adata, adatalen)) != CRYPT_OK) {
         goto LBL_ERR;
      }
   }

   if ((err = ocb3_decrypt_last(ocb, ct, ctlen, pt)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   buflen = taglen;
   if ((err = ocb3_done(ocb, buf, &buflen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   /* compare tags */
   if (buflen >= taglen && XMEM_NEQ(buf, tag, taglen) == 0) {
      *stat = 1;
   }

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(ocb, sizeof(ocb3_state));
#endif

   XFREE(ocb);
   XFREE(buf);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
