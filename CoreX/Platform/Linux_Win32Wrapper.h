// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Linux_Win32Wrapper.h
//  Version:     v1.00
//  Created:     23/6/2005 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Linux_Win32Wrapper_h__
#define __Linux_Win32Wrapper_h__
#pragma once

#include <drx3D/CoreX/Assert/DrxAssert.h>
/* Memory block identification */
#define _FREE_BLOCK   0
#define _NORMAL_BLOCK 1
#define _CRT_BLOCK    2
#define _IGNORE_BLOCK 3
#define _CLIENT_BLOCK 4
#define _MAX_BLOCKS   5

typedef uk HMODULE;

typedef struct _MEMORYSTATUS
{
	DWORD  dwLength;
	DWORD  dwMemoryLoad;
	SIZE_T dwTotalPhys;
	SIZE_T dwAvailPhys;
	SIZE_T dwTotalPageFile;
	SIZE_T dwAvailPageFile;
	SIZE_T dwTotalVirtual;
	SIZE_T dwAvailVirtual;
} MEMORYSTATUS, * LPMEMORYSTATUS;

extern void GlobalMemoryStatus(LPMEMORYSTATUS lpmem);

#if DRX_PLATFORM_64BIT
	#define MEMORY_ALLOCATION_ALIGNMENT 16
#else
	#define MEMORY_ALLOCATION_ALIGNMENT 8
#endif

#define S_OK                          0
#define THREAD_PRIORITY_NORMAL        0
#define THREAD_PRIORITY_IDLE          0
#define THREAD_PRIORITY_LOWEST        0
#define THREAD_PRIORITY_BELOW_NORMAL  0
#define THREAD_PRIORITY_ABOVE_NORMAL  0
#define THREAD_PRIORITY_HIGHEST       0
#define THREAD_PRIORITY_TIME_CRITICAL 0
#define MAX_COMPUTERNAME_LENGTH       15 // required by DrxOnline module

#if 0
//! For compatibility reason we got to create a class which actually contains an i32 rather than a uk and make sure it does not get mistreated.
//! U is default type for invalid handle value, T the encapsulated handle type to be used instead of uk (as under windows and never linux).
template<class T, T U>
class CHandle
{
public:
	typedef T     HandleType;
	typedef uk PointerType;  //!< For compatibility reason to encapsulate a uk as an i32.

	static const HandleType sciInvalidHandleValue = U;

	CHandle(const CHandle<T, U>& cHandle) : m_Value(cHandle.m_Value){}
	CHandle(const HandleType cHandle = U) : m_Value(cHandle){}
	//CHandle(const PointerType cpHandle) : m_Value(reinterpret_cast<HandleType>(cpHandle)){}
	//CHandle(INVALID_HANDLE_VALUE_ENUM) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types

	operator HandleType(){ return m_Value; }
	bool           operator!() const                           { return m_Value == sciInvalidHandleValue; }
	const CHandle& operator=(const CHandle& crHandle)          { m_Value = crHandle.m_Value; return *this; }
	const CHandle& operator=(const PointerType cpHandle)       { m_Value = reinterpret_cast<HandleType>(cpHandle); return *this; }
	const bool     operator==(const CHandle& crHandle)   const { return m_Value == crHandle.m_Value; }
	const bool     operator==(const HandleType cHandle)  const { return m_Value == cHandle; }
	//const bool operator ==(const PointerType cpHandle)const{return m_Value == reinterpret_cast<HandleType>(cpHandle);}
	const bool     operator!=(const HandleType cHandle)  const { return m_Value != cHandle; }
	const bool     operator!=(const CHandle& crHandle)   const { return m_Value != crHandle.m_Value; }
	//const bool operator !=(const PointerType cpHandle)const{return m_Value != reinterpret_cast<HandleType>(cpHandle);}
	const bool     operator<(const CHandle& crHandle)   const  { return m_Value < crHandle.m_Value; }
	HandleType     Handle() const                              { return m_Value; }

private:
	HandleType m_Value;             //!< The actual value, remember that file descriptors are ints under linux.
	typedef void ReferenceType;     //!< For compatibility reason to encapsulate a uk as an i32.

	// Forbid these function which would actually not work on an i32.
	PointerType   operator->();
	PointerType   operator->() const;
	ReferenceType operator*();
	ReferenceType operator*() const;
	operator PointerType();
};

typedef CHandle<INT_PTR, (INT_PTR) 0> HANDLE;

typedef HANDLE                        EVENT_HANDLE;
typedef HANDLE                        THREAD_HANDLE;

	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

