//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static tukk batchingKernelsNewCL =
	"/*\n"
	"Copyright (c) 2012 Advanced Micro Devices, Inc.  \n"
	"This software is provided 'as-is', without any express or implied warranty.\n"
	"In no event will the authors be held liable for any damages arising from the use of this software.\n"
	"Permission is granted to anyone to use this software for any purpose, \n"
	"including commercial applications, and to alter it and redistribute it freely, \n"
	"subject to the following restrictions:\n"
	"1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.\n"
	"2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.\n"
	"3. This notice may not be removed or altered from any source distribution.\n"
	"*/\n"
	"//Originally written by Erwin Coumans\n"
	"#ifndef D3_CONTACT4DATA_H\n"
	"#define D3_CONTACT4DATA_H\n"
	"#ifndef D3_FLOAT4_H\n"
	"#define D3_FLOAT4_H\n"
	"#ifndef D3_PLATFORM_DEFINITIONS_H\n"
	"#define D3_PLATFORM_DEFINITIONS_H\n"
	"struct MyTest\n"
	"{\n"
	"	i32 bla;\n"
	"};\n"
	"#ifdef __cplusplus\n"
	"#else\n"
	"//keep D3_LARGE_FLOAT*D3_LARGE_FLOAT < FLT_MAX\n"
	"#define D3_LARGE_FLOAT 1e18f\n"
	"#define D3_INFINITY 1e18f\n"
	"#define drx3DAssert(a)\n"
	"#define b3ConstArray(a) __global const a*\n"
	"#define b3AtomicInc atomic_inc\n"
	"#define b3AtomicAdd atomic_add\n"
	"#define b3Fabs fabs\n"
	"#define b3Sqrt native_sqrt\n"
	"#define b3Sin native_sin\n"
	"#define b3Cos native_cos\n"
	"#define D3_STATIC\n"
	"#endif\n"
	"#endif\n"
	"#ifdef __cplusplus\n"
	"#else\n"
	"	typedef float4	b3Float4;\n"
	"	#define b3Float4ConstArg const b3Float4\n"
	"	#define b3MakeFloat4 (float4)\n"
	"	float b3Dot3F4(b3Float4ConstArg v0,b3Float4ConstArg v1)\n"
	"	{\n"
	"		float4 a1 = b3MakeFloat4(v0.xyz,0.f);\n"
	"		float4 b1 = b3MakeFloat4(v1.xyz,0.f);\n"
	"		return dot(a1, b1);\n"
	"	}\n"
	"	b3Float4 b3Cross3(b3Float4ConstArg v0,b3Float4ConstArg v1)\n"
	"	{\n"
	"		float4 a1 = b3MakeFloat4(v0.xyz,0.f);\n"
	"		float4 b1 = b3MakeFloat4(v1.xyz,0.f);\n"
	"		return cross(a1, b1);\n"
	"	}\n"
	"	#define b3MinFloat4 min\n"
	"	#define b3MaxFloat4 max\n"
	"	#define b3Normalized(a) normalize(a)\n"
	"#endif \n"
	"		\n"
	"inline bool b3IsAlmostZero(b3Float4ConstArg v)\n"
	"{\n"
	"	if(b3Fabs(v.x)>1e-6 || b3Fabs(v.y)>1e-6 || b3Fabs(v.z)>1e-6)	\n"
	"		return false;\n"
	"	return true;\n"
	"}\n"
	"inline i32    b3MaxDot( b3Float4ConstArg vec, __global const b3Float4* vecArray, i32 vecLen, float* dotOut )\n"
	"{\n"
	"    float maxDot = -D3_INFINITY;\n"
	"    i32 i = 0;\n"
	"    i32 ptIndex = -1;\n"
	"    for( i = 0; i < vecLen; i++ )\n"
	"    {\n"
	"        float dot = b3Dot3F4(vecArray[i],vec);\n"
	"            \n"
	"        if( dot > maxDot )\n"
	"        {\n"
	"            maxDot = dot;\n"
	"            ptIndex = i;\n"
	"        }\n"
	"    }\n"
	"	drx3DAssert(ptIndex>=0);\n"
	"    if (ptIndex<0)\n"
	"	{\n"
	"		ptIndex = 0;\n"
	"	}\n"
	"    *dotOut = maxDot;\n"
	"    return ptIndex;\n"
	"}\n"
	"#endif //D3_FLOAT4_H\n"
	"typedef  struct b3Contact4Data b3Contact4Data_t;\n"
	"struct b3Contact4Data\n"
	"{\n"
	"	b3Float4	m_worldPosB[4];\n"
	"//	b3Float4	m_localPosA[4];\n"
	"//	b3Float4	m_localPosB[4];\n"
	"	b3Float4	m_worldNormalOnB;	//	w: m_nPoints\n"
	"	unsigned short  m_restituitionCoeffCmp;\n"
	"	unsigned short  m_frictionCoeffCmp;\n"
	"	i32 m_batchIdx;\n"
	"	i32 m_bodyAPtrAndSignBit;//x:m_bodyAPtr, y:m_bodyBPtr\n"
	"	i32 m_bodyBPtrAndSignBit;\n"
	"	i32	m_childIndexA;\n"
	"	i32	m_childIndexB;\n"
	"	i32 m_unused1;\n"
	"	i32 m_unused2;\n"
	"};\n"
	"inline i32 b3Contact4Data_getNumPoints(const struct b3Contact4Data* contact)\n"
	"{\n"
	"	return (i32)contact->m_worldNormalOnB.w;\n"
	"};\n"
	"inline void b3Contact4Data_setNumPoints(struct b3Contact4Data* contact, i32 numPoints)\n"
	"{\n"
	"	contact->m_worldNormalOnB.w = (float)numPoints;\n"
	"};\n"
	"#endif //D3_CONTACT4DATA_H\n"
	"#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable\n"
	"#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable\n"
	"#ifdef cl_ext_atomic_counters_32\n"
	"#pragma OPENCL EXTENSION cl_ext_atomic_counters_32 : enable\n"
	"#else\n"
	"#define counter32_t  __global i32*\n"
	"#endif\n"
	"#define SIMD_WIDTH 64\n"
	//"typedef u32 u32;\n"
