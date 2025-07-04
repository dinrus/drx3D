#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/Threads.h>

#ifdef __CELLOS_LV2__
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <stdio.h>
#endif

#if defined(SUNOS) || defined(__SUNOS__)
#include <stdio.h>
#endif
#ifdef __APPLE__
#include <mach/mach_time.h>
#include <TargetConditionals.h>
#endif

#if defined(WIN32) || defined(_WIN32)

#define DRX3D_USE_WINDOWS_TIMERS
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOMCX
#define NOIME

#ifdef _XBOX
#include <Xtl.h>
#else  //_XBOX
#include <windows.h>

#if WINVER < 0x0602
#define GetTickCount64 GetTickCount
#endif

#endif  //_XBOX

#include <time.h>

#else  //_WIN32
#include <sys/time.h>

#ifdef DRX3D_LINUX_REALTIME
//required linking against rt (librt)
#include <time.h>
#endif  //DRX3D_LINUX_REALTIME

#endif  //_WIN32

#define mymin(a, b) (a > b ? a : b)

struct ClockData
{
#ifdef DRX3D_USE_WINDOWS_TIMERS
	LARGE_INTEGER mClockFrequency;
	LONGLONG mStartTick;
	LARGE_INTEGER mStartTime;
#else
#ifdef __CELLOS_LV2__
	uint64_t mStartTime;
#else
#ifdef __APPLE__
	uint64_t mStartTimeNano;
#endif
	struct timeval mStartTime;
#endif
#endif  //__CELLOS_LV2__
};

///The Clock is a portable basic clock that measures accurate time in seconds, use for profiling.
Clock::Clock()
{
	m_data = new ClockData;
#ifdef DRX3D_USE_WINDOWS_TIMERS
	QueryPerformanceFrequency(&m_data->mClockFrequency);
#endif
	reset();
}

Clock::~Clock()
{
	delete m_data;
}

Clock::Clock(const Clock& other)
{
	m_data = new ClockData;
	*m_data = *other.m_data;
}

Clock& Clock::operator=(const Clock& other)
{
	*m_data = *other.m_data;
	return *this;
}

/// Resets the initial reference time.
void Clock::reset()
{
#ifdef DRX3D_USE_WINDOWS_TIMERS
	QueryPerformanceCounter(&m_data->mStartTime);
	m_data->mStartTick = GetTickCount64();
#else
#ifdef __CELLOS_LV2__

	typedef uint64_t ClockSize;
	ClockSize newTime;
	//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");
	SYS_TIMEBASE_GET(newTime);
	m_data->mStartTime = newTime;
#else
#ifdef __APPLE__
	m_data->mStartTimeNano = mach_absolute_time();
#endif
	gettimeofday(&m_data->mStartTime, 0);
#endif
#endif
}

/// Returns the time in ms since the last call to reset or since
/// the Clock was created.
zu64 Clock::getTimeMilliseconds()
{
#ifdef DRX3D_USE_WINDOWS_TIMERS
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	LONGLONG elapsedTime = currentTime.QuadPart -
						   m_data->mStartTime.QuadPart;
	// Compute the number of millisecond ticks elapsed.
	u64 msecTicks = (u64)(1000 * elapsedTime /
											  m_data->mClockFrequency.QuadPart);

	return msecTicks;
#else

#ifdef __CELLOS_LV2__
	uint64_t freq = sys_time_get_timebase_frequency();
	double dFreq = ((double)freq) / 1000.0;
	typedef uint64_t ClockSize;
	ClockSize newTime;
	SYS_TIMEBASE_GET(newTime);
	//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");

	return (u64)((double(newTime - m_data->mStartTime)) / dFreq);
#else

	struct timeval currentTime;
	gettimeofday(&currentTime, 0);
	return (currentTime.tv_sec - m_data->mStartTime.tv_sec) * 1000 +
		   (currentTime.tv_usec - m_data->mStartTime.tv_usec) / 1000;
#endif  //__CELLOS_LV2__
#endif
}

