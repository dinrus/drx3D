// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   RenderThread.h : Render thread commands processing.

   Revision история:
* Created by Honitch Andrey

   =============================================================================*/

#ifndef __RENDERTHREAD_H__
#define __RENDERTHREAD_H__

#include <drx3D/CoreX/AlignmentTools.h>

#define RENDER_THREAD_NAME         "RenderThread"
#define RENDER_LOADING_THREAD_NAME "RenderLoadingThread"

#include <drx3D/CoreX/Thread/IThreadUpr.h>

//====================================================================

struct IFlashPlayer;
struct IFlashPlayer_RenderProxy;
struct IRenderAuxGeomImpl;
struct SAuxGeomCBRawDataPackagedConst;
struct IColorGradingControllerInt;
struct SColorChartLayer;
class CRenderMesh;
struct IDynTextureSourceImpl;
struct SDynTexture;
struct STexStreamInState;
class CDeviceResourceSet;

#if DRX_PLATFORM_ORBIS || DRX_PLATFORM_DURANGO || DRX_PLATFORM_MOBILE
	#define USE_LOCKS_FOR_FLUSH_SYNC
#endif

enum ERenderCommand
{
	eRC_Unknown = 0,

	eRC_CreateDevice,
	eRC_ResetDevice,

#if DRX_PLATFORM_DURANGO
	eRC_SuspendDevice,
	eRC_ResumeDevice,
#endif

	eRC_BeginFrame,
	eRC_EndFrame,

	eRC_FlashRender,
	eRC_FlashRenderLockless,

	eRC_LambdaCall,
};

enum class ERenderCommandFlags : u32
{
	None = 0,                                  // RenderThread and RenderLoadingThread: execute command directly
	                                           // LevelLoadingThread and Main thread: push command to renderthread
	FlushAndWait                     = BIT(1),
	SkipDuringLoading                = BIT(2),

	RenderLoadingThread_defer        = BIT(3), // defer command to post loading when command is issued from RenderLoadingThread
	LevelLoadingThread_executeDirect = BIT(4), // execute command directly when command is issued from LevelLoadingThread
	LevelLoadingThread_defer         = BIT(5), // defer command to post loading when command is issued from LevelLoadingThread
	MainThread_defer                 = BIT(6), // defer command to post loading when command is issued from MainThread
};
DEFINE_ENUM_FLAG_OPERATORS(ERenderCommandFlags)

//====================================================================

class CRenderThread : public IThread
{
public:
	DrxEvent m_started;

protected:
	// Start accepting work on thread
	virtual void ThreadEntry();
};

class CRenderThreadLoading : public CRenderThread
{
protected:
	// Start accepting work on thread
	virtual void ThreadEntry();
};

struct DRX_ALIGN(128) SRenderThread
{
	struct SRenderThreadLambdaCallback
	{
		std::function<void()> callback;
		ERenderCommandFlags   flags;
	};

	CRenderThread* m_pThread;
	CRenderThreadLoading* m_pThreadLoading;
	ILoadtimeCallback* m_pLoadtimeCallback;
	DrxMutex m_rdldLock;
	bool m_bQuit;
	bool m_bQuitLoading;
	bool m_bSuccessful;
	SDisplayContextKey m_displayContextKey;
	bool m_bBeginFrameCalled;
	bool m_bEndFrameCalled;
#ifndef STRIP_RENDER_THREAD
	i32 m_nCurThreadProcess;
	i32 m_nCurThreadFill;
#endif
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
	i32 m_nFlush;
	DrxMutex m_LockFlushNotify;
	DrxConditionVariable m_FlushCondition;
	DrxConditionVariable m_FlushFinishedCondition;
#else
	 i32 m_nFlush;
#endif
	threadID m_nRenderThread;
	threadID m_nRenderThreadLoading;
	threadID m_nLevelLoadingThread;
	threadID m_nMainThread;
#if DRX_PLATFORM_DURANGO
	 u32 m_suspendWhileLoadingFlag;
	DrxEvent m_suspendWhileLoadingEvent;
#endif
	HRESULT m_hResult;
#if DRX_RENDERER_OPENGL && !DXGL_FULL_EMULATION
	#if !OGL_SINGLE_CONTEXT
	SDXGLContextThreadLocalHandle m_kDXGLContextHandle;
	#endif
	SDXGLDeviceContextThreadLocalHandle m_kDXGLDeviceContextHandle;
#endif //DRX_RENDERER_OPENGL && !DXGL_FULL_EMULATION
	float m_fTimeIdleDuringLoading;
	float m_fTimeBusyDuringLoading;
	TArray<byte> m_Commands[RT_COMMAND_BUF_COUNT]; // m_nCurThreadFill shows which commands are filled by main thread

