// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/ResFile.h>

// Prevent type-clash between LZMA and Scaleform
#define UInt32 UInt32_NoOverwrite
#define UInt64 UInt64_NoOverwrite
#include <lzss/LZSS.H>
#include <lzma/Lzma86.h>
static_assert(sizeof(UInt32) == 4 && sizeof(UInt64) == 8, "Bad type sizes");
#undef  UInt32
#undef  UInt64

CResFile CResFile::m_Root("Root");
CResFile CResFile::m_RootStream("RootStream");
i32 CResFile::m_nNumOpenResources;
u32 CResFile::m_nSizeComprDir;
u32 CResFile::m_nMaxOpenResFiles = MAX_OPEN_RESFILES;

namespace
{
DrxCriticalSection g_cResLock;
DrxCriticalSection g_cAsyncResLock;
}

bool CResFile::IsStreaming()
{
	if (m_RootStream.m_NextStream &&
	    m_RootStream.m_NextStream != m_RootStream.m_PrevStream)
		return true;

	return false;
}

// Directory garbage collector (must be executed in render thread)
void CResFile::Tick()
{
	if (!m_RootStream.m_NextStream)
	{
		m_RootStream.m_NextStream = &m_RootStream;
		m_RootStream.m_PrevStream = &m_RootStream;
	}
	//return;

	AUTO_LOCK(g_cAsyncResLock);

	i32 nCurFrame = gRenDev->m_nFrameSwapID;
	u32 nFrameDif = 300; // Release the directories in 300 frames (approx 10 secs)
	CResFile* pRF, * pNext;
	for (pRF = m_RootStream.m_PrevStream; pRF != &m_RootStream; pRF = pNext)
	{
		pNext = pRF->m_PrevStream;
		assert(pRF->m_pStreamInfo);
		if (!pRF->m_pStreamInfo)
		{
			pRF->UnlinkStream();
			continue;
		}
		if (pRF->m_bDirStreaming || pRF->m_pStreamInfo->m_EntriesQueue.size())
			continue; // Still streaming
		if (nCurFrame - pRF->m_nLastTickStreamed > nFrameDif)
		{
			pRF->UnlinkStream();
			pRF->mfReleaseDir();
		}
	}
}

void CResFile::mfTickStreaming()
{
	m_nLastTickStreamed = gRenDev->m_nFrameSwapID;
	UnlinkStream();
	LinkStream(&m_RootStream);
}

void CResFile::mfDeactivate(bool bReleaseDir)
{
	AUTO_LOCK(g_cResLock); // Not thread safe without this

	if (m_handle)
	{
		if (m_typeaccess != RA_READ)
			mfFlush();
		gEnv->pDrxPak->FClose(m_handle);
		m_handle = NULL;
	}

	if (m_bActive)
		m_nNumOpenResources--;
	m_bActive = false;

	Unlink();
}

bool CResFile::mfActivate(bool bFirstTime)
{
	AUTO_LOCK(g_cResLock); // Not thread safe without this

	if (!m_bActive)
	{
		Relink(&m_Root);
		if (m_nNumOpenResources >= (i32)m_nMaxOpenResFiles)
		{
			if (m_nNumOpenResources)
			{
				CResFile* rf = m_Root.m_Prev;
				assert(rf && (rf->m_handle || m_pStreamInfo));
				rf->mfDeactivate(false);
			}
		}

		LOADING_TIME_PROFILE_SECTION(iSystem);
		SCOPED_ALLOW_FILE_ACCESS_FROM_THIS_THREAD();

		i32 nFlags = !m_pLookupDataMan || m_pLookupDataMan->IsReadOnly() ? 0 : IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK;

		// don't open the file if we are trying to stream the data, defeats the idea of streaming it
		if (!m_pStreamInfo)
		{
			if (!bFirstTime && m_szAccess[0] == 'w')
			{
				char szAcc[16];
				drx_strcpy(szAcc, m_szAccess);
				szAcc[0] = 'r';
				m_handle = gEnv->pDrxPak->FOpen(m_name.c_str(), szAcc, nFlags | IDrxPak::FOPEN_HINT_DIRECT_OPERATION);
			}
			else
				m_handle = gEnv->pDrxPak->FOpen(m_name.c_str(), m_szAccess, nFlags | IDrxPak::FOPEN_HINT_DIRECT_OPERATION);
			if (!m_handle)
			{
				mfSetError("CResFile::Activate - Can't open resource file <%s>", m_name.c_str());
				Unlink();
				return false;
			}
		}

		m_nNumOpenResources++;
		m_bActive = true;
	}
	if (!bFirstTime && !m_bDirValid)
		mfPrepareDir();

	return true;
}

CResFile::CResFile(tukk name)
	: m_name(name)
	, m_szAccess(nullptr)
	, m_handle(nullptr)
	, m_pCompressedDir(nullptr)
	, m_typeaccess(0)
	, m_nNumFilesUnique(0)
	, m_nOffsDir(0)
	, m_nComprDirSize(0)
	, m_nOffset(OFFSET_BIG_POSITIVE)
	, m_bSwapEndianRead(false)
	, m_bSwapEndianWrite(false)
	, m_bDirty(false)
	, m_bDirValid(false)
	, m_bDirStreaming(false)
	, m_bDirCompressed(false)
	, m_bActive(false)
	, m_nLastTickStreamed(0)
	, m_version(0)
	, m_pStreamInfo(nullptr)
	, m_pLookupData(nullptr)
	, m_pLookupDataMan(nullptr)
	, m_Next(nullptr)
	, m_Prev(nullptr)
	, m_NextStream(nullptr)
	, m_PrevStream(nullptr)
{
	if (!m_Root.m_Next)
	{
		m_Root.m_Next = &m_Root;
		m_Root.m_Prev = &m_Root;
		m_RootStream.m_NextStream = &m_RootStream;
		m_RootStream.m_PrevStream = &m_RootStream;
	}
}

CResFile::~CResFile()
{
	if (this != &m_Root && this != &m_RootStream)
	{
		mfClose();
	}
	else
	{
		for (i32 i = 0; i < 2; ++i)
		{
			const CResFile* pCurrent = i ? m_Next : m_NextStream;
			while (true)
			{
				if (!pCurrent || pCurrent == this)
				{
					break;
				}
				DRX_ASSERT_TRACE(false, ("CResFile - File still open at shutdown: %s (streaming: %s)", pCurrent->mfGetFileName(), i ? "no" : "yes"));
				pCurrent = i ? pCurrent->m_Next : pCurrent->m_NextStream;
			}
		}
	}
}

void CResFile::mfSetError(tukk er, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, er);
	drx_vsprintf(buffer, er, args);
	m_ermes = buffer;
	va_end(args);
}

SResFileLookupData* CResFile::GetLookupData() const
{
	if (m_pLookupDataMan)
	{
		CDrxNameTSCRC name = m_pLookupDataMan->AdjustName(m_name.c_str());
		SResFileLookupData* pData = m_pLookupDataMan->GetData(name);
		return pData;
	}

	return nullptr;
}

