/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_int_xor_blocks.c
   OCB implementation, INTERNAL ONLY helper, by Karel Miko
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_OCB3_MODE

/**
   Compute xor for two blocks of bytes 'out = block_a XOR block_b' (internal function)
   @param out        The block of bytes (output)
   @param block_a    The block of bytes (input)
   @param block_b    The block of bytes (input)
   @param block_len  The size of block_a, block_b, out
*/
void ocb3_int_xor_blocks(u8 *out, u8k *block_a, u8k *block_b, u64 block_len)
{
   i32 x;
   if (out == block_a) {
     for (x = 0; x < (i32)block_len; x++) out[x] ^= block_b[x];
   }
   else {
     for (x = 0; x < (i32)block_len; x++) out[x] = block_a[x] ^ block_b[x];
   }
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
