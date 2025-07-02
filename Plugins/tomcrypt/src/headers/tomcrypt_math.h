/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/** math functions **/

#define LTC_MP_LT   -1
#define LTC_MP_EQ    0
#define LTC_MP_GT    1

#define LTC_MP_NO    0
#define LTC_MP_YES   1

#ifndef LTC_MECC
   typedef void ecc_point;
#endif

#ifndef LTC_MRSA
   typedef void rsa_key;
#endif

#ifndef LTC_MILLER_RABIN_REPS
   /* Number of rounds of the Miller-Rabin test
    * "Reasonable values of reps are between 15 and 50." c.f. gmp doc of mpz_probab_prime_p()
    * As of https://security.stackexchange.com/a/4546 we should use 40 rounds */
   #define LTC_MILLER_RABIN_REPS    40
#endif

i32 radix_to_bin(ukk in, i32 radix, uk out, u64 *len);

/** math descriptor */
typedef struct {
   /** Name of the math provider */
   tukk name;

   /** Bits per digit, amount of bits must fit in an u64 */
   i32  bits_per_digit;

/* ---- init/deinit functions ---- */

   /** initialize a bignum
     @param   a     The number to initialize
     @return  CRYPT_OK on success
   */
   i32 (*init)(uk *a);

   /** init copy
     @param  dst    The number to initialize and write to
     @param  src    The number to copy from
     @return CRYPT_OK on success
   */
   i32 (*init_copy)(uk *dst, uk src);

   /** deinit
      @param   a    The number to free
      @return CRYPT_OK on success
   */
   void (*deinit)(uk a);

/* ---- data movement ---- */

   /** negate
      @param   src   The number to negate
      @param   dst   The destination
      @return CRYPT_OK on success
   */
   i32 (*neg)(uk src, uk dst);

   /** copy
      @param   src   The number to copy from
      @param   dst   The number to write to
      @return CRYPT_OK on success
   */
   i32 (*copy)(uk src, uk dst);

/* ---- trivial low level functions ---- */

   /** set small constant
      @param a    Number to write to
      @param n    Source upto bits_per_digit (actually meant for very small constants)
      @return CRYPT_OK on success
   */
   i32 (*set_int)(uk a, ltc_mp_digit n);

   /** get small constant
      @param a  Small number to read,
                only fetches up to bits_per_digit from the number
      @return   The lower bits_per_digit of the integer (unsigned)
   */
   u64 (*get_int)(uk a);

   /** get digit n
     @param a  The number to read from
     @param n  The number of the digit to fetch
     @return  The bits_per_digit  sized n'th digit of a
   */
   ltc_mp_digit (*get_digit)(uk a, i32 n);

   /** Get the number of digits that represent the number
     @param a   The number to count
     @return The number of digits used to represent the number
   */
   i32 (*get_digit_count)(uk a);

   /** compare two integers
     @param a   The left side integer
     @param b   The right side integer
     @return LTC_MP_LT if a < b,
             LTC_MP_GT if a > b and
             LTC_MP_EQ otherwise.  (signed comparison)
   */
   i32 (*compare)(uk a, uk b);

   /** compare against i32
     @param a   The left side integer
     @param b   The right side integer (upto bits_per_digit)
     @return LTC_MP_LT if a < b,
             LTC_MP_GT if a > b and
             LTC_MP_EQ otherwise.  (signed comparison)
   */
   i32 (*compare_d)(uk a, ltc_mp_digit n);

   /** Count the number of bits used to represent the integer
     @param a   The integer to count
     @return The number of bits required to represent the integer
   */
   i32 (*count_bits)(uk  a);

   /** Count the number of LSB bits which are zero
     @param a   The integer to count
     @return The number of contiguous zero LSB bits
   */
   i32 (*count_lsb_bits)(uk a);

   /** Compute a power of two
     @param a  The integer to store the power in
     @param n  The power of two you want to store (a = 2^n)
     @return CRYPT_OK on success
   */
   i32 (*twoexpt)(uk a , i32 n);

/* ---- radix conversions ---- */

   /** read ascii string
     @param a     The integer to store into
     @param str   The string to read
     @param radix The radix the integer has been represented in (2-64)
     @return CRYPT_OK on success
   */
   i32 (*read_radix)(uk a, tukk str, i32 radix);

   /** write number to string
     @param a     The integer to store
     @param str   The destination for the string
     @param radix The radix the integer is to be represented in (2-64)
     @return CRYPT_OK on success
   */
   i32 (*write_radix)(uk a, char *str, i32 radix);

   /** get size as u8 string
     @param a  The integer to get the size (when stored in array of octets)
     @return   The length of the integer in octets
   */
   u64 (*unsigned_size)(uk a);

   /** store an integer as an array of octets
     @param src   The integer to store
     @param dst   The buffer to store the integer in
     @return CRYPT_OK on success
   */
   i32 (*unsigned_write)(uk src, u8 *dst);

   /** read an array of octets and store as integer
     @param dst   The integer to load
     @param src   The array of octets
     @param len   The number of octets
     @return CRYPT_OK on success
   */
   i32 (*unsigned_read)(         uk dst,
                        u8 *src,
                        u64  len);

/* ---- basic math ---- */

   /** add two integers
     @param a   The first source integer
     @param b   The second source integer
     @param c   The destination of "a + b"
     @return CRYPT_OK on success
   */
   i32 (*add)(uk a, uk b, uk c);

   /** add two integers
     @param a   The first source integer
     @param b   The second source integer
                (single digit of upto bits_per_digit in length)
     @param c   The destination of "a + b"
     @return CRYPT_OK on success
   */
   i32 (*addi)(uk a, ltc_mp_digit b, uk c);

   /** subtract two integers
     @param a   The first source integer
     @param b   The second source integer
     @param c   The destination of "a - b"
     @return CRYPT_OK on success
   */
   i32 (*sub)(uk a, uk b, uk c);

   /** subtract two integers
     @param a   The first source integer
     @param b   The second source integer
                (single digit of upto bits_per_digit in length)
     @param c   The destination of "a - b"
     @return CRYPT_OK on success
   */
   i32 (*subi)(uk a, ltc_mp_digit b, uk c);

   /** multiply two integers
     @param a   The first source integer
     @param b   The second source integer
                (single digit of upto bits_per_digit in length)
     @param c   The destination of "a * b"
     @return CRYPT_OK on success
   */
   i32 (*mul)(uk a, uk b, uk c);

   /** multiply two integers
     @param a   The first source integer
     @param b   The second source integer
                (single digit of upto bits_per_digit in length)
     @param c   The destination of "a * b"
     @return CRYPT_OK on success
   */
   i32 (*muli)(uk a, ltc_mp_digit b, uk c);

   /** Square an integer
     @param a    The integer to square
     @param b    The destination
     @return CRYPT_OK on success
   */
   i32 (*sqr)(uk a, uk b);

   /** Divide an integer
     @param a    The dividend
     @param b    The divisor
     @param c    The quotient (can be NULL to signify don't care)
     @param d    The remainder (can be NULL to signify don't care)
     @return CRYPT_OK on success
   */
   i32 (*mpdiv)(uk a, uk b, uk c, uk d);

   /** divide by two
      @param  a   The integer to divide (shift right)
      @param  b   The destination
      @return CRYPT_OK on success
   */
   i32 (*div_2)(uk a, uk b);

   /** Get remainder (small value)
      @param  a    The integer to reduce
      @param  b    The modulus (upto bits_per_digit in length)
      @param  c    The destination for the residue
      @return CRYPT_OK on success
   */
   i32 (*modi)(uk a, ltc_mp_digit b, ltc_mp_digit *c);

   /** gcd
      @param  a     The first integer
      @param  b     The second integer
      @param  c     The destination for (a, b)
      @return CRYPT_OK on success
   */
   i32 (*gcd)(uk a, uk b, uk c);

   /** lcm
      @param  a     The first integer
      @param  b     The second integer
      @param  c     The destination for [a, b]
      @return CRYPT_OK on success
   */
   i32 (*lcm)(uk a, uk b, uk c);

   /** Modular multiplication
      @param  a     The first source
      @param  b     The second source
      @param  c     The modulus
      @param  d     The destination (a*b mod c)
      @return CRYPT_OK on success
   */
   i32 (*mulmod)(uk a, uk b, uk c, uk d);

   /** Modular squaring
      @param  a     The first source
      @param  b     The modulus
      @param  c     The destination (a*a mod b)
      @return CRYPT_OK on success
   */
   i32 (*sqrmod)(uk a, uk b, uk c);

   /** Modular inversion
      @param  a     The value to invert
      @param  b     The modulus
      @param  c     The destination (1/a mod b)
      @return CRYPT_OK on success
   */
   i32 (*invmod)(uk , uk , uk );

/* ---- reduction ---- */

   /** setup Montgomery
       @param a  The modulus
       @param b  The destination for the reduction digit
       @return CRYPT_OK on success
   */
   i32 (*montgomery_setup)(uk a, uk *b);

   /** get normalization value
       @param a   The destination for the normalization value
       @param b   The modulus
       @return  CRYPT_OK on success
   */
   i32 (*montgomery_normalization)(uk a, uk b);

   /** reduce a number
       @param a   The number [and dest] to reduce
       @param b   The modulus
       @param c   The value "b" from montgomery_setup()
       @return CRYPT_OK on success
   */
   i32 (*montgomery_reduce)(uk a, uk b, uk c);

   /** clean up  (frees memory)
       @param a   The value "b" from montgomery_setup()
       @return CRYPT_OK on success
   */
   void (*montgomery_deinit)(uk a);

/* ---- exponentiation ---- */

   /** Modular exponentiation
       @param a    The base integer
       @param b    The power (can be negative) integer
       @param c    The modulus integer
       @param d    The destination
       @return CRYPT_OK on success
   */
   i32 (*exptmod)(uk a, uk b, uk c, uk d);

   /** Primality testing
       @param a     The integer to test
       @param b     The number of Miller-Rabin tests that shall be executed
       @param c     The destination of the result (FP_YES if prime)
       @return CRYPT_OK on success
   */
   i32 (*isprime)(uk a, i32 b, i32 *c);

/* ----  (optional) ecc point math ---- */

   /** ECC GF(p) point multiplication (from the NIST curves)
       @param k   The integer to multiply the point by
       @param G   The point to multiply
       @param R   The destination for kG
       @param modulus  The modulus for the field
       @param map Boolean indicated whether to map back to affine or not
                  (can be ignored if you work in affine only)
       @return CRYPT_OK on success
   */
   i32 (*ecc_ptmul)(     uk k,
                    ecc_point *G,
                    ecc_point *R,
                         uk modulus,
                          i32  map);

   /** ECC GF(p) point addition
       @param P    The first point
       @param Q    The second point
       @param R    The destination of P + Q
       @param modulus  The modulus
       @param mp   The "b" value from montgomery_setup()
       @return CRYPT_OK on success
   */
   i32 (*ecc_ptadd)(ecc_point *P,
                    ecc_point *Q,
                    ecc_point *R,
                         uk modulus,
                         uk mp);

   /** ECC GF(p) point double
       @param P    The first point
       @param R    The destination of 2P
       @param modulus  The modulus
       @param mp   The "b" value from montgomery_setup()
       @return CRYPT_OK on success
   */
   i32 (*ecc_ptdbl)(ecc_point *P,
                    ecc_point *R,
                         uk modulus,
                         uk mp);

   /** ECC mapping from projective to affine,
       currently uses (x,y,z) => (x/z^2, y/z^3, 1)
       @param P     The point to map
       @param modulus The modulus
       @param mp    The "b" value from montgomery_setup()
       @return CRYPT_OK on success
       @remark The mapping can be different but keep in mind a
               ecc_point only has three integers (x,y,z) so if
               you use a different mapping you have to make it fit.
   */
   i32 (*ecc_map)(ecc_point *P, uk modulus, uk mp);

   /** Computes kA*A + kB*B = C using Shamir's Trick
       @param A        First point to multiply
       @param kA       What to multiple A by
       @param B        Second point to multiply
       @param kB       What to multiple B by
       @param C        [out] Destination point (can overlap with A or B)
       @param modulus  Modulus for curve
       @return CRYPT_OK on success
   */
   i32 (*ecc_mul2add)(ecc_point *A, uk kA,
                      ecc_point *B, uk kB,
                      ecc_point *C,
                           uk modulus);

/* ---- (optional) rsa optimized math (for internal CRT) ---- */

   /** RSA Key Generation
       @param prng     An active PRNG state
       @param wprng    The index of the PRNG desired
       @param size     The size of the key in octets
       @param e        The "e" value (public key).
                       e==65537 is a good choice
       @param key      [out] Destination of a newly created private key pair
       @return CRYPT_OK if successful, upon error all allocated ram is freed
    */
    i32 (*rsa_keygen)(prng_state *prng,
                             i32  wprng,
                             i32  size,
                            long  e,
                         rsa_key *key);

   /** RSA exponentiation
      @param in       The octet array representing the base
      @param inlen    The length of the input
      @param out      The destination (to be stored in an octet array format)
      @param outlen   The length of the output buffer and the resulting size
                      (zero padded to the size of the modulus)
      @param which    PK_PUBLIC for public RSA and PK_PRIVATE for private RSA
      @param key      The RSA key to use
      @return CRYPT_OK on success
   */
   i32 (*rsa_me)(u8k *in,   u64 inlen,
                       u8 *out,  u64 *outlen, i32 which,
                       rsa_key *key);

/* ---- basic math continued ---- */

   /** Modular addition
      @param  a     The first source
      @param  b     The second source
      @param  c     The modulus
      @param  d     The destination (a + b mod c)
      @return CRYPT_OK on success
   */
   i32 (*addmod)(uk a, uk b, uk c, uk d);

   /** Modular substraction
      @param  a     The first source
      @param  b     The second source
      @param  c     The modulus
      @param  d     The destination (a - b mod c)
      @return CRYPT_OK on success
   */
   i32 (*submod)(uk a, uk b, uk c, uk d);

/* ---- misc stuff ---- */

   /** Make a pseudo-random mpi
      @param  a     The mpi to make random
      @param  size  The desired length
      @return CRYPT_OK on success
   */
   i32 (*rand)(uk a, i32 size);
} ltc_math_descriptor;

