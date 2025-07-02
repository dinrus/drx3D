// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxMemReplay.h
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DrxMemReplay_h__
#define __DrxMemReplay_h__
#pragma once

namespace EMemReplayAllocClass
{
enum Class
{
	C_UserPointer = 0,
	C_D3DDefault,
	C_D3DManaged,
};
}

namespace EMemReplayUserPointerClass
{
enum Class
{
	C_Unknown = 0,
	C_CrtNew,
	C_CrtNewArray,
	C_DrxNew,
	C_DrxNewArray,
	C_CrtMalloc,
	C_DrxMalloc,
	C_STL,
};
}

#if CAPTURE_REPLAY_LOG
//! Memory replay interface, access it with DrxGetMemReplay call.
struct IMemReplay
{
	virtual ~IMemReplay(){}
	virtual void DumpStats() = 0;
	virtual void DumpSymbols() = 0;

	virtual void StartOnCommandLine(tukk cmdLine) = 0;
	virtual void Start(bool bPaused = false, tukk openString = NULL) = 0;
	virtual void Stop() = 0;
	virtual void Flush() = 0;

	virtual void GetInfo(DrxReplayInfo& infoOut) = 0;

	//! Call to begin a new allocation scope.
	virtual bool EnterScope(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId) = 0;

	//! Records an event against the currently active scope and exits it.
	virtual void ExitScope_Alloc(UINT_PTR id, UINT_PTR sz, UINT_PTR alignment = 0) = 0;
	virtual void ExitScope_Realloc(UINT_PTR originalId, UINT_PTR newId, UINT_PTR sz, UINT_PTR alignment = 0) = 0;
	virtual void ExitScope_Free(UINT_PTR id) = 0;
	virtual void ExitScope() = 0;

	virtual bool EnterLockScope() = 0;
	virtual void LeaveLockScope() = 0;

	virtual void AllocUsage(EMemReplayAllocClass::Class allocClass, UINT_PTR id, UINT_PTR used) = 0;

	virtual void AddAllocReference(uk ptr, uk ref) = 0;
	virtual void RemoveAllocReference(uk ref) = 0;

	virtual void AddLabel(tukk label) = 0;
	virtual void AddLabelFmt(tukk labelFmt, ...) = 0;
	virtual void AddFrameStart() = 0;
	virtual void AddScreenshot() = 0;

	virtual void AddContext(i32 type, u32 flags, tukk str) = 0;
	virtual void AddContextV(i32 type, u32 flags, tukk format, va_list args) = 0;
	virtual void RemoveContext() = 0;

	virtual void MapPage(uk base, size_t size) = 0;
	virtual void UnMapPage(uk base, size_t size) = 0;

	virtual void RegisterFixedAddressRange(uk base, size_t size, tukk name) = 0;
	virtual void MarkBucket(i32 bucket, size_t alignment, uk base, size_t length) = 0;
	virtual void UnMarkBucket(i32 bucket, uk base) = 0;
	virtual void BucketEnableCleanups(uk allocatorBase, bool enabled) = 0;

	virtual void MarkPool(i32 pool, size_t alignment, uk base, size_t length, tukk name) = 0;
	virtual void UnMarkPool(i32 pool, uk base) = 0;
	virtual void AddTexturePoolContext(uk ptr, i32 mip, i32 width, i32 height, tukk name, u32 flags) = 0;

	virtual void AddSizerTree(tukk name) = 0;

	virtual void RegisterContainer(ukk key, i32 type) = 0;
	virtual void UnregisterContainer(ukk key) = 0;
	virtual void BindToContainer(ukk key, ukk alloc) = 0;
	virtual void UnbindFromContainer(ukk key, ukk alloc) = 0;
	virtual void SwapContainers(ukk keyA, ukk keyB) = 0;

	virtual bool EnableAsynchMode() = 0;
};
#endif