void CResFile::StoreLookupData(u32 CRC, float fVersion)
{
	if (m_pLookupDataMan)
	{
		CDrxNameTSCRC name = m_pLookupDataMan->AdjustName(m_name.c_str());

		m_pLookupDataMan->AddData(this, CRC);
		SResFileLookupData* pData = m_pLookupDataMan->GetData(name);
		m_pLookupDataMan->MarkDirty(true);

		DRX_ASSERT(pData);
	}
}

tukk CResFile::mfGetError(void)
{
	if (m_ermes.size())
		return m_ermes.c_str();
	else
		return NULL;
}

i32 CResFile::mfGetResourceSize()
{
	if (!m_handle)
		return 0;

	AUTO_LOCK(g_cResLock); // Not thread safe without this

	gEnv->pDrxPak->FSeek(m_handle, 0, SEEK_END);
	i32 length = gEnv->pDrxPak->FTell(m_handle);
	gEnv->pDrxPak->FSeek(m_handle, 0, SEEK_SET);

	return length;
}

uint64 CResFile::mfGetModifTime()
{
	if (!mfActivate(false))
		return 0;

	if (!m_handle)
		return 0;

	return gEnv->pDrxPak->GetModificationTime(m_handle);
}

bool CResFile::mfFileExist(CDrxNameTSCRC name)
{
	CDirEntry* de = mfGetEntry(name);
	if (!de)
		return false;
	assert(name == de->Name);
	return true;
}

bool CResFile::mfFileExist(tukk name)
{
	return mfFileExist(CDrxNameTSCRC(name));
}

void CResStreamDirCallback::StreamAsyncOnComplete(IReadStream* pStream, unsigned nError)
{
	/*
	   SResStreamInfo *pStreamInfo = (SResStreamInfo*)pStream->GetUserData();
	   assert(pStreamInfo);
	   if (!pStreamInfo)
	   return;
	   SShaderCache *pCache = pStreamInfo->m_pCache;
	   CResFile *pRes = pStreamInfo->m_pRes;
	   IF(!pRes,0)return;
	   assert(pRes->m_bDirStreaming);

	   if (nError == )
	   pStreamInfo->m_nRequestCount--;

	   // are all requests processed ?
	   if (pStreamInfo->m_nRequestCount == 0)
	   {
	   // check if both requests were valid !
	   pRes->m_bDirValid = true;
	   }
	 */
}

void CResStreamDirCallback::StreamOnComplete(IReadStream* pStream, unsigned nError)
{
	SResStreamInfo* pStreamInfo = (SResStreamInfo*)pStream->GetUserData();
	assert(pStreamInfo);
	if (!pStreamInfo)
		return;

	DrxAutoLock<DrxCriticalSection> lock(pStreamInfo->m_StreamLock);

	CResFile* pRes = pStreamInfo->m_pRes;
	IF (!pRes, 0) return;
	assert(pRes->m_bDirStreaming);

	if (!nError)
		pStreamInfo->m_nDirRequestCount--;

	for (std::vector<IReadStreamPtr>::iterator it = pStreamInfo->m_dirReadStreams.begin();
	     it != pStreamInfo->m_dirReadStreams.end(); ++it)
	{
		if (pStream == *it)
		{
			pStreamInfo->m_dirReadStreams.erase(it);
			break;
		}
	}

	// all requests processed ?
	if (pStreamInfo->m_dirReadStreams.size() == 0)
	{
		// were all requests processed successfully
		if (pStreamInfo->m_nDirRequestCount == 0)
		{
			// check if both requests were valid !
			pRes->m_bDirValid = true;
		}

		pRes->m_bDirStreaming = false;
	}
}

i32 CResFile::mfLoadDir(SResStreamInfo* pStreamInfo)
{
	i32 nRes = 1;
	if (pStreamInfo)
	{
		// if we are streaming the data, we need the lookup data to be valid!
		if (m_pLookupData == NULL)
			return -1;

		mfTickStreaming();
		if (m_bDirStreaming)
			return -1;
		m_bDirStreaming = true;

		i32 nSizeDir = m_nNumFilesUnique * sizeof(CDirEntry);

		if (nSizeDir)
		{
			m_Dir.resize(m_nNumFilesUnique);

			StreamReadParams StrParams;
			StrParams.nFlags = 0;
			StrParams.dwUserData = (DWORD_PTR)pStreamInfo;
			StrParams.nLoadTime = 1;
			StrParams.nMaxLoadTime = 4;
			StrParams.pBuffer = &m_Dir[0];
			StrParams.nOffset = m_nOffsDir;
			StrParams.nSize = nSizeDir;

			DrxAutoLock<DrxCriticalSection> lock(pStreamInfo->m_StreamLock);
			pStreamInfo->m_dirReadStreams.push_back(iSystem->GetStreamEngine()->StartRead(
			                                          eStreamTaskTypeShader, m_name.c_str(), &pStreamInfo->m_CallbackDir, &StrParams));
			pStreamInfo->m_nDirRequestCount++;
		}

		nRes = -1;
	}
	else
	{
		if (gEnv->pDrxPak->FSeek(m_handle, m_nOffsDir, SEEK_SET) > 0)
		{
			mfSetError("Open - Directory reading error");
			return 0;
		}

		i32 nSize = m_nNumFilesUnique * sizeof(CDirEntry);
		//i32 nSizeDir = m_nComprDirSize ? m_nComprDirSize : nSize;
		if (m_nNumFilesUnique)
		{
			m_Dir.resize(m_nNumFilesUnique);
			if (gEnv->pDrxPak->FReadRaw(&m_Dir[0], 1, nSize, m_handle) != nSize)
			{
				mfSetError("Open - Directory reading error");
				m_Dir.clear();
				return 0;
			}
		}
	}
	m_bDirValid = false;
	if (!m_nComprDirSize && nRes == 1)
	{
		m_bDirValid = true;
		if (m_bSwapEndianRead)
		{
			if (m_nNumFilesUnique)
				SwapEndian(&m_Dir[0], (size_t)m_nNumFilesUnique, eBigEndian);
		}
	}
	return nRes;
}

