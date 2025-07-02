/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifndef TOMCRYPT_H_
#define TOMCRYPT_H_
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>


/* use configuration data */
#include <drx3D/Plugins/tomcrypt/tomcrypt_custom.h>

#ifdef __cplusplus
extern "C" {
#endif

/* version */
#define CRYPT   0x0118
#define SCRYPT  "1.18.2"

/* max size of either a cipher/hash block or symmetric key [largest of the two] */
#define MAXBLOCKSIZE  128

#ifndef TAB_SIZE
/* descriptor table size */
#define TAB_SIZE      32
#endif

/* error codes [will be expanded in future releases] */
enum {
   CRYPT_OK=0,             /* Result OK */
   CRYPT_ERROR,            /* Generic Error */
   CRYPT_NOP,              /* Not a failure but no operation was performed */

   CRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
   CRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
   CRYPT_FAIL_TESTVECTOR,  /* Algorithm failed test vectors */

   CRYPT_BUFFER_OVERFLOW,  /* Not enough space for output */
   CRYPT_INVALID_PACKET,   /* Invalid input packet given */

   CRYPT_INVALID_PRNGSIZE, /* Invalid number of bits for a PRNG */
   CRYPT_ERROR_READPRNG,   /* Could not read enough from PRNG */

   CRYPT_INVALID_CIPHER,   /* Invalid cipher specified */
   CRYPT_INVALID_HASH,     /* Invalid hash specified */
   CRYPT_INVALID_PRNG,     /* Invalid PRNG specified */

   CRYPT_MEM,              /* Out of memory */

   CRYPT_PK_TYPE_MISMATCH, /* Not equivalent types of PK keys */
   CRYPT_PK_NOT_PRIVATE,   /* Requires a private PK key */

   CRYPT_INVALID_ARG,      /* Generic invalid argument */
   CRYPT_FILE_NOTFOUND,    /* File Not Found */

   CRYPT_PK_INVALID_TYPE,  /* Invalid type of PK key */

   CRYPT_OVERFLOW,         /* An overflow of a value was detected/prevented */

   CRYPT_UNUSED1,          /* UNUSED1 */

   CRYPT_INPUT_TOO_LONG,   /* The input was longer than expected. */

   CRYPT_PK_INVALID_SIZE,  /* Invalid size input for PK parameters */

   CRYPT_INVALID_PRIME_SIZE,/* Invalid size of prime requested */
   CRYPT_PK_INVALID_PADDING, /* Invalid padding on input */

   CRYPT_HASH_OVERFLOW      /* Hash applied to too many bits */
};
////////////////////////////////

/* some default configurations.
 *
 * A "mp_digit" must be able to hold MP_DIGIT_BIT + 1 bits
 * A "mp_word" must be able to hold 2*MP_DIGIT_BIT + 1 bits
 *
 * At the very least a mp_digit must be able to hold 7 bits
 * [any size beyond that is ok provided it doesn't overflow the data type]
 */

#ifdef MP_8BIT
typedef uint8_t              mp_digit;
typedef uint16_t             private_mp_word;
//#   define MP_DIGIT_BIT 7
#elif defined(MP_16BIT)
typedef uint16_t             mp_digit;
typedef uint32_t             private_mp_word;
//#   define MP_DIGIT_BIT 15
#elif defined(MP_64BIT)
/* for GCC only on supported platforms */
typedef uint64_t mp_digit;
#if defined(__GNUC__)
typedef u64        private_mp_word __attribute__((mode(TI)));
#endif
//#   define MP_DIGIT_BIT 60
#else
typedef uint32_t             mp_digit;
typedef uint64_t             private_mp_word;
#   ifdef MP_31BIT
/*
 * This is an extension that uses 31-bit digits.
 * Please be aware that not all functions support this size, especially s_mp_mul_digs_fast
 * will be reduced to work on small numbers only:
 * Up to 8 limbs, 248 bits instead of up to 512 limbs, 15872 bits with MP_28BIT.
 */
//#      define MP_DIGIT_BIT 31
#   else
/* default case is 28-bit digits, defines MP_28BIT as a handy macro to test */
//#      define MP_DIGIT_BIT 28
#      define MP_28BIT
#   endif
#endif

/* mp_word is a private type */
#define mp_word MP_DEPRECATED_PRAGMA("mp_word has been made private") private_mp_word

#define MP_SIZEOF_MP_DIGIT (MP_DEPRECATED_PRAGMA("MP_SIZEOF_MP_DIGIT has been deprecated, use sizeof (mp_digit)") sizeof (mp_digit))

#define MP_MASK          ((((mp_digit)1)<<((mp_digit)MP_DIGIT_BIT))-((mp_digit)1))
#define MP_DIGIT_MAX     MP_MASK

/* Primality generation flags */
#define MP_PRIME_BBS      0x0001 /* BBS style prime */
#define MP_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define MP_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

#define LTM_PRIME_BBS      (MP_DEPRECATED_PRAGMA("LTM_PRIME_BBS has been deprecated, use MP_PRIME_BBS") MP_PRIME_BBS)
#define LTM_PRIME_SAFE     (MP_DEPRECATED_PRAGMA("LTM_PRIME_SAFE has been deprecated, use MP_PRIME_SAFE") MP_PRIME_SAFE)
#define LTM_PRIME_2MSB_ON  (MP_DEPRECATED_PRAGMA("LTM_PRIME_2MSB_ON has been deprecated, use MP_PRIME_2MSB_ON") MP_PRIME_2MSB_ON)


typedef enum {
   MP_OKAY  = 0,   /* no error */
   MP_ERR   = -1,  /* unknown error */
   MP_MEM   = -2,  /* out of mem */
   MP_VAL   = -3,  /* invalid input */
   MP_ITER  = -4,  /* maximum iterations reached */
   MP_BUF   = -5   /* buffer overflow, supplied buffer too small */
} mp_err;
typedef enum {
   MP_ZPOS = 0,   /* positive */
   MP_NEG = 1     /* negative */
} mp_sign;
typedef enum {
   MP_LT = -1,    /* less than */
   MP_EQ = 0,     /* equal */
   MP_GT = 1      /* greater than */
} mp_ord;
typedef enum {
   MP_NO = 0,
   MP_YES = 1
} mp_bool;
typedef enum {
   MP_LSB_FIRST = -1,
   MP_MSB_FIRST =  1
} mp_order;
typedef enum {
   MP_LITTLE_ENDIAN  = -1,
   MP_NATIVE_ENDIAN  =  0,
   MP_BIG_ENDIAN     =  1
} mp_endian;

typedef struct  {
   i32 used, alloc;
   mp_sign sign;
   mp_digit *dp;
} mp_int;

#include <drx3D/Plugins/tomcrypt/tomcrypt_cfg.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_macros.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_cipher.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_hash.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_mac.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_prng.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_pk.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_math.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_misc.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_argchk.h>
#include <drx3D/Plugins/tomcrypt/tomcrypt_pkcs.h>

#ifdef __cplusplus
   }
#endif

#endif /* TOMCRYPT_H_ */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
