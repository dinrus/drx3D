// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: Handles the downloading and managing of various patch paks

	-------------------------------------------------------------------------
	История:
	- 31:01:2012  : Created by James Bamford

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PatchPakUpr.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Sys/ZLib/IZLibCompressor.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/WarningsUpr.h>


#if defined(_RELEASE) 
	#define PATCH_PAK_MGR_DEBUG 0
#else
	#define PATCH_PAK_MGR_DEBUG 1
#endif

#define k_defaultPatchPakEnabled						1
#define k_defaultPatchPakDownloadTimeOut		10
#define k_defaultPatchPakPollTime						300 // 5 mins default is overideable within permissions xml to get a new poll update time
#define k_defaultPatchPakDebug							0
#define k_defaultPatchPakDediServerMustPatch	0

void CPatchPakMemoryBlock::CopyMemoryRegion( uk pOutputBuffer,size_t nOffset,size_t nSize )
{
	if (nOffset+nSize <= m_nSize)
	{
		memcpy( pOutputBuffer,(u8*)m_pData + nOffset,nSize );
	}
	else
	{
		DrxFatalError( "Bad CopyMemoryRegion range" );
	}
}

CPatchPakUpr::SPatchPakData::SPatchPakData()
	: m_showingSaveMessageTimer(0.f)
	, m_actualCRC32(0)
	, m_MD5CRC32(0)
	, m_downloadSize(0)
	, m_state(es_Uninitialised)
	, m_MD5FileName(false)
	, m_showingSaveMessage(false)
{
	std::memset(m_pMD5, 0, sizeof(m_pMD5));
}

CPatchPakUpr::CPatchPakUpr()
	: m_numPatchPaksDownloading(0)
	, m_numPatchPaksFailedToDownload(0)
	, m_numPatchPaksFailedToCache(0)
	, m_patchPakVersion(0)
	, m_installedPermissionsCRC32(0)
	, m_updatedPermissionsAvailable(false)
	, m_wasInFrontend(true)
	, m_isUsingCachePaks(false)
	, m_patchPakRemoved(false)
	, m_timeThatUpdatedPermissionsAvailable(0)
	, m_userThatIsUsingCachePaks(-1)
	, m_versionMismatchTimer(0.f)
	, m_blockingUpdateInProgress(false)
	, m_haveParsedPermissionsXML(false)
	, m_failedToDownloadPermissionsXML(false)
	, m_versionMismatchOccurred(false)
{
	DrxLog("CPatchPakUpr::CPatchPakUpr()");

	SetState(eMS_waiting_to_initial_get_permissions);

	m_patchPakEnabled=REGISTER_INT("g_patchpak_enabled", k_defaultPatchPakEnabled, 0, "is patch paking enabled. Not suitable for runtime adjustment");
	m_patchPakDownloadTimeOut=REGISTER_FLOAT("g_patchpak_download_timeout", k_defaultPatchPakDownloadTimeOut, 0, 
		"Usage: g_patchpak_download_timeout <time in seconds>\n");
	m_patchPakPollTime=REGISTER_FLOAT("g_patchpak_poll_time", k_defaultPatchPakPollTime, 0, 
		"Usage: g_patchpak_poll_time <time in seconds between permissions polling\n");
	m_patchPakDebug=REGISTER_INT("g_patchpak_debug", k_defaultPatchPakDebug, 0, "turn on watch debugging of patch paks");
	m_patchPakDediServerMustPatch=REGISTER_INT("g_patchPakDediServerMustPatch", k_defaultPatchPakDediServerMustPatch, 0, "if set, dedi servers MUST be able to access the patch download server, or they will fatal error and exit");

	m_enabled = m_patchPakEnabled ? (m_patchPakEnabled->GetIVal() ? true : false) : false;	// don't allow enabled changes whilst running

	GetISystem()->GetPlatformOS()->AddListener(this, "CPatchPakUpr");

	m_pollPermissionsXMLTimer = m_patchPakPollTime ? m_patchPakPollTime->GetFVal() : 0.0f;
	m_okToCachePaks=true;	
	m_lastOkToCachePaks=true;
}

CPatchPakUpr::~CPatchPakUpr()
{
	DrxLog("CPatchPakUpr::~CPatchPakUpr()");
	
	u32 numPatchPaks=m_patchPaks.size();
	for (u32 i=0; i<numPatchPaks; i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];
		
		DrxLog("CPatchPakUpr::~CPatchPakUpr() pak[%d] url=%s; state=%d; m_downloadableResource=%p", i, patchPakData.m_url.c_str(), patchPakData.m_state, patchPakData.m_downloadableResource.get());
		{
			if (patchPakData.m_state == SPatchPakData::es_PakLoadedFromCache)
			{
				IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
				IPlatformOS::ECDP_Close closeResult = pPlatformOS->CloseCachePak(patchPakData.m_url.c_str());
				DRX_ASSERT(closeResult == IPlatformOS::eCDPC_Success);
				patchPakData.m_state = SPatchPakData::es_Cached;
			}
			else if (patchPakData.m_state == SPatchPakData::es_PakLoaded)
			{
				// close the pak file
				DrxFixedStringT<64> nameStr;
				GeneratePakFileNameFromURLName(nameStr, patchPakData.m_url.c_str());

				u32 nFlags = IDrxPak::FLAGS_NEVER_IN_PAK|IDrxPak::FLAGS_PATH_REAL|IDrxArchive::FLAGS_OVERRIDE_PAK;

				bool bSuccess=gEnv->pDrxPak->ClosePack(nameStr.c_str(), nFlags);
				DRX_ASSERT_MESSAGE(bSuccess, "we failed to close our patch pak, pack file. Not good!");

				DRX_ASSERT(patchPakData.m_pPatchPakMemBlock.get());

				// should deconstruct here as refcount == 1
				DRX_ASSERT_MESSAGE(patchPakData.m_pPatchPakMemBlock->GetRefCount() == 1, "we're trying to release our memblock but something else still holds a ref to it!");
			}

			// nuke the actual buffer
			if (patchPakData.m_downloadableResource.get())
			{
				patchPakData.m_downloadableResource->RemoveDataListener(this);	// ensure we don't receive failed to download and remove this patchpak element mid-iteration
				patchPakData.m_downloadableResource->CancelDownload();
				patchPakData.m_downloadableResource = NULL;
			}
		}
	}

	if (m_pPermissionsDownloadableResource.get())
	{
		m_pPermissionsDownloadableResource->RemoveDataListener(this);
		m_pPermissionsDownloadableResource->DispatchCallbacks();
		m_pPermissionsDownloadableResource->CancelDownload();
		m_pPermissionsDownloadableResource = NULL;
	}

	// all platforms can stop using cache paks on tear down - we may or may not be using them
	// depending on whether we've been into MP
	if (m_isUsingCachePaks)
	{
		i32k user = g_pGame->GetExclusiveControllerDeviceIndex();
		DrxLog("CPatchPakUpr::~CPatchPakUpr() m_isUsingCachePaks=%d; m_userThatIsUsingCachePaks=%d; current user=%d", m_isUsingCachePaks, m_userThatIsUsingCachePaks, user);
		i32k ret = gEnv->pSystem->GetPlatformOS()->EndUsingCachePaks(user);
		if (ret != IPlatformOS::eCDPE_Success)
		{
			DrxLog("CPatchPakUpr::~CPatchPakUpr() EndUsingCachePaks('%d') failed ERROR %d", user, ret);
		}
	}

	IConsole		*ic=gEnv->pConsole;
	if (ic)
	{
		ic->UnregisterVariable(m_patchPakEnabled->GetName());
		ic->UnregisterVariable(m_patchPakDownloadTimeOut->GetName());
		ic->UnregisterVariable(m_patchPakPollTime->GetName());
		ic->UnregisterVariable(m_patchPakDebug->GetName());
	}

	GetISystem()->GetPlatformOS()->RemoveListener(this);
}