/// Returns the time in us since the last call to reset or since
/// the Clock was created.
zu64 Clock::getTimeMicroseconds()
{
#ifdef DRX3D_USE_WINDOWS_TIMERS
	//see https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
	LARGE_INTEGER currentTime, elapsedTime;

	QueryPerformanceCounter(&currentTime);
	elapsedTime.QuadPart = currentTime.QuadPart -
						   m_data->mStartTime.QuadPart;
	elapsedTime.QuadPart *= 1000000;
	elapsedTime.QuadPart /= m_data->mClockFrequency.QuadPart;

	return (zu64)elapsedTime.QuadPart;
#else

#ifdef __CELLOS_LV2__
	uint64_t freq = sys_time_get_timebase_frequency();
	double dFreq = ((double)freq) / 1000000.0;
	typedef uint64_t ClockSize;
	ClockSize newTime;
	//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");
	SYS_TIMEBASE_GET(newTime);

	return (u64)((double(newTime - m_data->mStartTime)) / dFreq);
#else

	struct timeval currentTime;
	gettimeofday(&currentTime, 0);
	return (currentTime.tv_sec - m_data->mStartTime.tv_sec) * 1000000 +
		   (currentTime.tv_usec - m_data->mStartTime.tv_usec);
#endif  //__CELLOS_LV2__
#endif
}

zu64 Clock::getTimeNanoseconds()
{
#ifdef DRX3D_USE_WINDOWS_TIMERS
	//see https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
	LARGE_INTEGER currentTime, elapsedTime;

	QueryPerformanceCounter(&currentTime);
	elapsedTime.QuadPart = currentTime.QuadPart -
						   m_data->mStartTime.QuadPart;
	elapsedTime.QuadPart *= 1000000000;
	elapsedTime.QuadPart /= m_data->mClockFrequency.QuadPart;

	return (zu64)elapsedTime.QuadPart;
#else

#ifdef __CELLOS_LV2__
	uint64_t freq = sys_time_get_timebase_frequency();
	double dFreq = ((double)freq) / 1e9;
	typedef uint64_t ClockSize;
	ClockSize newTime;
	//__asm __volatile__( "mftb %0" : "=r" (newTime) : : "memory");
	SYS_TIMEBASE_GET(newTime);

	return (u64)((double(newTime - m_data->mStartTime)) / dFreq);
#else
#ifdef __APPLE__
	uint64_t ticks = mach_absolute_time() - m_data->mStartTimeNano;
	static long double conversion = 0.0L;
	if (0.0L == conversion)
	{
		// attempt to get conversion to nanoseconds
		mach_timebase_info_data_t info;
		i32 err = mach_timebase_info(&info);
		if (err)
		{
			Assert(0);
			conversion = 1.;
		}
		conversion = info.numer / info.denom;
	}
	return (ticks * conversion);

#else  //__APPLE__

#ifdef DRX3D_LINUX_REALTIME
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return 1000000000 * ts.tv_sec + ts.tv_nsec;
#else
	struct timeval currentTime;
	gettimeofday(&currentTime, 0);
	return (currentTime.tv_sec - m_data->mStartTime.tv_sec) * 1e9 +
		   (currentTime.tv_usec - m_data->mStartTime.tv_usec) * 1000;
#endif  //DRX3D_LINUX_REALTIME

#endif  //__APPLE__
#endif  //__CELLOS_LV2__
#endif
}

/// Returns the time in s since the last call to reset or since
/// the Clock was created.
Scalar Clock::getTimeSeconds()
{
	static const Scalar microseconds_to_seconds = Scalar(0.000001);
	return Scalar(getTimeMicroseconds()) * microseconds_to_seconds;
}

#ifndef DRX3D_NO_PROFILE

static Clock gProfileClock;

inline void Profile_Get_Ticks(u64* ticks)
{
	*ticks = (u64)gProfileClock.getTimeMicroseconds();
}

inline float Profile_Get_Tick_Rate(void)
{
	//	return 1000000.f;
	return 1000.f;
}

/***************************************************************************************************
**
** CProfileNode
**
***************************************************************************************************/

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *=============================================================================================*/
CProfileNode::CProfileNode(tukk name, CProfileNode* parent) : Name(name),
																	 TotalCalls(0),
																	 TotalTime(0),
																	 StartTime(0),
																	 RecursionCounter(0),
																	 Parent(parent),
																	 Child(NULL),
																	 Sibling(NULL),
																	 m_userPtr(0)
{
	Reset();
}

