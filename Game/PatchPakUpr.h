// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: Handles the downloading and managing of various patch paks

	-------------------------------------------------------------------------
	История:
	- 31:01:2012  : Created by James Bamford

*************************************************************************/

#ifndef __PATCHPAKMANAGER_H__
#define __PATCHPAKMANAGER_H__


#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Game/DownloadMgr.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/Game/IPatchPakUprListener.h>

#if defined(_RELEASE) 
	#define DEBUG_LATE_DOWNLOADING_OF_PATCH_PAKS 0
#else
	#define DEBUG_LATE_DOWNLOADING_OF_PATCH_PAKS 0
#endif

class CPatchPakMemoryBlock : public ICustomMemoryBlock
{
public:
	CPatchPakMemoryBlock( uk pMem, i32 size )
	{
		m_pData=pMem;
		m_nSize=size;
	}

	virtual ~CPatchPakMemoryBlock()
	{
		//do nothing, we don't own the mem
	}

	virtual uk  GetData() { return m_pData; }
	virtual i32 GetSize() { return m_nSize; }

	virtual void CopyMemoryRegion( uk pOutputBuffer,size_t nOffset,size_t nSize );

private:
	uk m_pData;
	size_t m_nSize;
};

class CPatchPakUpr : public IDataListener, public IPlatformOS::IPlatformListener
{
public:
	CPatchPakUpr();
	~CPatchPakUpr();

	void Update(float frameTime);
	void BlockingUpdateTillDone();

	// IPlatformOS::IPlatformListener
	virtual void OnPlatformEvent(const IPlatformOS::SPlatformEvent& event);
	// ~IPlatformOS::IPlatformListener

	// IDataListener
	virtual void DataDownloaded(CDownloadableResourcePtr inResource);
	virtual void DataFailedToDownload(CDownloadableResourcePtr inResource);
	// ~IDataListener

	void RequestPermissionsXMLDownload();
	void UnloadPatchPakFiles();
	u32 GetXOROfPatchPakCRCs();
	i32 GetPatchPakVersion() { return m_patchPakVersion; }

	bool HasPatchingSucceeded() const;
	bool AreUpdatedPermissionsAvailable() { return m_updatedPermissionsAvailable; }
	i32  TimeThatUpdatedPermissionsBecameAvailable() { return m_timeThatUpdatedPermissionsAvailable; }
	bool IsOkToCachePaks()	{ return m_okToCachePaks; } // TODO - use me in the frontend to popup the storage device or warning about having to have storage

	void RegisterPatchPakUprEventListener(IPatchPakUprListener *pListener);
	void UnregisterPatchPakUprEventListener(IPatchPakUprListener *pListener);
	void EventUpdatedPermissionsAvailable();

	void VersionMismatchErrorOccurred();

protected:

	typedef std::vector<IPatchPakUprListener*> TPatchPakUprEventListenersVec;
	TPatchPakUprEventListenersVec m_eventListeners;

	static i32k k_maxHttpHeaderSize=1024;
	static i32k k_maxPatchPaks=10;
	
	enum EMgrState
	{
		eMS_waiting_to_initial_get_permissions=0,
		eMS_initial_get_permissions_requested,
		eMS_initial_permissions_downloaded,
		eMS_patches_failed_to_download_in_time,
		eMS_patches_failed_to_download_in_time_but_now_all_cached_or_downloaded,
		eMS_patches_installed,
		eMS_patches_installed_permissions_requested,
		eMS_patches_installed_permissions_downloaded,
	};

	enum EVersionMismatchFault
	{
		eVMF_unknown=0,
		eVMF_our_fault,
		eVMF_their_fault
	};

	typedef _smart_ptr<CPatchPakMemoryBlock> CPatchPakMemoryBlockPtr;
	struct SPatchPakData
	{
		enum EState
		{
			es_Uninitialised=0,
			eS_Downloading,
			es_Downloaded,
			es_FailedToDownload,
			es_DownloadedButCorrupt,
			es_Cached,
			es_FailedToOpenFromCache,
			es_PakLoaded,
			es_PakLoadedFromCache,
		};

