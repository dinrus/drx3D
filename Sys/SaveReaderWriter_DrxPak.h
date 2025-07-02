// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SaveReaderWriter_DrxPak.h
//  Created:     15/02/2010 by Alex McCarthy.
//  Описание: Implementation of the ISaveReader and ISaveWriter
//               interfaces using DrxPak
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SAVE_READER_WRITER_DRXPAK_H__
#define __SAVE_READER_WRITER_DRXPAK_H__

#include <drx3D/CoreX/Platform/IPlatformOS.h>

class CDrxPakFile
{
protected:
	CDrxPakFile(tukk fileName, tukk szMode);
	virtual ~CDrxPakFile();

	IPlatformOS::EFileOperationCode CloseImpl();

	std::vector<u8>              m_data;
	string                          m_fileName;
	FILE*                           m_pFile;
	size_t                          m_filePos;
	IPlatformOS::EFileOperationCode m_eLastError;
	bool                            m_bWriting;
	static i32k                s_dataBlockSize = 128 * 1024;

	NO_INLINE_WEAK static FILE*  FOpen(tukk pName, tukk mode, unsigned nFlags = 0);
	NO_INLINE_WEAK static i32    FClose(FILE* fp);
	NO_INLINE_WEAK static size_t FGetSize(FILE* fp);
	NO_INLINE_WEAK static size_t FReadRaw(uk data, size_t length, size_t elems, FILE* fp);
	NO_INLINE_WEAK static size_t FWrite(ukk data, size_t length, size_t elems, FILE* fp);
	NO_INLINE_WEAK static size_t FSeek(FILE* fp, long offset, i32 mode);

private:
	CScopedAllowFileAccessFromThisThread m_allowFileAccess;
};

class CSaveReader_DrxPak : public IPlatformOS::ISaveReader, public CDrxPakFile
{
public:
	CSaveReader_DrxPak(tukk fileName);

	// ISaveReader
	virtual IPlatformOS::EFileOperationCode Seek(long seek, ESeekMode mode);
	virtual IPlatformOS::EFileOperationCode GetFileCursor(long& fileCursor);
	virtual IPlatformOS::EFileOperationCode ReadBytes(uk data, size_t numBytes);
	virtual IPlatformOS::EFileOperationCode GetNumBytes(size_t& numBytes);
	virtual IPlatformOS::EFileOperationCode Close()           { return CloseImpl(); }
	virtual IPlatformOS::EFileOperationCode LastError() const { return m_eLastError; }
	virtual void                            GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void                            TouchFile();
	//~ISaveReader
};

DECLARE_SHARED_POINTERS(CSaveReader_DrxPak);

class CSaveWriter_DrxPak : public IPlatformOS::ISaveWriter, public CDrxPakFile
{
public:
	CSaveWriter_DrxPak(tukk fileName);

	// ISaveWriter
	virtual IPlatformOS::EFileOperationCode AppendBytes(ukk data, size_t length);
	virtual IPlatformOS::EFileOperationCode Close()           { return CloseImpl(); }
	virtual IPlatformOS::EFileOperationCode LastError() const { return m_eLastError; }
	virtual void                            GetMemoryUsage(IDrxSizer* pSizer) const;
	//~ISaveWriter
};

DECLARE_SHARED_POINTERS(CSaveWriter_DrxPak);

#endif //__SAVE_READER_WRITER_DRXPAK_H__