	// The below loading queue contains all commands that were submitted and require full device access during loading.
	// Will be blit into the first render frame's command queue after loading and subsequently resized to 0.
	TArray<byte> m_CommandsLoading;
	DrxCriticalSectionNonRecursive m_CommandsLoadingLock;

	enum EVideoThreadMode
	{
		eVTM_Disabled = 0,
		eVTM_RequestStart,
		eVTM_Active,
		eVTM_RequestStop,
		eVTM_ProcessingStop,
	};
	EVideoThreadMode m_eVideoThreadMode;

	struct PoolSyncCriticalSection
	{
		void Lock() { m_cs.Lock(); }
		void Unlock() { m_cs.Unlock(); }
		DrxCriticalSectionNonRecursive m_cs;
	};
	// Pool for lambda callbacks.
	stl::TPoolAllocator<SRenderThreadLambdaCallback, PoolSyncCriticalSection> m_lambdaCallbacksPool;


	SRenderThread();
	~SRenderThread();

	static i32 GetLocalThreadCommandBufferId();

	inline void SignalFlushFinishedCond()
	{
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_LockFlushNotify.Lock();
#endif
		m_nFlush = 0;
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_LockFlushNotify.Unlock();
		m_FlushFinishedCondition.Notify();
#else
		READ_WRITE_BARRIER
#endif
	}

	inline void SignalFlushCond()
	{
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_LockFlushNotify.Lock();
#endif
		m_nFlush = 1;
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_LockFlushNotify.Unlock();
		m_FlushCondition.Notify();
#else
		READ_WRITE_BARRIER
#endif
	}

	inline void SignalQuitCond()
	{
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_LockFlushNotify.Lock();
#endif
		m_bQuit = 1;
#ifdef USE_LOCKS_FOR_FLUSH_SYNC
		m_FlushCondition.Notify();
		m_LockFlushNotify.Unlock();
#else
		READ_WRITE_BARRIER
#endif
	}

	void WaitFlushCond();

#if DRX_PLATFORM_WINDOWS
	HWND GetRenderWindowHandle();
#endif

	void WaitFlushFinishedCond();

	inline void InitFlushCond()
	{
		m_nFlush = 0;
#if !defined(USE_LOCKS_FOR_FLUSH_SYNC)
		READ_WRITE_BARRIER
#endif
	}

	inline bool CheckFlushCond()
	{
#if !defined(USE_LOCKS_FOR_FLUSH_SYNC)
		READ_WRITE_BARRIER
#endif
		return *(i32*)&m_nFlush != 0;
	}

	void StartRenderThread();

	void StartRenderLoadingThread();

	bool IsFailed();

	inline size_t Align4(size_t value)
	{
		return (value + 3) & ~((size_t)3);
	}

	//! Adds a callback to be executed on the RenderThread
	//! If Render thread doesn't exist it will be executed on the Main thread
	//! @see ERenderCommandFlags
	template<typename RenderThreadCallback>
	void ExecuteRenderThreadCommand(RenderThreadCallback&& callback, ERenderCommandFlags flags);


	inline byte* AddCommandTo(ERenderCommand eRC, size_t nParamBytes, TArray<byte>& queue)
	{
		u32 cmdSize = sizeof(u32) + nParamBytes;
#if !defined(_RELEASE)
		cmdSize += sizeof(u32);
#endif
		byte* ptr = queue.Grow(cmdSize);
		AddDWORD(ptr, eRC);
#if !defined(_RELEASE)
		// Processed flag
		AddDWORD(ptr, 0);
#endif
		return ptr;
	}

	inline void EndCommandTo(byte* ptr, TArray<byte>& queue)
	{
#ifndef _RELEASE
		if (ptr - queue.Data() != queue.Num())
			DrxFatalError("Bad render command size - check the parameters and round each up to 4-byte boundaries [expected queue size = %" PRISIZE_T ", actual size = %u]", (size_t)(ptr - queue.Data()), queue.Num());
#endif
	}

