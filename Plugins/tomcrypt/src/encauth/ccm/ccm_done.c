/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

#ifdef LTC_CCM_MODE

/**
  Terminate a CCM stream
  @param ccm     The CCM state
  @param tag     [out] The destination for the MAC tag
  @param taglen  [in/out]  The length of the MAC tag
  @return CRYPT_OK on success
 */
i32 ccm_done(ccm_state *ccm,
             u8 *tag,    u64 *taglen)
{
   u64 x, y;
   i32            err;

   LTC_ARGCHK(ccm != NULL);

   /* Check all data have been processed */
   if (ccm->ptlen != ccm->current_ptlen) {
      return CRYPT_ERROR;
   }

   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(taglen != NULL);

   if (ccm->x != 0) {
      if ((err = cipher_descriptor[ccm->cipher].ecb_encrypt(ccm->PAD, ccm->PAD, &ccm->K)) != CRYPT_OK) {
         return err;
      }
   }

   /* setup CTR for the TAG (zero the count) */
   for (y = 15; y > 15 - ccm->L; y--) {
      ccm->ctr[y] = 0x00;
   }
   if ((err = cipher_descriptor[ccm->cipher].ecb_encrypt(ccm->ctr, ccm->CTRPAD, &ccm->K)) != CRYPT_OK) {
      return err;
   }

   cipher_descriptor[ccm->cipher].done(&ccm->K);

   /* store the TAG */
   for (x = 0; x < 16 && x < *taglen; x++) {
      tag[x] = ccm->PAD[x] ^ ccm->CTRPAD[x];
   }
   *taglen = x;

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
