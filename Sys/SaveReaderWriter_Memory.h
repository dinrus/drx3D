// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SaveReaderWriter_Memory.h
//  Created:     30/4/2010 by Ian Masters.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SaveReaderWriter_Memory_h__
#define __SaveReaderWriter_Memory_h__

#include <drx3D/CoreX/Platform/IPlatformOS.h>

struct IMemoryFile
{
	virtual ~IMemoryFile() {}
	virtual size_t                          GetLength() const = 0;
	virtual long                            GetFilePos() const = 0;
	virtual tukk                     GetFilePath() const = 0;
	virtual ukk                     GetFileContents() const = 0;
	virtual IPlatformOS::EFileOperationCode Seek(long seek, IPlatformOS::ISaveReader::ESeekMode mode) = 0;
	virtual bool                            IsDirty() const = 0;
	virtual void                            SetDirty(bool bDirty) = 0;
	virtual void                            GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

DECLARE_SHARED_POINTERS(IMemoryFile);

struct IMemoryFileSystem
{
	virtual ~IMemoryFileSystem(){}

	static IMemoryFileSystem* CreateFileSystem();

	// LinkFile
	// Описание:
	//     Links a file to the file system. If the file already exists for the specified filename and user index it is replaced by the new one.
	virtual void LinkFile(IMemoryFilePtr file) = 0;

	// UnlinkFile
	// Описание:
	//     Unlinks a file to the file system. If the file no longer has any references it will be destroyed.
	virtual bool UnlinkFile(IMemoryFilePtr file) = 0;

	// GetFile
	// Описание:
	//     Find a file by filename and user index.
	virtual IMemoryFilePtr GetFile(tukk fileName, u32 user) = 0;

	virtual i32            GetNumFiles(u32 user) = 0;
	virtual i32            GetNumDirtyFiles(u32 user) = 0;

	virtual intptr_t       FindFirst(u32 userIndex, tukk filePattern, _finddata_t* fd) = 0;
	virtual i32            FindNext(intptr_t handle, _finddata_t* fd) = 0;
	virtual i32            FindClose(intptr_t handle) = 0;

	// Format
	// Описание:
	//     Delete all files and flush all memory.
	virtual void Format(u32 user) = 0;

	// Lock / Unlock
	// Описание:
	//     Multi-threaded access to file system so files can be written out asynchronously.
	virtual bool TryLock() = 0;
	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	virtual bool DebugSave(u32 user, IPlatformOS::SDebugDump& dump) = 0;
	virtual bool DebugLoad(u32 user, tukk fileName = NULL) = 0;

	virtual void DumpFiles(tukk label) = 0;
};
DECLARE_SHARED_POINTERS(IMemoryFileSystem);

class CSaveReader_Memory : public IPlatformOS::ISaveReader
{
public:
	CSaveReader_Memory(const IMemoryFileSystemPtr& pFileSystem, tukk fileName, u32 userIndex);
	virtual ~CSaveReader_Memory();

	// ISaveReader
	virtual IPlatformOS::EFileOperationCode Seek(long seek, ESeekMode mode);
	virtual IPlatformOS::EFileOperationCode GetFileCursor(long& fileCursor);
	virtual IPlatformOS::EFileOperationCode ReadBytes(uk data, size_t numBytes);
	virtual IPlatformOS::EFileOperationCode GetNumBytes(size_t& numBytes);
	virtual IPlatformOS::EFileOperationCode Close()           { return IPlatformOS::eFOC_Success; }
	virtual IPlatformOS::EFileOperationCode LastError() const { return m_eLastError; }
	virtual void                            GetMemoryUsage(IDrxSizer* pSizer) const;
	//~ISaveReader

private:
	IMemoryFileSystemPtr            m_pFileSystem;
	IMemoryFilePtr                  m_file;
	IPlatformOS::EFileOperationCode m_eLastError;
};

class CSaveWriter_Memory : public IPlatformOS::ISaveWriter
{
public:
	CSaveWriter_Memory(const IMemoryFileSystemPtr& pFileSystem, tukk fileName, u32 userIndex);
	virtual ~CSaveWriter_Memory();

	// ISaveWriter
	virtual IPlatformOS::EFileOperationCode AppendBytes(ukk data, size_t length);
	virtual IPlatformOS::EFileOperationCode Close()           { m_bClosed = true; return IPlatformOS::eFOC_Success; }
	virtual IPlatformOS::EFileOperationCode LastError() const { return m_eLastError; }
	virtual void                            GetMemoryUsage(IDrxSizer* pSizer) const;
	//~ISaveWriter

	bool IsClosed() const { return m_bClosed; }

protected:
	IMemoryFileSystemPtr            m_pFileSystem;
	IMemoryFilePtr                  m_file;
	IPlatformOS::EFileOperationCode m_eLastError;
	u32                    m_user;
	bool                            m_bClosed;
};

class CFileFinderMemory : public IPlatformOS::IFileFinder
{
public:
	CFileFinderMemory(const IMemoryFileSystemPtr& pFileSystem, u32 user);
	virtual ~CFileFinderMemory();

	virtual EFileState FileExists(tukk path);
	virtual intptr_t   FindFirst(tukk filePattern, _finddata_t* fd);
	virtual i32        FindNext(intptr_t handle, _finddata_t* fd);
	virtual i32        FindClose(intptr_t handle);
	virtual void       GetMemoryUsage(IDrxSizer* pSizer) const;

	static i32         PatternMatch(tukk str, tukk p);

private:
	IMemoryFileSystemPtr m_pFileSystem;
	u32         m_user;
};

#endif // __SaveReaderWriter_Memory_h__