void CPatchPakUpr::Update(float frameTime)
{
	if (!m_enabled)
	{
		return;
	}

#if PATCH_PAK_MGR_DEBUG
	if (m_patchPakDebug->GetIVal()) 
	{
		switch(m_state)
		{
			case eMS_waiting_to_initial_get_permissions:
				DrxWatch("waiting to initial get permissions");
				break;
			case eMS_initial_get_permissions_requested:
				DrxWatch("inital get permissions requested");
				break;
			case eMS_initial_permissions_downloaded:
				DrxWatch("inital permissions downloaded");
				break;
			case eMS_patches_failed_to_download_in_time:
				DrxWatch("failed to download patches in time");
				break;
			case eMS_patches_failed_to_download_in_time_but_now_all_cached_or_downloaded:
				DrxWatch("failed to download patches in time, but now all cached or downloaded");
				break;
			case eMS_patches_installed:
				DrxWatch("patches installed - timeTillNextPoll=%.2f", m_pollPermissionsXMLTimer);
				break;
			case eMS_patches_installed_permissions_requested:
				DrxWatch("patches installed permissions requested");
				break;
			case eMS_patches_installed_permissions_downloaded:
				DrxWatch("patches installed permissions_downloaded");
				break;
			default:
				DrxWatch("unhandled state=%d", m_state);
		}

		if (m_updatedPermissionsAvailable)
		{
			time_t currentTime;
			time( &currentTime);

			time_t timeDiff = currentTime - m_timeThatUpdatedPermissionsAvailable;
			DrxWatch("new permissions xml has been available for %d secs", timeDiff);
		}

		if (!m_okToCachePaks)
		{
			DrxWatch("NOT ok to cache paks yet");
		}
		if (!m_isUsingCachePaks)
		{
			DrxWatch("NOT using cache paks!");
		}
	}
#endif // PATCH_PAK_MGR_DEBUG

	i32 numPatchPaksShowingSaveMessage=0;
	for (u32 i=0; i<m_patchPaks.size(); i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];
		if (patchPakData.m_showingSaveMessage)
		{
			numPatchPaksShowingSaveMessage++;
		}
	}

	for (u32 i=0; i<m_patchPaks.size(); i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];

#if PATCH_PAK_MGR_DEBUG
		if (m_patchPakDebug->GetIVal())
		{
			DrxWatch("patch[%d] %s state=%d; bindRoot=%s", i, patchPakData.m_url.c_str(), patchPakData.m_state, patchPakData.m_pakBindRoot.c_str());
		}
#endif

		if (patchPakData.m_showingSaveMessage)
		{
			float newShowingSaveMessageTimer = patchPakData.m_showingSaveMessageTimer + frameTime;
			patchPakData.m_showingSaveMessageTimer = newShowingSaveMessageTimer;

			if (newShowingSaveMessageTimer > 0.5f)
			{
				patchPakData.m_showingSaveMessage=false;
				numPatchPaksShowingSaveMessage--;
			}
		}


		if (patchPakData.m_downloadableResource)
		{

#if PATCH_PAK_MGR_DEBUG
			if (m_patchPakDebug->GetIVal())
			{
#if DOWNLOAD_MGR_DBG
				if (m_blockingUpdateInProgress)
				{
					DrxFixedStringT<512> dbgStr;
					patchPakData.m_downloadableResource->DebugContentsToString(dbgStr);
					DrxLog("CPatchPakUpr::Update() doing a blocking update - resource %d in state %s", i, dbgStr.c_str());
				}
				else
				{
					patchPakData.m_downloadableResource->DebugWatchContents();
				}
#endif
			}
#endif // PATCH_PAK_MGR_DEBUG

			m_patchPakRemoved = false;
			patchPakData.m_downloadableResource->DispatchCallbacks();
			if (m_patchPakRemoved)
			{
				// this element has already been removed. adjust the iterator accordingly
				i--;
			}
		}
	}

	switch(m_state)
	{
		case eMS_patches_installed:
			if (!m_updatedPermissionsAvailable)	// only want to poll for updated permissions if we don't think we already have some
			{
				bool isInFrontend;
				if (gEnv->IsDedicated())
				{
					isInFrontend = !g_pGame->GetIGameFramework()->StartingGameContext() && !g_pGame->GetIGameFramework()->StartedGameContext();
				}
				else
				{
					isInFrontend = (!g_pGame->IsGameActive() && g_pGame->GetUI()->IsInMenu());
				}
				bool forcePolling=false;

				if (!m_wasInFrontend && isInFrontend)
				{
					DrxLog("CPatchPakUpr::Update() we weren't in the frontend last frame, and are this frame. We want to do a permissions check NOW!");
					forcePolling=true;
				}

				if (isInFrontend)
				{
					m_pollPermissionsXMLTimer-=frameTime;
					if (m_pollPermissionsXMLTimer <= 0.f || forcePolling)
					{
						DrxLog("CPatchPakUpr::Update() we're installed and it's time to checkout the latest permissions.xml");


						SetState(eMS_patches_installed_permissions_requested);

						if(m_pPermissionsDownloadableResource)
						{
							m_pPermissionsDownloadableResource->Purge();
							m_pPermissionsDownloadableResource->StartDownloading();
							DrxLog("CPatchPakUpr::Update() re-downloading permissions.xml with a purge and StartDownloading");
						}
						else
						{
							DrxLog("CPatchPakUpr::Update() invalid resource setup for permissions.xml, skipping...");
						}
					}
				}
				
				m_wasInFrontend = isInFrontend;
			}
			break;
		case eMS_patches_installed_permissions_requested:
			if (m_versionMismatchOccurred)
			{
				m_versionMismatchTimer -= frameTime;

				if (m_versionMismatchTimer <= 0.f)
				{
					DrxLog("CPatchPakUpr::Update() failed to re-download permissions.xml whilst waiting to show the user a version mismatch has occurred, showing the generic message");
					ShowErrorForVersionMismatch(eVMF_unknown);
					m_versionMismatchOccurred=false;
				}
			}
			break;
	}
}

