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
  @file omac_done.c
  OMAC1 support, terminate a stream, Tom St Denis
*/

#ifdef LTC_OMAC

/**
  Terminate an OMAC stream
  @param omac   The OMAC state
  @param out    [out] Destination for the authentication tag
  @param outlen [in/out]  The max size and resulting size of the authentication tag
  @return CRYPT_OK if successful
*/
i32 omac_done(omac_state *omac, u8 *out, u64 *outlen)
{
   i32       err, mode;
   unsigned  x;

   LTC_ARGCHK(omac   != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);
   if ((err = cipher_is_valid(omac->cipher_idx)) != CRYPT_OK) {
      return err;
   }

   if ((omac->buflen > (i32)sizeof(omac->block)) || (omac->buflen < 0) ||
       (omac->blklen > (i32)sizeof(omac->block)) || (omac->buflen > omac->blklen)) {
      return CRYPT_INVALID_ARG;
   }

   /* figure out mode */
   if (omac->buflen != omac->blklen) {
      /* add the 0x80 byte */
      omac->block[omac->buflen++] = 0x80;

      /* pad with 0x00 */
      while (omac->buflen < omac->blklen) {
         omac->block[omac->buflen++] = 0x00;
      }
      mode = 1;
   } else {
      mode = 0;
   }

   /* now xor prev + Lu[mode] */
   for (x = 0; x < (unsigned)omac->blklen; x++) {
       omac->block[x] ^= omac->prev[x] ^ omac->Lu[mode][x];
   }

   /* encrypt it */
   if ((err = cipher_descriptor[omac->cipher_idx].ecb_encrypt(omac->block, omac->block, &omac->key)) != CRYPT_OK) {
      return err;
   }
   cipher_descriptor[omac->cipher_idx].done(&omac->key);

   /* output it */
   for (x = 0; x < (unsigned)omac->blklen && x < *outlen; x++) {
       out[x] = omac->block[x];
   }
   *outlen = x;

#ifdef LTC_CLEAN_STACK
   zeromem(omac, sizeof(*omac));
#endif
   return CRYPT_OK;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
