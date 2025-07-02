// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)
#define AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Physics
#define PHYSICS_EXPORTS
#include <drx3D/CoreX/Platform/platform.h>

//#define MEMSTREAM_DEBUG 1
#define MEMSTREAM_DEBUG_TAG (0xcafebabe)

#if defined(MEMSTREAM_DEBUG)
#define MEMSTREAM_DEBUG_ASSERT(x) if (!(x)) { __debugbreak(); }
#else
#define MEMSTREAM_DEBUG_ASSERT(x)
#endif

// Entity profiling only possible in non-release builds
#if !defined(_RELEASE)
# define ENTITY_PROFILER_ENABLED
# define PHYSWORLD_SERIALIZATION
#endif

#if DRX_COMPILER_MSVC
#pragma warning (disable : 4554 4305 4244 4996)
#pragma warning (disable : 6326) //Potential comparison of a constant with another constant
#elif DRX_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // MSVC equivalent C4996
#elif DRX_COMPILER_GCC
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // MSVC equivalent C4996
#endif

// C6326: potential comparison of a constant with another constant
#define CONSTANT_COMPARISON_OK PREFAST_SUPPRESS_WARNING(6326)
// C6384: dividing sizeof a pointer by another value
#define SIZEOF_ARRAY_OK				 PREFAST_SUPPRESS_WARNING(6384)
// C6246: Local declaration of <variable> hides declaration of same name in outer scope.
#define LOCAL_NAME_OVERRIDE_OK PREFAST_SUPPRESS_WARNING(6246)
// C6201: buffer overrun for <variable>, which is possibly stack allocated: index <name> is out of valid index range <min> to <max>
#if defined(__clang__)
#define INDEX_NOT_OUT_OF_RANGE _Pragma("clang diagnostic ignored \"-Warray-bounds\"")
#else
#define INDEX_NOT_OUT_OF_RANGE PREFAST_SUPPRESS_WARNING(6201)
#endif
// C6385: invalid data: accessing <buffer name>, the readable size is <size1> bytes, but <size2> bytes may be read
#define NO_BUFFER_OVERRUN			 PREFAST_SUPPRESS_WARNING(6385 6386)
// C6255: _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
#define STACK_ALLOC_OK				 PREFAST_SUPPRESS_WARNING(6255)


#include <vector>
#include <map>
#include <algorithm>
#include <float.h>

#include <drx3D/CoreX/Thread/DrxThread.h>

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_XOptimise.h>

#define NO_DRX_STREAM
class CStream;

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
#undef min
#undef max
#endif

#define ENTGRID_2LEVEL
//#define MULTI_GRID

// uncomment the following block to effectively disable validations
/*#define VALIDATOR_LOG(pLog,str)
#define VALIDATORS_START
#define VALIDATOR(member)
#define VALIDATOR_NORM(member)
#define VALIDATOR_RANGE(member,minval,maxval)
#define VALIDATOR_RANGE2(member,minval,maxval)
#define VALIDATORS_END
#define ENTITY_VALIDATE(strSource,pStructure)*/
#if (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT) || (DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT)
#define DoBreak {__debugbreak();}
#else
#define DoBreak { __debugbreak(); }
#endif

ILINE bool is_valid(float op) { return op*op>=0 && op*op<1E30f; }
ILINE bool is_valid(i32 op) { return true; }
ILINE bool is_valid(u32 op) { return true; }
ILINE bool is_valid(const Quat& op) { return is_valid(op|op); }
template<class dtype> bool is_valid(const dtype &op) { return is_valid(op.x*op.x + op.y*op.y + op.z*op.z); }

