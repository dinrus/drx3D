// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/CoreX/Renderer/Tarray.h>

enum EDrxTCPServiceConnectionStatus
{
	eCTCPSCS_Pending,
	eCTCPSCS_Connected,
	eCTCPSCS_NotConnected,
	eCTCPSCS_NotConnectedUserNotSignedIn,
	eCTCPSCS_NotConnectedDNSFailed,
	eCTCPSCS_NotConnectedConnectionFailed
};

//! May be expanded in future, at present when callback fires data will be de-queued immediately after.
enum EDrxTCPServiceResult
{
	eCTCPSR_Ok,       //!< No errors in sending data to external site.
	eCTCPSR_Failed    //!< Some sort of error occurred (likely to be a fail in the socket send).
};

struct STCPServiceData;
typedef _smart_ptr<STCPServiceData> STCPServiceDataPtr;

// Only one of the callback members of an STCPServiceData should be non-NULL.

//! Callback for simple TCP data transfers that can be held in a single buffer and do not require a reply.
//! This will be called when all data has been sent or when an error occurs.
//! \param res result.
//! \param pArg user data.
typedef void (* DrxTCPServiceCallback)(EDrxTCPServiceResult res, uk pArg);

//! Callback for TCP data transfers that can be held in a single buffer and do
//! require a reply. This will be called when all data has been sent, when an
//! error occurs, or when data is received.
//! \param res result.
//! \param pArg user data.
//! \param pData received data.
//! \param dataLen length of received data.
//! \param endOfStream has the end of the reply been reached?.
//! \return Ignored when called on error or completion of send. When called on data
//!         received, return false to keep the socket open and wait for more data or
//!         true to close the socket.
typedef bool (* DrxTCPServiceReplyCallback)(EDrxTCPServiceResult res, uk pArg, STCPServiceDataPtr pUploadData, tukk pReplyData, size_t replyDataLen, bool endOfStream);

struct STCPServiceData : public CMultiThreadRefCount
{
	STCPServiceData()
		: tcpServCb(NULL),
		tcpServReplyCb(NULL),
		m_quietTimer(0.0f),
		m_socketIdx(-1),
		pUserArg(NULL),
		pData(NULL),
		length(0),
		sent(0),
		ownsData(true)
	{};

	~STCPServiceData()
	{
		if (ownsData)
		{
			SAFE_DELETE_ARRAY(pData);
		}
	};

	DrxTCPServiceCallback      tcpServCb;      //!< Callback function to indicate success/failure of posting.
	DrxTCPServiceReplyCallback tcpServReplyCb; //!< Callback function to receive reply.
	float                      m_quietTimer;   //!< Time in seconds since data was last sent or received for this data packet. timer is only incremented once a socket is allocated and the transaction begins.
	i32                      m_socketIdx;
	uk                      pUserArg;       //!< Application specific callback data.
	tuk                      pData;          //!< Pointer to data to upload.
	size_t                     length;         //!< Length of data.
	size_t                     sent;           //!< Data sent.
	bool                       ownsData;       //!< should pData be deleted when we're finished with it.
};

struct IDrxTCPService
{
	// <interfuscator:shuffle>
	virtual ~IDrxTCPService(){}

	//! \param isDestructing - is this objects destructor running.
	//! \return Result code.
	virtual EDrxTCPServiceResult Terminate(bool isDestructing) = 0;

	//! Has the address resolved?
	//! \return true if the address has resolved, otherwise false.
	virtual bool HasResolved() = 0;

	//! Queue a transaction.
	//! \param pData - transaction.
	//! \return true if transaction was successfully queued, otherwise false
	virtual bool UploadData(STCPServiceDataPtr pData) = 0;

	//! \param delta - time delta from last tick.
	virtual void Tick(CTimeValue delta) = 0;

	//! Increment reference count.
	//! \return Reference count.
	virtual i32 AddRef() = 0;

	//! Decrement reference count.
	//! \return Reference count.
	virtual i32 Release() = 0;

	//! Get the current connection status.
	//! \return Status.
	virtual EDrxTCPServiceConnectionStatus GetConnectionStatus() = 0;

	//! Get the number of items currently in the data queue.
	//! \return Number of items.
	virtual u32 GetDataQueueLength() = 0;

	//! Get the total data size currently in the data queue.
	//! \return Number of bytes.
	virtual u32 GetDataQueueSize() = 0;

	//! Get the server name.
	//! \return Server name.
	virtual tukk GetServerName() = 0;

	//! Get the port.
	//! \return Port
	virtual u16 GetPort() = 0;

	//! Get the server path.
	//! \return Server path.
	virtual tukk GetURLPrefix() = 0;
	// </interfuscator:shuffle>
};

typedef _smart_ptr<IDrxTCPService> IDrxTCPServicePtr;
