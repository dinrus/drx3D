/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

#ifdef LTC_MPI
#include <stdarg.h>

i32 ltc_init_multi(uk *a, ...)
{
   void    **cur = a;
   i32       np  = 0;
   va_list   args;

   va_start(args, a);
   while (cur != NULL) {
       if (mp_init(cur) != CRYPT_OK) {
          /* failed */
          va_list clean_list;

          va_start(clean_list, a);
          cur = a;
          while (np--) {
              mp_clear(*cur);
              cur = va_arg(clean_list, uk *);
          }
          va_end(clean_list);
          va_end(args);
          return CRYPT_MEM;
       }
       ++np;
       cur = va_arg(args, uk *);
   }
   va_end(args);
   return CRYPT_OK;
}

void ltc_deinit_multi(uk a, ...)
{
   void     *cur = a;
   va_list   args;

   va_start(args, a);
   while (cur != NULL) {
       mp_clear(cur);
       cur = va_arg(args, uk );
   }
   va_end(args);
}

void ltc_cleanup_multi(uk *a, ...)
{
   uk *cur = a;
   va_list args;

   va_start(args, a);
   while (cur != NULL) {
      if (*cur != NULL) {
         mp_clear(*cur);
         *cur = NULL;
      }
      cur = va_arg(args, uk *);
   }
   va_end(args);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