extern ltc_math_descriptor ltc_mp;

i32 ltc_init_multi(uk *a, ...);
void ltc_deinit_multi(uk a, ...);
void ltc_cleanup_multi(uk *a, ...);

#ifdef LTM_DESC
extern const ltc_math_descriptor ltm_desc;
#endif

#ifdef TFM_DESC
extern const ltc_math_descriptor tfm_desc;
#endif

#ifdef GMP_DESC
extern const ltc_math_descriptor gmp_desc;
#endif

#if !defined(DESC_DEF_ONLY) && defined(LTC_SOURCE)

#define MP_DIGIT_BIT                 ltc_mp.bits_per_digit

/* some handy macros */
#define mp_init(a)                   ltc_mp.init(a)
#define mp_init_multi                ltc_init_multi
#define mp_clear(a)                  ltc_mp.deinit(a)
#define mp_clear_multi               ltc_deinit_multi
#define mp_cleanup_multi             ltc_cleanup_multi
#define mp_init_copy(a, b)           ltc_mp.init_copy(a, b)

#define mp_neg(a, b)                 ltc_mp.neg(a, b)
#define mp_copy(a, b)                ltc_mp.copy(a, b)

#define mp_set(a, b)                 ltc_mp.set_int(a, b)
#define mp_set_int(a, b)             ltc_mp.set_int(a, b)
#define mp_get_int(a)                ltc_mp.get_int(a)
#define mp_get_digit(a, n)           ltc_mp.get_digit(a, n)
#define mp_get_digit_count(a)        ltc_mp.get_digit_count(a)
#define mp_cmp(a, b)                 ltc_mp.compare(a, b)
#define mp_cmp_d(a, b)               ltc_mp.compare_d(a, b)
#define mp_count_bits(a)             ltc_mp.count_bits(a)
#define mp_cnt_lsb(a)                ltc_mp.count_lsb_bits(a)
#define mp_2expt(a, b)               ltc_mp.twoexpt(a, b)

