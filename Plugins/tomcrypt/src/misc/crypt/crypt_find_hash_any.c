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
  @file crypt_find_hash_any.c
  Find a hash, Tom St Denis
*/

/**
   Find a hash flexibly.  First by name then if not present by digest size
   @param name        The name of the hash desired
   @param digestlen   The minimum length of the digest size (octets)
   @return >= 0 if found, -1 if not present
*/i32 find_hash_any(tukk name, i32 digestlen)
{
   i32 x, y, z;
   LTC_ARGCHK(name != NULL);

   x = find_hash(name);
   if (x != -1) return x;

   LTC_MUTEX_LOCK(&ltc_hash_mutex);
   y = MAXBLOCKSIZE+1;
   z = -1;
   for (x = 0; x < TAB_SIZE; x++) {
       if (hash_descriptor[x].name == NULL) {
          continue;
       }
       if ((i32)hash_descriptor[x].hashsize >= digestlen && (i32)hash_descriptor[x].hashsize < y) {
          z = x;
          y = hash_descriptor[x].hashsize;
       }
   }
   LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
   return z;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
