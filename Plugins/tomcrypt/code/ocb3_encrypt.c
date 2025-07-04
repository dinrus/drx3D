/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file ocb3_encrypt.c
   OCB implementation, encrypt data, by Tom St Denis
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_OCB3_MODE

/**
   Encrypt blocks of data with OCB
   @param ocb     The OCB state
   @param pt      The plaintext (length multiple of the block size of the block cipher)
   @param ptlen   The length of the input (octets)
   @param ct      [out] The ciphertext (same size as the pt)
   @return CRYPT_OK if successful
*/
i32 ocb3_encrypt(ocb3_state *ocb, u8k *pt, u64 ptlen, u8 *ct)
{
   u8 tmp[MAXBLOCKSIZE];
   i32 err, i, full_blocks;
   u8 *pt_b, *ct_b;

   LTC_ARGCHK(ocb != NULL);
   if (ptlen == 0) return CRYPT_OK; /* no data, nothing to do */
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);

   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      return err;
   }
   if (ocb->block_len != cipher_descriptor[ocb->cipher].block_length) {
      return CRYPT_INVALID_ARG;
   }

   if (ptlen % ocb->block_len) { /* ptlen has to bu multiple of block_len */
      return CRYPT_INVALID_ARG;
   }

   full_blocks = ptlen/ocb->block_len;
   for(i=0; i<full_blocks; i++) {
     pt_b = (u8*)pt+i*ocb->block_len;
     ct_b = (u8*)ct+i*ocb->block_len;

     /* ocb->Offset_current[] = ocb->Offset_current[] ^ Offset_{ntz(block_index)} */
     ocb3_int_xor_blocks(ocb->Offset_current, ocb->Offset_current, ocb->L_[ocb3_int_ntz(ocb->block_index)], ocb->block_len);

     /* tmp[] = pt[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(tmp, pt_b, ocb->Offset_current, ocb->block_len);

     /* encrypt */
     if ((err = cipher_descriptor[ocb->cipher].ecb_encrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
        goto LBL_ERR;
     }

     /* ct[] = tmp[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(ct_b, tmp, ocb->Offset_current, ocb->block_len);

     /* ocb->checksum[] = ocb->checksum[] XOR pt[] */
     ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt_b, ocb->block_len);

     ocb->block_index++;
   }

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