bool CResFile::mfPrepareDir()
{
	if (m_bDirValid)
		return true;
	assert(!m_Dir.size());
	CDirEntry* pFileDir = NULL;
	if (m_pCompressedDir)
	{
		assert(!m_Dir.size());
		pFileDir = new CDirEntry[m_nNumFilesUnique];
		if (m_version == RESVERSION_LZSS)
		{
			if (!Decodem(m_pCompressedDir, (byte*)pFileDir, m_nComprDirSize, sizeof(CDirEntry) * m_nNumFilesUnique))
			{
				DrxFatalError("Compressed res file directory corrupt!");
			}
		}
		else if (m_version == RESVERSION_LZMA)
		{
			SizeT size = sizeof(CDirEntry) * m_nNumFilesUnique;
			SizeT inSize = m_nComprDirSize;
			i32 res = Lzma86_Decode((byte*)pFileDir, &size, m_pCompressedDir, &inSize);
		}
		else if (m_version == RESVERSION_DEBUG)
		{
			memcpy(pFileDir, m_pCompressedDir, sizeof(CDirEntry) * m_nNumFilesUnique);
		}
		else
		{
			DrxFatalError("Bad Version: %d!", m_version);
		}
		m_Dir.resize(m_nNumFilesUnique);
		for (u32 i = 0; i < m_nNumFilesUnique; i++)
		{
			CDirEntry* deS = &m_Dir[i];
			CDirEntry& fdent = pFileDir[i];
			if (m_bSwapEndianRead)
				SwapEndian(fdent, eBigEndian);
			deS->Name = fdent.Name;
			deS->size = fdent.size;
			deS->offset = fdent.offset;
			deS->flags = fdent.flags;
		}
		m_nSizeComprDir += m_nComprDirSize;
		m_bDirValid = true;
	}
	else
	{
		i32 nRes = mfLoadDir(m_pStreamInfo);
		assert(nRes);
	}

	return true;
}

void CResFile::mfReleaseDir()
{
	// Never unload directory which wasn't flushed yet
	if (m_bDirty)
		return;
	if (m_bDirStreaming)
		return;
	if (m_pStreamInfo && m_pStreamInfo->m_EntriesQueue.size())
		return;

	if (m_bDirValid)
	{
		u32 i;
		for (i = 0; i < m_Dir.size(); i++)
		{
			CDirEntry* de = &m_Dir[i];
			assert(!(de->flags & RF_NOTSAVED));
			mfCloseEntry(de->GetName(), de->GetFlags(), false);
		}

		m_DirOpen.clear();
		//m_Dir.clear();
		ResDir r;
		m_Dir.swap(r);

		m_bDirValid = false;
	}
	else
	{
		assert(!m_Dir.size());
	}
}

i32 CResFile::mfOpen(i32 type, CResFileLookupDataMan* pMan, SResStreamInfo* pStreamInfo)
{
	SFileResHeader frh;
	CDirEntry fden;

	PROFILE_FRAME(Resource_Open);

	if (m_name.c_str()[0] == 0)
	{
		mfSetError("Open - No Resource name");
		return 0;
	}
	i32 nRes = 1;
	m_bSwapEndianWrite = (type & RA_ENDIANS) != 0;
	m_bSwapEndianRead = m_bSwapEndianWrite;
	m_pLookupDataMan = pMan;
	type &= ~RA_ENDIANS;
	if (type == RA_READ)
		m_szAccess = "rb";
	else if (type == (RA_WRITE | RA_READ))
		m_szAccess = "r+b";
	else if (type & RA_CREATE)
		m_szAccess = "w+b";
	else
	{
		mfSetError("Open - Wrong access mode");
		return 0;
	}
	m_typeaccess = type;

	if (type & RA_READ)
		m_pStreamInfo = pStreamInfo;

	mfActivate(true);

	AUTO_LOCK(g_cResLock); // Not thread safe without this

	if (!m_bActive)
	{
		if (type & (RA_WRITE | RA_CREATE))
		{
			char name[256];
			gEnv->pDrxPak->AdjustFileName(m_name.c_str(), name, 0);
			FILE* statusdst = fopen(name, "rb");
			if (statusdst)
			{
				/*
				   #if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
				        struct stat64 st;
				   #else
				        struct __stat64 st;
				   #endif
				        i32 result = _fstat64(fileno(statusdst), &st);
				        if (result == 0)
				        {
				   #if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
				          if (st.st_mode & FILE_ATTRIBUTE_READONLY)
				            chmod(name, 0x777);//set to full access
				   #else
				          if (!(st.st_mode & _S_IWRITE))
				            _chmod(name, _S_IREAD | _S_IWRITE);
				   #endif
				        }
				 */
				fclose(statusdst);
				m_ermes.clear();

				DrxSetFileAttributes(name, FILE_ATTRIBUTE_ARCHIVE);

				mfActivate(true);
			}
		}
		if (!m_handle)
		{
			mfSetError("Open - Can't open resource file <%s>", m_name.c_str());
			return 0;
		}
	}

	if (type & RA_READ)
	{
		// check the preloaded dir data, to see if we can get the dir data from there
		CDrxNameTSCRC name = m_pLookupDataMan->AdjustName(m_name.c_str());
		if (m_pLookupDataMan)
			m_pLookupData = m_pLookupDataMan->GetData(name);
		if (m_pLookupData)
		{
			m_version = m_pLookupDataMan->GetResVersion();
			m_nNumFilesUnique = m_pLookupData->m_NumOfFilesUnique;
			m_nOffsDir = m_pLookupData->m_OffsetDir;
			m_nComprDirSize = 0;//m_pResDirData->size_dir;
		}
		else
		{
			// make sure lookupdata is available when we are streaming the data
			if (!m_handle)
			{
				mfSetError("Open - no file handle (lookupdata not found, while streaming data?)");
				return 0;
			}

			// Detect file endianness automatically.
			if (gEnv->pDrxPak->FReadRaw(&frh, 1, sizeof(frh), m_handle) != sizeof(frh))
			{
				mfSetError("Open - Reading fault");
				return 0;
			}
			if (m_bSwapEndianRead)
				SwapEndian(frh, eBigEndian);
			if (frh.hid != IDRESHEADER)
			{
				mfSetError("Open - Wrong header MagicID");
				return 0;
			}
			if (frh.ver != RESVERSION_LZSS && frh.ver != RESVERSION_LZMA && frh.ver != RESVERSION_DEBUG)
			{
				mfSetError("Open - Wrong version number");
				return 0;
			}
			m_version = frh.ver;
			if (!frh.num_files)
			{
				mfSetError("Open - Empty resource file");
				return 0;
			}

			m_nNumFilesUnique = frh.num_files;
			m_nOffsDir = frh.ofs_dir;
			m_nComprDirSize = 0; //frh.size_dir;
		}

		if (pStreamInfo)
			m_pStreamInfo->m_pRes = this;

		m_bDirCompressed = false;
		nRes = mfLoadDir(pStreamInfo);
	}
	else
	{
		frh.hid = IDRESHEADER;
		i32 ver = RES_COMPRESSION;
		frh.ver = ver;
		frh.num_files = 0;
		frh.ofs_dir = -1;
		m_version = ver;
		m_nOffsDir = sizeof(frh);
		SFileResHeader frhTemp, * pFrh;
		pFrh = &frh;
		if (m_bSwapEndianWrite)
		{
			frhTemp = frh;
			SwapEndian(frhTemp, eBigEndian);
			pFrh = &frhTemp;
		}
		if (gEnv->pDrxPak->FWrite(pFrh, 1, sizeof(frh), m_handle) != sizeof(frh))
		{
			mfSetError("Open - Writing fault");
			return 0;
		}
		m_nComprDirSize = 0;
		m_bDirCompressed = false;
		m_nNumFilesUnique = 0;
		m_pCompressedDir = NULL;
		m_bDirValid = true;
	}

	return nRes;
}