	inline byte* AddCommand(ERenderCommand eRC, size_t nParamBytes)
	{
#ifdef STRIP_RENDER_THREAD
		return NULL;
#else
		return AddCommandTo(eRC, nParamBytes, m_Commands[m_nCurThreadFill]);
#endif
	}

	inline void EndCommand(byte* ptr)
	{
#ifndef STRIP_RENDER_THREAD
		EndCommandTo(ptr, m_Commands[m_nCurThreadFill]);
#endif
	}

	inline void AddDWORD(byte*& ptr, u32 nVal)
	{
		*(u32*)ptr = nVal;
		ptr += sizeof(u32);
	}

	inline void AddQWORD(byte*& ptr, uint64 nVal)
	{
		*(uint64*)ptr = nVal;
		ptr += sizeof(uint64);
	}

	inline void AddFloat(byte*& ptr, const float fVal)
	{
		*(float*)ptr = fVal;
		ptr += sizeof(float);
	}

	inline void AddColor(byte*& ptr, const ColorF& cVal)
	{
		float* fData = (float*)ptr;
		fData[0] = cVal[0];
		fData[1] = cVal[1];
		fData[2] = cVal[2];
		fData[3] = cVal[3];
		ptr += sizeof(ColorF);
	}
	inline void AddPointer(byte*& ptr, ukk pVal)
	{
		StoreUnaligned((u32*)ptr, pVal); // u32 because command list maintains 4-byte alignment
		ptr += sizeof(uk );
	}

	template<class T> T ReadCommand(i32& nIndex)
	{
		T Res;
		LoadUnaligned((u32*)&m_Commands[m_nCurThreadProcess][nIndex], Res);
		nIndex += (sizeof(T) + 3) & ~((size_t)3);
		return Res;
	}
	template<class T> void ReadCommand(i32& nIndex, SUninitialized<T>& value)
	{
		LoadUnaligned((u32*)&m_Commands[m_nCurThreadProcess][nIndex], value);
		nIndex += (sizeof(T) + 3) & ~((size_t)3);
	}

	inline threadID GetCurrentThreadId(bool bAlwaysCheck = false) const
	{
#ifdef STRIP_RENDER_THREAD
		return m_nRenderThread;
#else
		if (!bAlwaysCheck && m_nRenderThread == m_nMainThread)
			return m_nRenderThread;
		return ::GetCurrentThreadId();
#endif
	}
	void SwitchMode(bool enableVideo);

	void Init();
	void QuitRenderThread();
	void QuitRenderLoadingThread();
	void SyncMainWithRender();
	void FlushAndWait();
	void ProcessCommands();
	void Process();         // Render thread
	void ProcessLoading();  // Loading thread
	i32 GetThreadList() const;
	bool IsRenderThread(bool bAlwaysCheck = false) const;
	bool IsRenderLoadingThread(bool bAlwaysCheck = false);
	bool IsLevelLoadingThread(bool bAlwaysCheck=false) const;
	bool IsMainThread(bool bAlwaysCheck = false) const;
	bool IsMultithreaded();
	i32 CurThreadFill() const;

	bool RC_CreateDevice();
	void RC_ResetDevice();
#if DRX_PLATFORM_DURANGO
	void RC_SuspendDevice();
	void RC_ResumeDevice();
#endif

	void RC_PrecacheResource(ITexture * pTP, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter = 1);

	void RC_FlashRender(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer);
	void RC_FlashRenderPlayer(std::shared_ptr<IFlashPlayer> &&pPlayer);
	void RC_FlashRenderPlaybackLockless(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool finalPlayback);

	void RC_BeginFrame(const SDisplayContextKey& displayContextKey);
	void RC_EndFrame(bool bWait);
	void RC_TryFlush();
	void RC_StartVideoThread();
	void RC_StopVideoThread();

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		for (u32 i = 0; i < RT_COMMAND_BUF_COUNT; ++i)
			pSizer->AddObject(m_Commands[i]);
	}
};

inline i32 SRenderThread::GetThreadList() const
{
#ifdef STRIP_RENDER_THREAD
	return m_nCurThreadFill;
#else
	if (IsRenderThread()) return m_nCurThreadProcess;

	return m_nCurThreadFill;
#endif
}

