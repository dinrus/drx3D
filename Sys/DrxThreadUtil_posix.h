// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////
// NOTE: INTERNAL HEADER NOT FOR PUBLIC USE
// This header should only be include by SystemThreading.cpp only
// It provides an interface for PThread intrinsics
// It's only client should be CThreadUpr which should manage all thread interaction
#if !defined(INCLUDED_FROM_SYSTEM_THREADING_CPP)
	#error "DRXTEK INTERNAL HEADER. ONLY INCLUDE FROM SYSTEMTHRADING.CPP."
#endif
//////////////////////////////////////////////////////////////////////////

#define DEFAULT_THREAD_STACK_SIZE_KB 0
#define DRX_PTHREAD_THREAD_NAME_MAX  16

//////////////////////////////////////////////////////////////////////////
// THREAD CREATION AND MANAGMENT
//////////////////////////////////////////////////////////////////////////
namespace DrxThreadUtil
{
// Define type for platform specific thread handle
typedef pthread_t TThreadHandle;

struct SThreadCreationDesc
{
	// Define platform specific thread entry function functor type
	typedef uk (* EntryFunc)(uk );

	tukk szThreadName;
	EntryFunc   fpEntryFunc;
	uk       pArgList;
	u32      nStackSizeInBytes;
};

//////////////////////////////////////////////////////////////////////////
TThreadHandle DrxGetCurrentThreadHandle()
{
	return (TThreadHandle)pthread_self();
}

//////////////////////////////////////////////////////////////////////////
// Note: Handle must be closed lated via DrxCloseThreadHandle()
TThreadHandle DrxDuplicateThreadHandle(const TThreadHandle& hThreadHandle)
{
	// Do not do anything
	// If you add a new platform which duplicates handles make sure to mirror the change in DrxCloseThreadHandle(..)
	return hThreadHandle;
}

//////////////////////////////////////////////////////////////////////////
void DrxCloseThreadHandle(TThreadHandle& hThreadHandle)
{
	pthread_detach(hThreadHandle);
}

//////////////////////////////////////////////////////////////////////////
threadID DrxGetCurrentThreadId()
{
	return threadID(pthread_self());
}

//////////////////////////////////////////////////////////////////////////
threadID DrxGetThreadId(TThreadHandle hThreadHandle)
{
	return threadID(hThreadHandle);
}

//////////////////////////////////////////////////////////////////////////
// Note: On OSX the thread name can only be set by the thread itself.
void DrxSetThreadName(TThreadHandle pThreadHandle, tukk sThreadName)
{
	char threadName[DRX_PTHREAD_THREAD_NAME_MAX];
	if (!drx_strcpy(threadName, sThreadName))
	{
		DrxLog("<ThreadInfo> DrxSetThreadName: input thread name '%s' truncated to '%s'", sThreadName, threadName);
	}
#if DRX_PLATFORM_APPLE
	// On OSX the thread name can only be set by the thread itself.
	assert(pthread_equal(pthread_self(), (pthread_t)pThreadHandle));

	if (pthread_setname_np(threadName) != 0)
#else
	if (pthread_setname_np(pThreadHandle, threadName) != 0)
#endif
	{
		switch (errno)
		{
		case ERANGE:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadName: Unable to rename thread \"%s\". Error Msg: \"Name to long. Exceeds %d bytes.\"", sThreadName, DRX_PTHREAD_THREAD_NAME_MAX);
			break;
		default:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadName: Unsupported error code: %i", errno);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void DrxSetThreadAffinityMask(TThreadHandle pThreadHandle, DWORD dwAffinityMask)
{
#if DRX_PLATFORM_ANDROID
	// Not supported on ANDROID
	// Alternative solution
	// Watch out that android will clear the mask after a core has been switched off hence loosing the affinity mask setting!
	// http://stackoverflow.com/questions/16319725/android-set-thread-affinity
#elif DRX_PLATFORM_APPLE
	#pragma message "Warning: <ThreadInfo> DrxSetThreadAffinityMask not implemented for platform"
	// Implementation details can be found here
	// https://developer.apple.com/library/mac/releasenotes/Performance/RN-AffinityAPI/
#else
	cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);
	for (i32 cpu = 0; cpu < sizeof(cpu_mask) * 8; ++cpu)
	{
		if (dwAffinityMask & (1 << cpu))
		{
			CPU_SET(cpu, &cpu_mask);
		}
	}

	if (sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask) != 0)
	{
		switch (errno)
		{
		case EFAULT:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadAffinityMask: Supplied memory address was invalid.");
			break;
		case EINVAL:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadAffinityMask: The affinity bit mask [%u] contains no processors that are currently physically on the system and permitted to the	process .", dwAffinityMask);
			break;
		case EPERM:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadAffinityMask: The calling process does not have appropriate privileges. Mask [%u].", dwAffinityMask);
			break;
		case ESRCH:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadAffinityMask: The process whose ID is pid could not be found.");
			break;
		default:
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> DrxSetThreadAffinityMask: Unsupported error code: %i", errno);
			break;
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void DrxSetThreadPriority(TThreadHandle pThreadHandle, DWORD dwPriority)
{
	i32 policy;
	struct sched_param param;

	pthread_getschedparam(pThreadHandle, &policy, &param);
	param.sched_priority = sched_get_priority_max(dwPriority);
	pthread_setschedparam(pThreadHandle, policy, &param);
}

//////////////////////////////////////////////////////////////////////////
void DrxSetThreadPriorityBoost(TThreadHandle pThreadHandle, bool bEnabled)
{
	// Not supported
}

//////////////////////////////////////////////////////////////////////////
bool DrxCreateThread(TThreadHandle* pThreadHandle, const SThreadCreationDesc& threadDesc)
{
	u32 nStackSize = threadDesc.nStackSizeInBytes != 0 ? threadDesc.nStackSizeInBytes : DEFAULT_THREAD_STACK_SIZE_KB * 1024;

	assert(pThreadHandle != THREADID_NULL);
	pthread_attr_t threadAttr;
	sched_param schedParam;
	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&threadAttr, nStackSize);

	i32k err = pthread_create(
	  pThreadHandle,
	  &threadAttr,
	  threadDesc.fpEntryFunc,
	  threadDesc.pArgList);

	// Handle error on thread creation
	switch (err)
	{
	case 0:
		// No error
		break;
	case EAGAIN:
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> Unable to create thread \"%s\". Error Msg: \"Insufficient resources to create another thread, or a system-imposed limit on the number of threads was encountered.\"", threadDesc.szThreadName);
		return false;
	case EINVAL:
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> Unable to create thread \"%s\". Error Msg: \"Invalid attribute setting for thread creation.\"", threadDesc.szThreadName);
		return false;
	case EPERM:
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> Unable to create thread \"%s\". Error Msg: \"No permission to set the scheduling policy and parameters specified in attribute setting\"", threadDesc.szThreadName);
		return false;
	default:
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "<ThreadInfo> Unable to create thread \"%s\". Unknown error message. Error code %i", threadDesc.szThreadName, err);
		break;
	}

	// Print info to log
	DrxComment("<ThreadInfo>: New thread \"%s\" | StackSize: %u(KB)", threadDesc.szThreadName, threadDesc.nStackSizeInBytes / 1024);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void DrxThreadExitCall()
{
	// Notes on: pthread_exit
	// A thread that was create with pthread_create implicitly calls pthread_exit when the thread returns from its start routine (the function that was first called after a thread was created).
	// pthread_exit(NULL);
}
}

//////////////////////////////////////////////////////////////////////////
// FLOATING POINT EXCEPTIONS
//////////////////////////////////////////////////////////////////////////
namespace DrxThreadUtil
{
///////////////////////////////////////////////////////////////////////////
void EnableFloatExceptions(EFPE_Severity eFPESeverity)
{
	// TODO:
	// Not implemented
	// for potential implementation see http://linux.die.net/man/3/feenableexcept
}

//////////////////////////////////////////////////////////////////////////
void EnableFloatExceptions(threadID nThreadId, EFPE_Severity eFPESeverity)
{
	// TODO:
	// Not implemented
	// for potential implementation see http://linux.die.net/man/3/feenableexcept
}

//////////////////////////////////////////////////////////////////////////
uint GetFloatingPointExceptionMask()
{
	// Not implemented
	return ~0;
}

//////////////////////////////////////////////////////////////////////////
void SetFloatingPointExceptionMask(uint nMask)
{
	// Not implemented
}
}
