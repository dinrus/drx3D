// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/LoaderDBA.h>

#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/String/Path.h>
#include <drx3D/Animation/ControllerPQ.h>
#include <drx3D/Animation/ControllerOpt.h>
#include <drx3D/Eng3D/CGFContent.h>
#include <drx3D/Animation/CharacterUpr.h>

void CGlobalHeaderDBA::CreateDatabaseDBA(tukk filename)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_DBA, 0, filename);
	stack_string strPath = filename;
	PathUtil::UnifyFilePath(strPath);
	m_strFilePathDBA = strPath.c_str();
	m_FilePathDBACRC32 = CCrc32::Compute(strPath.c_str());
	m_nUsedAnimations = 0;
}

void CGlobalHeaderDBA::LoadDatabaseDBA(tukk sForCharacter)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_DBA, 0, m_strFilePathDBA.c_str());
	m_nLastUsedTimeDelta = 0;
	if (m_pStream || m_pDatabaseInfo)
		return;

	_smart_ptr<IChunkFile> pChunkFile = g_pI3DEngine->CreateChunkFile(true);
	if (!pChunkFile->Read(m_strFilePathDBA))
	{
		ReportLoadError(sForCharacter, pChunkFile->GetLastError());
		return;
	}

	// Load mesh from chunk file.

	SAFE_DELETE(m_pDatabaseInfo);
	m_pDatabaseInfo = new CInternalDatabaseInfo(m_FilePathDBACRC32, m_strFilePathDBA);

	if (!m_pDatabaseInfo->LoadChunks(pChunkFile, false))
	{
		delete m_pDatabaseInfo;
		m_pDatabaseInfo = 0;
		ReportLoadError(sForCharacter, "Failed to load chunks");
	}
}

//////////////////////////////////////////////////////////////////////////
bool CGlobalHeaderDBA::StartStreamingDBA(bool highPriority)
{
	//	LoadDatabaseDBA(0);
	//	return true;

	if (m_pStream || m_pDatabaseInfo)
		return true;

	CInternalDatabaseInfo* pStreamInfo = new CInternalDatabaseInfo(m_FilePathDBACRC32, m_strFilePathDBA);

	// start streaming
	StreamReadParams params;
	params.dwUserData = 0;
	params.ePriority = highPriority ? estpUrgent : estpNormal;
	params.nSize = 0;
	params.pBuffer = NULL;
	params.nLoadTime = 10000;
	params.nMaxLoadTime = 1000;

	m_pStream = g_pISystem->GetStreamEngine()->StartRead(eStreamTaskTypeAnimation, m_strFilePathDBA, pStreamInfo, &params);

	return true;
}

void CGlobalHeaderDBA::CompleteStreamingDBA(CInternalDatabaseInfo* pInfo)
{
	SAFE_DELETE(m_pDatabaseInfo);
	m_pDatabaseInfo = pInfo;

	if (pInfo->m_bLoadFailed)
	{
		ReportLoadError(m_strFilePathDBA.c_str(), pInfo->m_strLastError.c_str());
	}

	if (g_pCharacterUpr->m_pStreamingListener)
	{
		u32 numHeadersCAF = g_AnimationUpr.m_arrGlobalCAF.size();
		for (u32 i = 0; i < numHeadersCAF; i++)
		{
			GlobalAnimationHeaderCAF& rGAH = g_AnimationUpr.m_arrGlobalCAF[i];
			if (rGAH.m_FilePathDBACRC32 == m_FilePathDBACRC32)
			{
				i32 globalID = g_AnimationUpr.GetGlobalIDbyFilePath_CAF(rGAH.GetFilePath());
				g_pCharacterUpr->m_pStreamingListener->NotifyAnimLoaded(globalID);
			}
		}
	}

	m_pStream.reset();
}

CInternalDatabaseInfo::CInternalDatabaseInfo(u32 filePathCRC, const string& strFilePath)
	: m_FilePathCRC(filePathCRC)
	, m_strFilePath(strFilePath)
	, m_bLoadFailed(false)
	, m_nRelocatableCAFs(0)
	, m_iTotalControllers(0)
	, m_nStorageLength(0)
	, m_pControllersInplace(nullptr)
{
}