inline bool SRenderThread::IsRenderThread(bool bAlwaysCheck) const
{
#ifdef STRIP_RENDER_THREAD
	return true;
#else
	threadID d = this->GetCurrentThreadId(bAlwaysCheck);
	return (d == m_nRenderThreadLoading || d == m_nRenderThread);
#endif
}

inline bool SRenderThread::IsRenderLoadingThread(bool bAlwaysCheck)
{
#ifdef STRIP_RENDER_THREAD
	return false;
#else
	threadID d = this->GetCurrentThreadId(bAlwaysCheck);
	return (d == m_nRenderThreadLoading);
#endif
}

inline bool SRenderThread::IsLevelLoadingThread(bool bAlwaysCheck) const
{
#ifdef STRIP_RENDER_THREAD
	return false;
#else
	if (!m_nLevelLoadingThread)
		return false;
	threadID d = this->GetCurrentThreadId(bAlwaysCheck);
	return (d == m_nLevelLoadingThread);
#endif
}

inline bool SRenderThread::IsMainThread(bool bAlwaysCheck) const
{
#ifdef STRIP_RENDER_THREAD
	return false;
#else
	threadID d = this->GetCurrentThreadId(bAlwaysCheck);
	return (d == m_nMainThread);
#endif
}

inline bool SRenderThread::IsMultithreaded()
{
#ifdef STRIP_RENDER_THREAD
	return false;
#else
	return m_pThread != NULL;
#endif
}

inline i32 SRenderThread::CurThreadFill() const
{
#ifdef STRIP_RENDER_THREAD
	return 0;
#else
	return m_nCurThreadFill;
#endif
}

#ifdef STRIP_RENDER_THREAD
inline void SRenderThread::FlushAndWait()
{
	return;
}
#endif

//////////////////////////////////////////////////////////////////////////
template<typename RenderThreadCallback>
inline void SRenderThread::ExecuteRenderThreadCommand(RenderThreadCallback&& callback, ERenderCommandFlags flags)
{
#ifdef STRIP_RENDER_THREAD
	FUNCTION_PROFILER_RENDERER();
	callback();
#else

	// *INDENT-OFF*
	const bool deferPostLoading = (IsRenderLoadingThread()  && u32(flags & ERenderCommandFlags::RenderLoadingThread_defer) != 0) ||
	                              (IsLevelLoadingThread()   && u32(flags & ERenderCommandFlags::LevelLoadingThread_defer)  != 0) ||
	                              (IsMainThread()           && u32(flags & ERenderCommandFlags::MainThread_defer)          != 0);

	const bool executeDirect    = !deferPostLoading && 
	                              (IsRenderThread() || (IsLevelLoadingThread()  && u32(flags & ERenderCommandFlags::LevelLoadingThread_executeDirect) != 0));
	// *INDENT-ON*

	if (executeDirect)
	{
		FUNCTION_PROFILER_RENDERER();
		callback();
		return;
	}

	if (deferPostLoading)
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, m_CommandsLoadingLock);
		byte* p = AddCommandTo(eRC_LambdaCall, sizeof(uk ), m_CommandsLoading);
		uk pCallbackPtr = ::new(m_lambdaCallbacksPool.Allocate())SRenderThreadLambdaCallback{callback,flags};
		AddPointer(p, pCallbackPtr);
		EndCommandTo(p, m_CommandsLoading);

		DRX_ASSERT(u32(flags & ERenderCommandFlags::FlushAndWait) == 0); // FlushAndWait does nto work with deferred commands
	}
	else
	{
		DRX_ASSERT(!IsLevelLoadingThread());
//		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, m_CommandsLock);
		byte* p = AddCommandTo(eRC_LambdaCall, sizeof(uk ), m_Commands[m_nCurThreadFill]);
		uk pCallbackPtr = ::new(m_lambdaCallbacksPool.Allocate())SRenderThreadLambdaCallback{callback,flags};
		AddPointer(p, pCallbackPtr);
		EndCommandTo(p, m_Commands[m_nCurThreadFill]);
	}

	if (u32(flags & ERenderCommandFlags::FlushAndWait) != 0)
	{
		FlushAndWait();
	}
#endif
}


#endif  // __RENDERTHREAD_H__