void CProfileNode::CleanupMemory()
{
	delete (Child);
	Child = NULL;
	delete (Sibling);
	Sibling = NULL;
}

CProfileNode::~CProfileNode(void)
{
	CleanupMemory();
}

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *=============================================================================================*/
CProfileNode* CProfileNode::Get_Sub_Node(tukk name)
{
	// Try to find this sub node
	CProfileNode* child = Child;
	while (child)
	{
		if (child->Name == name)
		{
			return child;
		}
		child = child->Sibling;
	}

	// We didn't find it, so add it

	CProfileNode* node = new CProfileNode(name, this);
	node->Sibling = Child;
	Child = node;
	return node;
}

void CProfileNode::Reset(void)
{
	TotalCalls = 0;
	TotalTime = 0.0f;

	if (Child)
	{
		Child->Reset();
	}
	if (Sibling)
	{
		Sibling->Reset();
	}
}

void CProfileNode::Call(void)
{
	TotalCalls++;
	if (RecursionCounter++ == 0)
	{
		Profile_Get_Ticks(&StartTime);
	}
}

bool CProfileNode::Return(void)
{
	if (--RecursionCounter == 0 && TotalCalls != 0)
	{
		u64 time;
		Profile_Get_Ticks(&time);

		time -= StartTime;
		TotalTime += (float)time / Profile_Get_Tick_Rate();
	}
	return (RecursionCounter == 0);
}

/***************************************************************************************************
**
** CProfileIterator
**
***************************************************************************************************/
CProfileIterator::CProfileIterator(CProfileNode* start)
{
	CurrentParent = start;
	CurrentChild = CurrentParent->Get_Child();
}

void CProfileIterator::First(void)
{
	CurrentChild = CurrentParent->Get_Child();
}

void CProfileIterator::Next(void)
{
	CurrentChild = CurrentChild->Get_Sibling();
}

bool CProfileIterator::Is_Done(void)
{
	return CurrentChild == NULL;
}

void CProfileIterator::Enter_Child(i32 index)
{
	CurrentChild = CurrentParent->Get_Child();
	while ((CurrentChild != NULL) && (index != 0))
	{
		index--;
		CurrentChild = CurrentChild->Get_Sibling();
	}

	if (CurrentChild != NULL)
	{
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->Get_Child();
	}
}

void CProfileIterator::Enter_Parent(void)
{
	if (CurrentParent->Get_Parent() != NULL)
	{
		CurrentParent = CurrentParent->Get_Parent();
	}
	CurrentChild = CurrentParent->Get_Child();
}

/***************************************************************************************************
**
** CProfileManager
**
***************************************************************************************************/

CProfileNode gRoots[DRX3D_QUICKPROF_MAX_THREAD_COUNT] = {
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL),
	CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL), CProfileNode("Root", NULL)};

CProfileNode* gCurrentNodes[DRX3D_QUICKPROF_MAX_THREAD_COUNT] =
	{
		&gRoots[0],
		&gRoots[1],
		&gRoots[2],
		&gRoots[3],
		&gRoots[4],
		&gRoots[5],
		&gRoots[6],
		&gRoots[7],
		&gRoots[8],
		&gRoots[9],
		&gRoots[10],
		&gRoots[11],
		&gRoots[12],
		&gRoots[13],
		&gRoots[14],
		&gRoots[15],
		&gRoots[16],
		&gRoots[17],
		&gRoots[18],
		&gRoots[19],
		&gRoots[20],
		&gRoots[21],
		&gRoots[22],
		&gRoots[23],
		&gRoots[24],
		&gRoots[25],
		&gRoots[26],
		&gRoots[27],
		&gRoots[28],
		&gRoots[29],
		&gRoots[30],
		&gRoots[31],
		&gRoots[32],
		&gRoots[33],
		&gRoots[34],
		&gRoots[35],
		&gRoots[36],
		&gRoots[37],
		&gRoots[38],
		&gRoots[39],
		&gRoots[40],
		&gRoots[41],
		&gRoots[42],
		&gRoots[43],
		&gRoots[44],
		&gRoots[45],
		&gRoots[46],
		&gRoots[47],
		&gRoots[48],
		&gRoots[49],
		&gRoots[50],
		&gRoots[51],
		&gRoots[52],
		&gRoots[53],
		&gRoots[54],
		&gRoots[55],
		&gRoots[56],
		&gRoots[57],
		&gRoots[58],
		&gRoots[59],
		&gRoots[60],
		&gRoots[61],
		&gRoots[62],
		&gRoots[63],
};

