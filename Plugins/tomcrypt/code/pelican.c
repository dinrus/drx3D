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
   @file pelican.c
   Pelican MAC, initialize state, by Tom St Denis
*/

#ifdef LTC_PELICAN

#define __LTC_AES_TAB_C__
#define ENCRYPT_ONLY
#define PELI_TAB
#include "aes_tab.c"

/**
   Initialize a Pelican state
   @param pelmac    The Pelican state to initialize
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
i32 pelican_init(pelican_state *pelmac, u8k *key, u64 keylen)
{
    i32 err;

    LTC_ARGCHK(pelmac != NULL);
    LTC_ARGCHK(key    != NULL);

#ifdef LTC_FAST
    if (16 % sizeof(LTC_FAST_TYPE)) {
        return CRYPT_INVALID_ARG;
    }
#endif

    if ((err = aes_setup(key, keylen, 0, &pelmac->K)) != CRYPT_OK) {
       return err;
    }

    zeromem(pelmac->state, 16);
    aes_ecb_encrypt(pelmac->state, pelmac->state, &pelmac->K);
    pelmac->buflen = 0;

    return CRYPT_OK;
}

static void _four_rounds(pelican_state *pelmac)
{
    ulong32 s0, s1, s2, s3, t0, t1, t2, t3;
    i32 r;

    LOAD32H(s0, pelmac->state      );
    LOAD32H(s1, pelmac->state  +  4);
    LOAD32H(s2, pelmac->state  +  8);
    LOAD32H(s3, pelmac->state  + 12);
    for (r = 0; r < 4; r++) {
        t0 =
            Te0(byte(s0, 3)) ^
            Te1(byte(s1, 2)) ^
            Te2(byte(s2, 1)) ^
            Te3(byte(s3, 0));
        t1 =
            Te0(byte(s1, 3)) ^
            Te1(byte(s2, 2)) ^
            Te2(byte(s3, 1)) ^
            Te3(byte(s0, 0));
        t2 =
            Te0(byte(s2, 3)) ^
            Te1(byte(s3, 2)) ^
            Te2(byte(s0, 1)) ^
            Te3(byte(s1, 0));
        t3 =
            Te0(byte(s3, 3)) ^
            Te1(byte(s0, 2)) ^
            Te2(byte(s1, 1)) ^
            Te3(byte(s2, 0));
        s0 = t0; s1 = t1; s2 = t2; s3 = t3;
    }
    STORE32H(s0, pelmac->state      );
    STORE32H(s1, pelmac->state  +  4);
    STORE32H(s2, pelmac->state  +  8);
    STORE32H(s3, pelmac->state  + 12);
}

/**
  Process a block of text through Pelican
  @param pelmac       The Pelican MAC state
  @param in           The input
  @param inlen        The length input (octets)
  @return CRYPT_OK on success
  */
i32 pelican_process(pelican_state *pelmac, u8k *in, u64 inlen)
{

   LTC_ARGCHK(pelmac != NULL);
   LTC_ARGCHK(in     != NULL);

   /* check range */
   if (pelmac->buflen < 0 || pelmac->buflen > 15) {
      return CRYPT_INVALID_ARG;
   }

#ifdef LTC_FAST
   if (pelmac->buflen == 0) {
      while (inlen & ~15) {
         i32 x;
         for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
            *(LTC_FAST_TYPE_PTR_CAST((u8*)pelmac->state + x)) ^= *(LTC_FAST_TYPE_PTR_CAST((u8*)in + x));
         }
         _four_rounds(pelmac);
         in    += 16;
         inlen -= 16;
      }
   }
#endif

   while (inlen--) {
       pelmac->state[pelmac->buflen++] ^= *in++;
       if (pelmac->buflen == 16) {
          _four_rounds(pelmac);
          pelmac->buflen = 0;
       }
   }
   return CRYPT_OK;
}

/**
  Terminate Pelican MAC
  @param pelmac      The Pelican MAC state
  @param out         [out] The TAG
  @return CRYPT_OK on sucess
*/
i32 pelican_done(pelican_state *pelmac, u8 *out)
{
   LTC_ARGCHK(pelmac  != NULL);
   LTC_ARGCHK(out     != NULL);

   /* check range */
   if (pelmac->buflen < 0 || pelmac->buflen > 16) {
      return CRYPT_INVALID_ARG;
   }

   if  (pelmac->buflen == 16) {
       _four_rounds(pelmac);
       pelmac->buflen = 0;
   }
   pelmac->state[pelmac->buflen++] ^= 0x80;
   aes_ecb_encrypt(pelmac->state, out, &pelmac->K);
   aes_done(&pelmac->K);
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