#if CAPTURE_REPLAY_LOG
struct CDummyMemReplay : IMemReplay
#else //CAPTURE_REPLAY_LOG
struct IMemReplay
#endif
{
	void DumpStats()                                                {}
	void DumpSymbols()                                              {}

	void StartOnCommandLine(tukk cmdLine)                    {}
	void Start(bool bPaused = false, tukk openString = NULL) {}
	void Stop()                                                     {}
	void Flush()                                                    {}

	void GetInfo(DrxReplayInfo& infoOut)                            {}

	//! Call to begin a new allocation scope.
	bool EnterScope(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId) { return false; }

	//! Records an event against the currently active scope and exits it.
	void ExitScope_Alloc(UINT_PTR id, UINT_PTR sz, UINT_PTR alignment = 0)                                {}
	void ExitScope_Realloc(UINT_PTR originalId, UINT_PTR newId, UINT_PTR sz, UINT_PTR alignment = 0)      {}
	void ExitScope_Free(UINT_PTR id)                                                                      {}
	void ExitScope()                                                                                      {}

	bool EnterLockScope()                                                                                 { return false; }
	void LeaveLockScope()                                                                                 {}

	void AllocUsage(EMemReplayAllocClass::Class allocClass, UINT_PTR id, UINT_PTR used)                   {}

	void AddAllocReference(uk ptr, uk ref)                                                          {}
	void RemoveAllocReference(uk ref)                                                                  {}

	void AddLabel(tukk label)                                                                      {}
	void AddLabelFmt(tukk labelFmt, ...)                                                           {}
	void AddFrameStart()                                                                                  {}
	void AddScreenshot()                                                                                  {}

	void AddContext(i32 type, u32 flags, tukk str)                                              {}
	void AddContextV(i32 type, u32 flags, tukk format, va_list args)                            {}
	void RemoveContext()                                                                                  {}

	void MapPage(uk base, size_t size)                                                                 {}
	void UnMapPage(uk base, size_t size)                                                               {}

	void RegisterFixedAddressRange(uk base, size_t size, tukk name)                             {}
	void MarkBucket(i32 bucket, size_t alignment, uk base, size_t length)                              {}
	void UnMarkBucket(i32 bucket, uk base)                                                             {}
	void BucketEnableCleanups(uk allocatorBase, bool enabled)                                          {}

	void MarkPool(i32 pool, size_t alignment, uk base, size_t length, tukk name)                {}
	void UnMarkPool(i32 pool, uk base)                                                                 {}
	void AddTexturePoolContext(uk ptr, i32 mip, i32 width, i32 height, tukk name, u32 flags) {}

	void AddSizerTree(tukk name)                                                                   {}

	void RegisterContainer(ukk key, i32 type)                                                     {}
	void UnregisterContainer(ukk key)                                                             {}
	void BindToContainer(ukk key, ukk alloc)                                              {}
	void UnbindFromContainer(ukk key, ukk alloc)                                          {}
	void SwapContainers(ukk keyA, ukk keyB)                                               {}

	bool EnableAsynchMode()                                                                               { return false; }
};

#if CAPTURE_REPLAY_LOG
inline IMemReplay* DrxGetIMemReplay()
{
	static CDummyMemReplay s_dummyMemReplay;
	static IMemReplay* s_pMemReplay = 0;
	if (!s_pMemReplay)
	{
		// get it from System
		IMemoryUpr* pMemMan = DrxGetIMemoryUpr();
		if (pMemMan)
			s_pMemReplay = pMemMan->GetIMemReplay();
		if (!s_pMemReplay)
			return &s_dummyMemReplay;
	}
	return s_pMemReplay;
}
#else
inline IMemReplay* DrxGetIMemReplay()
{
	return NULL;
}
#endif

#if defined(__cplusplus) && CAPTURE_REPLAY_LOG
namespace EMemStatContextTypes
{
// Add new types at the end, do not modify existing values.
enum Type
{
	MSC_MAX             = 0,
	MSC_CGF             = 1,
	MSC_MTL             = 2,
	MSC_DBA             = 3,
	MSC_CHR             = 4,
	MSC_LMG             = 5,
	MSC_AG              = 6,
	MSC_Texture         = 7,
	MSC_ParticleLibrary = 8,

