// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/******************************************************************************
** DataPatchDownloader.h
** 13/12/2010
******************************************************************************/

#ifndef __DATAPATCHDOWNLOADER_H__
#define __DATAPATCHDOWNLOADER_H__

#if DRX_PLATFORM_WINDOWS && !defined(_RELEASE)
#define DATA_PATCH_DEBUG	1
#else
#define DATA_PATCH_DEBUG	0
#endif

#include <drx3D/Game/DownloadMgr.h>

struct IDataPatcherListener
{
	virtual void DataPatchAvailable()=0;
	virtual void DataPatchNotAvailable()=0;
	virtual ~IDataPatcherListener() {}
};

class CDataPatchDownloader : public IDataListener
{
	protected:
		IDataPatcherListener				*m_pListener;
		u32											m_patchCRC;
		i32													m_patchId;								// 0 for no id supplied or no patch present
		XmlNodeRef									m_patchXML;
		const char									*m_pFileBeingPatched;
		bool												m_patchingEnabled;
		bool												m_bNeedWeaponSystemReload;

		void												PatchFail(
																	const char									*pInReason);
		void												AssertPatchDownloaded();

		virtual void								DataDownloaded(
																	CDownloadableResourcePtr		inResource);
		virtual void								DataFailedToDownload(
																	CDownloadableResourcePtr		inResource);

	public:
																CDataPatchDownloader();
		virtual											~CDataPatchDownloader();

		CDownloadableResourcePtr		GetDownloadableResource();

		void												SetPatchingEnabled(
																	bool												inEnable);
		bool												IsPatchingEnabled() const												{ return m_patchingEnabled; }

		void												AddListener(
																	IDataPatcherListener				*pInListener);
		void												RemoveListener(
																	IDataPatcherListener				*pInListener);

		i32													GetPatchId()																		{ return m_patchId; } 
		u32											GetDataPatchHash()															{ return m_patchCRC; }

		bool												NeedsWeaponSystemReload() { return m_bNeedWeaponSystemReload; }
		void												DoneWeaponSystemReload() { m_bNeedWeaponSystemReload=false; }

		void												ApplyCVarPatch();

		void												CancelDownload();

#if DATA_PATCH_DEBUG
		void												LoadPatchFromFile(
			tukk szFilename);
#endif //DATA_PATCH_DEBUG
};

#endif // __DATAPATCHDOWNLOADER_H__

