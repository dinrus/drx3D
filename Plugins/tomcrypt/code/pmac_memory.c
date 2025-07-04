/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

/**
   @file pmac_memory.c
   PMAC implementation, process a block of memory, by Tom St Denis
*/

#ifdef LTC_PMAC

/**
   PMAC a block of memory
   @param cipher   The index of the cipher desired
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @param in       The data you wish to send through PMAC
   @param inlen    The length of data you wish to send through PMAC (octets)
   @param out      [out] Destination for the authentication tag
   @param outlen   [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful
*/
i32 pmac_memory(i32 cipher,
                u8k *key, u64 keylen,
                u8k *in, u64 inlen,
                      u8 *out, u64 *outlen)
{
   i32 err;
   pmac_state *pmac;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in    != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* allocate ram for pmac state */
   pmac = XMALLOC(sizeof(pmac_state));
   if (pmac == NULL) {
      return CRYPT_MEM;
   }

   if ((err = pmac_init(pmac, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if ((err = pmac_process(pmac, in, inlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if ((err = pmac_done(pmac, out, outlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(pmac, sizeof(pmac_state));
#endif

   XFREE(pmac);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
