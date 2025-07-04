/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
  @file ocb_encrypt_authenticate_memory.c
  OCB implementation, encrypt block of memory, by Tom St Denis
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_OCB_MODE

/**
   Encrypt and generate an authentication code for a buffer of memory
   @param cipher     The index of the cipher desired
   @param key        The secret key
   @param keylen     The length of the secret key (octets)
   @param nonce      The session nonce (length of the block ciphers block size)
   @param pt         The plaintext
   @param ptlen      The length of the plaintext (octets)
   @param ct         [out] The ciphertext
   @param tag        [out] The authentication tag
   @param taglen     [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
i32 ocb_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen)
{
   i32 err;
   ocb_state *ocb;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(nonce  != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(taglen != NULL);

   /* allocate ram */
   ocb = XMALLOC(sizeof(ocb_state));
   if (ocb == NULL) {
      return CRYPT_MEM;
   }

   if ((err = ocb_init(ocb, cipher, key, keylen, nonce)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   while (ptlen > (u64)ocb->block_len) {
        if ((err = ocb_encrypt(ocb, pt, ct)) != CRYPT_OK) {
           goto LBL_ERR;
        }
        ptlen   -= ocb->block_len;
        pt      += ocb->block_len;
        ct      += ocb->block_len;
   }

   err = ocb_done_encrypt(ocb, pt, ptlen, ct, tag, taglen);
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
