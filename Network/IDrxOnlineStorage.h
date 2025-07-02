// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct DrxOnlineStorageQueryData;

struct IOnlineStorageListener
{
	// <interfuscator:shuffle>
	virtual ~IOnlineStorageListener(){}
	virtual void OnOnlineStorageOperationComplete(const DrxOnlineStorageQueryData& QueryData) = 0;
	// </interfuscator:shuffle>
};

enum EDrxOnlineStorageLocation
{
	eCOSL_Title,
	eCOSL_User
};

enum EDrxOnlineStorageOperation
{
	eCOSO_Upload,
	eCOSO_Download
};

#if DRX_PLATFORM_DURANGO
enum EDrxOnlineStorageDataType
{
	eCOSDT_Binary,
	eCOSDT_Json,
	eCOSDT_Config
};
#endif

struct DrxOnlineStorageQueryData
{
	DrxLobbyTaskID             lTaskID;           //!< No need to set this, it's done internally.
	EDrxLobbyError             eResult;
	u32                     userID;
	EDrxOnlineStorageOperation operationType;
	EDrxOnlineStorageLocation  storageLocation;
#if DRX_PLATFORM_DURANGO
	EDrxOnlineStorageDataType  dataType;
	const wchar_t*             targetXuid;        //!< Null or empty string when querying against active user.
	u32                     httpStatusCode;
#endif
	tukk                szItemURL;         //!< Name that identifies the data in online storage.
	tuk                      pBuffer;           //!< Memory to upload to or download from online storage.
	u32                     bufferSize;        //!< Size of the buffer.
	IOnlineStorageListener*    pListener;

	DrxOnlineStorageQueryData()
	{
		memset(this, 0, sizeof(DrxOnlineStorageQueryData));
	}
	DrxOnlineStorageQueryData(const DrxOnlineStorageQueryData& from)
	{
		memcpy(this, &from, sizeof(DrxOnlineStorageQueryData));
	}
};

//! Interface for uploading and downloading data from platform specific online storage.
//! PSN and Xbox LIVE! both implement flavors of this.
struct IDrxOnlineStorage
{
	// <interfuscator:shuffle>
	virtual ~IDrxOnlineStorage(){};

	virtual void Tick(CTimeValue tv) = 0;

	//! Add an operation to the queue.
	virtual EDrxLobbyError OnlineStorageDataQuery(DrxOnlineStorageQueryData* pQueryData) = 0;
	// </interfuscator:shuffle>
};

//! \endcond