bool CResFile::mfClose(void)
{
	AUTO_LOCK(g_cResLock); // Not thread safe without this

	assert(!m_bDirStreaming);
	assert(!m_pStreamInfo || !m_pStreamInfo->m_EntriesQueue.size());

	//if (m_bDirStreaming || (m_pStreamInfo && m_pStreamInfo->m_EntriesQueue.size()))
	//  Warning("Warning: CResFile::mfClose: Streaming task is in progress!");
	UnlinkStream();

	if (m_typeaccess != RA_READ)
		mfFlush();

	// Close the handle and release directory
	mfDeactivate(true);
	assert(!m_bDirty);
	mfReleaseDir();

	SAFE_DELETE_ARRAY(m_pCompressedDir);

	return true;
}

struct ResDirSortByName
{
	bool operator()(const CDirEntry& left, const CDirEntry& right) const
	{
		return left.GetName() < right.GetName();
	}
	bool operator()(const CDrxNameTSCRC left, const CDirEntry& right) const
	{
		return left < right.GetName();
	}
	bool operator()(const CDirEntry& left, CDrxNameTSCRC right) const
	{
		return left.GetName() < right;
	}
};

struct ResDirOpenSortByName
{
	bool operator()(const SDirEntryOpen& left, const SDirEntryOpen& right) const
	{
		return left.Name < right.Name;
	}
	bool operator()(const CDrxNameTSCRC left, const SDirEntryOpen& right) const
	{
		return left < right.Name;
	}
	bool operator()(const SDirEntryOpen& left, CDrxNameTSCRC right) const
	{
		return left.Name < right;
	}
};

SDirEntryOpen* CResFile::mfGetOpenEntry(const CDrxNameTSCRC &Name)
{
	ResDirOpenIt it = std::lower_bound(m_DirOpen.begin(), m_DirOpen.end(), Name, ResDirOpenSortByName());
	if (it != m_DirOpen.end() && Name == (*it).Name)
		return &(*it);
	return NULL;
}
SDirEntryOpen* CResFile::mfOpenEntry(const CDrxNameTSCRC &Name)
{
	SDirEntryOpen* pOE = NULL;
	ResDirOpenIt it = std::lower_bound(m_DirOpen.begin(), m_DirOpen.end(), Name, ResDirOpenSortByName());
	if (it == m_DirOpen.end() || Name != (*it).Name)
	{
		AUTO_LOCK(g_cAsyncResLock);
		SDirEntryOpen OE;
		OE.Name = Name;
		OE.curOffset = 0;
		OE.pData = NULL;
		it = m_DirOpen.insert(it, OE);
		return &(*it);
	}
	pOE = &(*it);
	pOE->curOffset = 0;
	//assert(pOE->pData);
	//SAFE_DELETE_ARRAY(pOE->pData);

	return pOE;
}
bool CResFile::mfCloseEntry(const CDrxNameTSCRC &Name, u32 flags, bool bEraseOpenEntry)
{
	ResDirOpenIt it = std::lower_bound(m_DirOpen.begin(), m_DirOpen.end(), Name, ResDirOpenSortByName());
	if (it == m_DirOpen.end() || Name != (*it).Name)
		return false;
	SDirEntryOpen& OE = (*it);
	OE.curOffset = 0;
	if (flags & RF_TEMPDATA)
	{
		if (OE.pData)
		{
			delete[] (tuk)OE.pData;
			OE.pData = NULL;
		}
	}
	if (bEraseOpenEntry)
	{
		AUTO_LOCK(g_cAsyncResLock);
		m_DirOpen.erase(it);
	}

	return true;
}

CDirEntry* CResFile::mfGetEntry(const CDrxNameTSCRC &name, bool* pAsync)
{
	if (pAsync)
	{
		*pAsync = m_bDirStreaming;
		if (m_bDirStreaming)
			return 0;
	}

	if (!m_Dir.size() || m_bDirStreaming)
	{
		if (!mfActivate(false))
			return NULL;
		if (!m_Dir.size() || m_bDirStreaming)
		{
			if (pAsync && m_bDirStreaming)
				*pAsync = true;
			return NULL;
		}
	}

	ResDirIt it = std::lower_bound(m_Dir.begin(), m_Dir.end(), name, ResDirSortByName());
	if (it != m_Dir.end() && name == (*it).Name)
	{
		assert(m_bDirValid);
		return &(*it);
	}

	return NULL;
}

i32 CResFile::mfFileClose(const CDrxNameTSCRC &Name, u32 flags)
{
	if (!(flags & RF_NOTSAVED))
	{
		mfCloseEntry(Name, flags);
	}

	return 0;
}
i32 CResFile::mfFileAdd(CDirEntry* de)
{
	AUTO_LOCK(g_cResLock); // Not thread safe without this

	assert(!m_pStreamInfo);

	if (m_typeaccess == RA_READ)
	{
		mfSetError("FileAdd - wrong access mode");
		return 0;
	}
	CDrxNameTSCRC name = de->Name;
	ResDirIt it = std::lower_bound(m_Dir.begin(), m_Dir.end(), name, ResDirSortByName());
	if (it != m_Dir.end() && name == (*it).Name)
		return m_Dir.size();

	if (de->offset == 0)
		de->offset = m_nOffset++;

	if (de->size != 0)
	{
		if (!m_Dir.size())
			mfActivate(false);

		it = std::lower_bound(m_Dir.begin(), m_Dir.end(), name, ResDirSortByName());
		if (it != m_Dir.end() && name == (*it).Name)
			return m_Dir.size();

		m_Dir.emplace(it, de->GetName(), de->GetSize(), de->GetOffset(), de->GetFlags() | RF_NOTSAVED);
		m_bDirty = true;
	}
	return m_Dir.size();
}

