/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#define DESC_DEF_ONLY
#include "tomcrypt.h"

#ifdef GMP_DESC

#include <stdio.h>
#include <gmp.h>

static i32 init(uk *a)
{
   LTC_ARGCHK(a != NULL);

   *a = XCALLOC(1, sizeof(__mpz_struct));
   if (*a == NULL) {
      return CRYPT_MEM;
   }
   mpz_init(((__mpz_struct *)*a));
   return CRYPT_OK;
}

static void deinit(uk a)
{
   LTC_ARGCHKVD(a != NULL);
   mpz_clear(a);
   XFREE(a);
}

static i32 neg(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_neg(b, a);
   return CRYPT_OK;
}

static i32 copy(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_set(b, a);
   return CRYPT_OK;
}

static i32 init_copy(uk *a, uk b)
{
   if (init(a) != CRYPT_OK) {
      return CRYPT_MEM;
   }
   return copy(b, *a);
}

/* ---- trivial ---- */
static i32 set_int(uk a, ltc_mp_digit b)
{
   LTC_ARGCHK(a != NULL);
   mpz_set_ui(((__mpz_struct *)a), b);
   return CRYPT_OK;
}

static u64 get_int(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mpz_get_ui(a);
}

static ltc_mp_digit get_digit(uk a, i32 n)
{
   LTC_ARGCHK(a != NULL);
   return mpz_getlimbn(a, n);
}

static i32 get_digit_count(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mpz_size(a);
}

static i32 compare(uk a, uk b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   ret = mpz_cmp(a, b);
   if (ret < 0) {
      return LTC_MP_LT;
   } else if (ret > 0) {
      return LTC_MP_GT;
   } else {
      return LTC_MP_EQ;
   }
}

static i32 compare_d(uk a, ltc_mp_digit b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   ret = mpz_cmp_ui(((__mpz_struct *)a), b);
   if (ret < 0) {
      return LTC_MP_LT;
   } else if (ret > 0) {
      return LTC_MP_GT;
   } else {
      return LTC_MP_EQ;
   }
}

static i32 count_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mpz_sizeinbase(a, 2);
}

static i32 count_lsb_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mpz_scan1(a, 0);
}


static i32 twoexpt(uk a, i32 n)
{
   LTC_ARGCHK(a != NULL);
   mpz_set_ui(a, 0);
   mpz_setbit(a, n);
   return CRYPT_OK;
}

/* ---- conversions ---- */

static const char rmap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

/* read ascii string */
static i32 read_radix(uk a, tukk b, i32 radix)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   if (radix == 64) {
      /* Sadly, GMP only supports radixes up to 62, but we need 64.
       * So, although this is not the most elegant or efficient way,
       * let's just convert the base 64 string (6 bits per digit) to
       * an octal string (3 bits per digit) that's twice as long. */
      char c, *tmp, *q;
      tukk p;
      i32 i;
      tmp = XMALLOC (1 + 2 * strlen (b));
      if (tmp == NULL) {
         return CRYPT_MEM;
      }
      p = b;
      q = tmp;
      while ((c = *p++) != 0) {
         for (i = 0; i < 64; i++) {
            if (c == rmap[i])
               break;
         }
         if (i == 64) {
            XFREE (tmp);
            /* printf ("c = '%c'\n", c); */
            return CRYPT_ERROR;
         }
         *q++ = '0' + (i / 8);
         *q++ = '0' + (i % 8);
      }
      *q = 0;
      ret = mpz_set_str(a, tmp, 8);
      /* printf ("ret = %d for '%s'\n", ret, tmp); */
      XFREE (tmp);
   } else {
      ret = mpz_set_str(a, b, radix);
   }
   return (ret == 0 ? CRYPT_OK : CRYPT_ERROR);
}

/* write one */
static i32 write_radix(uk a, char *b, i32 radix)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   if (radix >= 11 && radix <= 36)
      /* If radix is positive, GMP uses lowercase, and if negative, uppercase.
       * We want it to use uppercase, to match the test vectors (presumably
       * generated with LibTomMath). */
      radix = -radix;
   mpz_get_str(b, radix, a);
   return CRYPT_OK;
}

/* get size as u8 string */
static u64 unsigned_size(uk a)
{
   u64 t;
   LTC_ARGCHK(a != NULL);
   t = mpz_sizeinbase(a, 2);
   if (mpz_cmp_ui(((__mpz_struct *)a), 0) == 0) return 0;
   return (t>>3) + ((t&7)?1:0);
}

/* store */
static i32 unsigned_write(uk a, u8 *b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_export(b, NULL, 1, 1, 1, 0, ((__mpz_struct*)a));
   return CRYPT_OK;
}

/* read */
static i32 unsigned_read(uk a, u8 *b, u64 len)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_import(a, len, 1, 1, 1, 0, b);
   return CRYPT_OK;
}

/* add */
static i32 add(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_add(c, a, b);
   return CRYPT_OK;
}

static i32 addi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_add_ui(c, a, b);
   return CRYPT_OK;
}