// will block until all downloads completed or a timeout occurs
void CPatchPakUpr::BlockingUpdateTillDone()
{
	if (!m_enabled)
	{
		DrxLog("CPatchPakUpr::BlockingUpdateTillDone() is early returning as patchpaks aren't enabled!");
		return;
	}

	bool showingSaveMessage=false;

	if (!m_isUsingCachePaks)
	{
		DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we're not using cache paks. m_okToCachePaks=%d", m_okToCachePaks);
		if (m_okToCachePaks)
		{
			i32k user = g_pGame->GetExclusiveControllerDeviceIndex();
			bool writesOccurred=false;
			i32k ret = gEnv->pSystem->GetPlatformOS()->StartUsingCachePaks(user, &writesOccurred);

			if (writesOccurred)
			{
				DrxLog("CPatchPakUpr::BlockingUpdateTillDone() writesOccurred==true so showing save message");
#if 0 // old frontend
				// Ok so the writes have technically already occurred but its the best we can do and the writes are very fast here, tell the user
				CFlashFrontEnd *pFlashFrontEnd = g_pGame->GetFlashMenu();
				if (pFlashFrontEnd)
				{
					pFlashFrontEnd->ShowSaveMessage(true);
					showingSaveMessage=true;
				}
#endif
			}

			DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we're ok to cache paks, starting using paks with user=%d starting ret=%d", user, ret);
			if (ret == IPlatformOS::eCDPS_Success)
			{
				DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we succeeded in start using cache paks, hurray!");
				m_isUsingCachePaks=true;
				m_userThatIsUsingCachePaks=user;
			}
			else
			{
				DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we failed to start using cache paks. This should NOT happen!");
			}
		}
		else
		{
			DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we're not using cache paks, nor are we okToCachePaks... eek");
		}
	}

	m_numPatchPaksFailedToDownload=0;
	m_failedToDownloadPermissionsXML=false;
	RequestPermissionsXMLDownload(); // Done super late now.. we have caching now so any delays into MP will be only suffered once per new patch

	CTimeValue startTime=gEnv->pTimer->GetAsyncCurTime();
	float downloadTimeOut=m_patchPakDownloadTimeOut->GetFVal();
	bool success=true;

	if (m_failedToDownloadPermissionsXML)
	{
		DrxLog("CPatchPakUpr::BlockingUpdateTillDone() we've failed to download permissions xml. So just bailing with no patches");
	}
	else if (m_haveParsedPermissionsXML && m_numPatchPaksDownloading <= 0)
	{
		DrxLog("CPatchPakUpr::BlockingUpdateTillDone() has found no patch paks needing to be downloaded.");
	}
	else
	{
		DrxLog("CPatchPakUpr::BlockingUpdateTillDone()");
		m_blockingUpdateInProgress=true;

		float timeTaken=0.f;
		do 
		{
			CTimeValue now=gEnv->pTimer->GetAsyncCurTime();
			timeTaken=(now-startTime).GetSeconds();

			i32k sleepTimeMs=100;
			const float sleepTimeS=sleepTimeMs/1000.f;

			DrxSleep(sleepTimeMs);
			Update(sleepTimeS);

			// we need to leave the save message on for an amount of time to ensure that the RT has picked it up and is displaying it
			if (showingSaveMessage && timeTaken > 0.5f)
			{
#if 0 // old frontend
				CFlashFrontEnd *pFlashFrontEnd = g_pGame->GetFlashMenu();
				if (pFlashFrontEnd)
				{
					pFlashFrontEnd->ShowSaveMessage(false);		// the frontend will keep this displaying for the correct amount of time to be visible
				}
#endif
				showingSaveMessage=false;
			}


			if (!m_haveParsedPermissionsXML && m_pPermissionsDownloadableResource)
			{
				m_pPermissionsDownloadableResource->DispatchCallbacks();	
			}

			downloadTimeOut=m_patchPakDownloadTimeOut->GetFVal();		// permissions xml COULD have updated the download time out during its DataDownloaded callback

		} while (timeTaken < downloadTimeOut && (!m_haveParsedPermissionsXML || m_numPatchPaksDownloading>0) );

		// only presume we've failed if we still have work left to do after running out of time. 
		// Or we've actually failed to download any patch
		if ( (timeTaken >= downloadTimeOut && (!m_haveParsedPermissionsXML || m_numPatchPaksDownloading>0)) ||
			   m_numPatchPaksFailedToDownload>0 )
		{
			DrxLog("CPatchPakUpr::BlockingUpdateTillDone() has timed out waiting %f seconds for the patch paks to finish downloading", downloadTimeOut);
			success=false;
			SetState(eMS_patches_failed_to_download_in_time);

			if (gEnv->IsDedicated() && m_patchPakDediServerMustPatch->GetIVal())
			{
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				DrxLogAlways("This server has failed to download patch paks or permissions, so quitting. This server cannot host multiplayer correctly");
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				DrxLogAlways("--------------------------------------------------------------------------------------------------------------------");
				gEnv->pSystem->Quit();
			}
		}

		m_blockingUpdateInProgress=false;
	}

	if (success)
	{
		DRX_ASSERT(m_numPatchPaksDownloading<=0);

		DrxLog("CPatchPakUpr::BlockingUpdateTillDone() has finished downloading all patch paks ready to actually open them as pak files");

		for (u32 i=0; i<m_patchPaks.size(); i++)
		{
			SPatchPakData &patchPakData=m_patchPaks[i];
	
			if ( (patchPakData.m_state == SPatchPakData::es_Downloaded && patchPakData.m_downloadableResource) ||
				   (patchPakData.m_state == SPatchPakData::es_Cached) )
			{
				OpenPatchPakDataAsPak(&patchPakData);
			}
		}
		
		SetState(eMS_patches_installed);

		m_pollPermissionsXMLTimer = m_patchPakPollTime->GetFVal();
	}
		
	// incase we didn't block long enough (due to only grabbing permissions) to cancel showing the save message do it here
	if (showingSaveMessage)
	{
#if 0 // old frontend
		CFlashFrontEnd *pFlashFrontEnd = g_pGame->GetFlashMenu();
		if (pFlashFrontEnd)
		{
			pFlashFrontEnd->ShowSaveMessage(false);		// the frontend will keep this displaying for the correct amount of time to be visible
		}
#endif
		showingSaveMessage=false;
	}

	m_updatedPermissionsAvailable=false; // if we failed to download intime we'll re-enable this after all patches have finished downloading.
}



// IPlatformOS::IPlatformListener
void CPatchPakUpr::OnPlatformEvent(const IPlatformOS::SPlatformEvent& event)
{
	DrxLog("CPatchPakUpr::OnPlatformEvent() eventType=%d; user=%d (exclusive user=%d)", event.m_eEventType, event.m_user, g_pGame->GetExclusiveControllerDeviceIndex());

	switch(event.m_eEventType)
	{
		case IPlatformOS::SPlatformEvent::eET_InstallComplete:
			DrxLog("CPatchPakUpr::OnPlatformEvent() eET_InstallComplete - ok to cache paks now");
			m_okToCachePaks=true;
			break;
		case IPlatformOS::SPlatformEvent::eET_SignIn:
		{
			break;
		}
		case IPlatformOS::SPlatformEvent::eET_StorageMounted: 
		{
			DrxLog("CPatchPakUpr::OnPlatformEvent() eET_StorageMounted; bPhysicalMedia=%d", event.m_uParams.m_storageMounted.m_bPhysicalMedia);
			break;
		}
		case IPlatformOS::SPlatformEvent::eET_StorageRemoved:
			DrxLog("CPatchPakUpr::OnPlatformEvent() eET_StorageRemoved deviceRemovedIsPrimary=%d", event.m_uParams.m_storageRemoved.m_bDeviceRemovedIsPrimary);
			break;
	}
}