// Lets unpack the entry asynchronously
void CResStreamCallback::StreamAsyncOnComplete(IReadStream* pStream, unsigned nError)
{
	SResStreamEntry* pEntry = (SResStreamEntry*)pStream->GetUserData();
	SResStreamInfo* pStreamInfo = pEntry->m_pParent;
	assert(pStreamInfo);
	if (!pStreamInfo)
		return;
	//DrxHeapCheck();
	CResFile* pRes = pStreamInfo->m_pRes;
	assert(pRes->m_bDirValid);

	if (nError)
	{
		pRes->mfSetError("FileRead - Error during streaming data");
		return;
	}

	CDirEntry* pDE = pRes->mfGetEntry(pEntry->m_Name);
	byte* pBuf = (byte*)pStream->GetBuffer();
	assert(pBuf);
	byte* pData = NULL;

	i32 size;
	if (pDE->GetFlags() & RF_COMPRESS)
	{
		assert(pRes->m_version == RESVERSION_LZSS);
		size = *(i32*)pBuf;
		if (size < 10000000)
		{
			pData = new byte[size];
			if (!pData)
				pRes->mfSetError("FileRead - Allocation fault");
			else if (!Decodem(&pBuf[4], (byte*)pData, pDE->GetSize() - 4, size))
				pRes->mfSetError("FileRead - Decode fault");
		}
	}
	else
	{
		size = pDE->GetSize();
		pData = new byte[size];
		if (!pData)
			pRes->mfSetError("FileRead - Allocation fault");
		else
			memcpy(pData, pBuf, size);
	}
	{
		AUTO_LOCK(g_cAsyncResLock);

		SDirEntryOpen* pOE = pRes->mfGetOpenEntry(pDE->GetName());
		if (pOE)
		{
			pDE->MarkTemp();
			pOE->nSize = size;
			pOE->pData = pData;
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "mfGetOpenEntry() returned NULL, possibly because r_shadersAllowCompilation=1 and r_shadersAsyncActivation=1. Try r_shadersAsyncActivation=0 in your user.cfg.");
		}
	}
}
// Release the data synchronously
void CResStreamCallback::StreamOnComplete(IReadStream* pStream, unsigned nError)
{
	SResStreamEntry* pEntry = (SResStreamEntry*)pStream->GetUserData();
	SResStreamInfo* pStreamInfo = pEntry->m_pParent;
	assert(pStreamInfo);
	if (!pStreamInfo)
		return;
	u32 i;
	bool bFound = false;

	DrxAutoLock<DrxCriticalSection> lock(pStreamInfo->m_StreamLock);

	//DrxHeapCheck();
	if (!nError)
	{
		for (i = 0; i < pStreamInfo->m_EntriesQueue.size(); i++)
		{
			SResStreamEntry* pE = pStreamInfo->m_EntriesQueue[i];
			if (pE == pEntry)
			{
				SAFE_DELETE(pE);
				pStreamInfo->m_EntriesQueue.erase(pStreamInfo->m_EntriesQueue.begin() + i);
				bFound = true;
				break;
			}
		}
		assert(bFound);
	}
}

i32 CResFile::mfFileRead(CDirEntry* de)
{
	u32 size = 0;

	SDirEntryOpen* pOE = mfOpenEntry(de->GetName());

	if (pOE->pData)
		return pOE->nSize;

	if (!mfActivate(false))
		return 0;

	AUTO_LOCK(g_cResLock); // Not thread safe without this

	//if (strstr(m_name, "FPVS") && m_Dir.size()==3)
	{
		//i32 nnn = 0;
	}
	if (m_pStreamInfo)
	{
		mfTickStreaming();
		if (!m_bDirValid)
		{
			assert(m_bDirStreaming);
			return -1;
		}

		DrxAutoLock<DrxCriticalSection> lock(m_pStreamInfo->m_StreamLock);

		SResStreamEntry* pEntry;
		{
			//AUTO_LOCK(g_cAsyncResLock);
			pEntry = m_pStreamInfo->AddEntry(de->Name);
			if (!pEntry)  // Already processing
				return -1;
		}
		if (pOE->pData)
			return pOE->nSize;
		StreamReadParams StrParams;
		StrParams.nFlags = 0;
		StrParams.dwUserData = (DWORD_PTR)pEntry;
		StrParams.nLoadTime = 1;
		StrParams.nMaxLoadTime = 4;
		//    StrParams.nPriority = 0;
		StrParams.pBuffer = NULL;
		StrParams.nOffset = de->offset;
		StrParams.nSize = de->size;
		//Warning("Warning: CResFile::mfFileRead: '%s' Ref: %d", m_name.c_str(), n);
		pEntry->m_readStream = iSystem->GetStreamEngine()->StartRead(eStreamTaskTypeShader, m_name.c_str(), &m_pStreamInfo->m_Callback, &StrParams);
		return -1;
	}
	else if (de->flags & RF_COMPRESS)
	{
		if (gEnv->pDrxPak->FSeek(m_handle, de->offset, SEEK_SET) > 0)
		{
			mfSetError("FileRead - Seek error");
			return 0;
		}

		byte* buf = new byte[de->size];
		if (!buf)
		{
			mfSetError("FileRead - Allocation fault");
			return 0;
		}
		if (m_version == RESVERSION_LZSS)
		{
			gEnv->pDrxPak->FReadRaw(buf, de->size, 1, m_handle);
			size = *(i32*)buf;
			if (m_bSwapEndianRead)
				SwapEndian(size, eBigEndian);
			if (size >= 10000000)
			{
				SAFE_DELETE_ARRAY(buf);
				return 0;
			}
			pOE->pData = new byte[size];
			de->MarkTemp();
			if (!pOE->pData)
			{
				SAFE_DELETE_ARRAY(buf);
				mfSetError("FileRead - Allocation fault");
				return 0;
			}
			if (!Decodem(&buf[4], (byte*)pOE->pData, de->size - 4, size))
			{
				SAFE_DELETE_ARRAY(buf);
				mfSetError("FileRead - Decodem fault");
				return 0;
			}
		}
		else if (m_version == RESVERSION_LZMA)
		{
			SizeT inSize = de->size;
			gEnv->pDrxPak->FReadRaw(buf, inSize, 1, m_handle);
			uint64 outSize64;
			if (Lzma86_GetUnpackSize(buf, inSize, &outSize64) != 0)
			{
				delete[] buf;
				mfSetError("FileRead - data error");
				return 0;
			}
			SizeT outSize = (u32)outSize64;
			if (outSize != 0)
			{
				u8* outBuffer = (u8*)MyAlloc(outSize);
				if (outBuffer == 0)
				{
					delete[] buf;
					mfSetError("FileRead - can't allocate");
					return 0;
				}
				SRes res = Lzma86_Decode(outBuffer, &outSize, buf, &inSize);
				if (res != 0)
				{
					delete[] buf;
					mfSetError("FileRead - LzmaDecoder error");
					return 0;
				}
				size = outSize;
				pOE->pData = outBuffer;
				de->MarkTemp();
			}
		}
		else if (m_version == RESVERSION_DEBUG)
		{
			gEnv->pDrxPak->FReadRaw(buf, de->size, 1, m_handle);
			i32 allocSize = (i32)de->size - 20;
			if (allocSize < 0 || allocSize > 128*1024*1024)
			{
				mfSetError("FileRead - Corrupt DirEntiy size");
				SAFE_DELETE_ARRAY(buf);
				return 0;
			}
			pOE->pData = new byte[allocSize];
			de->MarkTemp();
			memcpy(pOE->pData, &buf[10], allocSize);
		}
		else
		{
			DrxFatalError("Bad Version: %d!", m_version);
			return 0;
		}

		delete[] buf;

		pOE->nSize = size;
		return size;
	}

	pOE->pData = new byte[de->size];
	pOE->nSize = de->size;
	de->MarkTemp();
	if (!pOE->pData)
	{
		mfSetError("FileRead - Allocation fault");
		return 0;
	}

	if (!m_handle)
	{
		mfSetError("FileRead - Invalid file handle");
		return 0;
	}

	if (gEnv->pDrxPak->FSeek(m_handle, de->offset, SEEK_SET) > 0)
	{
		mfSetError("FileRead - Seek error");
		return 0;
	}

	if (gEnv->pDrxPak->FReadRaw(pOE->pData, 1, de->size, m_handle) != de->size)
	{
		mfSetError("FileRead - Reading fault");
		return 0;
	}

	//if (strstr(m_name, "FPVS") && m_Dir.size()==3)
	{
		//gEnv->pDrxPak->FSeek(m_handle, 0, SEEK_END);
		//i32 nSize = gEnv->pDrxPak->FTell(m_handle);
		//i32 nnn = 0;
	}

	return de->size;
}

