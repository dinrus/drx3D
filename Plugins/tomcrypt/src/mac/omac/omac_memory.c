/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file omac_memory.c
  OMAC1 support, process a block of memory, Tom St Denis
*/

#ifdef LTC_OMAC

/**
   OMAC a block of memory
   @param cipher    The index of the desired cipher
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param in        The data to send through OMAC
   @param inlen     The length of the data to send through OMAC (octets)
   @param out       [out] The destination of the authentication tag
   @param outlen    [in/out]  The max size and resulting size of the authentication tag (octets)
   @return CRYPT_OK if successful
*/
i32 omac_memory(i32 cipher,
                u8k *key, u64 keylen,
                u8k *in,  u64 inlen,
                      u8 *out, u64 *outlen)
{
   i32 err;
   omac_state *omac;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* is the cipher valid? */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   /* Use accelerator if found */
   if (cipher_descriptor[cipher].omac_memory != NULL) {
      return cipher_descriptor[cipher].omac_memory(key, keylen, in, inlen, out, outlen);
   }

   /* allocate ram for omac state */
   omac = XMALLOC(sizeof(omac_state));
   if (omac == NULL) {
      return CRYPT_MEM;
   }

   /* omac process the message */
   if ((err = omac_init(omac, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if ((err = omac_process(omac, in, inlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if ((err = omac_done(omac, out, outlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(omac, sizeof(omac_state));
#endif

   XFREE(omac);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
