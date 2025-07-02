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
  @file omac_memory_multi.c
  OMAC1 support, process multiple blocks of memory, Tom St Denis
*/

#ifdef LTC_OMAC

/**
   OMAC multiple blocks of memory
   @param cipher    The index of the desired cipher
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param out       [out] The destination of the authentication tag
   @param outlen    [in/out]  The max size and resulting size of the authentication tag (octets)
   @param in        The data to send through OMAC
   @param inlen     The length of the data to send through OMAC (octets)
   @param ...       tuples of (data,len) pairs to OMAC, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
i32 omac_memory_multi(i32 cipher,
                u8k *key, u64 keylen,
                      u8 *out, u64 *outlen,
                u8k *in,  u64 inlen, ...)
{
   i32                  err;
   omac_state          *omac;
   va_list              args;
   u8k *curptr;
   u64        curlen;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* allocate ram for omac state */
   omac = XMALLOC(sizeof(omac_state));
   if (omac == NULL) {
      return CRYPT_MEM;
   }

   /* omac process the message */
   if ((err = omac_init(omac, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   va_start(args, inlen);
   curptr = in;
   curlen = inlen;
   for (;;) {
      /* process buf */
      if ((err = omac_process(omac, curptr, curlen)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      /* step to next */
      curptr = va_arg(args, u8k*);
      if (curptr == NULL) {
         break;
      }
      curlen = va_arg(args, u64);
   }
   if ((err = omac_done(omac, out, outlen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(omac, sizeof(omac_state));
#endif
   XFREE(omac);
   va_end(args);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