byte* CResFile::mfFileReadCompressed(CDirEntry* de, u32& nSizeDecomp, u32& nSizeComp)
{
	if (!mfActivate(false))
		return nullptr;

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Shader, 0, "Shaders File Read Compressed");

	if (!m_handle)
	{
		mfSetError("FileReadCompressed - Invalid file handle");
		return 0;
	}

	if (de->flags & RF_COMPRESS)
	{
		if (de->offset >= 0x10000000)
			return nullptr;

		if (gEnv->pDrxPak->FSeek(m_handle, de->offset, SEEK_SET) > 0)
		{
			mfSetError("FileReadCompressed - Seek error");
			return nullptr;
		}

		byte* buf = new byte[de->size];
		if (!buf)
		{
			mfSetError("FileRead - Allocation fault");
			return nullptr;
		}
		if (m_version == RESVERSION_LZSS)
		{
			i32 nSize = -1;
			gEnv->pDrxPak->FReadRaw(&nSize, 1, sizeof(i32), m_handle);
			if (m_bSwapEndianRead)
				SwapEndian(nSize, eBigEndian);
			if (nSize <= 0)
			{
				assert(0);
				SAFE_DELETE_ARRAY(buf);
				return nullptr;
			}
			nSizeDecomp = nSize;
			nSizeComp = de->size - 4;
			gEnv->pDrxPak->FReadRaw(buf, nSizeComp, 1, m_handle);
		}
		else if (m_version == RESVERSION_LZMA)
		{
			u32 inSize = de->size;
			gEnv->pDrxPak->FReadRaw(buf, inSize, 1, m_handle);
			uint64 outSize64;
			if (Lzma86_GetUnpackSize(buf, inSize, &outSize64) != 0)
			{
				mfSetError("FileRead - data error");
				return nullptr;
			}
			nSizeDecomp = (u32)outSize64;
			nSizeComp = inSize;
		}
		else if (m_version == RESVERSION_DEBUG)
		{
			gEnv->pDrxPak->FReadRaw(buf, 10, 1, m_handle);
			gEnv->pDrxPak->FReadRaw(buf, de->size - 20, 1, m_handle);
			nSizeDecomp = de->size - 20;
			nSizeComp = de->size - 20;
		}
		else
		{
			DrxFatalError("Bad Version: %d!", m_version);
			return nullptr;
		}
		return buf;
	}

	nSizeComp = nSizeDecomp = de->size;
	byte* buf = new byte[de->size];

	if (gEnv->pDrxPak->FSeek(m_handle, de->offset, SEEK_SET) > 0)
	{
		mfSetError("FileReadCompressed - Seek error");
		delete[] buf;
		return nullptr;
	}

	if (gEnv->pDrxPak->FReadRaw(buf, 1, de->size, m_handle) != de->size)
	{
		mfSetError("FileRead - Reading fault");
		delete[] buf;
		return nullptr;
	}
	return buf;
}

i32 CResFile::mfFileRead(CDrxNameTSCRC name)
{
	CDirEntry* de = mfGetEntry(name);

	if (!de)
	{
		mfSetError("FileRead - Wrong FileId");
		return 0;
	}
	return mfFileRead(de);
}

i32 CResFile::mfFileRead(tukk name)
{
	return mfFileRead(CDrxNameTSCRC(name));
}

i32 CResFile::mfFileWrite(CDrxNameTSCRC name, uk data)
{
	CDirEntry* de = mfGetEntry(name);

	if (!de)
	{
		mfSetError("FileWrite - Wrong FileId");
		return 0;
	}
	if (!data)
	{
		mfSetError("FileWrite - Wrong data");
		return 0;
	}

	if (!mfActivate(false))
		return 0;

	if (de->flags & RF_COMPRESS)
	{
		assert(0);
		return 0;
	}

	if (!m_handle)
	{
		mfSetError("FileWrite - Invalid file handle");
		return 0;
	}

	if (gEnv->pDrxPak->FSeek(m_handle, de->offset, SEEK_SET) > 0)
	{
		mfSetError("FileWrite - Seek error");
		return 0;
	}

	if (gEnv->pDrxPak->FWrite(data, 1, de->size, m_handle) != de->size)
	{
		mfSetError("FileWrite - Writing fault");
		return 0;
	}

	return de->size;
}

void CResFile::mfFileRead2(CDirEntry* de, i32 size, uk buf)
{
	if (!buf)
	{
		mfSetError("FileRead - Wrong data");
		return;
	}
	SDirEntryOpen* pOE = mfOpenEntry(de->GetName());

	if (pOE->pData)
	{
		memcpy(buf, (byte*)(pOE->pData) + pOE->curOffset, size);
		pOE->curOffset += size;
		return;
	}
	if (!mfActivate(false))
		return;

	if (!m_handle)
	{
		mfSetError("FileRead2 - Invalid file handle");
		return;
	}

	if (gEnv->pDrxPak->FSeek(m_handle, de->offset + pOE->curOffset, SEEK_SET) > 0)
	{
		mfSetError("FileRead2 - Seek error");
		return;
	}

	if (gEnv->pDrxPak->FReadRaw(buf, 1, size, m_handle) != size)
	{
		mfSetError("FileRead - Reading fault");
		return;
	}
	pOE->curOffset += size;
}

void CResFile::mfFileRead2(CDrxNameTSCRC name, i32 size, uk buf)
{
	CDirEntry* de = mfGetEntry(name);
	if (!de)
	{
		mfSetError("FileRead2 - wrong file id");
		return;
	}
	return mfFileRead2(de, size, buf);
}

uk CResFile::mfFileGetBuf(CDirEntry* de)
{
	SDirEntryOpen* pOE = mfGetOpenEntry(de->GetName());
	if (!pOE)
		return NULL;
	return pOE->pData;
}

