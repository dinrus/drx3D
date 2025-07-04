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
  @file xcbc_process.c
  XCBC Support, process blocks with XCBC
*/

#ifdef LTC_XCBC

/** Process data through XCBC-MAC
  @param xcbc     The XCBC-MAC state
  @param in       Input data to process
  @param inlen    Length of input in octets
  Return CRYPT_OK on success
*/
i32 xcbc_process(xcbc_state *xcbc, u8k *in, u64 inlen)
{
   i32 err;
#ifdef LTC_FAST
   i32 x;
#endif

   LTC_ARGCHK(xcbc != NULL);
   LTC_ARGCHK(in   != NULL);

   /* check structure */
   if ((err = cipher_is_valid(xcbc->cipher)) != CRYPT_OK) {
      return err;
   }

   if ((xcbc->blocksize > cipher_descriptor[xcbc->cipher].block_length) || (xcbc->blocksize < 0) ||
       (xcbc->buflen > xcbc->blocksize) || (xcbc->buflen < 0)) {
      return CRYPT_INVALID_ARG;
   }

#ifdef LTC_FAST
   if (xcbc->buflen == 0) {
       while (inlen > (u64)xcbc->blocksize) {
           for (x = 0; x < xcbc->blocksize; x += sizeof(LTC_FAST_TYPE)) {
              *(LTC_FAST_TYPE_PTR_CAST(&(xcbc->IV[x]))) ^= *(LTC_FAST_TYPE_PTR_CAST(&(in[x])));
           }
           cipher_descriptor[xcbc->cipher].ecb_encrypt(xcbc->IV, xcbc->IV, &xcbc->key);
           in    += xcbc->blocksize;
           inlen -= xcbc->blocksize;
       }
   }
#endif

   while (inlen) {
      if (xcbc->buflen == xcbc->blocksize) {
         cipher_descriptor[xcbc->cipher].ecb_encrypt(xcbc->IV, xcbc->IV, &xcbc->key);
         xcbc->buflen = 0;
      }
      xcbc->IV[xcbc->buflen++] ^= *in++;
      --inlen;
   }
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

