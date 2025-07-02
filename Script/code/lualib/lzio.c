/*
** $Id: lzio.c $
** Buffered streams
** See Разработка Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#include <plugin/lua/lprefix.h>


#include <string.h>

#include <plugin/lua/lua.h>

#include <plugin/lua/llimits.h>
#include <plugin/lua/lmem.h>
#include <plugin/lua/lstate.h>
#include <plugin/lua/lzio.h>


i32 luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  tukk buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  if (buff == NULL || size == 0)
    return EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  return cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, uk data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */
size_t luaZ_read (ZIO *z, uk b, size_t n) {
  while (n) {
    size_t m;
    if (z->n == 0) {  /* no bytes in buffer? */
      if (luaZ_fill(z) == EOZ)  /* try to read more */
        return n;  /* no more input; return number of missing bytes */
      else {
        z->n++;  /* luaZ_fill consumed first byte; put it back */
        z->p--;
      }
    }
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}

