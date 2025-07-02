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

#ifdef TFM_DESC

#include <tfm.h>

static const struct {
    i32 tfm_code, ltc_code;
} tfm_to_ltc_codes[] = {
   { FP_OKAY ,  CRYPT_OK},
   { FP_MEM  ,  CRYPT_MEM},
   { FP_VAL  ,  CRYPT_INVALID_ARG},
};

/**
   Convert a tfm error to a LTC error (Possibly the most powerful function ever!  Oh wait... no)
   @param err    The error to convert
   @return The equivalent LTC error code or CRYPT_ERROR if none found
*/
static i32 tfm_to_ltc_error(i32 err)
{
   i32 x;

   for (x = 0; x < (i32)(sizeof(tfm_to_ltc_codes)/sizeof(tfm_to_ltc_codes[0])); x++) {
       if (err == tfm_to_ltc_codes[x].tfm_code) {
          return tfm_to_ltc_codes[x].ltc_code;
       }
   }
   return CRYPT_ERROR;
}

static i32 init(uk *a)
{
   LTC_ARGCHK(a != NULL);

   *a = XCALLOC(1, sizeof(fp_int));
   if (*a == NULL) {
      return CRYPT_MEM;
   }
   fp_init(*a);
   return CRYPT_OK;
}

static void deinit(uk a)
{
   LTC_ARGCHKVD(a != NULL);
   XFREE(a);
}

static i32 neg(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_neg(((fp_int*)a), ((fp_int*)b));
   return CRYPT_OK;
}

static i32 copy(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_copy(a, b);
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
   fp_set(a, b);
   return CRYPT_OK;
}

static u64 get_int(uk a)
{
   fp_int *A;
   LTC_ARGCHK(a != NULL);
   A = a;
   return A->used > 0 ? A->dp[0] : 0;
}

static ltc_mp_digit get_digit(uk a, i32 n)
{
   fp_int *A;
   LTC_ARGCHK(a != NULL);
   A = a;
   return (n >= A->used || n < 0) ? 0 : A->dp[n];
}

static i32 get_digit_count(uk a)
{
   fp_int *A;
   LTC_ARGCHK(a != NULL);
   A = a;
   return A->used;
}

static i32 compare(uk a, uk b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   ret = fp_cmp(a, b);
   switch (ret) {
      case FP_LT: return LTC_MP_LT;
      case FP_EQ: return LTC_MP_EQ;
      case FP_GT: return LTC_MP_GT;
   }
   return 0;
}

static i32 compare_d(uk a, ltc_mp_digit b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   ret = fp_cmp_d(a, b);
   switch (ret) {
      case FP_LT: return LTC_MP_LT;
      case FP_EQ: return LTC_MP_EQ;
      case FP_GT: return LTC_MP_GT;
   }
   return 0;
}

static i32 count_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return fp_count_bits(a);
}

static i32 count_lsb_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return fp_cnt_lsb(a);
}

static i32 twoexpt(uk a, i32 n)
{
   LTC_ARGCHK(a != NULL);
   fp_2expt(a, n);
   return CRYPT_OK;
}

/* ---- conversions ---- */

/* read ascii string */
static i32 read_radix(uk a, tukk b, i32 radix)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return tfm_to_ltc_error(fp_read_radix(a, (char *)b, radix));
}

/* write one */
static i32 write_radix(uk a, char *b, i32 radix)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return tfm_to_ltc_error(fp_toradix(a, b, radix));
}

/* get size as u8 string */
static u64 unsigned_size(uk a)
{
   LTC_ARGCHK(a != NULL);
   return fp_unsigned_bin_size(a);
}

/* store */
static i32 unsigned_write(uk a, u8 *b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_to_unsigned_bin(a, b);
   return CRYPT_OK;
}

/* read */
static i32 unsigned_read(uk a, u8 *b, u64 len)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_read_unsigned_bin(a, b, len);
   return CRYPT_OK;
}

/* add */
static i32 add(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_add(a, b, c);
   return CRYPT_OK;
}

static i32 addi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   fp_add_d(a, b, c);
   return CRYPT_OK;
}

/* sub */
static i32 sub(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_sub(a, b, c);
   return CRYPT_OK;
}

static i32 subi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   fp_sub_d(a, b, c);
   return CRYPT_OK;
}

/* mul */
static i32 mul(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_mul(a, b, c);
   return CRYPT_OK;
}

static i32 muli(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   fp_mul_d(a, b, c);
   return CRYPT_OK;
}

