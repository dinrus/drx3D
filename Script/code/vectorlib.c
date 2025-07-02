// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <drx/plugin/lua/lua.h>
#include <drx/plugin/lua/lauxlib.h>

#ifdef __cplusplus
} //extern "C" {
#endif //__cplusplus

i32 g_vectortag=0;

i32 g_vectorMetatable = 0;
ukk g_metatablePointer = 0;

i32 vl_isvector(lua_State *L,i32 index);

float *newvector(lua_State *L)
{
	float *v=(float *)lua_newuserdata(L, sizeof(float)*3);
	i32 nparams=lua_gettop(L);
	if(nparams>0)
	{
		v[0]=lua_tonumber(L,1);
		v[1]=lua_tonumber(L,2);
		v[2]=lua_tonumber(L,3);
	}
	else{
		v[0]=v[1]=v[2]=0.0f;
	}
	luaL_ref(L,g_vectorMetatable);
	lua_setmetatable(L,-2);
	return v;
}

lua_Number luaL_check_number(lua_State *L,i32 index)
{
	return lua_tonumber(L,index);
}

tukk luaL_check_string(lua_State *L,i32 index)
{
	return lua_tostring(L,index);
}

i32 vector_set(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		tukk idx=luaL_check_string(L,2);
		if(idx)
		{
			switch(idx[0])
			{
			case 'x':case 'r':
				v[0]=luaL_check_number(L,3);
				return 0;
			case 'y':case 'g':
				v[1]=luaL_check_number(L,3);
				return 0;
			case 'z':case 'b':
				v[2]=luaL_check_number(L,3);
				return 0;
			default:
				break;
			}
		}
	}
	return 0;
}

i32 vector_get(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		tukk idx=luaL_check_string(L,2);
		if(idx)
		{
			switch(idx[0])
			{
			case 'x':case 'r':
				lua_pushnumber(L,v[0]);
				return 1;
			case 'y':case 'g':
				lua_pushnumber(L,v[1]);
				return 1;
			case 'z':case 'b':
				lua_pushnumber(L,v[2]);
				return 1;
			default:
				return 0;
				break;
			}
		}
	}
	return 0;
}

i32 vector_mul(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float res=v[0]*v2[0] + v[1]*v2[1] + v[2]*v2[2];
			lua_pushnumber(L,res);
			return 1;
		}
		else if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]*f;
			newv[1]=v[1]*f;
			newv[2]=v[2]*f;
			return 1;

		}
		//else lua_error(L,"mutiplying a vector with an invalid type");
	}
	return 0;
}

i32 vector_add(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]+v2[0];
			newv[1]=v[1]+v2[1];
			newv[2]=v[2]+v2[2];
			return 1;
		}
		//else lua_error(L,"adding a vector with an invalid type");
	}
	return 0;
}

i32 vector_div(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]/f;
			newv[1]=v[1]/f;
			newv[2]=v[2]/f;
			return 1;

		}
		//else lua_error(L,"dividing a vector with an invalid type");
	}
	return 0;
}

i32 vector_sub(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]-v2[0];
			newv[1]=v[1]-v2[1];
			newv[2]=v[2]-v2[2];
			return 1;
		}
		else if(lua_isnumber(L,2))
		{
			float f=lua_tonumber(L,2);
			float *newv=newvector(L);
			newv[0]=v[0]-f;
			newv[1]=v[1]-f;
			newv[2]=v[2]-f;
			return 1;

		}
		//else lua_error(L,"subtracting a vector with an invalid type");
	}
	return 0;
}

i32 vector_unm(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		float *newv=newvector(L);
		newv[0]=-v[0];
		newv[1]=-v[1];
		newv[2]=-v[2];
		return 1;
	}
	return 0;
}

i32 vector_pow(lua_State *L)
{
	float *v=(float *)lua_touserdata(L,1);
	if(v)
	{
		if(vl_isvector(L,2))
		{
			float *v2=(float *)lua_touserdata(L,2);
			float *newv=newvector(L);
			newv[0]=v[1]*v2[2]-v[2]*v2[1];
			newv[1]=v[2]*v2[0]-v[0]*v2[2];
			newv[2]=v[0]*v2[1]-v[1]*v2[0];
			return 1;
		}
		//else lua_error(L,"cross product between vector and an invalid type");
	}
	return 0;
}

i32 vl_newvector(lua_State *L)
{
	newvector(L);
	return 1;
}

i32 vl_isvector(lua_State *L,i32 index)
{
	ukk ptr;
	if (lua_type(L,index) != LUA_TUSERDATA)
		return 0;
    lua_getmetatable(L,index);
	ptr = lua_topointer(L,-1);
	lua_pop(L,1);
	return (ptr == g_metatablePointer);
}

void vl_SetEventFunction(lua_State *L,tukk sEvent,lua_CFunction fn,i32 nTable)
{
	lua_pushstring(L,sEvent);
	lua_pushcclosure(L,fn, 0);
	lua_rawset(L,nTable);
}


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

LUALIB_API i32 vl_initvectorlib(lua_State *L)
{
	i32 nTable;
	// Create a new vector metatable.
	lua_newtable(L);
	nTable = lua_gettop(L);

	g_metatablePointer = lua_topointer(L,nTable);

	vl_SetEventFunction( L,"__newindex",vector_set,nTable );
	vl_SetEventFunction( L,"__index",vector_get,nTable );
	vl_SetEventFunction( L,"__mul",vector_mul,nTable );
	vl_SetEventFunction( L,"__div",vector_div,nTable );
	vl_SetEventFunction( L,"__add",vector_add,nTable );
	vl_SetEventFunction( L,"__sub",vector_sub,nTable );
	vl_SetEventFunction( L,"__pow",vector_pow,nTable );
	vl_SetEventFunction( L,"__unm",vector_unm,nTable );
	
	g_vectorMetatable = luaL_ref(L,nTable); // pop table
	return 1;
}

#ifdef __cplusplus
} //extern "C" {
#endif //__cplusplus