CInternalDatabaseInfo::~CInternalDatabaseInfo()
{
	m_pControllersInplace = 0;
	m_nRelocatableCAFs = 0;

	if (m_hStorage.IsValid())
	{
		g_controllerHeap.Free(m_hStorage);
		m_hStorage = CControllerDefragHdl();
	}
}

void CInternalDatabaseInfo::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_pControllersInplace, sizeof(CControllerOptNonVirtual) * m_iTotalControllers);
}

void CInternalDatabaseInfo::StreamOnComplete(IReadStream* pStream, unsigned nError)
{
	CGlobalHeaderDBA* pDBA = g_AnimationUpr.FindGlobalHeaderByCRC_DBA(m_FilePathCRC);

	if (pDBA)
	{
		pDBA->CompleteStreamingDBA(this);
	}
	else
	{
		delete this;
	}
}

uk CInternalDatabaseInfo::StreamOnNeedStorage(IReadStream* pStream, unsigned nSize, bool& bAbortOnFailToAlloc)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_DBA, 0, m_strFilePath.c_str());

	tuk pData = NULL;

	if (Console::GetInst().ca_StreamDBAInPlace)
	{
		m_hStorage = g_controllerHeap.AllocPinned(nSize, this);
		if (m_hStorage.IsValid())
			pData = (tuk)g_controllerHeap.WeakPin(m_hStorage);
	}

	return pData;
}

void CInternalDatabaseInfo::StreamAsyncOnComplete(IReadStream* pStream, unsigned nError)
{
	DEFINE_PROFILER_FUNCTION();
	//LOADING_TIME_PROFILE_SECTION(g_pISystem);
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_DBA, 0, m_strFilePath.c_str());

	if (pStream->IsError())
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, m_strFilePath.c_str(), "Failed to stream DBA-file %x", nError);
		return;
	}

	//--------------------------------------------------------------------------

	tukk buffer = (tuk)pStream->GetBuffer();
	size_t size = pStream->GetBytesRead();
	_smart_ptr<IChunkFile> pChunkFile = g_pI3DEngine->CreateChunkFile(true);
	if (!pChunkFile->ReadFromMemory(buffer, size))
	{
		m_bLoadFailed = true;
		m_strLastError = pChunkFile->GetLastError();
		return;
	}

	if (!LoadChunks(pChunkFile, true))
	{
		if (!m_bLoadFailed)
		{
			m_bLoadFailed = true;
			m_strLastError = "Failed to load chunks";
		}
	}

#ifndef _RELEASE
	if (Console::GetInst().ca_DebugAnimUsageOnFileAccess)
		g_AnimationUpr.DebugAnimUsage(0);
#endif

	pStream->FreeTemporaryMemory();
}

void CGlobalHeaderDBA::DeleteDatabaseDBA()
{
	if (m_pStream)
	{
		m_pStream->Abort();
		m_pStream.reset();
	}

	if (!m_pDatabaseInfo)
		return;

	if (m_nUsedAnimations)
		return;

	// make sure that the DBA is GONE from the outside first
	// so it cannot be accessed from another thread while this is
	// running. Also we have to make sure to communicate that we are currently in the process of deleting
	// this DBA.
	CInternalDatabaseInfo* dbInfo = m_pDatabaseInfo;
	m_pDatabaseInfo = NULL;

	u32 numHeadersCAF = g_AnimationUpr.m_arrGlobalCAF.size();
	for (u32 i = 0; i < numHeadersCAF; i++)
	{
		GlobalAnimationHeaderCAF& rGAH = g_AnimationUpr.m_arrGlobalCAF[i];
		if (rGAH.m_FilePathDBACRC32 != m_FilePathDBACRC32)
			continue;
		rGAH.m_nControllers = 0;
		rGAH.m_arrController.clear();
		rGAH.ClearControllerData();
		rGAH.m_arrControllerLookupVector.clear();
	}

	// now we can really delete it, since the data is no longer referenced through
	// CAF headers
	if (dbInfo)
		delete dbInfo;

	if (g_pCharacterUpr->m_pStreamingListener)
	{
		for (u32 i = 0; i < numHeadersCAF; i++)
		{
			GlobalAnimationHeaderCAF& rGAH = g_AnimationUpr.m_arrGlobalCAF[i];
			if (rGAH.m_FilePathDBACRC32 == m_FilePathDBACRC32)
			{
				i32 globalID = g_AnimationUpr.GetGlobalIDbyFilePath_CAF(rGAH.GetFilePath());
				g_pCharacterUpr->m_pStreamingListener->NotifyAnimUnloaded(globalID);
			}
		}
	}
#ifndef _RELEASE
	if (Console::GetInst().ca_DebugAnimUsageOnFileAccess)
		g_AnimationUpr.DebugAnimUsage(0);
#endif
}