// IDataListener
void CPatchPakUpr::DataDownloaded(CDownloadableResourcePtr inResource)
{
	DrxLog("CPatchPakUpr::DataDownloaded()");

	if (inResource == m_pPermissionsDownloadableResource)
	{
		DrxLog("CPatchPakUpr::DataDownloaded() we've downloaded our permissions.xml okToCachePaks=%d", m_okToCachePaks);

		if (m_state == eMS_initial_get_permissions_requested)
		{
			SetState(eMS_initial_permissions_downloaded);
		}
		else
		{
			SetState(eMS_patches_installed_permissions_downloaded);
		}

		// we've received our permissions file back
		ProcessPermissionsXML(inResource);
	}
	else
	{
		// we've received a patch pak itself back
		PatchPakDataDownloaded(inResource);
	}
}

void CPatchPakUpr::DataFailedToDownload(CDownloadableResourcePtr inResource)
{
	DrxLog("CPatchPakUpr::DataFailedToDownload()");
	if (inResource == m_pPermissionsDownloadableResource)
	{
		DrxLog("failed to download permissions.xml - no patch manifest available");
		m_failedToDownloadPermissionsXML=true;

		if (m_state == eMS_patches_installed_permissions_requested)
		{
			DrxLog("CPatchPakUpr::DataFailedToDownload() whilst in state eMS_patches_installed_permissions_requested, we need to return to our installed state. Hopefully this will work next time!");
			SetState(eMS_patches_installed);
		}

		if (m_versionMismatchOccurred)
		{
			ShowErrorForVersionMismatch(eVMF_unknown);
			m_versionMismatchOccurred=false;
		}
	}
	else
	{
		i32 patchPakIndex=GetPatchPakDataIndexFromDownloadableResource(inResource);

		DrxLog("failed to download patch pak url=%s", patchPakIndex >= 0 ? m_patchPaks[patchPakIndex].m_url.c_str() : "N/A");
		if (patchPakIndex >= 0)
		{
			m_patchPaks.removeAt(patchPakIndex); // will deconstruct and clear the smart ptr
			m_patchPakRemoved=true;
			m_numPatchPaksFailedToDownload++;
			m_numPatchPaksDownloading--;
		}
	}
}
// ~IDataListener

void CPatchPakUpr::RequestPermissionsXMLDownload()
{
	if (!m_enabled)
	{
		DrxLog("CPatchPakUpr::RequestPermissionsXMLDownload() is early returning as patchpaks aren't enabled!");
		return;
	}

	CDownloadMgr		*pDL=g_pGame->GetDownloadMgr();
	if (pDL)
	{
		CDownloadableResourcePtr		pRes=pDL->FindResourceByName("permissions");

		// if data already downloaded our callbacks will fire immediately. So we need to cache the resource ready for comparison in that case.
		m_pPermissionsDownloadableResource = pRes;

		if (pRes.get())
		{
			// ensure state is updated before adding listener, incase we're already downloaded and will receive callbacks immediately
			{
				m_haveParsedPermissionsXML=false;	// this is only used when actually installing a permissions.xml, not when we're polling after already being installed
				SetState(eMS_initial_get_permissions_requested);
			}

			pRes->AddDataListener(this);	
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME,VALIDATOR_ERROR,"CPatchPakUpr failed to start downloading permissions.xml");
			m_failedToDownloadPermissionsXML = true;
		}

	}
	else
	{
		DrxLog("CPatchPakUpr::RequestPermissionsXMLDownload() - ERROR - failed to find download mgr. CPatchPakUpr must be instantiated after the CDownloadMgr");		
	}
}

// will unload all patch paks, but still keep the data ready to be reloaded when needed
void CPatchPakUpr::UnloadPatchPakFiles()
{
	DrxLog("CPatchPakUpr::UnloadPatchPakFiles()");

	for (u32 i=0; i<m_patchPaks.size(); i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];
		
		if (patchPakData.m_state == SPatchPakData::es_PakLoadedFromCache)
		{
			IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
			IPlatformOS::ECDP_Close closeResult = pPlatformOS->CloseCachePak(patchPakData.m_url.c_str());
			DrxLog("CPatchPakUpr::UnloadPatchPakFiles() closing cache pak %s with result=%d", patchPakData.m_url.c_str(), closeResult);
			DRX_ASSERT(closeResult == IPlatformOS::eCDPC_Success);
			patchPakData.m_state = SPatchPakData::es_Cached;
		}
		else if (patchPakData.m_state == SPatchPakData::es_PakLoaded)
		{
			// close the pak file
			DrxFixedStringT<64> nameStr;
			GeneratePakFileNameFromURLName(nameStr, patchPakData.m_url.c_str());

			u32 nFlags = IDrxPak::FLAGS_NEVER_IN_PAK|IDrxPak::FLAGS_PATH_REAL|IDrxArchive::FLAGS_OVERRIDE_PAK;

			bool bSuccess=gEnv->pDrxPak->ClosePack(nameStr.c_str(), nFlags);
			DRX_ASSERT_MESSAGE(bSuccess, "we failed to close our patch pak, pack file. Not good!");

			// should deconstruct here as refcount == 1
			DRX_ASSERT(patchPakData.m_pPatchPakMemBlock.get());
			DRX_ASSERT_MESSAGE(patchPakData.m_pPatchPakMemBlock->GetRefCount() == 1, "we're trying to release our memblock but something else still holds a ref to it!");
			patchPakData.m_pPatchPakMemBlock = NULL;
			patchPakData.m_state = SPatchPakData::es_Downloaded;
		}
		else
		{
			DrxLog("CPatchPakUpr::UnloadPatchPakFiles() ERROR - skipping a patch pak in unexepected state =%d", patchPakData.m_state);
		}

		if (patchPakData.m_downloadableResource)
		{
			patchPakData.m_downloadableResource->RemoveDataListener(this);
			patchPakData.m_downloadableResource->CancelDownload();
			patchPakData.m_downloadableResource = NULL;
		}
	}
	
	m_patchPaks.clear();
	
	if (m_pPermissionsDownloadableResource.get())
	{
		m_pPermissionsDownloadableResource->RemoveDataListener(this);
		m_pPermissionsDownloadableResource->DispatchCallbacks(); // ensure we remove ourselves as a data listener
		m_pPermissionsDownloadableResource->CancelDownload();
		m_pPermissionsDownloadableResource->Purge();		// should Purge or CancelDownload() here?
		m_pPermissionsDownloadableResource = NULL;
	}

	SetState(eMS_waiting_to_initial_get_permissions);
}

u32 CPatchPakUpr::GetXOROfPatchPakCRCs()
{
	u32 result=0;

	for (u32 i=0; i<m_patchPaks.size(); i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];

		if (patchPakData.m_state == SPatchPakData::es_PakLoaded || patchPakData.m_state == SPatchPakData::es_PakLoadedFromCache)
		{
			result ^= patchPakData.m_MD5CRC32;
		}
	}

	return result;
}