//	"typedef unsigned short u16;\n"
	//"typedef u8 u8;\n"
	"#define GET_GROUP_IDX get_group_id(0)\n"
	"#define GET_LOCAL_IDX get_local_id(0)\n"
	"#define GET_GLOBAL_IDX get_global_id(0)\n"
	"#define GET_GROUP_SIZE get_local_size(0)\n"
	"#define GET_NUM_GROUPS get_num_groups(0)\n"
	"#define GROUP_LDS_BARRIER barrier(CLK_LOCAL_MEM_FENCE)\n"
	"#define GROUP_MEM_FENCE mem_fence(CLK_LOCAL_MEM_FENCE)\n"
	"#define AtomInc(x) atom_inc(&(x))\n"
	"#define AtomInc1(x, out) out = atom_inc(&(x))\n"
	"#define AppendInc(x, out) out = atomic_inc(x)\n"
	"#define AtomAdd(x, value) atom_add(&(x), value)\n"
	"#define AtomCmpxhg(x, cmp, value) atom_cmpxchg( &(x), cmp, value )\n"
	"#define AtomXhg(x, value) atom_xchg ( &(x), value )\n"
	"#define SELECT_UINT4( b, a, condition ) select( b,a,condition )\n"
	"#define make_float4 (float4)\n"
	"#define make_float2 (float2)\n"
	"#define make_uint4 (uint4)\n"
	"#define make_int4 (int4)\n"
	"#define make_uint2 (uint2)\n"
	"#define make_int2 (int2)\n"
	"#define max2 max\n"
	"#define min2 min\n"
	"#define WG_SIZE 64\n"
	"typedef struct \n"
	"{\n"
	"	i32 m_n;\n"
	"	i32 m_start;\n"
	"	i32 m_staticIdx;\n"
	"	i32 m_paddings[1];\n"
	"} ConstBuffer;\n"
	"typedef struct \n"
	"{\n"
	"	i32 m_a;\n"
	"	i32 m_b;\n"
	"	u32 m_idx;\n"
	"}Elem;\n"
	"//	batching on the GPU\n"
	"__kernel void CreateBatchesBruteForce( __global struct b3Contact4Data* gConstraints, 	__global u32k* gN, __global u32k* gStart, i32 m_staticIdx )\n"
	"{\n"
	"	i32 wgIdx = GET_GROUP_IDX;\n"
	"	i32 lIdx = GET_LOCAL_IDX;\n"
	"	\n"
	"	i32k m_n = gN[wgIdx];\n"
	"	i32k m_start = gStart[wgIdx];\n"
	"		\n"
	"	if( lIdx == 0 )\n"
	"	{\n"
	"		for (i32 i=0;i<m_n;i++)\n"
	"		{\n"
	"			i32 srcIdx = i+m_start;\n"
	"			i32 batchIndex = i;\n"
	"			gConstraints[ srcIdx ].m_batchIdx = batchIndex;	\n"
	"		}\n"
	"	}\n"
	"}\n"
	"#define CHECK_SIZE (WG_SIZE)\n"
	"u32 readBuf(__local u32* buff, i32 idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	i32 bitIdx = idx%32;\n"
	"	i32 bufIdx = idx/32;\n"
	"	return buff[bufIdx] & (1<<bitIdx);\n"
	"}\n"
	"void writeBuf(__local u32* buff, i32 idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	i32 bitIdx = idx%32;\n"
	"	i32 bufIdx = idx/32;\n"
	"	buff[bufIdx] |= (1<<bitIdx);\n"
	"	//atom_or( &buff[bufIdx], (1<<bitIdx) );\n"
	"}\n"
	"u32 tryWrite(__local u32* buff, i32 idx)\n"
	"{\n"
	"	idx = idx % (32*CHECK_SIZE);\n"
	"	i32 bitIdx = idx%32;\n"
	"	i32 bufIdx = idx/32;\n"
	"	u32 ans = (u32)atom_or( &buff[bufIdx], (1<<bitIdx) );\n"
	"	return ((ans >> bitIdx)&1) == 0;\n"
	"}\n"
	"//	batching on the GPU\n"
	"__kernel void CreateBatchesNew( __global struct b3Contact4Data* gConstraints, __global u32k* gN, __global u32k* gStart, __global i32* batchSizes, i32 staticIdx )\n"
	"{\n"
	"	i32 wgIdx = GET_GROUP_IDX;\n"
	"	i32 lIdx = GET_LOCAL_IDX;\n"
	"	i32k numConstraints = gN[wgIdx];\n"
	"	i32k m_start = gStart[wgIdx];\n"
	"	b3Contact4Data_t tmp;\n"
	"	\n"
	"	__local u32 ldsFixedBuffer[CHECK_SIZE];\n"
	"		\n"
	"	\n"
	"	\n"
	"	\n"
	"	\n"
	"	if( lIdx == 0 )\n"
	"	{\n"
	"	\n"
	"		\n"
	"		__global struct b3Contact4Data* cs = &gConstraints[m_start];	\n"
	"	\n"
	"		\n"
	"		i32 numValidConstraints = 0;\n"
	"		i32 batchIdx = 0;\n"
	"		while( numValidConstraints < numConstraints)\n"
	"		{\n"
	"			i32 nCurrentBatch = 0;\n"
	"			//	clear flag\n"
	"	\n"
	"			for(i32 i=0; i<CHECK_SIZE; i++) \n"
	"				ldsFixedBuffer[i] = 0;		\n"
	"			for(i32 i=numValidConstraints; i<numConstraints; i++)\n"
	"			{\n"
	"				i32 bodyAS = cs[i].m_bodyAPtrAndSignBit;\n"
	"				i32 bodyBS = cs[i].m_bodyBPtrAndSignBit;\n"
	"				i32 bodyA = abs(bodyAS);\n"
	"				i32 bodyB = abs(bodyBS);\n"
	"				bool aIsStatic = (bodyAS<0) || bodyAS==staticIdx;\n"
	"				bool bIsStatic = (bodyBS<0) || bodyBS==staticIdx;\n"
	"				i32 aUnavailable = aIsStatic ? 0 : readBuf( ldsFixedBuffer, bodyA);\n"
	"				i32 bUnavailable = bIsStatic ? 0 : readBuf( ldsFixedBuffer, bodyB);\n"
	"				\n"
	"				if( aUnavailable==0 && bUnavailable==0 ) // ok\n"
	"				{\n"
	"					if (!aIsStatic)\n"
	"					{\n"
	"						writeBuf( ldsFixedBuffer, bodyA );\n"
	"					}\n"
	"					if (!bIsStatic)\n"
	"					{\n"
	"						writeBuf( ldsFixedBuffer, bodyB );\n"
	"					}\n"
	"					cs[i].m_batchIdx = batchIdx;\n"
	"					if (i!=numValidConstraints)\n"
	"					{\n"
	"						tmp = cs[i];\n"
	"						cs[i] = cs[numValidConstraints];\n"
	"						cs[numValidConstraints]  = tmp;\n"
	"					}\n"
	"					numValidConstraints++;\n"
	"					\n"
	"					nCurrentBatch++;\n"
	"					if( nCurrentBatch == SIMD_WIDTH)\n"
	"					{\n"
	"						nCurrentBatch = 0;\n"
	"						for(i32 i=0; i<CHECK_SIZE; i++) \n"
	"							ldsFixedBuffer[i] = 0;\n"
	"						\n"
	"					}\n"
	"				}\n"
	"			}//for\n"
	"			batchIdx ++;\n"
	"		}//while\n"
	"		\n"
	"		batchSizes[wgIdx] = batchIdx;\n"
	"	}//if( lIdx == 0 )\n"
	"	\n"
	"	//return batchIdx;\n"
	"}\n";
