// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Logs data to a centralised server for later analysis
various game subsystems, forwarding necessary events to stats
recording system.

-------------------------------------------------------------------------
История:
- 18:11:2009  : Created by Mark Tully

*************************************************************************/

#ifndef __ITELEMETRYCOLLECTOR_H__
#define __ITELEMETRYCOLLECTOR_H__

struct ITelemetryProducer
{
	enum EResult
	{
			eTS_Pending,
			eTS_Available,
			eTS_EndOfStream
	};

	// telemetry producers can be deleted from the network thread, but will never be deleted whilst ProduceTelemetry() is being called
	// they may be deleted before the ProduceTelemetry() has indicated the end of the stream has been reached
	virtual							~ITelemetryProducer() {}

	// reads a minimum of inMinRequired and maximum of inBufferSize bytes of telemetry into the provided buffer
	// returns the number of bytes produced via the pOutWritten parameter
	// can return eTS_Pending if not data is currently available, the and caller is expected to poll again on subsequent frames
	// can return eTS_EndOfStream as well as providing data if this is the last chunk of data. if eTS_EndOfStream is returned, the data may be less that inMinRequired
	virtual EResult			ProduceTelemetry(
												char				*pOutBuffer,
												i32					inMinRequired,
												i32					inBufferSize,
												i32					*pOutWritten)=0;

	uk operator new (size_t sz)
	{
	//	ScopedSwitchToGlobalHeap useGlobalHeap;
		return ::operator new(sz);
	}
};

typedef void (*TTelemetryMemBufferDisposalCallback)(uk inUserData);

class ITelemetryCollector
{
public:
	typedef u32			TTelemetrySubmitFlags;
	enum
	{
		k_tf_none									= 0,
		k_tf_appendToRemoteFile		= (1<<0),
		k_tf_gzipRemoteFile				= (1<<1),
		k_tf_chunked							= (1<<2),
		k_tf_md5Digest						= (1<<3),
		k_tf_isStream							= (1<<4),
	};

	virtual					~ITelemetryCollector() {};

	virtual bool		SubmitFromMemory(
										const char				*inRemoteFilePath,
										const char				*inDataToStore,
										i32k				inDataLength,
										TTelemetrySubmitFlags	inFlags)=0;

	virtual bool		SubmitFile(
										const char			*inLocalFilePath,
										const char			*inRemoteFilePath,
										const char			*inHeaderData=NULL,
										i32k				inHeaderLength=0)=0;
	virtual bool		SubmitLargeFile(
										const char			*inLocalFilePath,
										const char			*inRemoteFilePath,
										i32							inLocalFileOffset=0,
										const char			*inHintFileData=NULL,
										i32k				inHintFileDataLength=0,
										TTelemetrySubmitFlags	inFlags=k_tf_none)=0;
	virtual bool		AppendStringToFile(
										const char		*inLocalFilePath,
										const char		*inDataToAppend)=0;

	virtual bool		AppendToFile(
										const char		*inLocalFilePath,
										const char		*inDataToAppend,
										i32k			inDataLength)=0;

	virtual void		OutputMemoryUsage(tukk message, tukk newLevelName) = 0;

	virtual void		SetNewSessionId( bool includeMatchDetails )=0;
	virtual string	GetSessionId()=0;
	virtual void		SetSessionId(
										string			inNewSessionId)=0;

	virtual bool		ShouldSubmitTelemetry()=0;
	virtual void		Update()=0;

	virtual bool		AreTransfersInProgress()=0;

	virtual void		GetMemoryUsage(IDrxSizer* pSizer) const =0;
};

#endif // __ITELEMETRYCOLLECTOR_H__