//#define __TIMESTAMP__ __DATE__" "__TIME__

// stdlib.h stuff
#define _MAX_DRIVE 3    // max. length of drive component
#define _MAX_DIR   256  // max. length of path component
#define _MAX_FNAME 256  // max. length of file name component
#define _MAX_EXT   256  // max. length of extension component

// fcntl.h
#define _O_RDONLY      0x0000   /* open for reading only */
#define _O_WRONLY      0x0001   /* open for writing only */
#define _O_RDWR        0x0002   /* open for reading and writing */
#define _O_APPEND      0x0008   /* writes done at eof */
#define _O_CREAT       0x0100   /* create and open file */
#define _O_TRUNC       0x0200   /* open and truncate */
#define _O_EXCL        0x0400   /* open only if file doesn't already exist */
#define _O_TEXT        0x4000   /* file mode is text (translated) */
#define _O_BINARY      0x8000   /* file mode is binary (untranslated) */
#define _O_RAW         _O_BINARY
#define _O_NOINHERIT   0x0080   /* child process doesn't inherit file */
#define _O_TEMPORARY   0x0040   /* temporary file bit */
#define _O_SHORT_LIVED 0x1000   /* temporary storage file, try not to flush */
#define _O_SEQUENTIAL  0x0020   /* file access is primarily sequential */
#define _O_RANDOM      0x0010   /* file access is primarily random */

#define MemoryBarrier() __sync_synchronize()
// Memory barrier implementation taken from https://code.google.com/p/gperftools/
//inline void MemoryBarrier() {
//  __asm__ __volatile__("mfence" : : : "memory");
//}

//! Replacement for MoveFileEx
//! TODO: Flag implementations for the editor
inline i32 MoveFileEx(tukk pOld, tukk pNew, u32k flags)
{
	(void) flags;
	return rename(pOld, pNew);
}

//////////////////////////////////////////////////////////////////////////
// io.h stuff
#if !DRX_PLATFORM_ANDROID
extern i32 errno;
#endif
typedef u32 _fsize_t;

struct _OVERLAPPED;
typedef _OVERLAPPED* LPOVERLAPPED;

typedef void (*      LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, struct _OVERLAPPED* lpOverlapped);

typedef struct tagRECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, * PRECT;

typedef struct tagPOINT
{
	LONG x;
	LONG y;
} POINT, * PPOINT;

#ifndef _FILETIME_
	#define _FILETIME_
typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, * PFILETIME, * LPFILETIME;
#endif

typedef union _ULARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		DWORD HighPart;
	};
	zu64 QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER* PULARGE_INTEGER;

#ifdef __cplusplus
inline LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2)
#else
static LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2)
#endif
{
	ULARGE_INTEGER u1, u2;
	memcpy(&u1, lpFileTime1, sizeof u1);
	memcpy(&u2, lpFileTime2, sizeof u2);
	if (u1.QuadPart < u2.QuadPart)
		return -1;
	else if (u1.QuadPart > u2.QuadPart)
		return 1;
	return 0;
}

typedef struct _SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, * PSYSTEMTIME, * LPSYSTEMTIME;

typedef struct _TIME_FIELDS
{
	short Year;
	short Month;
	short Day;
	short Hour;
	short Minute;
	short Second;
	short Milliseconds;
	short Weekday;
} TIME_FIELDS, * PTIME_FIELDS;

#define DAYSPERNORMALYEAR 365
#define DAYSPERLEAPYEAR   366
#define MONSPERYEAR       12

inline void ZeroMemory(uk pPtr, i32 nSize)
{
	memset(pPtr, 0, nSize);
}

