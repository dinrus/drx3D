/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#define DESC_DEF_ONLY
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTM_DESC

//#include <drx3D/Plugins/tommath/tommath.h>


static const struct {
    i32 mpi_code, ltc_code;
} mpi_to_ltc_codes[] = {
   { MP_OKAY ,  CRYPT_OK},
   { MP_MEM  ,  CRYPT_MEM},
   { MP_VAL  ,  CRYPT_INVALID_ARG},
};

/**
   Convert a MPI error to a LTC error (Possibly the most powerful function ever!  Oh wait... no)
   @param err    The error to convert
   @return The equivalent LTC error code or CRYPT_ERROR if none found
*/
static i32 mpi_to_ltc_error(i32 err)
{
   i32 x;

   for (x = 0; x < (i32)(sizeof(mpi_to_ltc_codes)/sizeof(mpi_to_ltc_codes[0])); x++) {
       if (err == mpi_to_ltc_codes[x].mpi_code) {
          return mpi_to_ltc_codes[x].ltc_code;
       }
   }
   return CRYPT_ERROR;
}

static i32 init(uk *a)
{
   i32 err;

   LTC_ARGCHK(a != NULL);

   *a = XCALLOC(1, sizeof(mp_int));
   if (*a == NULL) {
      return CRYPT_MEM;
   }

   if ((err = mpi_to_ltc_error(mp_init(*a))) != CRYPT_OK) {
      XFREE(*a);
   }
   return err;
}

static void deinit(uk a)
{
   LTC_ARGCHKVD(a != NULL);
   mp_clear(a);
   XFREE(a);
}

static i32 neg(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_neg(a, b));
}

static i32 copy(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_copy(a, b));
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
   return mpi_to_ltc_error(mp_set_int(a, b));
}

static u64 get_int(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mp_get_int(a);
}

static ltc_mp_digit get_digit(uk a, i32 n)
{
   mp_int *A;
   LTC_ARGCHK(a != NULL);
   A = a;
   return (n >= A->used || n < 0) ? 0 : A->dp[n];
}

static i32 get_digit_count(uk a)
{
   mp_int *A;
   LTC_ARGCHK(a != NULL);
   A = a;
   return A->used;
}

static i32 compare(uk a, uk b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   ret = mp_cmp(a, b);
   switch (ret) {
      case MP_LT: return LTC_MP_LT;
      case MP_EQ: return LTC_MP_EQ;
      case MP_GT: return LTC_MP_GT;
      default:    return 0;
   }
}

static i32 compare_d(uk a, ltc_mp_digit b)
{
   i32 ret;
   LTC_ARGCHK(a != NULL);
   ret = mp_cmp_d(a, b);
   switch (ret) {
      case MP_LT: return LTC_MP_LT;
      case MP_EQ: return LTC_MP_EQ;
      case MP_GT: return LTC_MP_GT;
      default:    return 0;
   }
}

static i32 count_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mp_count_bits(a);
}

static i32 count_lsb_bits(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mp_cnt_lsb(a);
}


static i32 twoexpt(uk a, i32 n)
{
   LTC_ARGCHK(a != NULL);
   return mpi_to_ltc_error(mp_2expt(a, n));
}

/* ---- conversions ---- */

/* read ascii string */
static i32 read_radix(uk a, tukk b, i32 radix)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_read_radix(a, b, radix));
}

/* write one */
static i32 write_radix(uk a, char *b, i32 radix)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_toradix(a, b, radix));
}

/* get size as u8 string */
static u64 unsigned_size(uk a)
{
   LTC_ARGCHK(a != NULL);
   return mp_unsigned_bin_size(a);
}

/* store */
static i32 unsigned_write(uk a, u8 *b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_to_unsigned_bin(a, b));
}

/* read */
static i32 unsigned_read(uk a, u8 *b, u64 len)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_read_unsigned_bin(a, b, len));
}

/* add */
static i32 add(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_add(a, b, c));
}

static i32 addi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_add_d(a, b, c));
}

/* sub */
static i32 sub(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_sub(a, b, c));
}

static i32 subi(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_sub_d(a, b, c));
}

/* mul */
static i32 mul(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_mul(a, b, c));
}

static i32 muli(uk a, ltc_mp_digit b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_mul_d(a, b, c));
}

/* sqr */
static i32 sqr(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_sqr(a, b));
}

/* div */
static i32 divide(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_div(a, b, c, d));
}

static i32 div_2(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_div_2(a, b));
}

/* modi */
static i32 modi(uk a, ltc_mp_digit b, ltc_mp_digit *c)
{
   mp_digit tmp;
   i32      err;

   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);

   if ((err = mpi_to_ltc_error(mp_mod_d(a, b, &tmp))) != CRYPT_OK) {
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
   return mpi_to_ltc_error(mp_gcd(a, b, c));
}

/* lcm */
static i32 lcm(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_lcm(a, b, c));
}

static i32 addmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return mpi_to_ltc_error(mp_addmod(a,b,c,d));
}

static i32 submod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return mpi_to_ltc_error(mp_submod(a,b,c,d));
}

static i32 mulmod(uk a, uk b, uk c, uk d)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   LTC_ARGCHK(d != NULL);
   return mpi_to_ltc_error(mp_mulmod(a,b,c,d));
}

static i32 sqrmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_sqrmod(a,b,c));
}

/* invmod */
static i32 invmod(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_invmod(a, b, c));
}

/* setup */
static i32 montgomery_setup(uk a, uk *b)
{
   i32 err;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   *b = XCALLOC(1, sizeof(mp_digit));
   if (*b == NULL) {
      return CRYPT_MEM;
   }
   if ((err = mpi_to_ltc_error(mp_montgomery_setup(a, (mp_digit *)*b))) != CRYPT_OK) {
      XFREE(*b);
   }
   return err;
}

/* get normalization value */
static i32 montgomery_normalization(uk a, uk b)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   return mpi_to_ltc_error(mp_montgomery_calc_normalization(a, b));
}

/* reduce */
static i32 montgomery_reduce(uk a, uk b, uk c)
{
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);
   LTC_ARGCHK(c != NULL);
   return mpi_to_ltc_error(mp_montgomery_reduce(a, b, (uk ) *((mp_digit *)c)));
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
   return mpi_to_ltc_error(mp_exptmod(a,b,c,d));
}

static i32 isprime(uk a, i32 b, i32 *c)
{
   i32 err;
   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(c != NULL);
   if (b == 0) {
       b = LTC_MILLER_RABIN_REPS;
   } /* if */
   err = mpi_to_ltc_error(mp_prime_is_prime(a, b, c));
   *c = (*c == MP_YES) ? LTC_MP_YES : LTC_MP_NO;
   return err;
}

static i32 set_rand(uk a, i32 size)
{
   LTC_ARGCHK(a != NULL);
   return mpi_to_ltc_error(mp_rand(a, size));
}


#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
