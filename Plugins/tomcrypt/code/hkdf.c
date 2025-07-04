/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_HKDF

/* This is mostly just a wrapper around hmac_memory */
i32 hkdf_extract(i32 hash_idx, u8k *salt, u64  saltlen,
                               u8k *in,   u64  inlen,
                                     u8 *out,  u64 *outlen)
{
   /* libtomcrypt chokes on a zero length HMAC key, so we need to check for
      that.  HMAC specifies that keys shorter than the hash's blocksize are
      0 padded to the block size.  HKDF specifies that a NULL salt is to be
      substituted with a salt comprised of hashLen 0 bytes.  HMAC's padding
      means that in either case the HMAC is actually using a blocksize long
      zero filled key.  Unless blocksize < hashLen (which wouldn't make any
      sense), we can use a single 0 byte as the HMAC key and still generate
      valid results for HKDF. */
   if (salt == NULL || saltlen == 0) {
      return hmac_memory(hash_idx, (u8k*)"",   1,       in, inlen, out, outlen);
   } else {
      return hmac_memory(hash_idx, salt, saltlen, in, inlen, out, outlen);
   }
}

i32 hkdf_expand(i32 hash_idx, u8k *info, u64 infolen,
                              u8k *in,   u64 inlen,
                                    u8 *out,  u64 outlen)
{
   u64 hashsize;
   i32 err;
   u8 N;
   u64 Noutlen, outoff;

   u8 *T,  *dat;
   u64 Tlen, datlen;

   /* make sure hash descriptor is valid */
   if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
      return err;
   }

   hashsize = hash_descriptor[hash_idx].hashsize;

   /* RFC5869 parameter restrictions */
   if (inlen < hashsize || outlen > hashsize * 255)
      return CRYPT_INVALID_ARG;
   if (info == NULL && infolen != 0)
      return CRYPT_INVALID_ARG;
   LTC_ARGCHK(out != NULL);

   Tlen = hashsize + infolen + 1;
   T = XMALLOC(Tlen); /* Replace with static buffer? */
   if (T == NULL) {
      return CRYPT_MEM;
   }
   if (info != NULL) {
      XMEMCPY(T + hashsize, info, infolen);
   }

   /* HMAC data T(1) doesn't include a previous hash value */
   dat    = T    + hashsize;
   datlen = Tlen - hashsize;

   N = 0;
   outoff = 0; /* offset in out to write to */
   while (1) { /* an exit condition breaks mid-loop */
      Noutlen = MIN(hashsize, outlen - outoff);
      T[Tlen - 1] = ++N;
      if ((err = hmac_memory(hash_idx, in, inlen, dat, datlen,
                             out + outoff, &Noutlen)) != CRYPT_OK) {
         zeromem(T, Tlen);
         XFREE(T);
         return err;
      }
      outoff += Noutlen;

      if (outoff >= outlen) /* loop exit condition */
         break;

      /* All subsequent HMAC data T(N) DOES include the previous hash value */
      XMEMCPY(T, out + hashsize * (N-1), hashsize);
      if (N == 1) {
         dat = T;
         datlen = Tlen;
      }
   }
   zeromem(T, Tlen);
   XFREE(T);
   return CRYPT_OK;
}

/* all in one step */
i32 hkdf(i32 hash_idx, u8k *salt, u64 saltlen,
                       u8k *info, u64 infolen,
                       u8k *in,   u64 inlen,
                             u8 *out,  u64 outlen)
{
   u64 hashsize;
   i32 err;
   u8 *extracted;

   /* make sure hash descriptor is valid */
   if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
      return err;
   }

   hashsize = hash_descriptor[hash_idx].hashsize;

   extracted = XMALLOC(hashsize); /* replace with static buffer? */
   if (extracted == NULL) {
      return CRYPT_MEM;
   }
   if ((err = hkdf_extract(hash_idx, salt, saltlen, in, inlen, extracted, &hashsize)) != 0) {
      zeromem(extracted, hashsize);
      XFREE(extracted);
      return err;
   }
   err = hkdf_expand(hash_idx, info, infolen, extracted, hashsize, out, outlen);
   zeromem(extracted, hashsize);
   XFREE(extracted);
   return err;
}
#endif /* LTC_HKDF */


/* vim: set ts=2 sw=2 et ai si: */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
