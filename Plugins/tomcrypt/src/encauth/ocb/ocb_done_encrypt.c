/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb_done_encrypt.c
   OCB implementation, terminate encryption, by Tom St Denis
*/
#include "tomcrypt.h"

#ifdef LTC_OCB_MODE

/**
   Terminate an encryption OCB state
   @param ocb       The OCB state
   @param pt        Remaining plaintext (if any)
   @param ptlen     The length of the plaintext (octets)
   @param ct        [out] The ciphertext (if any)
   @param tag       [out] The tag for the OCB stream
   @param taglen    [in/out] The max size and resulting size of the tag
   @return CRYPT_OK if successful
*/
i32 ocb_done_encrypt(ocb_state *ocb, u8k *pt, u64 ptlen,
                     u8 *ct, u8 *tag, u64 *taglen)
{
   LTC_ARGCHK(ocb    != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(tag    != NULL);
   LTC_ARGCHK(taglen != NULL);
   return s_ocb_done(ocb, pt, ptlen, ct, tag, taglen, 0);
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