uk CResFile::mfFileGetBuf(CDrxNameTSCRC name)
{
	CDirEntry* de = mfGetEntry(name);
	if (!de)
	{
		mfSetError("FileGetBuf - wrong file id");
		return NULL;
	}
	return mfFileGetBuf(de);
}

i32 CResFile::mfFileSeek(CDirEntry* de, i32 ofs, i32 type)
{
	i32 m;

	mfActivate(false);

	if (!m_handle)
	{
		mfSetError("FileSeek - Invalid file handle");
		return -1;
	}

	AUTO_LOCK(g_cResLock); // Not thread safe without this

	SDirEntryOpen* pOE = mfOpenEntry(de->GetName());

	switch (type)
	{
	case SEEK_CUR:
		pOE->curOffset += ofs;
		m = gEnv->pDrxPak->FSeek(m_handle, de->offset + pOE->curOffset, SEEK_SET);
		break;

	case SEEK_SET:
		m = gEnv->pDrxPak->FSeek(m_handle, de->offset + ofs, SEEK_SET);
		pOE->curOffset = ofs;
		break;

	case SEEK_END:
		pOE->curOffset = de->size - ofs;
		m = gEnv->pDrxPak->FSeek(m_handle, de->offset + pOE->curOffset, SEEK_SET);
		break;

	default:
		mfSetError("FileSeek - wrong seek type");
		return -1;
	}

	return m;
}

i32 CResFile::mfFileSeek(CDrxNameTSCRC name, i32 ofs, i32 type)
{
	CDirEntry* de = mfGetEntry(name);

	if (!de)
	{
		mfSetError("FileSeek - invalid file id");
		return -1;
	}
	return mfFileSeek(de, ofs, type);
}

i32 CResFile::mfFileSeek(tuk name, i32 ofs, i32 type)
{
	return mfFileSeek(CDrxNameTSCRC(name), ofs, type);
}

i32 CResFile::mfFileLength(CDirEntry* de)
{
	return de->size;
}

i32 CResFile::mfFileLength(CDrxNameTSCRC name)
{
	CDirEntry* de = mfGetEntry(name);

	if (!de)
	{
		mfSetError("FileLength - invalid file id");
		return -1;
	}
	return mfFileLength(de);
}

i32 CResFile::mfFileLength(tuk name)
{
	return mfFileLength(CDrxNameTSCRC(name));
}

i32 CResFile::mfFlushDir(long nOffset)
{
	u32 i;
#ifdef _DEBUG
	// Check for sorted array and duplicated values
	ResDir Sorted;
	for (i = 0; i < m_Dir.size(); i++)
	{
		CDirEntry& DE = m_Dir[i];
		ResDirIt it = std::lower_bound(Sorted.begin(), Sorted.end(), DE.Name, ResDirSortByName());
		if (it != Sorted.end() && DE.Name == (*it).Name)
		{
			assert(0);  // Duplicated value
			continue;
		}
		Sorted.insert(it, DE);
	}
	assert(Sorted.size() == m_Dir.size());
	for (i = 0; i < m_Dir.size(); i++)
	{
		CDirEntry& DE1 = m_Dir[i];
		CDirEntry& DE2 = Sorted[i];
		assert(DE1.Name == DE2.Name);
	}
#endif

	TArray<CDirEntry> FDir;
	FDir.ReserveNoClear(m_Dir.size());

	for (i = 0; i < (i32)m_Dir.size(); i++)
	{
		CDirEntry* de = &m_Dir[i];
		CDirEntry& fden = FDir[i];
		fden.Name = de->Name;
		fden.size = de->size;
		assert(de->offset > 0);
		fden.offset = de->offset;
		fden.flags = de->flags;
		if (m_bSwapEndianWrite)
			SwapEndian(fden, eBigEndian);
	}

	gEnv->pDrxPak->FSeek(m_handle, nOffset, SEEK_SET);
	i32 sizeUn = FDir.Num() * sizeof(CDirEntry);
	byte* buf = NULL;
	if (m_bDirCompressed)
	{
#if RES_COMPRESSION == RESVERSION_LZSS
		buf = new byte[sizeUn * 2 + 128];
		if (!buf)
		{
			mfSetError("FlushDir - Allocation fault");
			return false;
		}
		sizeUn = Encodem((byte*)&FDir[0], buf, sizeUn);
#elif RES_COMPRESSION == RESVERSION_LZMA
		// we allocate 105% of original size for output buffer
		u32 outSize = sizeUn / 20 * 21 + (1 << 16);
		buf = (byte*)MyAlloc(outSize);
		if (!buf)
		{
			mfSetError("FlushDir - Allocation fault");
			return false;
		}
		u32 dict = 1 << 23;
		i32 res = Lzma86_Encode(buf, &outSize, (byte*)&FDir[0], sizeUn, 5, dict, SZ_FILTER_AUTO);
		if (res != 0)
		{
			mfSetError("FlushDir - Encoder error = %d", res);
			return 1;
		}
		sizeUn = outSize;
#elif RES_COMPRESSION == RESVERSION_DEBUG
		buf = new byte[sizeUn];
		memcpy(buf, (byte*)&FDir[0], sizeUn);
#else
		static_assert(false, "This should never be compiled...");
#endif
	}
	else
	{
		buf = new byte[sizeUn];
		memcpy(buf, &FDir[0], sizeUn);
	}

	if (gEnv->pDrxPak->FWrite(buf, 1, sizeUn, m_handle) != sizeUn)
	{
		mfSetError("FlushDir - Writing fault");
		return false;
	}

	m_nOffsDir = nOffset;
	m_nNumFilesUnique = FDir.Num();
	SAFE_DELETE_ARRAY(m_pCompressedDir);
	if (m_bDirCompressed)
	{
		m_nComprDirSize = sizeUn;
		m_pCompressedDir = new byte[sizeUn];
		memcpy(m_pCompressedDir, buf, sizeUn);
	}
	SAFE_DELETE_ARRAY(buf);
	m_bDirValid = true;

	SResFileLookupData* pLookup = GetLookupData();
	if (pLookup)
	{
		pLookup->m_NumOfFilesUnique = m_nNumFilesUnique;
		pLookup->m_OffsetDir = nOffset;
		m_pLookupDataMan->MarkDirty(true);
		m_pLookupDataMan->Flush();
	}
	return sizeUn;
}