void CPatchPakUpr::ProcessPatchPaksFromPermissionsXML(
	XmlNodeRef root)
{
	tukk pServer=root->getAttr("server");
	tukk pPrefix=root->getAttr("prefix");
	tukk pPort=root->getAttr("port");
	tukk pPatchVersion=root->getAttr("patchVersion");
	tukk pPatchPollTime=root->getAttr("pollTime");
	tukk pBlockingUpdateTimeOut=root->getAttr("blockingUpdateTimeOut");
	
	if (pPatchPollTime && (pPatchPollTime[0] != 0))
	{
		PREFAST_SUPPRESS_WARNING(6387)
		const float newPollTime=(float)(atof(pPatchPollTime));

		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() got newPollTime of %f from str=%s", newPollTime, pPatchPollTime);
		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() updating m_patchPakPollTime from %f to %f", m_patchPakPollTime->GetFVal(), newPollTime);
		m_patchPakPollTime->Set(newPollTime);
	}

	if (pBlockingUpdateTimeOut && (pBlockingUpdateTimeOut[0] != 0))
	{
		PREFAST_SUPPRESS_WARNING(6387)
		const float newBlockingUpdateTimeOut=(float)(atof(pBlockingUpdateTimeOut));

		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() got newBlockingUpdateTimeOut of %f from str=%s", newBlockingUpdateTimeOut, pBlockingUpdateTimeOut);
		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() updating m_patchPakDownloadTimeOut from %f to %f", m_patchPakDownloadTimeOut->GetFVal(), newBlockingUpdateTimeOut);
		m_patchPakDownloadTimeOut->Set(newBlockingUpdateTimeOut);
	}

	if (m_state == eMS_patches_installed_permissions_downloaded)
	{
		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() we've finished polling this permissions.xml now, so early returning. Getting ready to wait for the next poll");
		SetState(eMS_patches_installed);
		m_pollPermissionsXMLTimer = m_patchPakPollTime->GetFVal();
		return;
	}


	DRX_ASSERT(m_enabled);

	if (m_state == eMS_initial_permissions_downloaded)
	{
		// only want to actually change our current patched state if this is not a polling permissions download
		if (pPatchVersion)
		{
			m_patchPakVersion = atoi(pPatchVersion);
		}
	}


	DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() pServer=%s; pPort=%s; pPrefix=%s; patchVersion=%s (%d) m_state=%d", pServer ? pServer : "NULL", pPort ? pPort : "NULL", pPrefix ? pPrefix : "NULL", pPatchVersion ? pPatchVersion : "NULL", m_patchPakVersion, m_state);

	DRX_ASSERT(m_numPatchPaksDownloading==0);

	if (pServer && (pServer[0] != 0) && pPort && (pPort[0] != 0) && pPrefix && (pPrefix[0] != 0))
	{	
		PREFAST_SUPPRESS_WARNING(6387)
		i32 port=atoi(pPort);

		i32k numChildren=root->getChildCount();
		for (i32 i=0; i<numChildren; i++)
		{
			if (XmlNodeRef xmlChild=root->getChild(i))
			{
				if (stricmp(xmlChild->getTag(), "pak")==0)
				{
					tukk pName=xmlChild->getAttr("name");
					tukk pURL=xmlChild->getAttr("url");
					tukk pPakBindRoot=xmlChild->getAttr("pakBindRoot");
					tukk pMD5FileName=xmlChild->getAttr("md5FileName");
					tukk pMD5Str=xmlChild->getAttr("md5");
					tukk pCRC32=xmlChild->getAttr("crc32");
					tukk pSize=xmlChild->getAttr("size");
					tukk pCacheToDisk=xmlChild->getAttr("cacheToDisk");
					tukk pType=xmlChild->getAttr("type");

					DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() pak %d name=%s; url=%s; md5=%s; cacheToDisk=%s; type=%s", i, pName ? pName : "NULL", pURL ? pURL : "NULL", pMD5Str ? pMD5Str : "NULL", pCacheToDisk ? pCacheToDisk : "NULL", pType ? pType : "NULL"); 
					
					if (pName && (pName[0] != 0) && pURL && (pURL[0] != 0) && pMD5Str && (pMD5Str[0] != 0) && 
						  pSize && (pSize[0] != 0) && pCacheToDisk && (pCacheToDisk[0] != 0) && pType && (pType[0] != 0))
					{
						PREFAST_SUPPRESS_WARNING(6387)
						i32 downloadSize=atoi(pSize);
						i32 maxSize=downloadSize+k_maxHttpHeaderSize;
						bool bMD5FileName=false;
						if (pMD5FileName && pMD5FileName[0] != 0)
						{
							i32 md5FileName=atoi(pMD5FileName);
							if (md5FileName)
							{
								bMD5FileName=true;
							}
						}

						if (m_state == eMS_initial_permissions_downloaded)
						{
							DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() pak url=%s this is the initial permissions download so starting new download", pURL);
							StartNewDownload(pServer, port, pPrefix, pURL, pPakBindRoot, downloadSize, pMD5Str, bMD5FileName, pName);
						}
					}
					else
					{
						DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() has found a pak file (%s) with incomplete parameters", pURL ? pURL : "NULL");
					}
				}
			}
		}
	}
	else
	{
		DrxLog("CPatchPakUpr::ProcessPatchPaksFromPermissionsXML() has failed to find valid server, port and prefix");
	}

	m_haveParsedPermissionsXML=true;
}

i32 CPatchPakUpr::GetPatchPakDataIndexFromDownloadableResource(CDownloadableResourcePtr inResource)
{
	i32 retIndex=-1;

	u32 numPatchPaks=m_patchPaks.size();
	for (u32 i=0; i<numPatchPaks; i++)
	{
		SPatchPakData &patchPakData=m_patchPaks[i];
		if (patchPakData.m_downloadableResource == inResource)
		{
			retIndex = i;
			break;
		}
	}

	return retIndex;
}

CPatchPakUpr::SPatchPakData *CPatchPakUpr::GetPatchPakDataFromDownloadableResource(CDownloadableResourcePtr inResource)
{
	SPatchPakData *retData=NULL;
	i32 index=GetPatchPakDataIndexFromDownloadableResource(inResource);

	if (index >= 0)
	{
		retData = &m_patchPaks[index];
	}

	return retData;
}