	MSC_Physics         = 9,
	MSC_Terrain         = 10,
	MSC_Shader          = 11,
	MSC_Other           = 12,
	MSC_RenderMesh      = 13,
	MSC_Entity          = 14,
	MSC_Navigation      = 15,
	MSC_ScriptCall      = 16,

	MSC_CDF             = 17,

	MSC_RenderMeshType  = 18,

	MSC_ANM             = 19,
	MSC_CGA             = 20,
	MSC_CAF             = 21,
	MSC_ArchetypeLib    = 22,

	MSC_SoundProject    = 23,
	MSC_EntityArchetype = 24,

	MSC_LUA             = 25,
	MSC_D3D             = 26,
	MSC_ParticleEffect  = 27,
	MSC_SoundBuffer     = 28,
	MSC_FSB             = 29, //!< Sound bank data

	MSC_AIObjects       = 30,
	MSC_Animation       = 31,
	MSC_Debug           = 32,

	MSC_FSQ             = 33,
	MSC_Mannequin       = 34,

	MSC_GeomCache       = 35
};
}

namespace EMemStatContextFlags
{
enum Flags
{
	MSF_None     = 0,
	MSF_Instance = 1,
};
}

namespace EMemStatContainerType
{
enum Type
{
	MSC_Vector,
	MSC_Tree,
};
}

class CMemStatContext
{
public:
	CMemStatContext(EMemStatContextTypes::Type type, u32 flags, tukk str)
	{
		DrxGetIMemReplay()->AddContext(type, flags, str);
	}
	~CMemStatContext()
	{
		DrxGetIMemReplay()->RemoveContext();
	}
private:
	CMemStatContext(const CMemStatContext&);
	CMemStatContext& operator=(const CMemStatContext&);
};

class CMemStatContextFormat
{
public:
	CMemStatContextFormat(EMemStatContextTypes::Type type, u32 flags, tukk format, ...)
	{
		va_list args;
		va_start(args, format);
		DrxGetIMemReplay()->AddContextV(type, flags, format, args);
		va_end(args);
	}
	~CMemStatContextFormat()
	{
		DrxGetIMemReplay()->RemoveContext();
	}
private:
	CMemStatContextFormat(const CMemStatContextFormat&);
	CMemStatContextFormat& operator=(const CMemStatContextFormat&);
};

class CCondMemStatContext
{
public:
	CCondMemStatContext(bool cond, EMemStatContextTypes::Type type, u32 flags, tukk str)
		: m_cond(cond)
	{
		if (cond)
			DrxGetIMemReplay()->AddContext(type, flags, str);
	}
	~CCondMemStatContext()
	{
		if (m_cond)
			DrxGetIMemReplay()->RemoveContext();
	}
private:
	CCondMemStatContext(const CCondMemStatContext&);
	CCondMemStatContext& operator=(const CCondMemStatContext&);
private:
	const bool m_cond;
};

class CCondMemStatContextFormat
{
public:
	CCondMemStatContextFormat(bool cond, EMemStatContextTypes::Type type, u32 flags, tukk format, ...)
		: m_cond(cond)
	{
		if (cond)
		{
			va_list args;
			va_start(args, format);
			DrxGetIMemReplay()->AddContextV(type, flags, format, args);
			va_end(args);
		}
	}
	~CCondMemStatContextFormat()
	{
		if (m_cond)
			DrxGetIMemReplay()->RemoveContext();
	}
private:
	CCondMemStatContextFormat(const CCondMemStatContextFormat&);
	CCondMemStatContextFormat& operator=(const CCondMemStatContextFormat&);
private:
	const bool m_cond;
};
#endif // CAPTURE_REPLAY_LOG

#if CAPTURE_REPLAY_LOG
	#define INCLUDE_MEMSTAT_CONTEXTS     1
	#define INCLUDE_MEMSTAT_ALLOC_USAGES 1
	#define INCLUDE_MEMSTAT_CONTAINERS   1
