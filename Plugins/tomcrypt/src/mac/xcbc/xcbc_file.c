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
  @file xcbc_file.c
  XCBC support, process a file, Tom St Denis
*/

#ifdef LTC_XCBC

/**
   XCBC a file
   @param cipher   The index of the cipher desired
   @param key      The secret key
   @param keylen   The length of the secret key (octets)
   @param filename The name of the file you wish to XCBC
   @param out      [out] Where the authentication tag is to be stored
   @param outlen   [in/out] The max size and resulting size of the authentication tag
   @return CRYPT_OK if successful, CRYPT_NOP if file support has been disabled
*/
i32 xcbc_file(i32 cipher,
              u8k *key, u64 keylen,
              tukk filename,
                    u8 *out, u64 *outlen)
{
#ifdef LTC_NO_FILE
   LTC_UNUSED_PARAM(cipher);
   LTC_UNUSED_PARAM(key);
   LTC_UNUSED_PARAM(keylen);
   LTC_UNUSED_PARAM(filename);
   LTC_UNUSED_PARAM(out);
   LTC_UNUSED_PARAM(outlen);
   return CRYPT_NOP;
#else
   size_t x;
   i32 err;
   xcbc_state xcbc;
   FILE *in;
   u8 *buf;

   LTC_ARGCHK(key      != NULL);
   LTC_ARGCHK(filename != NULL);
   LTC_ARGCHK(out      != NULL);
   LTC_ARGCHK(outlen   != NULL);

   if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
      return CRYPT_MEM;
   }

   if ((err = xcbc_init(&xcbc, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   in = fopen(filename, "rb");
   if (in == NULL) {
      err = CRYPT_FILE_NOTFOUND;
      goto LBL_ERR;
   }

   do {
      x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
      if ((err = xcbc_process(&xcbc, buf, (u64)x)) != CRYPT_OK) {
         fclose(in);
         goto LBL_CLEANBUF;
      }
   } while (x == LTC_FILE_READ_BUFSIZE);

   if (fclose(in) != 0) {
      err = CRYPT_ERROR;
      goto LBL_CLEANBUF;
   }

   err = xcbc_done(&xcbc, out, outlen);

LBL_CLEANBUF:
   zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&xcbc, sizeof(xcbc_state));
#endif
   XFREE(buf);
   return err;
#endif
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