void CPatchPakUpr::StartNewDownload(tukk inServerName, i32k inPort, tukk inURLPrefix, tukk inURL, tukk inPakBindRoot, i32k inDownloadSize, tukk inMD5, const bool inMD5FileName, tukk inDescName)
{
	DRX_ASSERT(m_enabled);

	if (m_patchPaks.isfull())
	{
		DrxLog("CPatchPakUpr::StartNewDownload() cannot start new download as our array of patchpaks is already full. Adjust the patch pak manifest or increase CPatchPakUpr::k_maxPatchPaks");
	}
	else
	{
		SPatchPakData newPatchPakData;
		DRX_ASSERT(strlen(inMD5)==32);
		newPatchPakData.m_pMD5Str = inMD5;
		newPatchPakData.m_downloadSize = inDownloadSize;
		newPatchPakData.m_url = inURL;
		newPatchPakData.m_MD5FileName = inMD5FileName;

		if (inPakBindRoot)
		{
			newPatchPakData.m_pakBindRoot = inPakBindRoot;
		}

		DrxLog("CPatchPakUpr::StartNewDownload() inURL=%s", inURL);

		tukk pMD5Iter=&inMD5[0];
		for (i32 j=0; j<16; j++)
		{
			u32 element;
			PREFAST_SUPPRESS_WARNING(6387)
			i32 numMatches = sscanf(pMD5Iter, "%02x", &element); // sscanf with %x param writes an i32 regardless of the size of the input definition
			DRX_ASSERT_MESSAGE(numMatches==1, "failed to parse our file's MD5 from permissions");
			newPatchPakData.m_pMD5[j] = static_cast<u8>(element);
			pMD5Iter += 2;
		}

		newPatchPakData.m_MD5CRC32 = CCrc32::Compute(newPatchPakData.m_pMD5, sizeof(newPatchPakData.m_pMD5));

		DRX_ASSERT(newPatchPakData.m_state == SPatchPakData::es_Uninitialised);

		if (m_okToCachePaks && m_isUsingCachePaks)
		{
			DrxLog("CPatchPakUpr::StartNewDownload() we're okToCachePaks, and isUsingCachePaks. Trying the cache pak route");

			IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
			IPlatformOS::ECDP_Open cacheResult = pPlatformOS->DoesCachePakExist(inURL, inDownloadSize, newPatchPakData.m_pMD5);

			switch(cacheResult)
			{
				case IPlatformOS::eCDPO_Success:
					DrxLog("CPatchPakUpr::StartNewDownload() has found this patch pak %s is already cached on this system", inURL);
					newPatchPakData.m_state = SPatchPakData::es_Cached;
					break;
				case IPlatformOS::eCDPO_MD5Failed:
				case IPlatformOS::eCDPO_HashNotMatch:
				case IPlatformOS::eCDPO_SizeNotMatch:
				{
					DrxLog("CPatchPakUpr::StartNewDownload() has found this patch pak %s is already cached on this system but we failed to validate this cached file. Deleting it", inURL);
					IPlatformOS::ECDP_Delete deleteResult = pPlatformOS->DeleteCachePak(inURL);
					if (deleteResult != IPlatformOS::eCDPD_Success)
					{
						DrxLog("CPatchPakUpr::StartNewDownload() was trying to delete cache pak %s but failed to do so. I'm all out of ideas", inURL);
					}

					DRX_ASSERT(pPlatformOS->DoesCachePakExist(inURL, inDownloadSize, newPatchPakData.m_pMD5) == IPlatformOS::eCDPO_FileNotFound);

					// no break; intentionally to fall through
				}
				case IPlatformOS::eCDPO_FileNotFound:
				{
					DrxLog("CPatchPakUpr::StartNewDownload() has found we need to actually download this patch pak %s", inURL);
					i32 maxSize=inDownloadSize+k_maxHttpHeaderSize;

					newPatchPakData.m_state = SPatchPakData::eS_Downloading;

					newPatchPakData.m_downloadableResource = new CDownloadableResource;

					DrxFixedStringT<50> md5FileName;
					tukk remoteFileName = inURL;

					if (inMD5FileName)
					{
						md5FileName.Format("%s.pak", inMD5);
						remoteFileName = md5FileName.c_str();
						DrxLog("CPatchPakUpr::StartNewDownload() has MD5FileName set, so using %s as the remoteFileName instead of inURL=%s", remoteFileName, inURL);
					}

					newPatchPakData.m_downloadableResource->SetDownloadInfo(remoteFileName, inURLPrefix, inServerName, inPort, maxSize, inDescName);
					newPatchPakData.m_downloadableResource->AddDataListener( this );

					m_numPatchPaksDownloading++;

					break;
				}

				default:
					DrxLog("CPatchPakUpr::StartNewDownload() has received an unhandled return of %d from DoesCachePakExist()", cacheResult);
					break;
			}
		}
		else
		{
			DrxLog("CPatchPakUpr::StartNewDownload() we're either not okToCachePaks (%d) or not usingCachePaks (%d). We need to just download our patches as required", m_okToCachePaks, m_isUsingCachePaks);

			i32 maxSize=inDownloadSize+k_maxHttpHeaderSize;

			newPatchPakData.m_state = SPatchPakData::eS_Downloading;
			newPatchPakData.m_downloadableResource = new CDownloadableResource;

			DrxFixedStringT<50> md5FileName;
			tukk remoteFileName = inURL;

			if (inMD5FileName)
			{
				md5FileName.Format("%s.pak", inMD5);
				remoteFileName = md5FileName.c_str();
				DrxLog("CPatchPakUpr::StartNewDownload() has MD5FileName set, so using %s as the remoteFileName instead of inURL=%s", remoteFileName, inURL);
			}

			newPatchPakData.m_downloadableResource->SetDownloadInfo(remoteFileName, inURLPrefix, inServerName, inPort, maxSize, inDescName);
			newPatchPakData.m_downloadableResource->AddDataListener( this );

			m_numPatchPaksDownloading++;
		}

		m_patchPaks.push_back(newPatchPakData);
	}
}

bool CPatchPakUpr::CheckForNewDownload(tukk inServerName, i32k inPort, tukk inURLPrefix, tukk inURL, i32k inDownloadSize, tukk inMD5, tukk inDescName)
{		
	bool newDownloadAvailable=false;

	u8				pMD5[16];

	tukk pMD5Iter=&inMD5[0];
	for (i32 j=0; j<16; j++)
	{
		u32 element;
		PREFAST_SUPPRESS_WARNING(6387)
		i32 numMatches = sscanf(pMD5Iter, "%02x", &element); // sscanf with %x param writes an i32 regardless of the size of the input definition
		DRX_ASSERT_MESSAGE(numMatches==1, "failed to parse our file's MD5 from permissions");
		pMD5[j] = static_cast<u8>(element);
		pMD5Iter += 2;
	}

	IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
	IPlatformOS::ECDP_Open cacheResult = pPlatformOS->DoesCachePakExist(inURL, inDownloadSize, pMD5);

	if (cacheResult != IPlatformOS::eCDPO_Success)
	{
		DrxLog("CPatchPakUpr::CheckForNewDownload() has found cached patch pak %s %d MD5=%s doesn't exist with error=%d", inURL, inDownloadSize, inMD5, cacheResult);
		newDownloadAvailable = true;
	}

	return newDownloadAvailable;
}

void CPatchPakUpr::GeneratePakFileNameFromURLName(DrxFixedStringT<64> &outPakFileName, tukk inUrlName)
{
	tukk gameFolder = gEnv->pDrxPak->GetGameFolder();
	outPakFileName.Format("%s\\%s", gameFolder, inUrlName);
}

void CPatchPakUpr::ProcessPermissionsXML(CDownloadableResourcePtr inResource)
{
	i32		    bufferSize = 0;
	tuk			buffer;

	DRX_ASSERT(m_enabled);

	DrxLog("CPatchPakUpr::ProcessPermissionsXML()");

	inResource->GetRawData(&buffer,&bufferSize);

	if (m_state == eMS_initial_permissions_downloaded)
	{
		m_installedPermissionsCRC32 = CCrc32::Compute(buffer, bufferSize);
		DrxLog("CPatchPakUpr::ProcessPermissionsXML() in state initial permissions downloaded. Saving new installedPermissionsCRC32 of %x", m_installedPermissionsCRC32);
	}
	else
	{
		DRX_ASSERT(m_state == eMS_patches_installed_permissions_downloaded);
		u32 newPermissionsCRC32 = CCrc32::Compute(buffer, bufferSize);
		DrxLog("CPatchPakUpr::ProcessPermissionsXML() this is a polling permissions download. m_state=%d; Testing new permissions CRC=%x against installed permissions CRC=%x", m_state, newPermissionsCRC32, m_installedPermissionsCRC32);
		if (newPermissionsCRC32 == m_installedPermissionsCRC32)
		{
			DrxLog("CPatchPakUpr::ProcessPermissionsXML() polled permissions.xml is unchanged. We don't need to do anything!");

			if (m_versionMismatchOccurred)
			{
				ShowErrorForVersionMismatch(eVMF_their_fault);
				m_versionMismatchOccurred=false;
			}
		}
		else
		{
			DrxLog("CPatchPakUpr::ProcessPermissionsXML() polled permissions.xml is different to our installed permissions. There's a new permissions/patch to download!");
			m_updatedPermissionsAvailable=true;
			time_t currentTime;
			time(&currentTime);
			m_timeThatUpdatedPermissionsAvailable = (i32)currentTime;
			EventUpdatedPermissionsAvailable();

			if (m_versionMismatchOccurred)
			{
				ShowErrorForVersionMismatch(eVMF_our_fault);
				m_versionMismatchOccurred=false;
			}
		}
	}

	IXmlParser *parser=GetISystem()->GetXmlUtils()->CreateXmlParser();
	XmlNodeRef root=parser->ParseBuffer(buffer, bufferSize, false);

	if (!root)
	{
		DrxWarning(VALIDATOR_MODULE_GAME,VALIDATOR_ERROR,"CPatchPakUpr failed to parse the permissions.xml it downloaded");
	}

	if (root && stricmp(root->getTag(),"permissions")==0)
	{
		i32k numChildren = root->getChildCount();
		for (i32 i=0; i<numChildren; i++)
		{
			if (XmlNodeRef xmlChild=root->getChild(i))
			{
				if (stricmp(xmlChild->getTag(), "patch_paks")==0)
				{
					ProcessPatchPaksFromPermissionsXML(xmlChild);
				}
			}
		}
	}

	parser->Release();
}