#else
	#define INCLUDE_MEMSTAT_CONTEXTS     0
	#define INCLUDE_MEMSTAT_ALLOC_USAGES 0
	#define INCLUDE_MEMSTAT_CONTAINERS   0
#endif

#if CAPTURE_REPLAY_LOG
	#define MEMSTAT_CONCAT_(a, b) a ## b
	#define MEMSTAT_CONCAT(a, b)  MEMSTAT_CONCAT_(a, b)
#endif

#if INCLUDE_MEMSTAT_CONTEXTS
	#define MEMSTAT_CONTEXT(type, id, str)                        CMemStatContext MEMSTAT_CONCAT(_memctx, __LINE__) (type, id, str);
	#define MEMSTAT_CONTEXT_FMT(type, id, format, ...)            CMemStatContextFormat MEMSTAT_CONCAT(_memctx, __LINE__) (type, id, format, __VA_ARGS__);
	#define MEMSTAT_COND_CONTEXT(cond, type, id, str)             CCondMemStatContext MEMSTAT_CONCAT(_memctx, __LINE__) (cond, type, id, str);
	#define MEMSTAT_COND_CONTEXT_FMT(cond, type, id, format, ...) CCondMemStatContextFormat MEMSTAT_CONCAT(_memctx, __LINE__) (cond, type, id, format, __VA_ARGS__);
#else
	#define MEMSTAT_CONTEXT(...)
	#define MEMSTAT_CONTEXT_FMT(...)
	#define MEMSTAT_COND_CONTEXT(...)
	#define MEMSTAT_COND_CONTEXT_FMT(...)
#endif

#if INCLUDE_MEMSTAT_CONTAINERS
template<typename T>
static void MemReplayRegisterContainerStub(ukk key, i32 type)
{
	DrxGetIMemReplay()->RegisterContainer(key, type);
}
	#define MEMSTAT_REGISTER_CONTAINER(key, type, T)         MemReplayRegisterContainerStub<T>(key, type)
	#define MEMSTAT_UNREGISTER_CONTAINER(key)                DrxGetIMemReplay()->UnregisterContainer(key)
	#define MEMSTAT_BIND_TO_CONTAINER(key, ptr)              DrxGetIMemReplay()->BindToContainer(key, ptr)
	#define MEMSTAT_UNBIND_FROM_CONTAINER(key, ptr)          DrxGetIMemReplay()->UnbindFromContainer(key, ptr)
	#define MEMSTAT_SWAP_CONTAINERS(keyA, keyB)              DrxGetIMemReplay()->SwapContainers(keyA, keyB)
	#define MEMSTAT_REBIND_TO_CONTAINER(key, oldPtr, newPtr) DrxGetIMemReplay()->UnbindFromContainer(key, oldPtr); DrxGetIMemReplay()->BindToContainer(key, newPtr)
#else
	#define MEMSTAT_REGISTER_CONTAINER(key, type, T)
	#define MEMSTAT_UNREGISTER_CONTAINER(key)
	#define MEMSTAT_BIND_TO_CONTAINER(key, ptr)
	#define MEMSTAT_UNBIND_FROM_CONTAINER(key, ptr)
	#define MEMSTAT_SWAP_CONTAINERS(keyA, keyB)
	#define MEMSTAT_REBIND_TO_CONTAINER(key, oldPtr, newPtr)
#endif

#if INCLUDE_MEMSTAT_ALLOC_USAGES
	#define MEMSTAT_USAGE(ptr, size) DrxGetIMemReplay()->AllocUsage(EMemReplayAllocClass::C_UserPointer, (UINT_PTR)ptr, size)
#else
	#define MEMSTAT_USAGE(ptr, size)
#endif

#if CAPTURE_REPLAY_LOG

class CMemStatScopedLabel
{
public:
	explicit CMemStatScopedLabel(tukk name)
		: m_name(name)
	{
		DrxGetIMemReplay()->AddLabelFmt("%s Begin", name);
	}

