/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

#ifndef LTC_NO_FILE
/**
   @file hash_filehandle.c
   Hash open files, Tom St Denis
*/

/**
  Hash data from an open file handle.
  @param hash   The index of the hash you want to use
  @param in     The FILE* handle of the file you want to hash
  @param out    [out] The destination of the digest
  @param outlen [in/out] The max size and resulting size of the digest
  @result CRYPT_OK if successful
*/
i32 hash_filehandle(i32 hash, FILE *in, u8 *out, u64 *outlen)
{
    hash_state md;
    u8 *buf;
    size_t x;
    i32 err;

    LTC_ARGCHK(out    != NULL);
    LTC_ARGCHK(outlen != NULL);
    LTC_ARGCHK(in     != NULL);

    if ((buf = XMALLOC(LTC_FILE_READ_BUFSIZE)) == NULL) {
        return CRYPT_MEM;
    }

    if ((err = hash_is_valid(hash)) != CRYPT_OK) {
        goto LBL_ERR;
    }

    if (*outlen < hash_descriptor[hash].hashsize) {
       *outlen = hash_descriptor[hash].hashsize;
       err = CRYPT_BUFFER_OVERFLOW;
       goto LBL_ERR;
    }
    if ((err = hash_descriptor[hash].init(&md)) != CRYPT_OK) {
       goto LBL_ERR;
    }

    do {
        x = fread(buf, 1, LTC_FILE_READ_BUFSIZE, in);
        if ((err = hash_descriptor[hash].process(&md, buf, (u64)x)) != CRYPT_OK) {
           goto LBL_CLEANBUF;
        }
    } while (x == LTC_FILE_READ_BUFSIZE);
    if ((err = hash_descriptor[hash].done(&md, out)) == CRYPT_OK) {
       *outlen = hash_descriptor[hash].hashsize;
    }

LBL_CLEANBUF:
    zeromem(buf, LTC_FILE_READ_BUFSIZE);
LBL_ERR:
    XFREE(buf);
    return err;
}
#endif /* #ifndef LTC_NO_FILE */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
