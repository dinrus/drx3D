// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Animation/Controller.h>
#include <drx3D/Animation/ControllerPQ.h>
#include <drx3D/Animation/ControllerOpt.h>

class CControllerOptNonVirtual;

struct CInternalDatabaseInfo : public IStreamCallback, public IControllerRelocatableChain
{
	u32                    m_FilePathCRC;
	string                    m_strFilePath;

	bool                      m_bLoadFailed;
	string                    m_strLastError;

	u32                    m_nRelocatableCAFs;
	i32                       m_iTotalControllers;

	CControllerDefragHdl      m_hStorage;
	size_t                    m_nStorageLength;
	CControllerOptNonVirtual* m_pControllersInplace;

	CInternalDatabaseInfo(u32 filePathCRC, const string& strFilePath);
	~CInternalDatabaseInfo();

	void         GetMemoryUsage(IDrxSizer* pSizer) const;

	uk        StreamOnNeedStorage(IReadStream* pStream, unsigned nSize, bool& bAbortOnFailToAlloc);
	void         StreamAsyncOnComplete(IReadStream* pStream, unsigned nError);
	void         StreamOnComplete(IReadStream* pStream, unsigned nError);

	bool         LoadChunks(IChunkFile* pChunkFile, bool bStreaming);

	bool         ReadControllers(IChunkFile::ChunkDesc* pChunkDesc, bool bStreaming);
	bool         ReadController905(IChunkFile::ChunkDesc* pChunkDesc, bool bStreaming);

	void         Relocate(tuk pDst, tuk pSrc);

	static i32 FindFormat(u32 num, std::vector<u32>& vec)
	{
		for (size_t i = 0; i < vec.size(); ++i)
		{
			if (num < vec[i + 1])
				return (i32)i;
		}
		return -1;
	}

private:
	CInternalDatabaseInfo(const CInternalDatabaseInfo&);
	CInternalDatabaseInfo& operator=(const CInternalDatabaseInfo&);
};

struct CGlobalHeaderDBA
{
	friend class CAnimationUpr;

	CGlobalHeaderDBA();
	~CGlobalHeaderDBA();

	void         GetMemoryUsage(IDrxSizer* pSizer) const;
	tukk  GetLastError() const { return m_strLastError.c_str(); }

	void         CreateDatabaseDBA(tukk filename);
	void         LoadDatabaseDBA(tukk sForCharacter);
	bool         StartStreamingDBA(bool highPriority);
	void         CompleteStreamingDBA(CInternalDatabaseInfo* pInfo);

	void         DeleteDatabaseDBA();
	const size_t SizeOf_DBA() const;
	bool         InMemory();
	void         ReportLoadError(tukk sForCharacter, tukk sReason);

	CInternalDatabaseInfo* m_pDatabaseInfo;
	IReadStreamPtr         m_pStream;
	string                 m_strFilePathDBA;
	string                 m_strLastError;
	u32                 m_FilePathDBACRC32;
	u16                 m_nUsedAnimations;
	u16                 m_nEmpty;
	u32                 m_nLastUsedTimeDelta;
	u8                  m_bDBALock;
	bool                   m_bLoadFailed;
};

// mark CGlobalHeaderDBA as moveable, to prevent problems with missing copy-constructors
// and to not waste performance doing expensive copy-constructor operations
template<>
inline bool raw_movable<CGlobalHeaderDBA>(CGlobalHeaderDBA const& dest)
{
	return true;
}
