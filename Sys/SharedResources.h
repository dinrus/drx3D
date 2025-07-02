// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SHARED_RESOURCES_H_
#define _SHARED_RESOURCES_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

#pragma warning(push)
#pragma warning(disable : 6326)   // Potential comparison of a constant with another constant
#pragma warning(disable : 6011)   // Dereferencing NULL pointer
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <GFxLoader.h>            // includes <windows.h>
#include <drx3D/CoreX/Renderer/IScaleform.h>
#pragma warning(pop)
#include <drx3D/Sys/ConfigScaleform.h>
#include <drx3D/Sys/GAllocatorDrxMem.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

class GFxLoader2;
class IDrxSizer;
class GSystemInitWrapper;
class MeshCacheResetThread;
class GImeHelper;

class CSharedFlashPlayerResources
{
public:
	static CSharedFlashPlayerResources& GetAccess();

	static void Init();
	static void Shutdown();

public:
	GFxLoader2*       GetLoader(bool getRawInterface = false);
	IScaleformRecording* GetRenderer(bool getRawInterface = false);

	DrxGFxMemInterface::Stats GetSysAllocStats() const;
	void                      GetMemoryUsage(IDrxSizer* pSizer) const;

	i32  CreateMemoryArena(u32 arenaID, bool resetCache) const;
	void DestoryMemoryArena(u32 arenaID) const;
	bool AreCustomMemoryArenasActive() const;

	void ResetMeshCache() const;
	bool IsFlashVideoIOStarving() const;

	float GetFlashHeapFragmentation() const;
	void  SetImeFocus(GFxMovieView* pMovie, bool bSet);

private:
	CSharedFlashPlayerResources();
	~CSharedFlashPlayerResources();

private:
	static CSharedFlashPlayerResources* ms_pSharedFlashPlayerResources;

private:
	GSystemInitWrapper* m_pGSystemInit;
	GFxLoader2* m_pLoader;
	IScaleformRecording* m_pRecorder;
	MeshCacheResetThread* m_pMeshCacheResetThread;
#if defined(USE_GFX_IME)
	GImeHelper* m_pImeHelper;
#endif
};

class GFxLoader2:public GFxLoader
{
	friend class CSharedFlashPlayerResources;

public:
	// cannot configure GFC's RefCountBase via public inheritance so implement own ref counting
	void AddRef();
	void Release();
	i32  GetRefCount() const { return m_refCount; }

	void UpdateParserVerbosity();

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(this, sizeof(*this));  // just add object, internals are counted implicitly by provided GFxDrxMemInterface / GSysAlloc
	}

private:
	GFxLoader2();
	virtual ~GFxLoader2();
	void SetupDynamicFontCache();

private:
	 i32 m_refCount;
	GFxParseControl m_parserVerbosity;
};

class MeshCacheResetThread:public IThread
{
public:
	// IThread
	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

public:
	MeshCacheResetThread();
	~MeshCacheResetThread();

	void IssueReset();

private:
	static tukk ms_pThreadName;

private:
	 bool m_cancelRequestSent;
	DrxEvent m_awakeThread;
};

#endif // #ifdef INCLUDE_SCALEFORM_SDK

#endif // #ifndef _SHARED_RESOURCES_H_