//--------------------------------------------------------------------------------

bool CGlobalHeaderDBA::InMemory()
{
	return (m_pDatabaseInfo != 0);
}

//--------------------------------------------------------------------------------

bool CInternalDatabaseInfo::LoadChunks(IChunkFile* pChunkFile, bool bStreaming)
{
	u32 numChunck = pChunkFile->NumChunks();
	for (u32 i = 0; i < numChunck; i++)
	{
		IChunkFile::ChunkDesc* const pChunkDesc = pChunkFile->GetChunk(i);
		switch (pChunkDesc->chunkType)
		{

		case ChunkType_Controller:
			if (!ReadControllers(pChunkDesc, bStreaming))
				return false;
			break;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------

bool CInternalDatabaseInfo::ReadControllers(IChunkFile::ChunkDesc* pChunkDesc, bool bStreaming)
{
	if (pChunkDesc->chunkVersion == CONTROLLER_CHUNK_DESC_0905::VERSION)
		return ReadController905(pChunkDesc, bStreaming);
	DrxFatalError("DinrusXAnimation: version 0x%03x of controllers is not supported. Use DBAs generated with latest RC", (i32)pChunkDesc->chunkVersion);
	return true;
}

bool CInternalDatabaseInfo::ReadController905(IChunkFile::ChunkDesc* pChunkDesc, bool bStreaming)
{
	LOADING_TIME_PROFILE_SECTION(g_pISystem);

	if (pChunkDesc->bSwapEndian)
		DrxFatalError("%s: data are stored in non-native endian format", __FUNCTION__);

	CONTROLLER_CHUNK_DESC_0905* pCtrlChunk = (CONTROLLER_CHUNK_DESC_0905*)pChunkDesc->data;
	tuk pCtrlChunkEnd = reinterpret_cast<tuk>(pChunkDesc->data) + pChunkDesc->size;

	m_iTotalControllers = 0;

	uint64 startedStatistics = 0;
	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		startedStatistics = info.allocated - info.freed;
		g_pILog->UpdateLoadingScreen("ReadController905. Init. Memstat %li", (i32)(info.allocated - info.freed));
	}

	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("ReadController905. TracksResize. Memstat %li", (i32)(info.allocated - info.freed));
	}

	tuk pData = (tuk)(pCtrlChunk + 1);
	tuk pData2 = pData;

	u32 totalAnims = pCtrlChunk->numAnims;
	u32 keyTime = pCtrlChunk->numKeyTime;
	u32 keyPos = pCtrlChunk->numKeyPos;
	u32 keyRot = pCtrlChunk->numKeyRot;

	// Keytimes pointers
	std::vector<u16> pktSizes((u16*)pData, (u16*)pData + keyTime);
	pData += pktSizes.size() * sizeof(u16);
	std::vector<u32> pktFormats((u32*)pData, (u32*)pData + (eBitset + 1));
	pData += pktFormats.size() * sizeof(u32);

	// Positions pointers
	std::vector<u16> ppSizes((u16*)pData, (u16*)pData + keyPos);
	pData += keyPos * sizeof(u16);
	std::vector<u32> ppFormats((u32*)pData, (u32*)pData + eAutomaticQuat);
	pData += eAutomaticQuat * sizeof(u32);

	//Rotations pointers
	std::vector<u16> prSizes((u16*)pData, (u16*)pData + keyRot);
	pData += keyRot * sizeof(u16);
	std::vector<u32> prFormats((u32*)pData, (u32*)pData + eAutomaticQuat);
	pData += eAutomaticQuat * sizeof(u32);

	std::vector<i32> ktOffsets((i32*)pData, (i32*)pData + keyTime);
	pData += keyTime * sizeof(i32);

	std::vector<i32> pOffsets((i32*)pData, (i32*)pData + keyPos);
	pData += keyPos * sizeof(i32);

	std::vector<i32> rOffsets((i32*)pData, (i32*)pData + (keyRot + 1));
	pData += (keyRot + 1) * sizeof(i32);

	// Determine if this appears to be an in-place streamable file. If it is, the offsets will be negative.
	bool bIsInPlacePrepared = rOffsets[keyRot] < 0;

	u32 nPaddingLength = 0;
	if (bIsInPlacePrepared)
	{
		memcpy(&nPaddingLength, pData, sizeof(nPaddingLength));
		pData += sizeof(nPaddingLength);
	}

	ptrdiff_t diff = pData - pData2;
	pData += diff % 4 > 0 ? 4 - diff % 4 : 0;

	u32 nTrackLen = Align(abs(rOffsets[keyRot]), 4);

	if (nTrackLen == 0)
	{
		if (!m_bLoadFailed)
		{
			m_bLoadFailed = true;
			m_strLastError = "Empty controller found";
		}
		return false;
	}

	u32 rCountSize = eAutomaticQuat + 1;
	std::vector<u32> rCount(rCountSize);
	for (u32 i = 1; i < rCountSize; ++i)
	{
		rCount[i] += rCount[i - 1] + prFormats[i - 1];
	}

	u32 pCountSize = eAutomaticQuat + 1;
	std::vector<u32> pCount(pCountSize);
	for (u32 i = 1; i < pCountSize; ++i)
	{
		pCount[i] += pCount[i - 1] + ppFormats[i - 1];
	}

	u32 ktCountSize = eBitset + 2;
	std::vector<u32> ktCount(ktCountSize);
	for (u32 i = 1; i < ktCountSize; ++i)
	{
		ktCount[i] += ktCount[i - 1] + pktFormats[i - 1];
	}

	u32 curFormat = 0;

	while (curFormat + 1 < ktCountSize && ktCount[curFormat + 1] == 0)
		++curFormat;
	u32 curSizeof = ControllerHelper::GetKeyTimesFormatSizeOf(curFormat);

	for (u32 i = 0; i < keyTime; ++i)
	{
		while (i >= ktCount[curFormat + 1])
		{
			++curFormat;
			curSizeof = ControllerHelper::GetKeyTimesFormatSizeOf(curFormat);
		}
	}

	curFormat = 0;
	while (curFormat + 1 < pCountSize && pCount[curFormat + 1] == 0)
		++curFormat;

	curSizeof = ControllerHelper::GetPositionsFormatSizeOf(curFormat);

	for (u32 i = 0; i < keyPos; ++i)
	{
		while (i >= pCount[curFormat + 1])
		{
			++curFormat;
			curSizeof = ControllerHelper::GetPositionsFormatSizeOf(curFormat);
		}

	}

	curFormat = 0;
	while (curFormat + 1 < rCountSize && rCount[curFormat + 1] == 0)
		++curFormat;

	curSizeof = ControllerHelper::GetRotationFormatSizeOf(curFormat);

	for (u32 i = 0; i < keyRot; ++i)
	{
		while (i >= rCount[curFormat + 1])
		{
			++curFormat;
			curSizeof = ControllerHelper::GetRotationFormatSizeOf(curFormat);
		}
	}

	tukk pStorageData = NULL;

	if (!bIsInPlacePrepared)
	{
		// Old assets have the tracks in the middle of the file
		pStorageData = pData;
		pData += nTrackLen;
	}
	else
	{
		pStorageData = pCtrlChunkEnd - nTrackLen;
	}

	// set pointers to keytimes, positions, rotations
	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("ReadController905. All dynamic data resize. Memstat %li", (i32)(info.allocated - info.freed));
	}

	std::vector<CControllerInfo*> controllersOffsets(totalAnims, 0);

	u32 nDBACRC = CCrc32::Compute(m_strFilePath.c_str());

	u32 nRelocatableCAFs = 0;

	pData += nPaddingLength;

	tuk pAnimBlockStart = pData;

	std::vector<u16> controllerCount(totalAnims, 0);

	std::vector<u16> arrGlobalAnimID(totalAnims, (u16) - 1);
	for (u32 i = 0; i < totalAnims; ++i)
	{

		u16 strSize;// = pData;
		memcpy(&strSize, pData, sizeof(u16));
		pData += sizeof(u16);

#define BIG_STRING 1024

		if (strSize > BIG_STRING)
		{
			assert(0);
			return false;
		}
		char tmp[BIG_STRING];

		memset(tmp, 0, BIG_STRING);
		memcpy(tmp, pData, strSize);

#undef BIG_STRING

		pData += strSize;

		i32 nGlobalAnimID = -1;

		if (bStreaming)
		{
			nGlobalAnimID = g_AnimationUpr.GetGlobalIDbyFilePath_CAF(tmp);
			if (nGlobalAnimID < 0)
				DrxLog("Going to create a CAF header on streaming thread - possible race condition!");
		}

		if (nGlobalAnimID < 0)
		{
			nGlobalAnimID = g_AnimationUpr.CreateGAH_CAF(tmp);
			if (nGlobalAnimID < 0)
				DrxFatalError("GAH does not exist");
		}

		arrGlobalAnimID[i] = nGlobalAnimID;

		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID];
		if (rCAF.m_FilePathDBACRC32 == 0 && Console::GetInst().ca_UseIMG_CAF)
			DrxFatalError("DBA/CAF mismatch in %s", rCAF.GetFilePath());

		MotionParams905 info;
		memcpy(&info, pData, sizeof(info));
		pData += sizeof(info);

		if (rCAF.m_FilePathDBACRC32)
		{
			rCAF.SetFlags(info.m_nAssetFlags);

			rCAF.OnAssetCreated();

			rCAF.m_FilePathDBACRC32 = nDBACRC;
			//	m_pDatabaseInfo->m_SkinningInfo[i]->m_nCompression		= info.m_nCompression;
			i32 nStartKey = info.m_nStart;
			i32 nEndKey = info.m_nEnd;
			if (rCAF.GetFlags() & CA_ASSET_ADDITIVE)
				nStartKey++;
			rCAF.m_fStartSec = nStartKey / ANIMATION_30Hz;
			rCAF.m_fEndSec = nEndKey / ANIMATION_30Hz;
			if (rCAF.m_fEndSec <= rCAF.m_fStartSec)
				rCAF.m_fEndSec = rCAF.m_fStartSec;
			rCAF.m_fTotalDuration = rCAF.m_fEndSec - rCAF.m_fStartSec;
			rCAF.m_StartLocation = info.m_StartLocation;

			++nRelocatableCAFs;
		}

		// footplants
		u16 footplans;// = m_arrAnimations[i].m_FootPlantBits.size();
		memcpy(&footplans, pData, sizeof(u16));
		pData += sizeof(u16);
		pData += footplans;

		//
		u16 controllerInfo;// = m_arrAnimations[i].m_arrControlerInfo.size();
		memcpy(&controllerInfo, pData, sizeof(u16));
		pData += sizeof(u16);

		i32 offsetToControllerInfos = static_cast<i32>(pData - pAnimBlockStart);
		if (bIsInPlacePrepared)
		{
			memcpy(&offsetToControllerInfos, pData, sizeof(i32));
			pData += sizeof(i32);
		}

		if (controllerInfo == 0)
			continue;

		CControllerInfo* pController = (CControllerInfo*)(pAnimBlockStart + offsetToControllerInfos);
		controllersOffsets[i] = pController;

		if (!bIsInPlacePrepared)
		{
			pData += controllerInfo * sizeof(CControllerInfo);
		}

		if (rCAF.m_FilePathDBACRC32)
		{
			rCAF.m_arrController.resize(controllerInfo);
		}

		//		rCAF.m_bEmpty=u8(controllerInfo);
		rCAF.m_bEmpty = (controllerInfo == 0) ? 1 : 0;
		controllerCount[i] = controllerInfo;

		m_iTotalControllers += controllerInfo;
	}

	u32 nCAFListSize = Align(nRelocatableCAFs * sizeof(u16), 16);
	u32 nAlignedStorageSize = Align(nTrackLen, 16);
	u32 nAlignedControllerSize = Align(sizeof(CControllerOptNonVirtual) * m_iTotalControllers, 16);
	u32 nTotalSize = nCAFListSize + nAlignedControllerSize + nAlignedStorageSize;

	CControllerDefragHdl hStorage;

	// If we're currently attempting to stream in place, the file is prepared for in place, and
	// there's room to install the controllers, then don't allocate. Otherwise do.

	bool bIsInPlace = false;

	if (m_hStorage.IsValid())
	{
		tuk pStorageBase = reinterpret_cast<tuk>(pCtrlChunk);
		if (bIsInPlacePrepared)
		{
			if ((pStorageBase + nCAFListSize + nAlignedControllerSize) <= (pCtrlChunkEnd - nTrackLen))
			{
				hStorage = m_hStorage;
				bIsInPlace = true;
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "Failed to stream DBA '%s' inplace - has sizeof(CControllerOptNonVirtual) changed?", m_strFilePath.c_str());
			}
		}
	}

	if (!hStorage.IsValid())
	{
		hStorage = g_controllerHeap.AllocPinned(nTotalSize, this);
	}

	if (!hStorage.IsValid())
	{
		return false;
	}

	tuk pAllocData = (tuk)g_controllerHeap.WeakPin(hStorage);
	u16* pCAFList = (u16*)pAllocData;
	tuk pControllers = pAllocData + nCAFListSize;

	tuk pStorage = NULL;
	if (bIsInPlace)
	{
		// offsets are negative, relative to the chunk end
		pStorage = pCtrlChunkEnd;
	}
	else
	{
		pStorage = pControllers + nAlignedControllerSize;
		memcpy(pStorage, pStorageData, nTrackLen);

		if (bIsInPlacePrepared)
		{
			// Offsets are relative to the end of the storage block
			pStorage += nTrackLen;
		}
	}

	m_pControllersInplace = reinterpret_cast<CControllerOptNonVirtual*>(pControllers);

	u32 controllersCount = 0, nRelocateCAFIdx = 0;
	for (u32 i = 0; i < totalAnims; ++i)
	{
		u16 nGlobalAnimID = arrGlobalAnimID[i];
		if (nGlobalAnimID == 0xffff)
			DrxFatalError("Invalid GAH ID");
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[nGlobalAnimID];
		//		u32 numController = rCAF.m_bEmpty;
		u32 numController = controllerCount[i];

		CControllerInfo* pController = controllersOffsets[i];
		if (pController)
		{
			for (u32 t = 0; t < numController; ++t)
			{
				// Must load all pController members, as controller construction may trample it
				u32 p = const_cast< u32&>(pController[t].m_nPosTrack);
				u32 r = const_cast< u32&>(pController[t].m_nRotTrack);
				u32 pk = const_cast< u32&>(pController[t].m_nPosKeyTimeTrack);
				u32 rk = const_cast< u32&>(pController[t].m_nRotKeyTimeTrack);
				u32 id = const_cast< u32&>(pController[t].m_nControllerID);

				u32 rotkt = (rk == ~0) ? eNoFormat : FindFormat(rk, ktCount);
				u32 rotk = (r == ~0) ? eNoFormat : FindFormat(r, rCount);
				u32 poskt = (pk == ~0) ? eNoFormat : FindFormat(pk, ktCount);
				u32 posk = (p == ~0) ? eNoFormat : FindFormat(p, pCount);

				CControllerOptNonVirtual* pNewController = new(&m_pControllersInplace[controllersCount++])CControllerOptNonVirtual;
				pNewController->Init(rotk, rotkt, posk, poskt);

				if (r != ~0 && rk != ~0)
				{
					tuk pRKT = &pStorage[ktOffsets[rk]];
					tuk pRK = &pStorage[rOffsets[r]];

					pNewController->SetRotationKeyTimes(pktSizes[rk], pRKT);
					pNewController->SetRotationKeys(prSizes[r], pRK);
				}

				if (p != ~0 && pk != ~0)
				{
					tuk pPKT = &pStorage[ktOffsets[pk]];
					tuk pPK = &pStorage[pOffsets[p]];

					pNewController->SetPositionKeyTimes(pktSizes[pk], pPKT);
					pNewController->SetPositionKeys(ppSizes[p], pPK);
				}

				pNewController->m_nControllerId = id;

				if (rCAF.m_FilePathDBACRC32)
					rCAF.m_arrController[t] = pNewController;
			}
		}

		if (rCAF.m_FilePathDBACRC32)
		{
			pCAFList[nRelocateCAFIdx++] = nGlobalAnimID;
			std::sort(rCAF.m_arrController.begin(), rCAF.m_arrController.end(), AnimCtrlSortPred());
			rCAF.InitControllerLookup(numController);
			//		rCAF.m_FilePathDBACRC32 = nDBACRC32;
			rCAF.OnAssetCreated();
			rCAF.ClearAssetRequested();
			rCAF.ClearAssetNotFound();
			rCAF.m_nControllers = numController;
			rCAF.m_nControllers2 = numController;
		}
	}

	if (Console::GetInst().ca_MemoryUsageLog)
	{
		DrxModuleMemoryInfo info;
		DrxGetMemoryInfoForModule(&info);
		g_pILog->UpdateLoadingScreen("ReadController903. Finished. Memstat %li", (i32)(info.allocated - info.freed));
		uint64 endStatistics = info.allocated - info.freed;
		g_AnimStatisticsInfo.m_iDBASizes += endStatistics - startedStatistics;
		g_pILog->UpdateLoadingScreen("Current DBA. Memstat %li", (i32)(endStatistics - startedStatistics));
		g_pILog->UpdateLoadingScreen("ALL DBAs %li", (i32)g_AnimStatisticsInfo.m_iDBASizes);
	}

	m_nRelocatableCAFs = nRelocatableCAFs;

	// Couldn't stream in place for some reason :(. Free the temporary alloc.
	if (m_hStorage.IsValid() && m_hStorage != hStorage)
	{
		g_controllerHeap.Free(m_hStorage);
	}

	m_hStorage = hStorage;
	m_nStorageLength = g_controllerHeap.UsableSize(hStorage);

	g_controllerHeap.Unpin(hStorage);

	return true;
}

