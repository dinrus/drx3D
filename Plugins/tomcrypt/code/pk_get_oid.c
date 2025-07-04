/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_DER
static const oid_st rsa_oid = {
   { 1, 2, 840, 113549, 1, 1, 1  },
   7,
};

static const oid_st dsa_oid = {
   { 1, 2, 840, 10040, 4, 1  },
   6,
};

/*
   Returns the OID of the public key algorithm.
   @return CRYPT_OK if valid
*/
i32 pk_get_oid(i32 pk, oid_st *st)
{
   switch (pk) {
      case PKA_RSA:
         XMEMCPY(st, &rsa_oid, sizeof(*st));
         break;
      case PKA_DSA:
         XMEMCPY(st, &dsa_oid, sizeof(*st));
         break;
      default:
         return CRYPT_INVALID_ARG;
   }
   return CRYPT_OK;
}
#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