		float m_showingSaveMessageTimer;
		CDownloadableResourcePtr m_downloadableResource;
		CPatchPakMemoryBlockPtr m_pPatchPakMemBlock;
		u32 m_actualCRC32;
		DrxFixedStringT<32> m_pMD5Str; 
		u8				m_pMD5[16];
		u32							m_MD5CRC32; // for setting a suitably different matchmaking version
		u32				m_downloadSize;
		DrxFixedStringT<32> m_url; 
		DrxFixedStringT<128> m_pakBindRoot;
		EState m_state;
		bool m_MD5FileName;
		bool m_showingSaveMessage;

		// type
		// cache to disk



		SPatchPakData();
	};

	typedef DrxFixedArray<SPatchPakData, k_maxPatchPaks> TPatchPaksArray;

	void ProcessPatchPaksFromPermissionsXML(XmlNodeRef root);
	i32 GetPatchPakDataIndexFromDownloadableResource(CDownloadableResourcePtr inResource);
	SPatchPakData *GetPatchPakDataFromDownloadableResource(CDownloadableResourcePtr inResource);

	void StartNewDownload(
		tukk inServerName, 
		i32k inPort, 
		tukk inURLPrefix, 
		tukk inURL, 
		tukk inPakBindRoot,
		i32k inDownloadSize,
		tukk inMD5, 
		const bool inMD5FileName,
		tukk inDescName);
	bool CheckForNewDownload(
		tukk inServerName, 
		i32k inPort, 
		tukk inURLPrefix, 
		tukk inURL, 
		i32k inDownloadSize, 
		tukk inMD5, 
		tukk inDescName);
	
	void GeneratePakFileNameFromURLName(DrxFixedStringT<64> &outPakFileName, tukk inUrlName);
	void ProcessPermissionsXML(CDownloadableResourcePtr inResource);
	bool CachePakDataToDisk(SPatchPakData *pInPakData);
	void PatchPakDataDownloaded(CDownloadableResourcePtr inResource);
	void OpenPatchPakDataAsPak(SPatchPakData *inPakData);
	void SetState(EMgrState inState);
	void ShowErrorForVersionMismatch(EVersionMismatchFault inWhosFault);

	TPatchPaksArray m_patchPaks;
	CDownloadableResourcePtr m_pPermissionsDownloadableResource;
	i32							m_numPatchPaksDownloading;
	i32							m_numPatchPaksFailedToDownload;
	i32							m_numPatchPaksFailedToCache;
	i32							m_patchPakVersion;
	u32					m_installedPermissionsCRC32;
	ICVar						*m_patchPakEnabled;
	ICVar						*m_patchPakDownloadTimeOut;
	ICVar						*m_patchPakPollTime;
	ICVar						*m_patchPakDebug;
	ICVar						*m_patchPakDediServerMustPatch;
	EMgrState				m_state;
	float						m_pollPermissionsXMLTimer;
	i32							m_timeThatUpdatedPermissionsAvailable;
	i32							m_userThatIsUsingCachePaks;
	float						m_versionMismatchTimer;
	
	bool						m_enabled;
	bool						m_patchPakRemoved;
	bool						m_blockingUpdateInProgress;
	bool						m_haveParsedPermissionsXML;
	bool						m_failedToDownloadPermissionsXML;
	bool						m_okToCachePaks;					// consoles have requirements to be allowed to patch
	bool						m_lastOkToCachePaks;			// consoles have requirements to be allowed to patch
	bool						m_updatedPermissionsAvailable;
	bool						m_wasInFrontend;
	bool						m_isUsingCachePaks;
	bool						m_versionMismatchOccurred;
};


#endif // __PATCHPAKMANAGER_H__