	~CMemStatScopedLabel()
	{
		DrxGetIMemReplay()->AddLabelFmt("%s End", m_name);
	}

private:
	CMemStatScopedLabel(const CMemStatScopedLabel&);
	CMemStatScopedLabel& operator=(const CMemStatScopedLabel&);

private:
	tukk m_name;
};

	#define MEMSTAT_LABEL(a)           DrxGetIMemReplay()->AddLabel(a)
	#define MEMSTAT_LABEL_FMT(a, ...)  DrxGetIMemReplay()->AddLabelFmt(a, __VA_ARGS__)
	#define MEMSTAT_LABEL_SCOPED(name) CMemStatScopedLabel MEMSTAT_CONCAT(_memctxlabel, __LINE__) (name);

#else

	#define MEMSTAT_LABEL(a)
	#define MEMSTAT_LABEL_FMT(a, ...)
	#define MEMSTAT_LABEL_SCOPED(...)

#endif

#if CAPTURE_REPLAY_LOG

class CMemReplayScope
{
public:
	CMemReplayScope(EMemReplayAllocClass::Class cls, u16 subCls, i32 moduleId)
		: m_needsExit(DrxGetIMemReplay()->EnterScope(cls, subCls, moduleId))
	{
	}

	~CMemReplayScope()
	{
		if (m_needsExit)
			DrxGetIMemReplay()->ExitScope();
	}

	void Alloc(UINT_PTR id, size_t sz, size_t alignment)
	{
		if (m_needsExit)
		{
			m_needsExit = false;
			DrxGetIMemReplay()->ExitScope_Alloc(id, sz, alignment);
		}
	}

	void Realloc(UINT_PTR origId, UINT_PTR newId, size_t newSz, size_t newAlign)
	{
		if (m_needsExit)
		{
			m_needsExit = false;
			DrxGetIMemReplay()->ExitScope_Realloc(origId, newId, newSz, newAlign);
		}
	}

	void Free(UINT_PTR id)
	{
		if (m_needsExit)
		{
			m_needsExit = false;
			DrxGetIMemReplay()->ExitScope_Free(id);
		}
	}
private:
	CMemReplayScope(const CMemReplayScope&);
	CMemReplayScope& operator=(const CMemReplayScope&);

private:
	bool m_needsExit;
};

class CMemReplayLockScope
{
public:
	CMemReplayLockScope()
	{
		m_bNeedsExit = DrxGetIMemReplay()->EnterLockScope();
	}

	~CMemReplayLockScope()
	{
		if (m_bNeedsExit)
			DrxGetIMemReplay()->LeaveLockScope();
	}

private:
	CMemReplayLockScope(const CMemReplayLockScope&);
	CMemReplayLockScope& operator=(const CMemReplayLockScope&);

private:
	bool m_bNeedsExit;
};

	#ifdef eDrxModule
		#define MEMREPLAY_SCOPE(cls, subCls)               CMemReplayScope _mrCls((cls), (subCls), eDrxModule)
	#else
		#define MEMREPLAY_SCOPE(cls, subCls)               CMemReplayScope _mrCls((cls), (subCls), eDrxM_Launcher)
	#endif
	#define MEMREPLAY_SCOPE_ALLOC(id, sz, align)         _mrCls.Alloc((UINT_PTR)(id), (sz), (align))
	#define MEMREPLAY_SCOPE_REALLOC(oid, nid, sz, align) _mrCls.Realloc((UINT_PTR)(oid), (UINT_PTR)nid, (sz), (align))
	#define MEMREPLAY_SCOPE_FREE(id)                     _mrCls.Free((UINT_PTR)(id))

	#define MEMREPLAY_LOCK_SCOPE()                       CMemReplayLockScope _mrCls

#else

	#define MEMREPLAY_SCOPE(cls, subCls)
	#define MEMREPLAY_SCOPE_ALLOC(id, sz, align)
	#define MEMREPLAY_SCOPE_REALLOC(oid, nid, sz, align)
	#define MEMREPLAY_SCOPE_FREE(id)

	#define MEMREPLAY_LOCK_SCOPE()

#endif

#endif  //__DrxMemReplay_h__