inline BOOL InflateRect(RECT* pRect, i32 dx, i32 dy)
{
	pRect->left -= dx;
	pRect->right += dx;
	pRect->top -= dy;
	pRect->bottom += dy;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
extern BOOL     SystemTimeToFileTime(const SYSTEMTIME* syst, LPFILETIME ft);
//! Win32API function declarations actually used .
extern bool     IsBadReadPtr(uk ptr, u32 size);

DLL_IMPORT void OutputDebugString(tukk);    //!< Defined in the launcher.

/*
   //critical section stuff
   #define pthread_attr_default NULL

   typedef pthread_mutex_t CRITICAL_SECTION;
   #ifdef __cplusplus
   inline void InitializeCriticalSection(CRITICAL_SECTION *lpCriticalSection)
   {
   pthread_mutexattr_t pthread_mutexattr_def;
   pthread_mutexattr_settype(&pthread_mutexattr_def, PTHREAD_MUTEX_RECURSIVE_NP);
   pthread_mutex_init(lpCriticalSection, &pthread_mutexattr_def);
   }
   inline void EnterCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_lock(lpCriticalSection);}
   inline void LeaveCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_unlock(lpCriticalSection);}
   inline void DeleteCriticalSection(CRITICAL_SECTION *lpCriticalSection){}
   #else
   static void InitializeCriticalSection(CRITICAL_SECTION *lpCriticalSection)
   {
   pthread_mutexattr_t pthread_mutexattr_def;
   pthread_mutexattr_settype(&pthread_mutexattr_def, PTHREAD_MUTEX_RECURSIVE_NP);
   pthread_mutex_init(lpCriticalSection, &pthread_mutexattr_def);
   }
   static void EnterCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_lock(lpCriticalSection);}
   static void LeaveCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_unlock(lpCriticalSection);}
   static void DeleteCriticalSection(CRITICAL_SECTION *lpCriticalSection){}
   #endif
 */

extern bool QueryPerformanceCounter(LARGE_INTEGER* counter);
extern bool QueryPerformanceFrequency(LARGE_INTEGER* frequency);
#ifdef __cplusplus

inline u32 GetTickCount()
{
	LARGE_INTEGER count, freq;
	QueryPerformanceCounter(&count);
	QueryPerformanceFrequency(&freq);
	return u32(count.QuadPart * 1000 / freq.QuadPart);
}

	#define IGNORE   0                // Ignore signal
	#define INFINITE 0xFFFFFFFF       // Infinite timeout
#endif
//begin--------------------------------findfirst/-next declaration/implementation----------------------------------------------------

//////////////////////////////////////////////////////////////////////////

typedef int64 __time64_t;     /* 64-bit time value */

//////////////////////////////////////////////////////////////////////////
// function renaming
#define _chmod    chmod
#define stricmp   strcasecmp
#define _stricmp  strcasecmp
#define strnicmp  strncasecmp
#define _strnicmp strncasecmp

#define _strlwr   strlwr
#define _strlwr_s(BUF, SIZE) strlwr(BUF)
#define _strups   strupr

#define _wtof(str) wcstod(str, 0)

// Need to include this before using it's used in finddata, but after the strnicmp definition
#include <drx3D/CoreX/String/DrxString.h>

//! < Atributes set by find request.
typedef struct __finddata64_t
{
	u32 attrib;            //!< Attributes, only directory and readonly flag actually set.
	__time64_t   time_create;       //!< Creation time, cannot parse under linux, last modification time is used instead (game does nowhere makes decision based on this values).
	__time64_t   time_access;       //!< Last access time.
	__time64_t   time_write;        //!< Last modification time.
	__time64_t   size;              //!< File size (for a directory it will be the block size).
	char         name[256];         //!< File/directory name.

private:
	i32                 m_LastIndex;          //!< Last index for findnext.
	char                m_DirectoryName[260]; //!< Directory name, needed when getting file attributes on the fly.
	char                m_ToMatch[260];       //!< Pattern to match with.
	DIR*                m_Dir;                //!< Directory handle.
	std::vector<string> m_Entries;            //!< All file entries in the current directories.
public:

	inline __finddata64_t() :
		attrib(0), time_create(0), time_access(0), time_write(0),
		size(0), m_LastIndex(-1), m_Dir(NULL)
	{
		memset(name, '0', 256);
	}
	~__finddata64_t();

	//! Copies and retrieves the data for an actual match (to not waste any effort retrioeving data for unused files).
	void CopyFoundData(tukk rMatchedFileName);

public:
	//! Global _findfirst64 function using struct above, can't be a member function due to required semantic match.
	friend intptr_t _findfirst64(tukk pFileName, __finddata64_t* pFindData);

	//! Global _findnext64 function using struct above, can't be a member function due to required semantic match.
	friend i32 _findnext64(intptr_t last, __finddata64_t* pFindData);
}__finddata64_t;

//! Need inheritance since in many places it get used as struct _finddata_t.
typedef struct _finddata_t : public __finddata64_t
{}_finddata_t;

extern i32      _findnext64(intptr_t last, __finddata64_t* pFindData);
extern intptr_t _findfirst64(tukk pFileName, __finddata64_t* pFindData);
//#endif
//end--------------------------------findfirst/-next declaration/implementation----------------------------------------------------

extern BOOL GetUserName(LPSTR lpBuffer, LPDWORD nSize);

//! Error code stuff. Not thread specific, just a coarse implementation for the main thread.
inline DWORD GetLastError() { return errno; }

//! Error code stuff. Not thread specific, just a coarse implementation for the main thread.
inline void SetLastError(DWORD dwErrCode) { errno = dwErrCode; }

//////////////////////////////////////////////////////////////////////////
#define GENERIC_READ              (0x80000000L)
#define GENERIC_WRITE             (0x40000000L)
#define GENERIC_EXECUTE           (0x20000000L)
#define GENERIC_ALL               (0x10000000L)

#define CREATE_NEW                1
#define CREATE_ALWAYS             2
#define OPEN_EXISTING             3
#define OPEN_ALWAYS               4
#define TRUNCATE_EXISTING         5

#define FILE_SHARE_READ           0x00000001
#define FILE_SHARE_WRITE          0x00000002
#define OPEN_EXISTING             3
#define FILE_FLAG_OVERLAPPED      0x40000000
#define INVALID_FILE_SIZE         ((DWORD)0xFFFFFFFFl)
#define FILE_BEGIN                0
#define FILE_CURRENT              1
#define FILE_END                  2
#define ERROR_NO_SYSTEM_RESOURCES 1450L
#define ERROR_INVALID_USER_BUFFER 1784L
#define ERROR_NOT_ENOUGH_MEMORY   8L
#define ERROR_PATH_NOT_FOUND      3L
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateFile(
  tukk lpFileName,
  DWORD dwDesiredAccess,
  DWORD dwShareMode,
  uk lpSecurityAttributes,
  DWORD dwCreationDisposition,
  DWORD dwFlagsAndAttributes,
  HANDLE hTemplateFile
  );

//////////////////////////////////////////////////////////////////////////
extern DWORD GetFileAttributes(LPCSTR lpFileName);

//////////////////////////////////////////////////////////////////////////
extern BOOL SetFileAttributes(LPCSTR, DWORD attributes);

//////////////////////////////////////////////////////////////////////////
extern BOOL SetFileTime(
  HANDLE hFile,
  const FILETIME* lpCreationTime,
  const FILETIME* lpLastAccessTime,
  const FILETIME* lpLastWriteTime);
//////////////////////////////////////////////////////////////////////////
extern const uint64 GetFileModifTime(FILE* hFile);

//////////////////////////////////////////////////////////////////////////
extern DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh);