#define VALIDATOR_LOG(pLog,str) if (pLog) pLog->Log("%s", str) //OutputDebugString(str)
#define VALIDATORS_START bool validate( tukk strSource, ILog *pLog, const Vec3 &pt,\
	IPhysRenderer *pStreamer, uk param0, i32 param1, i32 param2 ) { bool res=true; char errmsg[1024];
#define VALIDATOR(member) if (!is_unused(member) && !is_valid(member)) { \
	res=false; drx_sprintf(errmsg,"%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid",strSource,\
	pStreamer?pStreamer->GetForeignName(param0,param1,param2):"",pt.x,pt.y,pt.z,#member); \
	VALIDATOR_LOG(pLog,errmsg); } 
#define VALIDATOR_NORM(member) if (!is_unused(member) && !(is_valid(member) && fabs_tpl((member|member)-1.0f)<0.01f)) { \
	res=false; drx_sprintf(errmsg,"%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or unnormalized",\
	strSource,pStreamer?pStreamer->GetForeignName(param0,param1,param2):"",pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
#define VALIDATOR_NORM_MSG(member,msg,member1) if (!is_unused(member) && !(is_valid(member) && fabs_tpl((member|member)-1.0f)<0.01f)) { \
	PREFAST_SUPPRESS_WARNING(6053) \
	res=false; drx_sprintf(errmsg,"%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or unnormalized %s",\
	strSource,pStreamer?pStreamer->GetForeignName(param0,param1,param2):"",pt.x,pt.y,pt.z,#member,msg); \
	PREFAST_SUPPRESS_WARNING(6053) \
	if (!is_unused(member1)) { drx_sprintf(errmsg+strlen(errmsg),sizeof errmsg - strlen(errmsg)," "#member1": %.1f,%.1f,%.1f",member1.x,member1.y,member1.z);} \
	VALIDATOR_LOG(pLog,errmsg); }
#define VALIDATOR_RANGE(member,minval,maxval) if (!is_unused(member) && !(is_valid(member) && member>=minval && member<=maxval)) { \
	res=false; drx_sprintf(errmsg,"%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or out of range",\
	strSource,pStreamer?pStreamer->GetForeignName(param0,param1,param2):"",pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
#define VALIDATOR_RANGE2(member,minval,maxval) if (!is_unused(member) && !(is_valid(member) && member*member>=minval*minval && \
		member*member<=maxval*maxval)) { \
	res=false; drx_sprintf(errmsg,"%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or out of range",\
	strSource,pStreamer?pStreamer->GetForeignName(param0,param1,param2):"",pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
#define VALIDATORS_END return res; }

#define ENTITY_VALIDATE(strSource,pStructure) if (!pStructure->validate(strSource,m_pWorld->m_pLog,m_pos,\
	m_pWorld->m_pRenderer,m_pForeignData,m_iForeignData,m_iForeignFlags)) { \
	if (m_pWorld->m_vars.bBreakOnValidation) DoBreak return 0; }
#define ENTITY_VALIDATE_ERRCODE(strSource,pStructure,iErrCode) if (!pStructure->validate(strSource,m_pWorld->m_pLog,m_pos, \
	m_pWorld->m_pRenderer,m_pForeignData,m_iForeignData,m_iForeignFlags)) { \
	if (m_pWorld->m_vars.bBreakOnValidation) DoBreak return iErrCode; }

// TODO: reference additional headers your program requires here
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Phys/primitives.h>
#include <drx3D/Phys/physinterface.h>
#ifndef NO_DRX_STREAM
#include "Stream.h"
#else
class CStream : public CMemStream {
public:
	using CMemStream::CMemStream;
	void WriteBits(u8 *pBits, u32 nSize) { Write(pBits,nSize + 7 >> 3); }
	void ReadBits(u8 *pBits, u32 nSize) { ReadRaw(pBits,nSize + 7 >> 3); }
	void WriteNumberInBits(i32 n,size_t nSize) { Write(n); }
	template<class T> void ReadNumberInBits(T &n,size_t nSize) { Read(n); }
	void Seek(size_t dwPos = 0) { m_iPos = dwPos; }
	size_t GetReadPos() { return m_iPos; }
	size_t GetSize() const { return m_nSize; }
};
#endif
#include "utils.h"

#if MAX_PHYS_THREADS<=1
extern threadID g_physThreadId;
inline i32 IsPhysThread() { return iszero((i32)(DrxGetCurrentThreadId()-g_physThreadId)); }
inline void MarkAsPhysThread() { g_physThreadId = DrxGetCurrentThreadId(); }
inline void MarkAsPhysWorkerThread(i32*) {}
inline i32 get_iCaller() { return IsPhysThread()^1; }
inline i32 get_iCaller_int() { return 0; }
#else // MAX_PHYS_THREADS>1
TLS_DECLARE(i32*,g_pidxPhysThread)
inline i32 IsPhysThread() {
	i32 dummy = 0;
	INT_PTR ptr = (INT_PTR)TLS_GET(INT_PTR, g_pidxPhysThread);
	ptr += (INT_PTR)&dummy-ptr & (ptr-1>>sizeof(INT_PTR)*8-1 ^ ptr>>sizeof(INT_PTR)*8-1);
	return *(i32*)ptr;
}
void MarkAsPhysThread();
void MarkAsPhysWorkerThread(i32*);
inline i32 get_iCaller() {
	i32 dummy = MAX_PHYS_THREADS;
	INT_PTR ptr = (INT_PTR)TLS_GET(INT_PTR,g_pidxPhysThread);
	ptr += (INT_PTR)&dummy-ptr & (ptr-1>>sizeof(INT_PTR)*8-1 ^ ptr>>sizeof(INT_PTR)*8-1);
	return *(i32*)ptr;
}
#define get_iCaller_int get_iCaller
#endif


#ifndef MAIN_THREAD_NONWORKER
#define FIRST_WORKER_THREAD 1
#else
#define FIRST_WORKER_THREAD 0
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)
