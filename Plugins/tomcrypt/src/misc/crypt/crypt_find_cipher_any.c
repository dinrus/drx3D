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
  @file crypt_find_cipher_any.c
  Find a cipher in the descriptor tables, Tom St Denis
*/

/**
   Find a cipher flexibly.  First by name then if not present by block and key size
   @param name        The name of the cipher desired
   @param blocklen    The minimum length of the block cipher desired (octets)
   @param keylen      The minimum length of the key size desired (octets)
   @return >= 0 if found, -1 if not present
*/
i32 find_cipher_any(tukk name, i32 blocklen, i32 keylen)
{
   i32 x;

   if(name != NULL) {
      x = find_cipher(name);
      if (x != -1) return x;
   }

   LTC_MUTEX_LOCK(&ltc_cipher_mutex);
   for (x = 0; x < TAB_SIZE; x++) {
       if (cipher_descriptor[x].name == NULL) {
          continue;
       }
       if (blocklen <= (i32)cipher_descriptor[x].block_length && keylen <= (i32)cipher_descriptor[x].max_key_length) {
          LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
          return x;
       }
   }
   LTC_MUTEX_UNLOCK(&ltc_cipher_mutex);
   return -1;
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
