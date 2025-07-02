/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifndef LTC_NO_FILE
/**
  @file hash_file.c
  Hash a file, Tom St Denis
*/

/**
  @param hash   The index of the hash desired
  @param fname  The name of the file you wish to hash
  @param out    [out] The destination of the digest
  @param outlen [in/out] The max size and resulting size of the message digest
  @result CRYPT_OK if successful
*/
i32 hash_file(i32 hash, tukk fname, u8 *out, u64 *outlen)
{
    FILE *in;
    i32 err;
    LTC_ARGCHK(fname  != NULL);
    LTC_ARGCHK(out    != NULL);
    LTC_ARGCHK(outlen != NULL);

    if ((err = hash_is_valid(hash)) != CRYPT_OK) {
        return err;
    }

    in = fopen(fname, "rb");
    if (in == NULL) {
       return CRYPT_FILE_NOTFOUND;
    }

    err = hash_filehandle(hash, in, out, outlen);
    if (fclose(in) != 0) {
       return CRYPT_ERROR;
    }

    return err;
}
#endif /* #ifndef LTC_NO_FILE */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
