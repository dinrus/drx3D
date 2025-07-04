//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static tukk prefixScanKernelsCL =
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
	"//Originally written by Takahiro Harada\n"
	"typedef u32 u32;\n"
	"#define GET_GROUP_IDX get_group_id(0)\n"
	"#define GET_LOCAL_IDX get_local_id(0)\n"
	"#define GET_GLOBAL_IDX get_global_id(0)\n"
	"#define GET_GROUP_SIZE get_local_size(0)\n"
	"#define GROUP_LDS_BARRIER barrier(CLK_LOCAL_MEM_FENCE)\n"
	"// takahiro end\n"
	"#define WG_SIZE 128 \n"
	"#define m_numElems x\n"
	"#define m_numBlocks y\n"
	"#define m_numScanBlocks z\n"
	"/*typedef struct\n"
	"{\n"
	"	uint m_numElems;\n"
	"	uint m_numBlocks;\n"
	"	uint m_numScanBlocks;\n"
	"	uint m_padding[1];\n"
	"} ConstBuffer;\n"
	"*/\n"
	"u32 ScanExclusive(__local u32* data, u32 n, i32 lIdx, i32 lSize)\n"
	"{\n"
	"	u32 blocksum;\n"
	"    i32 offset = 1;\n"
	"    for(i32 nActive=n>>1; nActive>0; nActive>>=1, offset<<=1)\n"
	"    {\n"
	"        GROUP_LDS_BARRIER;\n"
	"        for(i32 iIdx=lIdx; iIdx<nActive; iIdx+=lSize)\n"
	"        {\n"
	"            i32 ai = offset*(2*iIdx+1)-1;\n"
	"            i32 bi = offset*(2*iIdx+2)-1;\n"
	"            data[bi] += data[ai];\n"
	"        }\n"
	"	}\n"
	"    GROUP_LDS_BARRIER;\n"
	"    if( lIdx == 0 )\n"
	"	{\n"
	"		blocksum = data[ n-1 ];\n"
	"        data[ n-1 ] = 0;\n"
	"	}\n"
	"	GROUP_LDS_BARRIER;\n"
	"	offset >>= 1;\n"
	"    for(i32 nActive=1; nActive<n; nActive<<=1, offset>>=1 )\n"
	"    {\n"
	"        GROUP_LDS_BARRIER;\n"
	"        for( i32 iIdx = lIdx; iIdx<nActive; iIdx += lSize )\n"
	"        {\n"
	"            i32 ai = offset*(2*iIdx+1)-1;\n"
	"            i32 bi = offset*(2*iIdx+2)-1;\n"
	"            u32 temp = data[ai];\n"
	"            data[ai] = data[bi];\n"
	"            data[bi] += temp;\n"
	"        }\n"
	"	}\n"
	"	GROUP_LDS_BARRIER;\n"
	"	return blocksum;\n"
	"}\n"
	"__attribute__((reqd_work_group_size(WG_SIZE,1,1)))\n"
	"__kernel\n"
	"void LocalScanKernel(__global u32* dst, __global u32 *src, __global u32 *sumBuffer,\n"
	"		uint4 cb)\n"
	"{\n"
	"	__local u32 ldsData[WG_SIZE*2];\n"
	"	i32 gIdx = GET_GLOBAL_IDX;\n"
	"	i32 lIdx = GET_LOCAL_IDX;\n"
	"	ldsData[2*lIdx]     = ( 2*gIdx < cb.m_numElems )? src[2*gIdx]: 0;\n"
	"	ldsData[2*lIdx + 1] = ( 2*gIdx+1 < cb.m_numElems )? src[2*gIdx + 1]: 0;\n"
	"	u32 sum = ScanExclusive(ldsData, WG_SIZE*2, GET_LOCAL_IDX, GET_GROUP_SIZE);\n"
	"	if( lIdx == 0 ) sumBuffer[GET_GROUP_IDX] = sum;\n"
	"	if( (2*gIdx) < cb.m_numElems )\n"
	"    {\n"
	"        dst[2*gIdx]     = ldsData[2*lIdx];\n"
	"	}\n"
	"	if( (2*gIdx + 1) < cb.m_numElems )\n"
	"	{\n"
	"        dst[2*gIdx + 1] = ldsData[2*lIdx + 1];\n"
	"    }\n"
	"}\n"
	"__attribute__((reqd_work_group_size(WG_SIZE,1,1)))\n"
	"__kernel\n"
	"void AddOffsetKernel(__global u32 *dst, __global u32 *blockSum, uint4 cb)\n"
	"{\n"
	"	u32k blockSize = WG_SIZE*2;\n"
	"	i32 myIdx = GET_GROUP_IDX+1;\n"
	"	i32 lIdx = GET_LOCAL_IDX;\n"
	"	u32 iBlockSum = blockSum[myIdx];\n"
	"	i32 endValue = min((myIdx+1)*(blockSize), cb.m_numElems);\n"
	"	for(i32 i=myIdx*blockSize+lIdx; i<endValue; i+=GET_GROUP_SIZE)\n"
	"	{\n"
	"		dst[i] += iBlockSum;\n"
	"	}\n"
	"}\n"
	"__attribute__((reqd_work_group_size(WG_SIZE,1,1)))\n"
	"__kernel\n"
	"void TopLevelScanKernel(__global u32* dst, uint4 cb)\n"
	"{\n"
	"	__local u32 ldsData[2048];\n"
	"	i32 gIdx = GET_GLOBAL_IDX;\n"
	"	i32 lIdx = GET_LOCAL_IDX;\n"
	"	i32 lSize = GET_GROUP_SIZE;\n"
	"	for(i32 i=lIdx; i<cb.m_numScanBlocks; i+=lSize )\n"
	"	{\n"
	"		ldsData[i] = (i<cb.m_numBlocks)? dst[i]:0;\n"
	"	}\n"
	"	GROUP_LDS_BARRIER;\n"
	"	u32 sum = ScanExclusive(ldsData, cb.m_numScanBlocks, GET_LOCAL_IDX, GET_GROUP_SIZE);\n"
	"	for(i32 i=lIdx; i<cb.m_numBlocks; i+=lSize )\n"
	"	{\n"
	"		dst[i] = ldsData[i];\n"
	"	}\n"
	"	if( gIdx == 0 )\n"
	"	{\n"
	"		dst[cb.m_numBlocks] = sum;\n"
	"	}\n"
	"}\n";