i32 CProfileManager::FrameCounter = 0;
u64 CProfileManager::ResetTime = 0;

CProfileIterator* CProfileManager::Get_Iterator(void)
{
	i32 threadIndex = QuickprofGetCurrentThreadIndex2();
	if ((threadIndex < 0) || threadIndex >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return 0;

	return new CProfileIterator(&gRoots[threadIndex]);
}

void CProfileManager::CleanupMemory(void)
{
	for (i32 i = 0; i < DRX3D_QUICKPROF_MAX_THREAD_COUNT; i++)
	{
		gRoots[i].CleanupMemory();
	}
}

/***********************************************************************************************
 * CProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *=============================================================================================*/
void CProfileManager::Start_Profile(tukk name)
{
	i32 threadIndex = QuickprofGetCurrentThreadIndex2();
	if ((threadIndex < 0) || threadIndex >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return;

	if (name != gCurrentNodes[threadIndex]->Get_Name())
	{
		gCurrentNodes[threadIndex] = gCurrentNodes[threadIndex]->Get_Sub_Node(name);
	}

	gCurrentNodes[threadIndex]->Call();
}

/***********************************************************************************************
 * CProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *=============================================================================================*/
void CProfileManager::Stop_Profile(void)
{
	i32 threadIndex = QuickprofGetCurrentThreadIndex2();
	if ((threadIndex < 0) || threadIndex >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return;

	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (gCurrentNodes[threadIndex]->Return())
	{
		gCurrentNodes[threadIndex] = gCurrentNodes[threadIndex]->Get_Parent();
	}
}

/***********************************************************************************************
 * CProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *=============================================================================================*/
void CProfileManager::Reset(void)
{
	gProfileClock.reset();
	i32 threadIndex = QuickprofGetCurrentThreadIndex2();
	if ((threadIndex < 0) || threadIndex >= DRX3D_QUICKPROF_MAX_THREAD_COUNT)
		return;
	gRoots[threadIndex].Reset();
	gRoots[threadIndex].Call();
	FrameCounter = 0;
	Profile_Get_Ticks(&ResetTime);
}

/***********************************************************************************************
 * CProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *=============================================================================================*/
void CProfileManager::Increment_Frame_Counter(void)
{
	FrameCounter++;
}

/***********************************************************************************************
 * CProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *=============================================================================================*/
float CProfileManager::Get_Time_Since_Reset(void)
{
	u64 time;
	Profile_Get_Ticks(&time);
	time -= ResetTime;
	return (float)time / Profile_Get_Tick_Rate();
}

#include <stdio.h>

void CProfileManager::dumpRecursive(CProfileIterator* profileIterator, i32 spacing)
{
	profileIterator->First();
	if (profileIterator->Is_Done())
		return;

	float accumulated_time = 0, parent_time = profileIterator->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : profileIterator->Get_Current_Parent_Total_Time();
	i32 i;
	i32 frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (i = 0; i < spacing; i++) printf(".");
	printf("----------------------------------\n");
	for (i = 0; i < spacing; i++) printf(".");
	printf("Profiling: %s (total running time: %.3f ms) ---\n", profileIterator->Get_Current_Parent_Name(), parent_time);
	float totalTime = 0.f;

	i32 numChildren = 0;

	for (i = 0; !profileIterator->Is_Done(); i++, profileIterator->Next())
	{
		numChildren++;
		float current_total_time = profileIterator->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;
		{
			i32 i;
			for (i = 0; i < spacing; i++) printf(".");
		}
		printf("%d -- %s (%.2f %%) :: %.3f ms / frame (%d calls)\n", i, profileIterator->Get_Current_Name(), fraction, (current_total_time / (double)frames_since_reset), profileIterator->Get_Current_Total_Calls());
		totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		//printf("what's wrong\n");
	}
	for (i = 0; i < spacing; i++) printf(".");
	printf("%s (%.3f %%) :: %.3f ms\n", "Unaccounted:", parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f, parent_time - accumulated_time);

	for (i = 0; i < numChildren; i++)
	{
		profileIterator->Enter_Child(i);
		dumpRecursive(profileIterator, spacing + 3);
		profileIterator->Enter_Parent();
	}
}

void CProfileManager::dumpAll()
{
	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

	dumpRecursive(profileIterator, 0);

	CProfileManager::Release_Iterator(profileIterator);
}


void EnterProfileZoneDefault(tukk name)
{
}
void btLeaveProfileZoneDefault()
{
}

#else
void EnterProfileZoneDefault(tukk name)
{
}
void btLeaveProfileZoneDefault()
{
}
#endif  //DRX3D_NO_PROFILE


// clang-format off
#if defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
  #define DRX3D_HAVE_TLS 1
#elif __APPLE__ && !TARGET_OS_IPHONE
  // TODO: Modern versions of iOS support TLS now with updated version checking.
  #define DRX3D_HAVE_TLS 1
#elif __linux__
  #define DRX3D_HAVE_TLS 1
#elif defined(__FreeBSD__) || defined(__NetBSD__)
  // TODO: At the moment disabling purposely OpenBSD, albeit tls support exists but not fully functioning
  #define DRX3D_HAVE_TLS 1
#endif

// __thread is broken on Andorid clang until r12b. See
// https://github.com/android-ndk/ndk/issues/8
#if defined(__ANDROID__) && defined(__clang__)
  #if __has_include(<android/ndk-version.h>)
    #include <android/ndk-version.h>
  #endif  // __has_include(<android/ndk-version.h>)
  #if defined(__NDK_MAJOR__) && \
    ((__NDK_MAJOR__ < 12) || ((__NDK_MAJOR__ == 12) && (__NDK_MINOR__ < 1)))
    #undef DRX3D_HAVE_TLS
  #endif
#endif  // defined(__ANDROID__) && defined(__clang__)
// clang-format on

u32 QuickprofGetCurrentThreadIndex2()
{
	u32k kNullIndex = ~0U;

#if DRX3D_THREADSAFE
	return GetCurrentThreadIndex();
#else
#if defined(DRX3D_HAVE_TLS)
	static __thread u32 sThreadIndex = kNullIndex;
#elif defined(_WIN32)
	__declspec(thread) static u32 sThreadIndex = kNullIndex;
#else
	u32 sThreadIndex = 0;
	return -1;
#endif

	static i32 gThreadCounter = 0;

	if (sThreadIndex == kNullIndex)
	{
		sThreadIndex = gThreadCounter++;
	}
	return sThreadIndex;
#endif  //DRX3D_THREADSAFE
}

static EnterProfileZoneFunc* bts_enterFunc = EnterProfileZoneDefault;
static LeaveProfileZoneFunc* bts_leaveFunc = btLeaveProfileZoneDefault;

void EnterProfileZone(tukk name)
{
	(bts_enterFunc)(name);
}
void LeaveProfileZone()
{
	(bts_leaveFunc)();
}

EnterProfileZoneFunc* GetCurrentEnterProfileZoneFunc()
{
	return bts_enterFunc;
}
LeaveProfileZoneFunc* GetCurrentLeaveProfileZoneFunc()
{
	return bts_leaveFunc;
}

void SetCustomEnterProfileZoneFunc(EnterProfileZoneFunc* enterFunc)
{
	bts_enterFunc = enterFunc;
}
void SetCustomLeaveProfileZoneFunc(LeaveProfileZoneFunc* leaveFunc)
{
	bts_leaveFunc = leaveFunc;
}

CProfileSample::CProfileSample(tukk name)
{
	EnterProfileZone(name);
}

CProfileSample::~CProfileSample(void)
{
	LeaveProfileZone();
}
