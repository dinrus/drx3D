/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>
#include <stdarg.h>

/**
  @file hmac_memory_multi.c
  HMAC support, process multiple blocks of memory, Tom St Denis/Dobes Vandermeer
*/

#ifdef LTC_HMAC

/**
   HMAC multiple blocks of memory to produce the authentication tag
   @param hash      The index of the hash to use
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param out       [out] Destination of the authentication tag
   @param outlen    [in/out] Max size and resulting size of authentication tag
   @param in        The data to HMAC
   @param inlen     The length of the data to HMAC (octets)
   @param ...       tuples of (data,len) pairs to HMAC, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
i32 hmac_memory_multi(i32 hash,
                u8k *key,  u64 keylen,
                      u8 *out,  u64 *outlen,
                u8k *in,   u64 inlen, ...)

{
    hmac_state          *hmac;
    i32                  err;
    va_list              args;
    u8k *curptr;
    u64        curlen;

    LTC_ARGCHK(key    != NULL);
    LTC_ARGCHK(in     != NULL);
    LTC_ARGCHK(out    != NULL);
    LTC_ARGCHK(outlen != NULL);

    /* allocate ram for hmac state */
    hmac = XMALLOC(sizeof(hmac_state));
    if (hmac == NULL) {
       return CRYPT_MEM;
    }

    if ((err = hmac_init(hmac, hash, key, keylen)) != CRYPT_OK) {
       goto LBL_ERR;
    }

    va_start(args, inlen);
    curptr = in;
    curlen = inlen;
    for (;;) {
       /* process buf */
       if ((err = hmac_process(hmac, curptr, curlen)) != CRYPT_OK) {
          goto LBL_ERR;
       }
       /* step to next */
       curptr = va_arg(args, u8k*);
       if (curptr == NULL) {
          break;
       }
       curlen = va_arg(args, u64);
    }
    if ((err = hmac_done(hmac, out, outlen)) != CRYPT_OK) {
       goto LBL_ERR;
    }
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(hmac, sizeof(hmac_state));
#endif
   XFREE(hmac);
   va_end(args);
   return err;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
