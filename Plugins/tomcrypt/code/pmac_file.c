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
   @file pmac_file.c
   PMAC implementation, process a file, by Tom St Denis
*/

#ifdef LTC_PMAC

/**
   PMAC a file
   @param cipher       The index of the cipher desired
   @param key          The secret key
   @param keylen       The length of the secret key (octets)
   @param filename     The name of the file to send through PMAC
   @param out          [out] Destination for the authentication tag
   @param outlen       [in/out] Max size and resulting size of the authentication tag
   @return CRYPT_OK if successful, CRYPT_NOP if file support has been disabled
*/
i32 pmac_file(i32 cipher,
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
   pmac_state pmac;
   FILE *in;
   u8 *buf;


   LTC_ARGCHK(key      != NULL);
   LTC_ARGCHK(filename != NULL);
   LTC_ARGCHK(out      != NULL);
   LTC_ARGCHK(outlen   != NULL);

   if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
      return CRYPT_MEM;
   }

   if ((err = pmac_init(&pmac, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   in = fopen(filename, "rb");
   if (in == NULL) {
      err = CRYPT_FILE_NOTFOUND;
      goto LBL_ERR;
   }

   do {
      x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
      if ((err = pmac_process(&pmac, buf, (u64)x)) != CRYPT_OK) {
         fclose(in);
         goto LBL_CLEANBUF;
      }
   } while (x == LTC_FILE_READ_BUFSIZE);

   if (fclose(in) != 0) {
      err = CRYPT_ERROR;
      goto LBL_CLEANBUF;
   }

   err = pmac_done(&pmac, out, outlen);

LBL_CLEANBUF:
   zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&pmac, sizeof(pmac_state));
#endif
   XFREE(buf);
   return err;
#endif
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