#define mp_read_radix(a, b, c)       ltc_mp.read_radix(a, b, c)
#define mp_toradix(a, b, c)          ltc_mp.write_radix(a, b, c)
#define mp_unsigned_bin_size(a)      ltc_mp.unsigned_size(a)
#define mp_to_unsigned_bin(a, b)     ltc_mp.unsigned_write(a, b)
#define mp_read_unsigned_bin(a, b, c) ltc_mp.unsigned_read(a, b, c)

#define mp_add(a, b, c)              ltc_mp.add(a, b, c)
#define mp_add_d(a, b, c)            ltc_mp.addi(a, b, c)
#define mp_sub(a, b, c)              ltc_mp.sub(a, b, c)
#define mp_sub_d(a, b, c)            ltc_mp.subi(a, b, c)
#define mp_mul(a, b, c)              ltc_mp.mul(a, b, c)
#define mp_mul_d(a, b, c)            ltc_mp.muli(a, b, c)
#define mp_sqr(a, b)                 ltc_mp.sqr(a, b)
#define mp_div(a, b, c, d)           ltc_mp.mpdiv(a, b, c, d)
#define mp_div_2(a, b)               ltc_mp.div_2(a, b)
#define mp_mod(a, b, c)              ltc_mp.mpdiv(a, b, NULL, c)
#define mp_mod_d(a, b, c)            ltc_mp.modi(a, b, c)
#define mp_gcd(a, b, c)              ltc_mp.gcd(a, b, c)
#define mp_lcm(a, b, c)              ltc_mp.lcm(a, b, c)

