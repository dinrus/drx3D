/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_RNG_GET_BYTES
/**
   @file rng_get_bytes.c
   portable way to get secure random bits to feed a PRNG (Tom St Denis)
*/

#if defined(LTC_DEVRANDOM) && !defined(_WIN32)
/* on *NIX read /dev/random */
static u64 _rng_nix(u8 *buf, u64 len,
                             void (*callback)(void))
{
#ifdef LTC_NO_FILE
    LTC_UNUSED_PARAM(callback);
    LTC_UNUSED_PARAM(buf);
    LTC_UNUSED_PARAM(len);
    return 0;
#else
    FILE *f;
    u64 x;
    LTC_UNUSED_PARAM(callback);
#ifdef LTC_TRY_URANDOM_FIRST
    f = fopen("/dev/urandom", "rb");
    if (f == NULL)
#endif /* LTC_TRY_URANDOM_FIRST */
       f = fopen("/dev/random", "rb");

    if (f == NULL) {
       return 0;
    }

    /* disable buffering */
    if (setvbuf(f, NULL, _IONBF, 0) != 0) {
       fclose(f);
       return 0;
    }

    x = (u64)fread(buf, 1, (size_t)len, f);
    fclose(f);
    return x;
#endif /* LTC_NO_FILE */
}

#endif /* LTC_DEVRANDOM */

#if !defined(_WIN32_WCE)

#define ANSI_RNG

static u64 _rng_ansic(u8 *buf, u64 len,
                               void (*callback)(void))
{
   clock_t t1;
   i32 l, acc, bits, a, b;

   l = len;
   bits = 8;
   acc  = a = b = 0;
   while (len--) {
       if (callback != NULL) callback();
       while (bits--) {
          do {
             t1 = XCLOCK(); while (t1 == XCLOCK()) a ^= 1;
             t1 = XCLOCK(); while (t1 == XCLOCK()) b ^= 1;
          } while (a == b);
          acc = (acc << 1) | a;
       }
       *buf++ = acc;
       acc  = 0;
       bits = 8;
   }
   return l;
}

#endif

/* Try the Microsoft CSP */
#if defined(_WIN32) || defined(_WIN32_WCE)
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0400
#endif
#ifdef _WIN32_WCE
   #define UNDER_CE
   #define ARM
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

static u64 _rng_win32(u8 *buf, u64 len,
                               void (*callback)(void))
{
   HCRYPTPROV hProv = 0;
   LTC_UNUSED_PARAM(callback);
   if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                            (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) &&
       !CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
      return 0;

   if (CryptGenRandom(hProv, len, buf) == TRUE) {
      CryptReleaseContext(hProv, 0);
      return len;
   } else {
      CryptReleaseContext(hProv, 0);
      return 0;
   }
}

#endif /* WIN32 */

/**
  Read the system RNG
  @param out       Destination
  @param outlen    Length desired (octets)
  @param callback  Pointer to void function to act as "callback" when RNG is slow.  This can be NULL
  @return Number of octets read
*/
u64 rng_get_bytes(u8 *out, u64 outlen,
                            void (*callback)(void))
{
   u64 x;

   LTC_ARGCHK(out != NULL);

#ifdef LTC_PRNG_ENABLE_LTC_RNG
   if (ltc_rng) {
      x = ltc_rng(out, outlen, callback);
      if (x != 0) {
         return x;
      }
   }
#endif

#if defined(_WIN32) || defined(_WIN32_WCE)
   x = _rng_win32(out, outlen, callback); if (x != 0) { return x; }
#elif defined(LTC_DEVRANDOM)
   x = _rng_nix(out, outlen, callback);   if (x != 0) { return x; }
#endif
#ifdef ANSI_RNG
   x = _rng_ansic(out, outlen, callback); if (x != 0) { return x; }
#endif
   return 0;
}
#endif /* #ifdef LTC_RNG_GET_BYTES */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
