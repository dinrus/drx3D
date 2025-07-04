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
  @file crypt_inits.c

  Provide math library functions for dynamic languages
  like Python - Larry Bugbee, February 2013
*/


#ifdef LTM_DESC
void init_LTM(void)
{
    ltc_mp = ltm_desc;
}
#endif

#ifdef TFM_DESC
void init_TFM(void)
{
    ltc_mp = tfm_desc;
}
#endif

#ifdef GMP_DESC
void init_GMP(void)
{
    ltc_mp = gmp_desc;
}
#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