i32 CResFile::mfFlush()
{
	PROFILE_FRAME(Resource_Flush);

	i32k nSizeDir = m_Dir.size() * sizeof(CDirEntry);

	if (m_typeaccess == RA_READ)
	{
		mfSetError("Flush - wrong access mode");
		return nSizeDir;
	}
	AUTO_LOCK(g_cResLock); // Not thread safe without this

	if (m_pLookupDataMan)
		m_pLookupDataMan->Flush();

	if (!m_bDirty)
		return nSizeDir;
	m_bDirty = false;
	if (!mfActivate(false))
		return nSizeDir;

	if (!m_handle)
	{
		mfSetError("Flush - Invalid file handle");
		return 0;
	}

	i32 updateCount = 0;
	long nSeek = m_nOffsDir;
	i32k nFiles = m_Dir.size();

	// store list of all duplicates: pair.first = original entry, pair.second = duplicate entry
	std::vector<std::pair<i32, i32>> duplicates;
	duplicates.reserve(nFiles);

	for (i32 i = 0; i < nFiles; ++i)
	{
		const CDirEntry& curDir = m_Dir[i];
		for (i32 j = i + 1; j < nFiles; ++j)
		{
			const CDirEntry& otherDir = m_Dir[j];
			if (curDir.GetOffset() == otherDir.GetOffset())
			{
				if (!curDir.IsDuplicate() && otherDir.IsDuplicate())
					duplicates.emplace_back(i, j);
				else if (curDir.IsDuplicate() && !otherDir.IsDuplicate())
					duplicates.emplace_back(j, i);
			}
		}
	}

	for (size_t i = 0; i < nFiles; i++)
	{
		CDirEntry& curDir = m_Dir[i];

		if (curDir.flags & RF_NOTSAVED)
		{
			SDirEntryOpen* pOE = mfGetOpenEntry(curDir.GetName());
			DRX_ASSERT(pOE);

			if (!curDir.IsDuplicate())
			{
				assert(pOE && pOE->pData);
				if (!pOE || !pOE->pData)
					continue;
				gEnv->pDrxPak->FSeek(m_handle, nSeek, SEEK_SET);
				if (curDir.flags & RF_COMPRESS)
				{
					byte* buf = static_cast<byte*>(pOE->pData);

#if RES_COMPRESSION == RESVERSION_LZSS
					if (!(curDir.flags & RF_COMPRESSED))
					{
						buf = new byte[curDir.size * 2 + 128];
						if (!buf)
						{
							mfSetError("Flush - Allocation fault");
							return nSizeDir;
						}

						// encode buffer size
						i32 originalSize = curDir.size;
						if (m_bSwapEndianWrite)
							SwapEndian(originalSize, eBigEndian);
						memcpy(buf, &originalSize, sizeof(i32));

						// encode data
						i32 sizeEnc = Encodem(static_cast<byte*>(pOE->pData), buf + sizeof(i32), curDir.size);
						curDir.size = sizeEnc + sizeof(i32);
					}
					else if (m_bSwapEndianWrite) // swap endianness of encoded data
					{
						u32 nS;
						memcpy(&nS, buf, sizeof(u32));
						SwapEndian(nS, eBigEndian);
						memcpy(buf, &nS, sizeof(u32));
					}

#elif RES_COMPRESSION == RESVERSION_LZMA
					// we allocate 105% of original size for output buffer
					if (!(curDir.flags & RF_COMPRESSED))
					{
						u32 outSize = de->size / 20 * 21 + (1 << 16);
						buf = (byte*)MyAlloc(outSize);
						if (!buf)
						{
							mfSetError("Flush - Allocation fault");
							return nSizeDir;
						}
						u32 dict = 1 << 23;
						i32 res = Lzma86_Encode(buf, &outSize, static_cast<byte*>(pOE->pData), curDir.size, 5, dict, SZ_FILTER_AUTO);
						if (res != 0)
						{
							mfSetError("Flush - Encoder error = %d", res);
							return nSizeDir;
						}
						curDir.size = outSize;
					}
#elif RES_COMPRESSION == RESVERSION_DEBUG
					buf = new byte[curDir.size + 20];
					memcpy(buf, ">>rawbuf>>", 10);
					memcpy(buf + 10, (byte*)pOE->pData, curDir.sizesize);
					memcpy(buf + 10 + curDir.size, "<<rawbuf<<", 10);
					curDir.size += 20;
#else
					static_assert(false, "This should never be compiled...");
#endif
					if (gEnv->pDrxPak->FWrite(buf, 1, curDir.size, m_handle) != curDir.size)
						mfSetError("Flush - Writing fault");
					if (!(curDir.flags & RF_COMPRESSED))
						delete[] buf;
				}
				else if (!pOE->pData || (gEnv->pDrxPak->FWrite(pOE->pData, 1, curDir.size, m_handle) != curDir.size))
				{
					mfSetError("Flush - Writing fault");
					continue;
				}

				mfCloseEntry(curDir.GetName(), curDir.flags);

				curDir.offset = nSeek;
				nSeek += curDir.size;
			}

			curDir.flags &= ~RF_NOTSAVED;
			updateCount++;
		}
	}

	for (const auto& it : duplicates)
	{
		const CDirEntry& originalEntry = m_Dir[it.first];
		CDirEntry& duplicateEntry      = m_Dir[it.second];

		DRX_ASSERT(!originalEntry.IsDuplicate() && duplicateEntry.IsDuplicate());
	
		duplicateEntry.offset = originalEntry.GetOffset();
		duplicateEntry.size   = originalEntry.GetSize();
		duplicateEntry.flags  = originalEntry.GetFlags() | RF_DUPLICATE;

		updateCount++;
	}

	if (updateCount > 0)
	{
		m_bDirCompressed = false;
		i32 sizeDir = mfFlushDir(nSeek);
		DRX_ASSERT(sizeDir == nSizeDir);

		SFileResHeader frh;
		frh.hid = IDRESHEADER;
		i32 ver = RES_COMPRESSION;
		frh.ver = ver;
		frh.num_files = m_Dir.size();
		frh.ofs_dir = nSeek;
		m_version = ver;
		SFileResHeader frhTemp, *pFrh;
		pFrh = &frh;
		if (m_bSwapEndianWrite)
		{
			frhTemp = frh;
			SwapEndian(frhTemp, eBigEndian);
			pFrh = &frhTemp;
		}
		gEnv->pDrxPak->FSeek(m_handle, 0, SEEK_SET);
		if (gEnv->pDrxPak->FWrite(pFrh, 1, sizeof(frh), m_handle) != sizeof(frh))
		{
			mfSetError("Flush - Writing fault");
			return nSizeDir;
		}
		gEnv->pDrxPak->FFlush(m_handle);
	}

	return nSizeDir;
}

i32 CResFile::Size()
{
	i32 nSize = sizeof(CResFile);

	u32 i;
	for (i = 0; i < m_Dir.size(); i++)
	{
		CDirEntry& DE = m_Dir[i];
		nSize += sizeof(CDirEntry);
		SDirEntryOpen* pOE = mfGetOpenEntry(DE.GetName());
		if (pOE)
		{
			nSize += sizeof(SDirEntryOpen);
			if (pOE->pData && (DE.flags & RF_TEMPDATA))
				nSize += DE.size;
		}
	}

	return nSize;
}

void CResFile::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_Dir);
	pSizer->AddObject(m_DirOpen);
}


#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Render/ResFile_info.h>
#ifndef _LIB
	#include <drx3D/CoreX/String/Name_TypeInfo.h>
#endif
