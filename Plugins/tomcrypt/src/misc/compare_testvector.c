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
  @file compare_testvector.c
  Function to compare two testvectors and print a (detailed) error-message if required, Steffen Jaeckel
*/

#if defined(LTC_TEST) && defined(LTC_TEST_DBG)
static void _print_hex(tukk what, ukk v, const u64 l)
{
  u8k* p = v;
  u64 x, y = 0, z;
  fprintf(stderr, "%s contents: \n", what);
  for (x = 0; x < l; ) {
      fprintf(stderr, "%02X ", p[x]);
      if (!(++x % 16) || x == l) {
         if((x % 16) != 0) {
            z = 16 - (x % 16);
            if(z >= 8)
               fprintf(stderr, " ");
            for (; z != 0; --z) {
               fprintf(stderr, "   ");
            }
         }
         fprintf(stderr, " | ");
         for(; y < x; y++) {
            if((y % 8) == 0)
               fprintf(stderr, " ");
            if(isgraph(p[y]))
               fprintf(stderr, "%c", p[y]);
            else
               fprintf(stderr, ".");
         }
         fprintf(stderr, "\n");
      }
      else if((x % 8) == 0) {
         fprintf(stderr, " ");
      }
  }
}
#endif

/**
  Compare two test-vectors

  @param is             The data as it is
  @param is_len         The length of is
  @param should         The data as it should
  @param should_len     The length of should
  @param what           The type of the data
  @param which          The iteration count
  @return 0 on equality, -1 or 1 on difference
*/
i32 compare_testvector(ukk is, const u64 is_len, ukk should, const u64 should_len, tukk what, i32 which)
{
   i32 res = 0;
   if(is_len != should_len)
      res = is_len > should_len ? -1 : 1;
   else
      res = XMEMCMP(is, should, is_len);

#if defined(LTC_TEST) && defined(LTC_TEST_DBG)
   if (res != 0) {
      fprintf(stderr, "Testvector #%i of %s failed:\n", which, what);
      _print_hex("SHOULD", should, should_len);
      _print_hex("IS    ", is, is_len);
   }
#else
   LTC_UNUSED_PARAM(which);
   LTC_UNUSED_PARAM(what);
#endif

   return res;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