#define mp_addmod(a, b, c, d)        ltc_mp.addmod(a, b, c, d)
#define mp_submod(a, b, c, d)        ltc_mp.submod(a, b, c, d)
#define mp_mulmod(a, b, c, d)        ltc_mp.mulmod(a, b, c, d)
#define mp_sqrmod(a, b, c)           ltc_mp.sqrmod(a, b, c)
#define mp_invmod(a, b, c)           ltc_mp.invmod(a, b, c)

#define mp_montgomery_setup(a, b)    ltc_mp.montgomery_setup(a, b)
#define mp_montgomery_normalization(a, b) ltc_mp.montgomery_normalization(a, b)
#define mp_montgomery_reduce(a, b, c)   ltc_mp.montgomery_reduce(a, b, c)
#define mp_montgomery_free(a)        ltc_mp.montgomery_deinit(a)

#define mp_exptmod(a,b,c,d)          ltc_mp.exptmod(a,b,c,d)
#define mp_prime_is_prime(a, b, c)   ltc_mp.isprime(a, b, c)

#define mp_iszero(a)                 (mp_cmp_d(a, 0) == LTC_MP_EQ ? LTC_MP_YES : LTC_MP_NO)
#define mp_isodd(a)                  (mp_get_digit_count(a) > 0 ? (mp_get_digit(a, 0) & 1 ? LTC_MP_YES : LTC_MP_NO) : LTC_MP_NO)
#define mp_exch(a, b)                do { uk ABC__tmp = a; a = b; b = ABC__tmp; } while(0)

#define mp_tohex(a, b)               mp_toradix(a, b, 16)

#define mp_rand(a, b)                ltc_mp.rand(a, b)

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