/* sqr */
static i32 sqr(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_sqr(a, b);
   return CRYPT_OK;
}

/* div */
static i32 divide(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return tfm_to_ltc_error(fp_div(a, b, c, d));
}

static i32 div_2(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_div_2(a, b);
   return CRYPT_OK;
}

/* modi */
static i32 modi(uk a, ltc_mp_digit b, ltc_mp_digit *c)
{
   fp_digit tmp;
   i32      err;

   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);

   if ((err = tfm_to_ltc_error(fp_mod_d(a, b, &tmp))) != CRYPT_OK) {
      return err;
   }
   *c = tmp;
   return CRYPT_OK;
}

/* gcd */
static i32 gcd(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_gcd(a, b, c);
   return CRYPT_OK;
}

/* lcm */
static i32 lcm(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_lcm(a, b, c);
   return CRYPT_OK;
}

static i32 addmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return tfm_to_ltc_error(fp_addmod(a,b,c,d));
}

static i32 submod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return tfm_to_ltc_error(fp_submod(a,b,c,d));
}

static i32 mulmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return tfm_to_ltc_error(fp_mulmod(a,b,c,d));
}

static i32 sqrmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return tfm_to_ltc_error(fp_sqrmod(a,b,c));
}

/* invmod */
static i32 invmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return tfm_to_ltc_error(fp_invmod(a, b, c));
}

/* setup */
static i32 montgomery_setup(uk a, uk *b)
{
   i32 err;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   *b = XCALLOC(1, sizeof(fp_digit));
   if (*b == NULL) {
      return CRYPT_MEM;
   }
   if ((err = tfm_to_ltc_error(fp_montgomery_setup(a, (fp_digit *)*b))) != CRYPT_OK) {
      XFREE(*b);
   }
   return err;
}

/* get normalization value */
static i32 montgomery_normalization(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   fp_montgomery_calc_normalization(a, b);
   return CRYPT_OK;
}

/* reduce */
static i32 montgomery_reduce(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   fp_montgomery_reduce(a, b, *((fp_digit *)c));
   return CRYPT_OK;
}

/* clean up */
static void montgomery_deinit(uk a)
{
   XFREE(a);
}

static i32 exptmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return tfm_to_ltc_error(fp_exptmod(a,b,c,d));
}

static i32 isprime(uk a, i32 b, i32 *c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   if (b == 0) {
       b = LTC_MILLER_RABIN_REPS;
   } /* if */
   *c = (fp_isprime_ex(a, b) == FP_YES) ? LTC_MP_YES : LTC_MP_NO;
   return CRYPT_OK;
}

#if defined(LTC_MECC) && defined(LTC_MECC_ACCEL)

