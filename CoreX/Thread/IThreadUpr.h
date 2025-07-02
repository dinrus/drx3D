// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class IThreadConfigUpr;

enum EJoinMode
{
	eJM_TryJoin,
	eJM_Join,
};

class IThread
{
public:
	// <interfuscator:shuffle>
	virtual ~IThread()
	{
	}

	//! Entry functions for code executed on thread.
	virtual void ThreadEntry() = 0;
	// </interfuscator:shuffle>
};

enum EFPE_Severity
{
	eFPE_None,  //!< No Floating Point Exceptions.
	eFPE_Basic, //!< Invalid operation, Div by 0.
	eFPE_All,   //!< Invalid operation, Div by 0, Denormalized operand, Overflow, Underflow, Inexact.
	eFPE_LastEntry
};

#define SCOPED_ENABLE_FLOAT_EXCEPTIONS(eFPESeverity) CScopedFloatingPointException scopedSetFloatExceptionMask(eFPESeverity)
#define SCOPED_DISABLE_FLOAT_EXCEPTIONS()            CScopedFloatingPointException scopedSetFloatExceptionMask(eFPE_None)

struct IThreadUpr
{
public:
	// <interfuscator:shuffle>
	virtual ~IThreadUpr()
	{
	}

	//! Get thread config manager.
	virtual IThreadConfigUpr* GetThreadConfigUpr() = 0;

	//! Spawn a new thread and apply thread config settings at thread beginning.
	virtual bool SpawnThread(IThread* pThread, tukk sThreadName, ...) = 0;

	//! Wait on another thread to exit (Blocking).
	//! Use eJM_TryJoin if you cannot be sure that the target thread is awake.
	//! \retval true if target thread has not been started yet or has already exited.
	//! \retval false if target thread is still running and therefore not in a state to exit.
	virtual bool JoinThread(IThread* pThreadTask, EJoinMode joinStatus) = 0;

	//! Register 3rd party thread with the thread manager.
	//! Applies thread config for thread if found.
	//! \param pThreadHandle If NULL, the current thread handle will be used.
	virtual bool RegisterThirdPartyThread(uk pThreadHandle, tukk sThreadName, ...) = 0;

	//! Unregister 3rd party thread with the thread manager.
	virtual bool UnRegisterThirdPartyThread(tukk sThreadName, ...) = 0;

	//! Get Thread Name.
	//! Returns "" if thread not found.
	virtual tukk GetThreadName(threadID nThreadId) = 0;

	//! Get ThreadID.
	virtual threadID GetThreadId(tukk sThreadName, ...) = 0;

	//! Execute function for each other thread but this one.
	typedef void (* ThreadModifFunction)(threadID nThreadId, uk pData);
	virtual void ForEachOtherThread(IThreadUpr::ThreadModifFunction fpThreadModiFunction, uk pFuncData = 0) = 0;

	virtual void EnableFloatExceptions(EFPE_Severity eFPESeverity, threadID nThreadId = 0) = 0;
	virtual void EnableFloatExceptionsForEachOtherThread(EFPE_Severity eFPESeverity) = 0;

	virtual uint GetFloatingPointExceptionMask() = 0;
	virtual void SetFloatingPointExceptionMask(uint nMask) = 0;
	// </interfuscator:shuffle>
};

class CScopedFloatingPointException
{
public:
	CScopedFloatingPointException(EFPE_Severity eFPESeverity)
	{
		oldMask = gEnv->pThreadUpr->GetFloatingPointExceptionMask();
		gEnv->pThreadUpr->EnableFloatExceptions(eFPESeverity);
	}
	~CScopedFloatingPointException()
	{
		gEnv->pThreadUpr->SetFloatingPointExceptionMask(oldMask);
	}
private:
	uint oldMask;
};