bool CPatchPakUpr::CachePakDataToDisk(SPatchPakData *pInPakData)
{
	bool succeeded=false;

	i32 bufferSize=-1;
	char *buffer=NULL;
	pInPakData->m_downloadableResource->GetRawData(&buffer, &bufferSize);

	DrxLog("CPatchPakUpr::CachePakDataToDisk() for pak %s", pInPakData->m_url.c_str());

#if 0 // old frontend
	CFlashFrontEnd *pFlashFrontEnd = g_pGame->GetFlashMenu();
	if (pFlashFrontEnd)
	{
		pFlashFrontEnd->ShowSaveMessage(true);
		// we need to ensure we always show the spinner.. 
		// some cache ops complete before the render thread gets a tick to process that it needs to show the save spinner
		// delay it here for at least 0.5f of a second, to ensure that it gets displayed
		pInPakData->m_showingSaveMessage=true;
		pInPakData->m_showingSaveMessageTimer=0.f;
	}
#endif

	IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
	IPlatformOS::ECDP_Write writeResult = pPlatformOS->WriteCachePak(pInPakData->m_url.c_str(), buffer, bufferSize);

	switch (writeResult)
	{
		case IPlatformOS::eCDPW_Success:
			DrxLog("CPatchPakUpr::CachePakDataToDisk() has succeeded in caching patch %s to disk", pInPakData->m_url.c_str());
			pInPakData->m_state = SPatchPakData::es_Cached;
			succeeded=true;
			break;
		case IPlatformOS::eCDPW_NoFreeSpace: // TODO - add any warnings here about free-ing up space if we're allowed to?
		case IPlatformOS::eCDPW_Failure:
			DrxLog("CPatchPakUpr::CachePakDataToDisk() has failed to cache patch %s to disk will have to work with it as is", pInPakData->m_url.c_str());
			break;
		default:
			DrxLog("CPatchPakUpr::CachePakDataToDisk() unhandled writeResult=%d whilst caching pak %s", writeResult, pInPakData->m_url.c_str());
			DRX_ASSERT_MESSAGE(0, string().Format("unhandled writeResult=%d whilst caching pak %s", writeResult, pInPakData->m_url.c_str()));
			break;
	}

	if (!succeeded)
	{
		m_numPatchPaksFailedToCache++;
	}

	return succeeded;
}

void CPatchPakUpr::PatchPakDataDownloaded(CDownloadableResourcePtr inResource)
{
	i32 bufferSize = -1;
	tuk buffer= nullptr;
	inResource->GetRawData( &buffer, &bufferSize );

	DRX_ASSERT(m_enabled);

	DrxLog("CPatchPakUpr::DataDownloaded() resource=%s downloaded %d bytes at %p m_state=%d", inResource->GetDescription(), bufferSize, buffer, m_state);

#if DOWNLOAD_MGR_DBG
	DrxFixedStringT<512> dbgStr;
	inResource->DebugContentsToString(dbgStr);
	DrxLog("CPatchPakUpr::DataDownloaded() with the following stats=%s", dbgStr.c_str());
#endif

	SPatchPakData *pakData = GetPatchPakDataFromDownloadableResource(inResource);
	if (pakData)
	{
		DrxLog("CPatchPakUprDataDownloaded() found corresponding patch pak data");
		DRX_ASSERT(pakData->m_state == SPatchPakData::eS_Downloading);
		m_numPatchPaksDownloading--;
		DRX_ASSERT(m_numPatchPaksDownloading >= 0);

		// verify the MD5
		IZLibCompressor			*pZLib=GetISystem()->GetIZLibCompressor();
		SMD5Context context;

		char	pMD5[16];

		pZLib->MD5Init(&context);
		pZLib->MD5Update(&context,(tukk)buffer, bufferSize);
		pZLib->MD5Final(&context, pMD5);

		DrxLog("CPatchPakUpr::PatchPakDataDownloaded() found patch %s downloaded MD5 of %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", pakData->m_url.c_str(), pMD5[0], pMD5[1], pMD5[2], pMD5[3], pMD5[4], pMD5[5], pMD5[6], pMD5[7], pMD5[8], pMD5[9], pMD5[10], pMD5[11], pMD5[12], pMD5[13], pMD5[14], pMD5[15]);

		if (memcmp(pMD5, pakData->m_pMD5, 16) == 0)
		{
			DrxLog("CPatchPakUpr::PatchPakDataDownloaded() found matching CRCs with expected value.. download is valid. Caching newly downloaded file");
			pakData->m_state = SPatchPakData::es_Downloaded;

			if (m_okToCachePaks && m_isUsingCachePaks)
			{
				DrxLog("CPatchPakUpr::PatchPakDataDownloaded() we're ok to cache paks, so doing so");
				bool succeeeded = CachePakDataToDisk(pakData);
				if (succeeeded)
				{
					DrxLog("CPatchPakUpr::PatchPakDataDownloaded() patch %s succeeded caching.", pakData->m_url.c_str());
				}

				if (m_numPatchPaksDownloading == 0)
				{
					DrxLog("CPatchPakUpr::PatchPakDataDownloaded() %d patch paks have failed to cache. This better be zero!", m_numPatchPaksFailedToCache);
					DRX_ASSERT(m_numPatchPaksFailedToCache==0);
				}
			}
				
			if (m_state == eMS_patches_failed_to_download_in_time && m_numPatchPaksDownloading == 0)
			{
				DrxLog("CPatchPakUpr::PatchPakDataDownloaded() patch %s downloaded too late, and we've got no more patches downloading. Ready to inform user that new patch is availabe", pakData->m_url.c_str());
				SetState(eMS_patches_failed_to_download_in_time_but_now_all_cached_or_downloaded);
				m_updatedPermissionsAvailable=true;
				time_t currentTime;
				time(&currentTime);
				m_timeThatUpdatedPermissionsAvailable = (i32)currentTime;
				EventUpdatedPermissionsAvailable();
			}
		}
		else
		{
			DrxLog("CPatchPakUprDataDownloaded() found mis-matching CRCs with exepected value.. download is corrupt!!");
			pakData->m_state = SPatchPakData::es_DownloadedButCorrupt;
		}

		// TODO - now we're cache pak dependent we should really be freeing up our downloadable resource NOW to save memory
		// although this would remove any possibility of coping (and patching) in a scenario where caching ALWAYS fails
		// if we can take the memory hit in SP then we can keep the old patching implementation as a fallback to failed caching
	}
}