static i32 tfm_ecc_projective_dbl_point(ecc_point *P, ecc_point *R, uk modulus, uk Mp)
{
   fp_int t1, t2;
   fp_digit mp;

   LTC_ARGCHK(P       != NULL);
   LTC_ARGCHK(R       != NULL);
   LTC_ARGCHK(modulus != NULL);
   LTC_ARGCHK(Mp      != NULL);

   mp = *((fp_digit*)Mp);

   fp_init(&t1);
   fp_init(&t2);

   if (P != R) {
      fp_copy(P->x, R->x);
      fp_copy(P->y, R->y);
      fp_copy(P->z, R->z);
   }

   /* t1 = Z * Z */
   fp_sqr(R->z, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);
   /* Z = Y * Z */
   fp_mul(R->z, R->y, R->z);
   fp_montgomery_reduce(R->z, modulus, mp);
   /* Z = 2Z */
   fp_add(R->z, R->z, R->z);
   if (fp_cmp(R->z, modulus) != FP_LT) {
      fp_sub(R->z, modulus, R->z);
   }

   /* &t2 = X - T1 */
   fp_sub(R->x, &t1, &t2);
   if (fp_cmp_d(&t2, 0) == FP_LT) {
      fp_add(&t2, modulus, &t2);
   }
   /* T1 = X + T1 */
   fp_add(&t1, R->x, &t1);
   if (fp_cmp(&t1, modulus) != FP_LT) {
      fp_sub(&t1, modulus, &t1);
   }
   /* T2 = T1 * T2 */
   fp_mul(&t1, &t2, &t2);
   fp_montgomery_reduce(&t2, modulus, mp);
   /* T1 = 2T2 */
   fp_add(&t2, &t2, &t1);
   if (fp_cmp(&t1, modulus) != FP_LT) {
      fp_sub(&t1, modulus, &t1);
   }
   /* T1 = T1 + T2 */
   fp_add(&t1, &t2, &t1);
   if (fp_cmp(&t1, modulus) != FP_LT) {
      fp_sub(&t1, modulus, &t1);
   }

   /* Y = 2Y */
   fp_add(R->y, R->y, R->y);
   if (fp_cmp(R->y, modulus) != FP_LT) {
      fp_sub(R->y, modulus, R->y);
   }
   /* Y = Y * Y */
   fp_sqr(R->y, R->y);
   fp_montgomery_reduce(R->y, modulus, mp);
   /* T2 = Y * Y */
   fp_sqr(R->y, &t2);
   fp_montgomery_reduce(&t2, modulus, mp);
   /* T2 = T2/2 */
   if (fp_isodd(&t2)) {
      fp_add(&t2, modulus, &t2);
   }
   fp_div_2(&t2, &t2);
   /* Y = Y * X */
   fp_mul(R->y, R->x, R->y);
   fp_montgomery_reduce(R->y, modulus, mp);

   /* X  = T1 * T1 */
   fp_sqr(&t1, R->x);
   fp_montgomery_reduce(R->x, modulus, mp);
   /* X = X - Y */
   fp_sub(R->x, R->y, R->x);
   if (fp_cmp_d(R->x, 0) == FP_LT) {
      fp_add(R->x, modulus, R->x);
   }
   /* X = X - Y */
   fp_sub(R->x, R->y, R->x);
   if (fp_cmp_d(R->x, 0) == FP_LT) {
      fp_add(R->x, modulus, R->x);
   }

   /* Y = Y - X */
   fp_sub(R->y, R->x, R->y);
   if (fp_cmp_d(R->y, 0) == FP_LT) {
      fp_add(R->y, modulus, R->y);
   }
   /* Y = Y * T1 */
   fp_mul(R->y, &t1, R->y);
   fp_montgomery_reduce(R->y, modulus, mp);
   /* Y = Y - T2 */
   fp_sub(R->y, &t2, R->y);
   if (fp_cmp_d(R->y, 0) == FP_LT) {
      fp_add(R->y, modulus, R->y);
   }

   return CRYPT_OK;
}

