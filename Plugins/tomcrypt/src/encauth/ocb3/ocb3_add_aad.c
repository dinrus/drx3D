/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_add_aad.c
   OCB implementation, add AAD data, by Karel Miko
*/
#include "tomcrypt.h"

#ifdef LTC_OCB3_MODE

/**
   Add one block of AAD data (internal function)
   @param ocb        The OCB state
   @param aad_block  [in] AAD data (block_len size)
   @return CRYPT_OK if successful
*/
static i32 _ocb3_int_aad_add_block(ocb3_state *ocb, u8k *aad_block)
{
   u8 tmp[MAXBLOCKSIZE];
   i32 err;

   /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
   ocb3_int_xor_blocks(ocb->aOffset_current, ocb->aOffset_current, ocb->L_[ocb3_int_ntz(ocb->ablock_index)], ocb->block_len);

   /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i) */
   ocb3_int_xor_blocks(tmp, aad_block, ocb->aOffset_current, ocb->block_len);
   if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
     return err;
   }
   ocb3_int_xor_blocks(ocb->aSum_current, ocb->aSum_current, tmp, ocb->block_len);

   ocb->ablock_index++;

   return CRYPT_OK;
}

/**
   Add AAD - additional associated data
   @param ocb       The OCB state
   @param aad       The AAD data
   @param aadlen    The size of AAD data (octets)
   @return CRYPT_OK if successful
*/
i32 ocb3_add_aad(ocb3_state *ocb, u8k *aad, u64 aadlen)
{
   i32 err, x, full_blocks, full_blocks_len, last_block_len;
   u8 *data;
   u64 datalen, l;

   LTC_ARGCHK(ocb != NULL);
   if (aadlen == 0) return CRYPT_OK;
   LTC_ARGCHK(aad != NULL);

   if (ocb->adata_buffer_bytes > 0) {
     l = ocb->block_len - ocb->adata_buffer_bytes;
     if (l > aadlen) l = aadlen;
     XMEMCPY(ocb->adata_buffer+ocb->adata_buffer_bytes, aad, l);
     ocb->adata_buffer_bytes += l;

     if (ocb->adata_buffer_bytes == ocb->block_len) {
       if ((err = _ocb3_int_aad_add_block(ocb, ocb->adata_buffer)) != CRYPT_OK) {
         return err;
       }
       ocb->adata_buffer_bytes = 0;
     }

     data = (u8*)aad + l;
     datalen = aadlen - l;
   }
   else {
     data = (u8*)aad;
     datalen = aadlen;
   }

   if (datalen == 0) return CRYPT_OK;

   full_blocks = datalen/ocb->block_len;
   full_blocks_len = full_blocks * ocb->block_len;
   last_block_len = datalen - full_blocks_len;

   for (x=0; x<full_blocks; x++) {
     if ((err = _ocb3_int_aad_add_block(ocb, data+x*ocb->block_len)) != CRYPT_OK) {
       return err;
     }
   }

   if (last_block_len>0) {
     XMEMCPY(ocb->adata_buffer, data+full_blocks_len, last_block_len);
     ocb->adata_buffer_bytes = last_block_len;
   }

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