void CPatchPakUpr::OpenPatchPakDataAsPak(SPatchPakData *inPakData)
{
	DRX_ASSERT(m_enabled);

	DrxFixedStringT<128> bindRootPath;
	i32k pakBindRootLength = inPakData->m_pakBindRoot.length();
	if (pakBindRootLength > 0)
	{
		if (pakBindRootLength == 1 && inPakData->m_pakBindRoot.at(0) == '\\')
		{
			// we want the root leave this path totally empty
		}
		else
		{
			bindRootPath.Format("%s\\", inPakData->m_pakBindRoot.c_str());
		}
	}
	else
	{
		tukk gameFolder = gEnv->pDrxPak->GetGameFolder();
		bindRootPath.Format("%s\\", gameFolder);	 // append the trailing \ required for pak paths
	}

	u32 nFlags = IDrxPak::FLAGS_NEVER_IN_PAK|IDrxPak::FLAGS_PATH_REAL|IDrxArchive::FLAGS_OVERRIDE_PAK;


	if (inPakData->m_state == SPatchPakData::es_Cached)
	{
		DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() opening cached pakdata for pak file %s", inPakData->m_url.c_str());
		IPlatformOS *pPlatformOS = GetISystem()->GetPlatformOS();
		IPlatformOS::ECDP_Open openResult = pPlatformOS->OpenCachePak(inPakData->m_url.c_str(), bindRootPath.c_str(), inPakData->m_downloadSize, inPakData->m_pMD5);

		DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() got a result of %d when trying to open cached pak file %s", openResult, inPakData->m_url.c_str());
		if (openResult == IPlatformOS::eCDPO_Success)
		{
			inPakData->m_state = SPatchPakData::es_PakLoadedFromCache;

			DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() succeeded loading pak file %s from cache", inPakData->m_url.c_str());
		}
		else
		{
			DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() failed to open from cache for cached pak file %s", inPakData->m_url.c_str());
			inPakData->m_state = SPatchPakData::es_FailedToOpenFromCache;
		}
	}
	else if (inPakData->m_state == SPatchPakData::es_Downloaded)
	{
		i32 bufferSize = -1;
		tuk buffer;
		inPakData->m_downloadableResource->GetRawData( &buffer, &bufferSize );

		DRX_ASSERT(inPakData->m_pPatchPakMemBlock == NULL);
		inPakData->m_pPatchPakMemBlock = new CPatchPakMemoryBlock(buffer, bufferSize);

		DrxFixedStringT<64> nameStr;
		GeneratePakFileNameFromURLName(nameStr, inPakData->m_url.c_str());

		DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() opening downloaded but not cached pakdata for pak file %s", nameStr.c_str());

		bool success=gEnv->pDrxPak->OpenPack(bindRootPath.c_str(), nameStr.c_str(), nFlags, inPakData->m_pPatchPakMemBlock);
		DRX_ASSERT_MESSAGE(success, string().Format("failed to open pak file for patch pak %s", nameStr.c_str()));
		if (success)
		{
			inPakData->m_state = SPatchPakData::es_PakLoaded;
		}
		else
		{
			gEnv->pDrxPak->ClosePack(nameStr.c_str());
		}
	}
	else if (inPakData->m_state == SPatchPakData::es_PakLoaded)
	{
		DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() pak data is already loaded as pakfile");
	}
	else
	{
		DrxLog("CPatchPakUpr::OpenPatchPakDataAsPak() pak data is in unhandled state %d, not ready to open as pak", inPakData->m_state);
	}
}

void CPatchPakUpr::SetState(EMgrState inState)
{
	DrxLog("CPatchPakUpr::SetState() %d -> %d", m_state, inState);
	m_state = inState;
}

void CPatchPakUpr::ShowErrorForVersionMismatch(EVersionMismatchFault inWhosFault)
{
	DrxLog("CPatchPakUpr::ShowErrorForVersionMismatch() inWhosFault=%d", inWhosFault);
	
	switch(inWhosFault)
	{
		case eVMF_unknown:
			DrxLog("CPatchPakUpr::ShowErrorForVersionMismatch() unknown fault");
			g_pGame->GetWarnings()->AddGameWarning("WrongVersion", NULL);
			break;
		case eVMF_our_fault:
			DrxLog("CPatchPakUpr::ShowErrorForVersionMismatch() our fault");
			g_pGame->GetWarnings()->AddGameWarning("WrongVersionOurFault", NULL);
			break;
		case eVMF_their_fault:
			DrxLog("CPatchPakUpr::ShowErrorForVersionMismatch() their fault");
			g_pGame->GetWarnings()->AddGameWarning("WrongVersionTheirFault", NULL);
			break;
	}
}

void CPatchPakUpr::VersionMismatchErrorOccurred()
{
	DrxLog("CPatchPakUpr::VersionMismatchErrorOccurred() we need to find out who's fault this is..");

	if (m_updatedPermissionsAvailable)
	{
		DrxLog("CPatchPakUpr::VersionMismatchErrorOccurred() we already know we have updated permissions available. It's our fault!");
		ShowErrorForVersionMismatch(eVMF_our_fault);
	}
	else
	{
		DrxLog("CPatchPakUpr::VersionMismatchErrorOccurred() we don't think we have updated permissions available. Let's check now to be sure");

		m_versionMismatchOccurred=true;
		m_versionMismatchTimer=2.0f;		// time for us to get an updated permissions back

		switch(m_state)
		{
			case eMS_patches_installed:
				DrxLog("CPatchPakUpr::VersionMismatchErrorOccurred() we're in state eMS_patches_installed. We're ready to check on permissions right now");
				
				SetState(eMS_patches_installed_permissions_requested);
				m_pPermissionsDownloadableResource->Purge();
				m_pPermissionsDownloadableResource->StartDownloading();
				break;

			case eMS_patches_installed_permissions_requested:
				DrxLog("CPatchPakUpr::VersionMismatchErrorOccurred() we're in state eMS_patches_installed_permissions_requested so we've already requested permissions, lets just wait to see what the results bring");
				break;
		}
	}
}

bool CPatchPakUpr::HasPatchingSucceeded() const
{
	return (m_state >= eMS_patches_installed); 
}

//-------------------------------------------------------------------------
void CPatchPakUpr::RegisterPatchPakUprEventListener(IPatchPakUprListener *pListener)
{
	stl::push_back_unique(m_eventListeners, pListener);
}

//-------------------------------------------------------------------------
void CPatchPakUpr::UnregisterPatchPakUprEventListener(IPatchPakUprListener *pListener)
{
	stl::find_and_erase(m_eventListeners, pListener);
}

// Sent out to patch pak Event Listeners
void CPatchPakUpr::EventUpdatedPermissionsAvailable()
{
	const size_t numListeners = m_eventListeners.size();
	for (size_t i = 0; i < numListeners; ++ i)
	{
		m_eventListeners[i]->UpdatedPermissionsNowAvailable();
	}
}
