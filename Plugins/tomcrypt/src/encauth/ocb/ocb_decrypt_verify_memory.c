/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file ocb_decrypt_verify_memory.c
  OCB implementation, helper to decrypt block of memory, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB_MODE

/**
   Decrypt and compare the tag with OCB.
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce (length of the block size of the block cipher)
   @param ct         The ciphertext
   @param ctlen      The length of the ciphertext (octets)
   @param pt         [out] The plaintext
   @param tag        The tag to compare against
   @param taglen     The length of the tag (octets)
   @param stat       [out] The result of the tag comparison (1==valid, 0==invalid)
   @return CRYPT_OK if successful regardless of the tag comparison
*/
i32 ocb_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,
    u8k *ct,     u64 ctlen,
          u8 *pt,
    u8k *tag,    u64 taglen,
          i32           *stat)
{
   i32 err;
   ocb_state *ocb;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(nonce  != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(stat    != NULL);

   /* allocate memory */
   ocb = XMALLOC(sizeof(ocb_state));
   if (ocb == NULL) {
      return CRYPT_MEM;
   }

   if ((err = ocb_init(ocb, cipher, key, keylen, nonce)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   while (ctlen > (u64)ocb->block_len) {
        if ((err = ocb_decrypt(ocb, ct, pt)) != CRYPT_OK) {
            goto LBL_ERR;
        }
        ctlen   -= ocb->block_len;
        pt      += ocb->block_len;
        ct      += ocb->block_len;
   }

   err = ocb_done_decrypt(ocb, ct, ctlen, pt, tag, taglen, stat);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(ocb, sizeof(ocb_state));
#endif

   XFREE(ocb);

   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