//////////////////////////////////////////////////////////////////////////
extern BOOL CloseHandle(HANDLE hObject);

//////////////////////////////////////////////////////////////////////////
extern BOOL    CancelIo(HANDLE hFile);
//////////////////////////////////////////////////////////////////////////
extern HRESULT GetOverlappedResult(HANDLE hFile, uk lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait);
//////////////////////////////////////////////////////////////////////////
extern BOOL    ReadFile(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped
  );

//////////////////////////////////////////////////////////////////////////
extern BOOL ReadFileEx(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPOVERLAPPED lpOverlapped,
  LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
  );

//////////////////////////////////////////////////////////////////////////
extern DWORD SetFilePointer(
  HANDLE hFile,
  LONG lDistanceToMove,
  PLONG lpDistanceToMoveHigh,
  DWORD dwMoveMethod
  );

//////////////////////////////////////////////////////////////////////////
extern threadID GetCurrentThreadId();

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateEvent(
  LPSECURITY_ATTRIBUTES lpEventAttributes,
  BOOL bManualReset,
  BOOL bInitialState,
  LPCSTR lpName
  );

//////////////////////////////////////////////////////////////////////////
extern DWORD Sleep(DWORD dwMilliseconds);

//////////////////////////////////////////////////////////////////////////
extern DWORD SleepEx(DWORD dwMilliseconds, BOOL bAlertable);

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForSingleObjectEx(
  HANDLE hHandle,
  DWORD dwMilliseconds,
  BOOL bAlertable);

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForMultipleObjectsEx(
  DWORD nCount,
  const HANDLE* lpHandles,
  BOOL bWaitAll,
  DWORD dwMilliseconds,
  BOOL bAlertable);

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

//////////////////////////////////////////////////////////////////////////
extern BOOL SetEvent(HANDLE hEvent);

//////////////////////////////////////////////////////////////////////////
extern BOOL ResetEvent(HANDLE hEvent);

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateMutex(
  LPSECURITY_ATTRIBUTES lpMutexAttributes,
  BOOL bInitialOwner,
  LPCSTR lpName
  );

