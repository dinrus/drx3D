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
   @file pkcs_5_1.c
   PKCS #5, Algorithm #1, Tom St Denis
*/
#ifdef LTC_PKCS_5
/**
   Execute PKCS #5 v1 in strict or OpenSSL EVP_BytesToKey()-compat mode.

   PKCS#5 v1 specifies that the output key length can be no larger than
   the hash output length.  OpenSSL unilaterally extended that by repeating
   the hash process on a block-by-block basis for as long as needed to make
   bigger keys.  If you want to be compatible with KDF for e.g. "openssl enc",
   you'll want that.

   If you want strict PKCS behavior, turn openssl_compat off.  Or (more
   likely), use one of the convenience functions below.

   @param password         The password (or key)
   @param password_len     The length of the password (octet)
   @param salt             The salt (or nonce) which is 8 octets long
   @param iteration_count  The PKCS #5 v1 iteration count
   @param hash_idx         The index of the hash desired
   @param out              [out] The destination for this algorithm
   @param outlen           [in/out] The max size and resulting size of the algorithm output
   @param openssl_compat   [in] Whether or not to grow the key to the buffer size ala OpenSSL
   @return CRYPT_OK if successful
*/
static i32 _pkcs_5_alg1_common(u8k *password,
                       u64 password_len,
                       u8k *salt,
                       i32 iteration_count,  i32 hash_idx,
                       u8 *out,   u64 *outlen,
                       i32 openssl_compat)
{
   i32 err;
   u64 x;
   hash_state    *md;
   u8 *buf;
   /* Storage vars in case we need to support > hashsize (OpenSSL compat) */
   u64 block = 0, iter;
   /* How many bytes to put in the outbut buffer (convenience calc) */
   u64 outidx = 0, nb = 0;

   LTC_ARGCHK(password != NULL);
   LTC_ARGCHK(salt     != NULL);
   LTC_ARGCHK(out      != NULL);
   LTC_ARGCHK(outlen   != NULL);

   /* test hash IDX */
   if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
      return err;
   }

   /* allocate memory */
   md  = XMALLOC(sizeof(hash_state));
   buf = XMALLOC(MAXBLOCKSIZE);
   if (md == NULL || buf == NULL) {
      if (md != NULL) {
         XFREE(md);
      }
      if (buf != NULL) {
         XFREE(buf);
      }
      return CRYPT_MEM;
   }

   while(block * hash_descriptor[hash_idx].hashsize < *outlen) {

      /* hash initial (maybe previous hash) + password + salt */
      if ((err = hash_descriptor[hash_idx].init(md)) != CRYPT_OK) {
          goto LBL_ERR;
      }
      /* in OpenSSL mode, we first hash the previous result for blocks 2-n */
      if (openssl_compat && block) {
          if ((err = hash_descriptor[hash_idx].process(md, buf, hash_descriptor[hash_idx].hashsize)) != CRYPT_OK) {
              goto LBL_ERR;
          }
      }
      if ((err = hash_descriptor[hash_idx].process(md, password, password_len)) != CRYPT_OK) {
          goto LBL_ERR;
      }
      if ((err = hash_descriptor[hash_idx].process(md, salt, 8)) != CRYPT_OK) {
          goto LBL_ERR;
      }
      if ((err = hash_descriptor[hash_idx].done(md, buf)) != CRYPT_OK) {
          goto LBL_ERR;
      }

      iter = iteration_count;
      while (--iter) {
         /* code goes here. */
         x = MAXBLOCKSIZE;
         if ((err = hash_memory(hash_idx, buf, hash_descriptor[hash_idx].hashsize, buf, &x)) != CRYPT_OK) {
            goto LBL_ERR;
         }
      }

      /* limit the size of the copy to however many bytes we have left in
         the output buffer (and how many bytes we have to copy) */
      outidx = block*hash_descriptor[hash_idx].hashsize;
      nb = hash_descriptor[hash_idx].hashsize;
      if(outidx+nb > *outlen)
          nb = *outlen - outidx;
      if(nb > 0)
          XMEMCPY(out+outidx, buf, nb);

      block++;
      if (!openssl_compat)
          break;
   }
   /* In strict mode, we always return the hashsize, in compat we filled it
      as much as was requested, so we leave it alone. */
   if(!openssl_compat)
      *outlen = hash_descriptor[hash_idx].hashsize;

   err = CRYPT_OK;
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, MAXBLOCKSIZE);
   zeromem(md, sizeof(hash_state));
#endif

   XFREE(buf);
   XFREE(md);

   return err;
}

/**
   Execute PKCS #5 v1 - Strict mode (no OpenSSL-compatible extension)
   @param password         The password (or key)
   @param password_len     The length of the password (octet)
   @param salt             The salt (or nonce) which is 8 octets long
   @param iteration_count  The PKCS #5 v1 iteration count
   @param hash_idx         The index of the hash desired
   @param out              [out] The destination for this algorithm
   @param outlen           [in/out] The max size and resulting size of the algorithm output
   @return CRYPT_OK if successful
*/
i32 pkcs_5_alg1(u8k *password, u64 password_len,
                u8k *salt,
                i32 iteration_count,  i32 hash_idx,
                u8 *out,   u64 *outlen)
{
   return _pkcs_5_alg1_common(password, password_len, salt, iteration_count,
                             hash_idx, out, outlen, 0);
}

/**
   Execute PKCS #5 v1 - OpenSSL-extension-compatible mode

   Use this one if you need to derive keys as "openssl enc" does by default.
   OpenSSL (for better or worse), uses MD5 as the hash and iteration_count=1.
   @param password         The password (or key)
   @param password_len     The length of the password (octet)
   @param salt             The salt (or nonce) which is 8 octets long
   @param iteration_count  The PKCS #5 v1 iteration count
   @param hash_idx         The index of the hash desired
   @param out              [out] The destination for this algorithm
   @param outlen           [in/out] The max size and resulting size of the algorithm output
   @return CRYPT_OK if successful
*/
i32 pkcs_5_alg1_openssl(u8k *password,
                        u64 password_len,
                        u8k *salt,
                        i32 iteration_count,  i32 hash_idx,
                        u8 *out,   u64 *outlen)
{
   return _pkcs_5_alg1_common(password, password_len, salt, iteration_count,
                             hash_idx, out, outlen, 1);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