/* sub */
static i32 sub(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_sub(c, a, b);
   return CRYPT_OK;
}

static i32 subi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_sub_ui(c, a, b);
   return CRYPT_OK;
}

/* mul */
static i32 mul(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_mul(c, a, b);
   return CRYPT_OK;
}

static i32 muli(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_mul_ui(c, a, b);
   return CRYPT_OK;
}

/* sqr */
static i32 sqr(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_mul(b, a, a);
   return CRYPT_OK;
}

/* div */
static i32 divide(uk a, uk b, uk c, uk d)
{
   mpz_t tmp;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   if (c != NULL) {
      mpz_init(tmp);
      mpz_divexact(tmp, a, b);
   }
   if (d != NULL) {
      mpz_mod(d, a, b);
   }
   if (c != NULL) {
      mpz_set(c, tmp);
      mpz_clear(tmp);
   }
   return CRYPT_OK;
}

static i32 div_2(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_divexact_ui(b, a, 2);
   return CRYPT_OK;
}

/* modi */
static i32 modi(uk a, ltc_mp_digit b, ltc_mp_digit *c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);

   *c = mpz_fdiv_ui(a, b);
   return CRYPT_OK;
}

/* gcd */
static i32 gcd(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_gcd(c, a, b);
   return CRYPT_OK;
}

/* lcm */
static i32 lcm(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_lcm(c, a, b);
   return CRYPT_OK;
}

static i32 addmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   mpz_add(d, a, b);
   mpz_mod(d, d, c);
   return CRYPT_OK;
}

static i32 submod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   mpz_sub(d, a, b);
   mpz_mod(d, d, c);
   return CRYPT_OK;
}

static i32 mulmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   mpz_mul(d, a, b);
   mpz_mod(d, d, c);
   return CRYPT_OK;
}

static i32 sqrmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_mul(c, a, a);
   mpz_mod(c, c, b);
   return CRYPT_OK;
}

/* invmod */
static i32 invmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_invert(c, a, b);
   return CRYPT_OK;
}

/* setup */
static i32 montgomery_setup(uk a, uk *b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   *b = (uk )1;
   return CRYPT_OK;
}

/* get normalization value */
static i32 montgomery_normalization(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   mpz_set_ui(a, 1);
   return CRYPT_OK;
}

/* reduce */
static i32 montgomery_reduce(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   mpz_mod(a, a, b);
   return CRYPT_OK;
}

/* clean up */
static void montgomery_deinit(uk a)
{
  LTC_UNUSED_PARAM(a);
}

static i32 exptmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   mpz_powm(d, a, b, c);
   return CRYPT_OK;
}

static i32 isprime(uk a, i32 b, i32 *c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   if (b == 0) {
       b = LTC_MILLER_RABIN_REPS;
   } /* if */
   *c = mpz_probab_prime_p(a, b) > 0 ? LTC_MP_YES : LTC_MP_NO;
   return CRYPT_OK;
}

static i32 set_rand(uk a, i32 size)
{
   LTC_ARGCHK(a != NULL);
   mpz_random(a, size);
   return CRYPT_OK;
}

const ltc_math_descriptor gmp_desc = {
   "GNU MP",
   sizeof(mp_limb_t) * CHAR_BIT - GMP_NAIL_BITS,

   &init,
   &init_copy,
   &deinit,

   &neg,
   &copy,

   &set_int,
   &get_int,
   &get_digit,
   &get_digit_count,
   &compare,
   &compare_d,
   &count_bits,
   &count_lsb_bits,
   &twoexpt,

   &read_radix,
   &write_radix,
   &unsigned_size,
   &unsigned_write,
   &unsigned_read,

   &add,
   &addi,
   &sub,
   &subi,
   &mul,
   &muli,
   &sqr,
   &divide,
   &div_2,
   &modi,
   &gcd,
   &lcm,

   &mulmod,
   &sqrmod,
   &invmod,

   &montgomery_setup,
   &montgomery_normalization,
   &montgomery_reduce,
   &montgomery_deinit,

   &exptmod,
   &isprime,

#ifdef LTC_MECC
#ifdef LTC_MECC_FP
   &ltc_ecc_fp_mulmod,
#else
   &ltc_ecc_mulmod,
#endif /* LTC_MECC_FP */
   &ltc_ecc_projective_add_point,
   &ltc_ecc_projective_dbl_point,
   &ltc_ecc_map,
#ifdef LTC_ECC_SHAMIR
#ifdef LTC_MECC_FP
   &ltc_ecc_fp_mul2add,
#else
   &ltc_ecc_mul2add,
#endif /* LTC_MECC_FP */
#else
   NULL,
#endif /* LTC_ECC_SHAMIR */
#else
   NULL, NULL, NULL, NULL, NULL,
#endif /* LTC_MECC */

#ifdef LTC_MRSA
   &rsa_make_key,
   &rsa_exptmod,
#else
   NULL, NULL,
#endif
   &addmod,
   &submod,

   &set_rand,

};


#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
