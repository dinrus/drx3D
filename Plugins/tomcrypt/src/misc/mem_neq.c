/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
   @file mem_neq.c
   Compare two blocks of memory for inequality in constant time.
   Steffen Jaeckel
*/

/**
   Compare two blocks of memory for inequality in constant time.

   The usage is similar to that of standard memcmp, but you can only test
   if the memory is equal or not - you can not determine by how much the
   first different byte differs.

   This function shall be used to compare results of cryptographic
   operations where inequality means most likely usage of a wrong key.
   The execution time has therefore to be constant as otherwise
   timing attacks could be possible.

   @param a     The first memory region
   @param b     The second memory region
   @param len   The length of the area to compare (octets)

   @return 0 when a and b are equal for len bytes, 1 they are not equal.
*/
i32 mem_neq(ukk a, ukk b, size_t len)
{
   u8 ret = 0;
   u8k* pa;
   u8k* pb;

   LTC_ARGCHK(a != NULL);
   LTC_ARGCHK(b != NULL);

   pa = a;
   pb = b;

   while (len-- > 0) {
      ret |= *pa ^ *pb;
      ++pa;
      ++pb;
   }

   ret |= ret >> 4;
   ret |= ret >> 2;
   ret |= ret >> 1;
   ret &= 1;

   return ret;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