//////////////////////////////////////////////////////////////////////////
extern BOOL ReleaseMutex(HANDLE hMutex);

//////////////////////////////////////////////////////////////////////////
typedef DWORD (*              PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateThread(
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  SIZE_T dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID lpParameter,
  DWORD dwCreationFlags,
  LPDWORD lpThreadId
  );

extern BOOL  DeleteFile(LPCSTR lpFileName);
extern BOOL  MoveFile(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
extern BOOL  RemoveDirectory(LPCSTR lpPathName);
extern DWORD GetCurrentDirectory(DWORD nBufferLength, tuk lpBuffer);
//extern BOOL SetCurrentDirectory(LPCSTR lpPathName);
extern BOOL  CopyFile(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists);
extern BOOL  GetComputerName(LPSTR lpBuffer, LPDWORD lpnSize);  //!< Required for DrxOnline.
extern DWORD GetCurrentProcessId(void);

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus

// Helper functions.
extern const bool ConvertFilenameNoCase(tuk file);
extern void       adaptFilenameToLinux(tuk rAdjustedFilename);
extern i32k  comparePathNames(tukk cpFirst, tukk cpSecond, u32 len); //!< \return 0 if identical.
extern void       replaceDoublePathFilename(tuk szFileName);                                   //!< Removes "\.\" to "\" and "/./" to "/".

//////////////////////////////////////////////////////////////////////////
extern tuk    _fullpath(tuk absPath, tukk relPath, size_t maxLength);
extern intptr_t _findfirst64(tukk filespec, struct __finddata64_t* fileinfo);
extern i32      _findnext64(intptr_t handle, struct __finddata64_t* fileinfo);
extern i32      _findclose(intptr_t handle);

//////////////////////////////////////////////////////////////////////////
extern void _makepath(tuk path, tukk drive, tukk dir, tukk filename, tukk ext);
extern void _splitpath(tukk inpath, tuk drv, tuk dir, tuk fname, tuk ext);

//////////////////////////////////////////////////////////////////////////
extern i32 memicmp(LPCSTR s1, LPCSTR s2, DWORD len);
extern i32 strcmpi(tukk str1, tukk str2);

extern "C" tuk strlwr(tuk str);
extern "C" tuk strupr(tuk str);

extern tuk     _ui64toa(zu64 value, tuk str, i32 radix);
extern long long _atoi64(tukk str);

extern i32*      _errno();

	#ifdef _isnan
		#undef _isnan
template<typename T>
bool _isnan(T value)
{
	return value != value;
}
	#endif

	#define fprintf_s fprintf
	#define sscanf_s  sscanf

//////////////////////////////////////////////////////////////////////////

	#ifndef __TRLTOA__
		#define __TRLTOA__
extern tuk ltoa(long i, tuk a, i32 radix);
	#endif
	#define itoa ltoa

//////////////////////////////////////////////////////////////////////////
	#if 0
inline i32 abs(i32 x)
{
	return labs(x);
}

inline float abs(float x)
{
	return fabsf(x);
}

inline double abs(double x)
{
	return fabs(x);
}

inline float sqrt(float x)
{
	return sqrtf(x);
}
	#else
		#include <cmath>
using std::abs;
using std::sqrt;
using std::fabs;
	#endif

extern tuk _strtime(tuk date);
extern tuk _strdate(tuk date);

#endif //__cplusplus
//////////////////////////////////////////////////////////////////////////
// Byte Swapping functions

inline unsigned short _byteswap_ushort(unsigned short input)
{
	return ((input & 0xff) << 8) | ((input & 0xff00) >> 8);
}

inline LONG _byteswap_ulong(LONG input)
{
	return (input & 0x000000ff) << 24 |
	       (input & 0x0000ff00) << 8 |
	       (input & 0x00ff0000) >> 8 |
	       (input & 0xff000000) >> 24;
}

inline zu64 _byteswap_uint64(zu64 input)
{
	return (((input & 0xff00000000000000ull) >> 56) |
	        ((input & 0x00ff000000000000ull) >> 40) |
	        ((input & 0x0000ff0000000000ull) >> 24) |
	        ((input & 0x000000ff00000000ull) >> 8) |
	        ((input & 0x00000000ff000000ull) << 8) |
	        ((input & 0x0000000000ff0000ull) << 24) |
	        ((input & 0x000000000000ff00ull) << 40) |
	        ((input & 0x00000000000000ffull) << 56));
}

#endif // __Linux_Win32Wrapper_h__