/**
   Add two ECC points
   @param P        The point to add
   @param Q        The point to add
   @param R        [out] The destination of the double
   @param modulus  The modulus of the field the ECC curve is in
   @param Mp       The "b" value from montgomery_setup()
   @return CRYPT_OK on success
*/
static i32 tfm_ecc_projective_add_point(ecc_point *P, ecc_point *Q, ecc_point *R, uk modulus, uk Mp)
{
   fp_int  t1, t2, x, y, z;
   fp_digit mp;

   LTC_ARGCHK(P       != NULL);
   LTC_ARGCHK(Q       != NULL);
   LTC_ARGCHK(R       != NULL);
   LTC_ARGCHK(modulus != NULL);
   LTC_ARGCHK(Mp      != NULL);

   mp = *((fp_digit*)Mp);

   fp_init(&t1);
   fp_init(&t2);
   fp_init(&x);
   fp_init(&y);
   fp_init(&z);

   /* should we dbl instead? */
   fp_sub(modulus, Q->y, &t1);
   if ( (fp_cmp(P->x, Q->x) == FP_EQ) &&
        (Q->z != NULL && fp_cmp(P->z, Q->z) == FP_EQ) &&
        (fp_cmp(P->y, Q->y) == FP_EQ || fp_cmp(P->y, &t1) == FP_EQ)) {
        return tfm_ecc_projective_dbl_point(P, R, modulus, Mp);
   }

   fp_copy(P->x, &x);
   fp_copy(P->y, &y);
   fp_copy(P->z, &z);

   /* if Z is one then these are no-operations */
   if (Q->z != NULL) {
      /* T1 = Z' * Z' */
      fp_sqr(Q->z, &t1);
      fp_montgomery_reduce(&t1, modulus, mp);
      /* X = X * T1 */
      fp_mul(&t1, &x, &x);
      fp_montgomery_reduce(&x, modulus, mp);
      /* T1 = Z' * T1 */
      fp_mul(Q->z, &t1, &t1);
      fp_montgomery_reduce(&t1, modulus, mp);
      /* Y = Y * T1 */
      fp_mul(&t1, &y, &y);
      fp_montgomery_reduce(&y, modulus, mp);
   }

   /* T1 = Z*Z */
   fp_sqr(&z, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);
   /* T2 = X' * T1 */
   fp_mul(Q->x, &t1, &t2);
   fp_montgomery_reduce(&t2, modulus, mp);
   /* T1 = Z * T1 */
   fp_mul(&z, &t1, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);
   /* T1 = Y' * T1 */
   fp_mul(Q->y, &t1, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);

   /* Y = Y - T1 */
   fp_sub(&y, &t1, &y);
   if (fp_cmp_d(&y, 0) == FP_LT) {
      fp_add(&y, modulus, &y);
   }
   /* T1 = 2T1 */
   fp_add(&t1, &t1, &t1);
   if (fp_cmp(&t1, modulus) != FP_LT) {
      fp_sub(&t1, modulus, &t1);
   }
   /* T1 = Y + T1 */
   fp_add(&t1, &y, &t1);
   if (fp_cmp(&t1, modulus) != FP_LT) {
      fp_sub(&t1, modulus, &t1);
   }
   /* X = X - T2 */
   fp_sub(&x, &t2, &x);
   if (fp_cmp_d(&x, 0) == FP_LT) {
      fp_add(&x, modulus, &x);
   }
   /* T2 = 2T2 */
   fp_add(&t2, &t2, &t2);
   if (fp_cmp(&t2, modulus) != FP_LT) {
      fp_sub(&t2, modulus, &t2);
   }
   /* T2 = X + T2 */
   fp_add(&t2, &x, &t2);
   if (fp_cmp(&t2, modulus) != FP_LT) {
      fp_sub(&t2, modulus, &t2);
   }

   /* if Z' != 1 */
   if (Q->z != NULL) {
      /* Z = Z * Z' */
      fp_mul(&z, Q->z, &z);
      fp_montgomery_reduce(&z, modulus, mp);
   }

   /* Z = Z * X */
   fp_mul(&z, &x, &z);
   fp_montgomery_reduce(&z, modulus, mp);

   /* T1 = T1 * X  */
   fp_mul(&t1, &x, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);
   /* X = X * X */
   fp_sqr(&x, &x);
   fp_montgomery_reduce(&x, modulus, mp);
   /* T2 = T2 * x */
   fp_mul(&t2, &x, &t2);
   fp_montgomery_reduce(&t2, modulus, mp);
   /* T1 = T1 * X  */
   fp_mul(&t1, &x, &t1);
   fp_montgomery_reduce(&t1, modulus, mp);

   /* X = Y*Y */
   fp_sqr(&y, &x);
   fp_montgomery_reduce(&x, modulus, mp);
   /* X = X - T2 */
   fp_sub(&x, &t2, &x);
   if (fp_cmp_d(&x, 0) == FP_LT) {
      fp_add(&x, modulus, &x);
   }

   /* T2 = T2 - X */
   fp_sub(&t2, &x, &t2);
   if (fp_cmp_d(&t2, 0) == FP_LT) {
      fp_add(&t2, modulus, &t2);
   }
   /* T2 = T2 - X */
   fp_sub(&t2, &x, &t2);
   if (fp_cmp_d(&t2, 0) == FP_LT) {
      fp_add(&t2, modulus, &t2);
   }
   /* T2 = T2 * Y */
   fp_mul(&t2, &y, &t2);
   fp_montgomery_reduce(&t2, modulus, mp);
   /* Y = T2 - T1 */
   fp_sub(&t2, &t1, &y);
   if (fp_cmp_d(&y, 0) == FP_LT) {
      fp_add(&y, modulus, &y);
   }
   /* Y = Y/2 */
   if (fp_isodd(&y)) {
      fp_add(&y, modulus, &y);
   }
   fp_div_2(&y, &y);

   fp_copy(&x, R->x);
   fp_copy(&y, R->y);
   fp_copy(&z, R->z);

   return CRYPT_OK;
}


#endif

static i32 set_rand(uk a, i32 size)
{
   LTC_ARGCHK(a != NULL);
   fp_rand(a, size);
   return CRYPT_OK;
}

const ltc_math_descriptor tfm_desc = {

   "TomsFastMath",
   (i32)DIGIT_BIT,

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
#ifdef LTC_MECC_ACCEL
   &tfm_ecc_projective_add_point,
   &tfm_ecc_projective_dbl_point,
#else
   &ltc_ecc_projective_add_point,
   &ltc_ecc_projective_dbl_point,
#endif /* LTC_MECC_ACCEL */
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

   set_rand,

};


#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
