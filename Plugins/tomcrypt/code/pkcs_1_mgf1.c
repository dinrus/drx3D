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
  @file pkcs_1_mgf1.c
  The Mask Generation Function (MGF1) for PKCS #1, Tom St Denis
*/

#ifdef LTC_PKCS_1

/**
   Perform PKCS #1 MGF1 (internal)
   @param hash_idx    The index of the hash desired
   @param seed        The seed for MGF1
   @param seedlen     The length of the seed
   @param mask        [out] The destination
   @param masklen     The length of the mask desired
   @return CRYPT_OK if successful
*/
i32 pkcs_1_mgf1(i32                  hash_idx,
                u8k *seed, u64 seedlen,
                      u8 *mask, u64 masklen)
{
   u64 hLen, x;
   ulong32       counter;
   i32           err;
   hash_state    *md;
   u8 *buf;

   LTC_ARGCHK(seed != NULL);
   LTC_ARGCHK(mask != NULL);

   /* ensure valid hash */
   if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
      return err;
   }

   /* get hash output size */
   hLen = hash_descriptor[hash_idx].hashsize;

   /* allocate memory */
   md  = XMALLOC(sizeof(hash_state));
   buf = XMALLOC(hLen);
   if (md == NULL || buf == NULL) {
      if (md != NULL) {
         XFREE(md);
      }
      if (buf != NULL) {
         XFREE(buf);
      }
      return CRYPT_MEM;
   }

   /* start counter */
   counter = 0;

   while (masklen > 0) {
       /* handle counter */
       STORE32H(counter, buf);
       ++counter;

       /* get hash of seed || counter */
       if ((err = hash_descriptor[hash_idx].init(md)) != CRYPT_OK) {
          goto LBL_ERR;
       }
       if ((err = hash_descriptor[hash_idx].process(md, seed, seedlen)) != CRYPT_OK) {
          goto LBL_ERR;
       }
       if ((err = hash_descriptor[hash_idx].process(md, buf, 4)) != CRYPT_OK) {
          goto LBL_ERR;
       }
       if ((err = hash_descriptor[hash_idx].done(md, buf)) != CRYPT_OK) {
          goto LBL_ERR;
       }

       /* store it */
       for (x = 0; x < hLen && masklen > 0; x++, masklen--) {
          *mask++ = buf[x];
       }
   }

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, hLen);
   zeromem(md,  sizeof(hash_state));
#endif

   XFREE(buf);
   XFREE(md);

   return err;
}

#endif /* LTC_PKCS_1 */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
