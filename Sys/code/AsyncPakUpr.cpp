// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AsyncPakUpr.cpp
//  Version:     v1.00
//  Created:     23/09/2010 by Kenzo.
//  Описание: Manage async pak files
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/AsyncPakUpr.h>
#include <drx3D/Sys/System.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Sys/DrxPak.h>
#include <drx3D/Sys/ResourceUpr.h>

#define MEGA_BYTE 1024* 1024

//////////////////////////////////////////////////////////////////////////

string& CAsyncPakUpr::SAsyncPak::GetStatus(string& status) const
{
	switch (eState)
	{
	case STATE_UNLOADED:
		status = "Unloaded";
		break;
	case STATE_REQUESTED:
		status = "Requested";
		break;
	case STATE_REQUESTUNLOAD:
		status = "RequestUnload";
		break;
	case STATE_LOADED:
		status = "Loaded";
		break;
	default:
		status = "Unknown";
		break;
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////

CAsyncPakUpr::CAsyncPakUpr()
{
	m_nTotalOpenLayerPakSize = 0;
	m_bRequestLayerUpdate = false;
}

CAsyncPakUpr::~CAsyncPakUpr()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::Clear()
{
	//float startTime = gEnv->pTimer->GetAsyncCurTime();

	for (TPakMap::iterator it = m_paks.begin();
	     it != m_paks.end(); ++it)
	{
		SAsyncPak& layerPak = it->second;
		if (layerPak.bStreaming)
		{
			// wait until finished
			layerPak.pReadStream->Abort();
		}

		ReleaseData(&layerPak);
	}
	m_paks.clear();
	m_bRequestLayerUpdate = false;

	assert(m_nTotalOpenLayerPakSize == 0);
	m_nTotalOpenLayerPakSize = 0;

	//printf("CAsyncPakUpr::Clear() %0.4f secs\n", gEnv->pTimer->GetAsyncCurTime() - startTime);
}

void CAsyncPakUpr::UnloadLevelLoadPaks()
{
	for (TPakMap::iterator it = m_paks.begin();
	     it != m_paks.end(); ++it)
	{
		SAsyncPak& layerPak = it->second;

		if (layerPak.eLifeTime == SAsyncPak::LIFETIME_LOAD_ONLY)
		{
			if (layerPak.bStreaming)
			{
				// wait until finished
				layerPak.pReadStream->Abort();
			}

			ReleaseData(&layerPak);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::ParseLayerPaks(const string& levelCachePath)
{
	string layerPath = levelCachePath + "/"; // "/layers/";
	string search = layerPath + "*.*";
	IDrxPak* pPak = gEnv->pDrxPak;

	_finddata_t fd;
	intptr_t handle = 0;

	// allow this find first to actually touch the file system
	handle = pPak->FindFirst(search.c_str(), &fd, 0, true);

	if (handle > -1)
	{
		do
		{
			if ((fd.attrib & _A_SUBDIR) || !strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
			{
				continue;
			}

			string pakName = fd.name;
			size_t findPos = pakName.find_last_of('.');
			if (findPos == string::npos)
				continue;

			string extension = pakName.substr(findPos + 1, pakName.size());
			if (extension != "pak")
				continue;

			SAsyncPak layerPak;
			layerPak.layername = pakName.substr(0, findPos);
			layerPak.filename = layerPath + pakName;
			layerPak.nSize = pPak->FGetSize(layerPak.filename, true); // allow to go to disc for this access
			layerPak.bClosePakOnRelease = true;

			m_paks[layerPak.layername] = layerPak;
		}
		while (pPak->FindNext(handle, &fd) >= 0);

		pPak->FindClose(handle);
	}
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::StartStreaming(SAsyncPak* pLayerPak)
{
	StreamReadParams params;
	params.dwUserData = (DWORD_PTR) pLayerPak;
	params.nSize = 0;
	params.pBuffer = NULL;
	params.nFlags = IStreamEngine::FLAGS_FILE_ON_DISK;
	params.ePriority = estpIdle;

	pLayerPak->pReadStream = gEnv->pSystem->GetStreamEngine()->StartRead(eStreamTaskTypePak, pLayerPak->filename.c_str(), this, &params);

	if (pLayerPak->pReadStream)
	{
		pLayerPak->bStreaming = true;
	}
	else
	{
		pLayerPak->eState = SAsyncPak::STATE_UNLOADED;
		SAFE_RELEASE(pLayerPak->pData);
	}
}

void CAsyncPakUpr::ReleaseData(SAsyncPak* pLayerPak)
{
	if (pLayerPak->eState == SAsyncPak::STATE_LOADED)
	{
		if (pLayerPak->bClosePakOnRelease)
		{
			gEnv->pDrxPak->ClosePack(pLayerPak->filename.c_str(), 0);
			//printf("Unload pak from mem: %s\n", pLayerPak->filename.c_str());
		}
		else
		{
			gEnv->pDrxPak->LoadPakToMemory(pLayerPak->filename.c_str(), IDrxPak::eInMemoryPakLocale_Unload);
			//printf("Close pak: %s\n", pLayerPak->filename.c_str());
		}

		m_nTotalOpenLayerPakSize -= pLayerPak->nSize;
	}

	if (pLayerPak->pData)
	{
		assert((!pLayerPak->pData) || (pLayerPak->pData && pLayerPak->pData->Unique()));
	}
	
	SAFE_RELEASE(pLayerPak->pData);
	pLayerPak->eState = SAsyncPak::STATE_UNLOADED;

	m_bRequestLayerUpdate = true;
}

//////////////////////////////////////////////////////////////////////////

bool CAsyncPakUpr::LoadLayerPak(tukk sLayerName)
{
	// only load layer paks from valid files
	TPakMap::iterator findResult = m_paks.find(sLayerName);
	if (findResult != m_paks.end())
	{
		return LoadPak(findResult->second);
	}

	return false;
}

bool CAsyncPakUpr::LoadPakToMemAsync(tukk pPath, bool bLevelLoadOnly)
{
	//check if pak reference exists
	TPakMap::iterator findResult = m_paks.find(pPath);
	if (findResult != m_paks.end())
	{
		return LoadPak(findResult->second);
	}
	else
	{
		char szFullPathBuf[IDrxPak::g_nMaxPath];
		tukk szFullPath = gEnv->pDrxPak->AdjustFileName(pPath, szFullPathBuf, IDrxPak::FOPEN_HINT_QUIET | IDrxPak::FLAGS_PATH_REAL);

		// Check if the pak file actually exists before trying to load
		if (!gEnv->pDrxPak->IsFileExist(szFullPath, IDrxPak::eFileLocation_Any))
		{
			// Cached file does not exist
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Level cache pak file %s does not exist", szFullPath);
			return false;
		}

		SAsyncPak layerPak;
		layerPak.layername = pPath;
		layerPak.filename = szFullPathBuf;
		layerPak.nSize = 0;
		layerPak.eLifeTime = bLevelLoadOnly ? SAsyncPak::LIFETIME_LOAD_ONLY : SAsyncPak::LIFETIME_LEVEL_COMPLETE;

		m_paks[layerPak.layername] = layerPak;

		return LoadPak(m_paks[layerPak.layername]);
	}
	return false;
}

bool CAsyncPakUpr::LoadPak(SAsyncPak& layerPak)
{
	layerPak.nRequestCount++;
	if (layerPak.eState == SAsyncPak::STATE_LOADED || layerPak.bStreaming ||
	    layerPak.eState == SAsyncPak::STATE_REQUESTED)
		return true;

	layerPak.eState = SAsyncPak::STATE_REQUESTED;

	//printf("Streaming level pak: %s\n", layerPak.layername.c_str());

	StartStreaming(&layerPak);

	return false;
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::UnloadLayerPak(tukk sLayerName)
{
	TPakMap::iterator findResult = m_paks.find(sLayerName);
	if (findResult == m_paks.end())
		return;

	SAsyncPak& layerPak = findResult->second;
	layerPak.nRequestCount--;
	assert(layerPak.nRequestCount >= 0);
	if (layerPak.nRequestCount > 0)
		return;

	if (layerPak.bStreaming)
	{
		if (layerPak.pReadStream)
			layerPak.pReadStream->Abort();
		layerPak.eState = SAsyncPak::STATE_REQUESTUNLOAD;
		return;
	}

	if (layerPak.eState == SAsyncPak::STATE_LOADED)
	{
		ReleaseData(&layerPak);
		m_bRequestLayerUpdate = true;
	}

	if (layerPak.eState == SAsyncPak::STATE_REQUESTED)
	{
		layerPak.eState = SAsyncPak::STATE_UNLOADED;
	}
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::GetLayerPakStats(
  SLayerPakStats& stats, bool bCollectAllStats) const
{
	stats.m_MaxSize = (g_cvars.pakVars.nTotalInMemoryPakSizeLimit * MEGA_BYTE);
	stats.m_UsedSize = m_nTotalOpenLayerPakSize;

	for (TPakMap::const_iterator it = m_paks.begin(); it != m_paks.end(); ++it)
	{
		const SAsyncPak& layerPak = it->second;
		if (bCollectAllStats || layerPak.eState != SAsyncPak::STATE_UNLOADED)
		{
			SLayerPakStats::SEntry entry;
			entry.name = it->first;
			entry.nSize = layerPak.nSize;
			entry.bStreaming = layerPak.bStreaming;
			layerPak.GetStatus(entry.status);

			stats.m_entries.push_back(entry);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::StreamAsyncOnComplete(
  IReadStream* pStream, unsigned nError)
{
	if (nError != 0)
		return;

	SAsyncPak* pLayerPak = (SAsyncPak*) pStream->GetUserData();

	//Check if pak is already open, if so, just assign mem
	if (gEnv->pDrxPak->LoadPakToMemory(pLayerPak->filename.c_str(), IDrxPak::eInMemoryPakLocale_GPU, pLayerPak->pData))
	{
		pLayerPak->bPakAlreadyOpen = true;
	}
	else
	{
		//
		// ugly hack - depending on the pak file pak may need special root info / open flags
		//
		if (pLayerPak->layername.find("level.pak") != string::npos)
		{
			gEnv->pDrxPak->OpenPack(pLayerPak->filename.c_str(),
			                        IDrxPak::FLAGS_FILENAMES_AS_CRC32, NULL);
		}
		else if (pLayerPak->layername.find("levelshadercache.pak") != string::npos)
		{
			DrxPathString pakBindRoot = PathUtil::AddSlash(DrxPathString(gEnv->pDrxPak->GetGameFolder()));
			gEnv->pDrxPak->OpenPack(pakBindRoot.c_str(), pLayerPak->filename.c_str(),
			                        IDrxPak::FLAGS_PATH_REAL, NULL);
		}
		else
		{
			DrxPathString pakBindRoot = PathUtil::AddSlash(DrxPathString(gEnv->pDrxPak->GetGameFolder()));
			gEnv->pDrxPak->OpenPack(pakBindRoot.c_str(), pLayerPak->filename.c_str(),
			                        IDrxPak::FLAGS_FILENAMES_AS_CRC32, NULL);
		}
		gEnv->pDrxPak->LoadPakToMemory(pLayerPak->filename.c_str(), IDrxPak::eInMemoryPakLocale_GPU, pLayerPak->pData);
	}

	pLayerPak->eState = SAsyncPak::STATE_LOADED;

	//printf("Finished streaming level pak: %s\n", pLayerPak->layername.c_str());
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::StreamOnComplete(
  IReadStream* pStream, unsigned nError)
{
	SAsyncPak* pLayerPak = (SAsyncPak*) pStream->GetUserData();

	if (nError != 0)
	{
		ReleaseData(pLayerPak);
	}

	pLayerPak->bStreaming = false;
	pLayerPak->pReadStream = NULL;

	m_bRequestLayerUpdate = true;
}

uk CAsyncPakUpr::StreamOnNeedStorage(IReadStream* pStream, unsigned nSize, bool& bAbortOnFailToAlloc)
{
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "In-Memory Pak: %s", pStream->GetName());

	SAsyncPak* pAsyncPak = (SAsyncPak*)pStream->GetUserData();

	pAsyncPak->nSize = nSize;

	if ((m_nTotalOpenLayerPakSize + nSize) > (size_t)(g_cvars.pakVars.nTotalInMemoryPakSizeLimit * MEGA_BYTE))
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Not enough space to load in memory layer pak %s (Current: %" PRISIZE_T " Required: %u)",
		           pAsyncPak->filename.c_str(), m_nTotalOpenLayerPakSize, nSize);

		//printf("Not enough space to load in memory layer pak %s (Current: %d Required: %d)\n", pAsyncPak->filename.c_str(), m_nTotalOpenLayerPakSize, nSize);

		pAsyncPak->eState = SAsyncPak::STATE_UNLOADED;
		pAsyncPak->bStreaming = false;
		pAsyncPak->pReadStream = NULL;

		bAbortOnFailToAlloc = true;

		return NULL;
	}

	if (nSize)
	{
		CDrxPak* pDrxPak = static_cast<CDrxPak*>(gEnv->pDrxPak);

		// allocate the data
		tukk szUsage = "In Memory Zip File";
		pAsyncPak->pData = pDrxPak->GetInMemoryPakHeap()->AllocateBlock(nSize, szUsage);
		pAsyncPak->pData->AddRef();

		m_nTotalOpenLayerPakSize += nSize;

		return pAsyncPak->pData->GetData();
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void CAsyncPakUpr::Update()
{
	if (!m_bRequestLayerUpdate)
		return;

	m_bRequestLayerUpdate = false;

	for (TPakMap::iterator it = m_paks.begin();
	     it != m_paks.end(); ++it)
	{
		SAsyncPak& layerPak = it->second;
		if (!layerPak.bStreaming)
		{
			if (layerPak.eState == SAsyncPak::STATE_REQUESTUNLOAD)
			{
				// done streaming and not interested in it anymore, then release it again
				ReleaseData(&layerPak);
			}
			else if (layerPak.eState == SAsyncPak::STATE_REQUESTED &&
			         (m_nTotalOpenLayerPakSize + layerPak.nSize <= ((size_t)g_cvars.pakVars.nTotalInMemoryPakSizeLimit * MEGA_BYTE)))
			{
				// do we have enough memory now to start streaming the pak
				StartStreaming(&layerPak);
			}
		}
	}
}

// Abort streaming jobs and prevent any more requests
// Paks which are loaded remain, they will be cleaned up as usual
void CAsyncPakUpr::CancelPendingJobs()
{
	for (TPakMap::iterator it = m_paks.begin(); it != m_paks.end(); ++it)
	{
		SAsyncPak& layerPak = it->second;

		if (layerPak.bStreaming)
		{
			layerPak.pReadStream->Abort();
			ReleaseData(&layerPak);

			//printf("Pak %s Aborted\n", layerPak.filename.c_str());
		}
		else if (layerPak.eState == SAsyncPak::STATE_REQUESTED)
		{
			layerPak.eState = SAsyncPak::STATE_UNLOADED;
			ReleaseData(&layerPak);

			//printf("Pak %s Cancelled\n", layerPak.filename.c_str());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
