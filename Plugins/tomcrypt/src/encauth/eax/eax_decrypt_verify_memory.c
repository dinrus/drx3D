/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
    @file eax_decrypt_verify_memory.c
    EAX implementation, decrypt block of memory, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_EAX_MODE

/**
   Decrypt a block of memory and verify the provided MAC tag with EAX
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the key (octets)
   @param nonce      The nonce data (use once) for the session
   @param noncelen   The length of the nonce data.
   @param header     The session header data
   @param headerlen  The length of the header (octets)
   @param ct         The ciphertext
   @param ctlen      The length of the ciphertext (octets)
   @param pt         [out] The plaintext
   @param tag        The authentication tag provided by the encoder
   @param taglen     [in/out] The length of the tag (octets)
   @param stat       [out] The result of the decryption (1==valid tag, 0==invalid)
   @return CRYPT_OK if successful regardless of the resulting tag comparison
*/
i32 eax_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *header, u64 headerlen,
    u8k *ct,     u64 ctlen,
          u8 *pt,
          u8 *tag,    u64 taglen,
          i32           *stat)
{
   i32            err;
   eax_state     *eax;
   u8 *buf;
   u64  buflen;

   LTC_ARGCHK(stat != NULL);
   LTC_ARGCHK(key  != NULL);
   LTC_ARGCHK(pt   != NULL);
   LTC_ARGCHK(ct   != NULL);
   LTC_ARGCHK(tag  != NULL);

   /* default to zero */
   *stat = 0;

   /* limit taglen */
   taglen = MIN(taglen, MAXBLOCKSIZE);

   /* allocate ram */
   buf = XMALLOC(taglen);
   eax = XMALLOC(sizeof(*eax));
   if (eax == NULL || buf == NULL) {
      if (eax != NULL) {
         XFREE(eax);
      }
      if (buf != NULL) {
         XFREE(buf);
      }
      return CRYPT_MEM;
   }

   if ((err = eax_init(eax, cipher, key, keylen, nonce, noncelen, header, headerlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   if ((err = eax_decrypt(eax, ct, pt, ctlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   buflen = taglen;
   if ((err = eax_done(eax, buf, &buflen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   /* compare tags */
   if (buflen >= taglen && XMEM_NEQ(buf, tag, taglen) == 0) {
      *stat = 1;
   }

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, taglen);
   zeromem(eax, sizeof(*eax));
#endif

   XFREE(eax);
   XFREE(buf);

   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
