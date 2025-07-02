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
  @file der_encode_setof.c
  ASN.1 DER, Encode SET OF, Tom St Denis
*/

#ifdef LTC_DER

struct edge {
   u8 *start;
   u64  size;
};

static i32 _qsort_helper(ukk a, ukk b)
{
   struct edge   *A = (struct edge *)a, *B = (struct edge *)b;
   i32            r;
   u64  x;

   /* compare min length */
   r = XMEMCMP(A->start, B->start, MIN(A->size, B->size));

   if (r == 0 && A->size != B->size) {
      if (A->size > B->size) {
         for (x = B->size; x < A->size; x++) {
            if (A->start[x]) {
               return 1;
            }
         }
      } else {
         for (x = A->size; x < B->size; x++) {
            if (B->start[x]) {
               return -1;
            }
         }
      }
   }

   return r;
}

/**
   Encode a SETOF stucture
   @param list      The list of items to encode
   @param inlen     The number of items in the list
   @param out       [out] The destination
   @param outlen    [in/out] The size of the output
   @return CRYPT_OK on success
*/
i32 der_encode_setof(ltc_asn1_list *list, u64 inlen,
                     u8 *out,  u64 *outlen)
{
   u64  x, y, z;
   ptrdiff_t hdrlen;
   i32            err;
   struct edge   *edges;
   u8 *ptr, *buf;

   /* check that they're all the same type */
   for (x = 1; x < inlen; x++) {
      if (list[x].type != list[x-1].type) {
         return CRYPT_INVALID_ARG;
      }
   }

   /* alloc buffer to store copy of output */
   buf = XCALLOC(1, *outlen);
   if (buf == NULL) {
      return CRYPT_MEM;
   }

   /* encode list */
   if ((err = der_encode_sequence_ex(list, inlen, buf, outlen, LTC_ASN1_SETOF)) != CRYPT_OK) {
       XFREE(buf);
       return err;
   }

   /* allocate edges */
   edges = XCALLOC(inlen, sizeof(*edges));
   if (edges == NULL) {
      XFREE(buf);
      return CRYPT_MEM;
   }

   /* skip header */
   ptr = buf + 1;

   /* now skip length data */
   x = *ptr++;
   if (x >= 0x80) {
      ptr += (x & 0x7F);
   }

   /* get the size of the static header */
   hdrlen = ptr - buf;


   /* scan for edges */
   x = 0;
   while (ptr < (buf + *outlen)) {
      /* store start */
      edges[x].start = ptr;

      /* skip type */
      z = 1;

      /* parse length */
      y = ptr[z++];
      if (y < 128) {
         edges[x].size = y;
      } else {
         y &= 0x7F;
         edges[x].size = 0;
         while (y--) {
            edges[x].size = (edges[x].size << 8) | ((u64)ptr[z++]);
         }
      }

      /* skip content */
      edges[x].size += z;
      ptr           += edges[x].size;
      ++x;
   }

   /* sort based on contents (using edges) */
   XQSORT(edges, inlen, sizeof(*edges), &_qsort_helper);

   /* copy static header */
   XMEMCPY(out, buf, hdrlen);

   /* copy+sort using edges+indecies to output from buffer */
   for (y = (u64)hdrlen, x = 0; x < inlen; x++) {
      XMEMCPY(out+y, edges[x].start, edges[x].size);
      y += edges[x].size;
   }

#ifdef LTC_CLEAN_STACK
   zeromem(buf, *outlen);
#endif

   /* free buffers */
   XFREE(edges);
   XFREE(buf);

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
