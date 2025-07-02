// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FileIOWrapper_h__
#define __FileIOWrapper_h__

class CIOWrapper
{
public:
	static void   SetLoggerState(i32 b);
	static size_t Fread(uk pData, size_t nSize, size_t nCount, FILE* hFile);
	static i32    Fclose(FILE* hFile) { return FcloseEx(hFile); }
	static i32    FcloseEx(FILE* hFile);
#ifdef USE_FILE_HANDLE_CACHE
	static FILE*  Fopen(tukk file, tukk mode, FileIoWrapper::FileAccessType type = FileIoWrapper::GENERAL, bool bSysAppHome = false) { return FopenEx(file, mode, type, bSysAppHome); }
	static FILE*  FopenEx(tukk file, tukk mode, FileIoWrapper::FileAccessType type = FileIoWrapper::GENERAL, bool bSysAppHome = false);
#else
	static FILE*  Fopen(tukk file, tukk mode) { return FopenEx(file, mode); }
	static FILE*  FopenEx(tukk file, tukk mode);
#endif

	// open file in locked/exclusive mode, so no other processes can then open the file.
	// on non-windows platforms this forwards to Fopen.
	static FILE* FopenLocked(tukk file, tukk mode);

	static int64 FTell(FILE* hFile);
	static i32   Fseek(FILE* hFile, int64 seek, i32 mode);

	static i32   FEof(FILE* hFile);
	static i32   FError(FILE* hFile);

	static void  LockReadIO(bool lock);

private:
	static bool               m_bLockReads;
	static DrxCriticalSection m_ReadCS;
};

#endif //__FileIOWrapper_h__