void CInternalDatabaseInfo::Relocate(tuk pDst, tuk pSrc)
{
	u16* pCAFList = reinterpret_cast<u16*>(pDst);

	size_t nStorageLength = m_nStorageLength;

	for (u32 i = 0; i < m_nRelocatableCAFs; ++i)
	{
		GlobalAnimationHeaderCAF& rCAF = g_AnimationUpr.m_arrGlobalCAF[pCAFList[i]];

		for (i32 ci = 0, cc = rCAF.m_arrController.size(); ci != cc; ++ci)
		{
			IController* pSrcCont = &*rCAF.m_arrController[ci];
			size_t offs = static_cast<size_t>((tuk)pSrcCont - pSrc);

			if (offs < nStorageLength)
				rCAF.m_arrController[ci] = reinterpret_cast<IController*>(pDst + offs);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGlobalHeaderDBA::ReportLoadError(tukk sForCharacter, tukk sReason)
{
	if (!m_bLoadFailed)
	{
		m_strLastError = sReason;
		// Report loading error only once.
		m_bLoadFailed = true;
		gEnv->pLog->LogError("Failed To Load Animation DBA '%s' for model '%s'.  Reason: %s", m_strFilePathDBA.c_str(), sForCharacter, sReason);
	}
}

const size_t CGlobalHeaderDBA::SizeOf_DBA() const
{
	size_t nSize = sizeof(CGlobalHeaderDBA);
	nSize += m_strFilePathDBA.capacity();
	nSize += m_strLastError.capacity();
	if (m_pDatabaseInfo == 0)
		return nSize;

	if (m_pDatabaseInfo->m_hStorage.IsValid())
		nSize += g_controllerHeap.UsableSize(m_pDatabaseInfo->m_hStorage);

	return nSize;
}

void CGlobalHeaderDBA::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_pDatabaseInfo);
	pSizer->AddObject(m_strLastError);
}

CGlobalHeaderDBA::CGlobalHeaderDBA()
{
	m_pDatabaseInfo = 0;
	m_FilePathDBACRC32 = 0;
	m_nUsedAnimations = 0;
	m_nLastUsedTimeDelta = 0;
	m_bDBALock = 0;
	m_bLoadFailed = false;
	m_nEmpty = 0;
	m_pStream = 0;
}

CGlobalHeaderDBA::~CGlobalHeaderDBA()
{
	if (m_pStream)
	{
		m_pStream->Abort();
		m_pStream.reset();
	}

	if (m_pDatabaseInfo)
		delete m_pDatabaseInfo;
}
