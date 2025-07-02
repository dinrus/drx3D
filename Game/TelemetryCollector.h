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

#ifndef __TELEMETRYCOLLECTOR_H__
#define __TELEMETRYCOLLECTOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/Game/ITelemetryCollector.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/CoreX/Lobby/IDrxTCPService.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/Game/DownloadMgr.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <queue>

#ifdef ENABLE_PROFILING_CODE
#define USE_TELEMETRY_BUFFERS (1)
#define USE_STATOSCOPE_TELEMETRY (1)
#else
#define USE_TELEMETRY_BUFFERS (0)
#define USE_STATOSCOPE_TELEMETRY (0)
#endif // #ifdef ENABLE_PROFILING_CODE

#ifndef _RELEASE
#define USE_TELEMETRY_EVENTS_LOG
#endif

struct IZLibDeflateStream;
class CStreamedTelemetryProxy;

typedef void (*TSubmitResultCallback)(uk inUserData, bool inSubmitWasSuccessful, tukk pInData, i32 inLength);

#define k_tf_defaultTelemetryProducer k_tf_md5Digest

class CTelemetryCollector : public ITelemetryCollector, public ILevelSystemListener
{
	public:
		static i32k		k_maxHttpHeaderSize=1024;
		
		static CTelemetryCollector *GetTelemetryCollector()
		{
			return s_telemetryCollector;
		}

	protected:
		static CTelemetryCollector* s_telemetryCollector;

		ICVar					*m_telemetryTransactionRecordings;
		ICVar					*m_telemetryEnabled;
		ICVar					*m_telemetryServerLogging;
		ICVar					*m_telemetryUploadErrorLog;
		ICVar					*m_telemetryUploadGameLog;
		ICVar					*m_telemetryCompressGameLog;
		ICVar					*m_telemetryUploadInProgress;

		string					m_curSessionId;
		string					m_websafeClientName;
#ifdef ENABLE_PROFILING_CODE
		DrxFixedStringT<512>	m_telemetryRecordingPath;
		DrxFixedStringT<512> m_telemetryMemoryLogPath;
#endif

		DrxMutex			m_largeFileMutex;

		_smart_ptr<CStreamedTelemetryProxy> m_eventsStream;
		DrxMutex			m_transferCounterMutex;
		i32						m_transfersCounter;

#ifdef ENABLE_PROFILING_CODE
		i32						m_fileUploadCounter;
#endif
		i32						m_lastLevelRotationIndex;
		bool					m_previousSessionCrashChecked;
		bool					m_previousSessionCrashed;

		IDrxTCPServicePtr		m_pTelemetry;

		struct SQueuedProducer
		{
			ITelemetryProducer		*pProducer;
			string								postHeader;
			TSubmitResultCallback	callback;
			void									*callbackData;
			TTelemetrySubmitFlags flags;
		};
		std::queue<SQueuedProducer>	m_queuedTransfers;

		static i32k k_maxNumLargeFilesSubmitting=2;
		static i32k k_largeFileSubmitChunkSize=64*1024; //1050; //1100; 
		static i32k k_largeFileSubmitChunkMaxDataSize=k_largeFileSubmitChunkSize-k_maxHttpHeaderSize;

	public:
		struct SLargeFileSubmitData
		{
			char m_remoteFileName[IDrxPak::g_nMaxPath];			// remote filename to submit to
			char m_chunkData[k_largeFileSubmitChunkSize];		// current chunk data being submitted
			char m_postHeaderContents[k_maxHttpHeaderSize];		// post header contents - we'll need to patch for each chunk send. 
			i32 m_postHeaderSize;															// post header size
			i32 m_chunkDataSize;														// size of current chunk data being submitted
			ITelemetryProducer *m_pProducer;								// telemetry producer provides data to upload
			bool m_isFirstChunk;														// the first chunk has different upload chunk to the subsequent ones
			TTelemetrySubmitFlags m_flags;
			CDownloadableResourcePtr	m_pDownloadableResource;
			TSubmitResultCallback			m_callback;
			void											*m_callbackData;
			
			enum EState
			{
				k_state_available=0,													// chunk is available
				k_state_chunk_submitted_waiting,							// chunk has been submitted, there is more data in the file waiting to be submitted
				k_state_chunk_submitted_send_next_one,				// chunk has been submitted, and sent. Send next chunk
				k_state_chunked_transfer_data_available,			// chunked transfer encoding data is available
				k_state_chunked_transfer_data_ended,					// chunked transfer encoding data has ended
				k_state_submitted,														// chunk has been submitted, all data in the file is submitted after this chunk
				k_state_dispatchCallbacksAndClear,						// all data has been submitted and we've received a reply which must be dispatched
			};
			EState m_state;

			SLargeFileSubmitData() :
				m_pProducer(NULL)
			{
				Clear();
			}

			~SLargeFileSubmitData()
			{
				SAFE_DELETE(m_pProducer);
			}
			
			void Clear()
			{
				m_callback=NULL;
				m_callbackData=NULL;
				m_pDownloadableResource=NULL;
				SAFE_DELETE(m_pProducer);
				memset(m_remoteFileName, 0, sizeof(m_remoteFileName));
				memset(m_chunkData, 0, sizeof(m_chunkData));
				memset(m_postHeaderContents, 0, sizeof(m_postHeaderContents));
				m_postHeaderSize=0;
				m_chunkDataSize=0;
				m_state = k_state_available;
				m_isFirstChunk=true;
				m_flags=k_tf_none;
			}
		};
	protected:

		DrxFixedArray<SLargeFileSubmitData, k_maxNumLargeFilesSubmitting> m_largeFileSubmits;

		IPlatformOS::TUserName  GetHostName();
		IPlatformOS::TUserName  GetProfileName();

		const char				*GetWebSafePlatformName();
		string					GetWebSafeClientName();
		string					GetWebSafeSessionId();
		void						UpdateClientName();

		static void				SubmitGameLog(
									IConsoleCmdArgs *inArgs);
		static void				OutputSessionId(
									IConsoleCmdArgs *inArgs);

		bool					InitService();

		bool					UploadData(
									STCPServiceDataPtr pData,
									const char			*inReferenceFilename);

		i32						MakePostHeader(
									const char			*inRemoteFileName,
									i32					inDataLength,
									char				*outBuffer,
									i32					inMaxBufferSize,
									TTelemetrySubmitFlags	inFlags);

		void					UpdateLargeFileChunkPostHeader(
										SLargeFileSubmitData				*pInLargeFile,
										bool												inFirstChunk,
										i32													inPayloadSize);

		i32						GetIndexOfFirstValidCharInFilePath(
									tukk pPath,
									i32k		len,
									const bool	stripDotSlash,
									const bool	stripDriveSpec);

		bool					MoveLogFileOutOfTheWay(
									tukk inSourceFilename,
									tukk inTargetFilename);

		bool					ShouldUploadGameLog(bool forPreviousSession);

		bool					UploadFileForPreviousSession(tukk inFileName, tukk inRemoteFileName);
		bool					UploadLargeFileForPreviousSession(tukk inFileName, tukk inRemoteFileName, TTelemetrySubmitFlags inFlags);
		void					UploadLastGameLogToPreviousSession();
		void					CheckForPreviousSessionCrash();

		void					UpdateTransfersInProgress(
										i32										inDiff);
		static bool		SubmitFileTelemetryCallback(
										EDrxTCPServiceResult	res,
										uk 									arg,
										STCPServiceDataPtr		pUpload,
										const char						*pData,
										size_t								dataLen,
										bool									endOfStream);

		bool					TrySubmitTelemetryProducer(
										ITelemetryProducer		*pInProducer,
										const char						*inPostHeader,
										i32										inPostHeaderLen,
										TSubmitResultCallback inCallback,
										void									*inCallbackData,
										TTelemetrySubmitFlags inFlags);

		void					ThreadSafeClear(
										SLargeFileSubmitData	*pInLargeFile);

	public:
		ICVar					*m_telemetryCompressionLevel;
		ICVar					*m_telemetryCompressionWindowBits;
		ICVar					*m_telemetryCompressionMemLevel;

								CTelemetryCollector();
								~CTelemetryCollector();

		bool					ShouldSubmitTelemetry();
		virtual void	Update();

		void					Log(i32 level, tukk format, ...);

		bool					SubmitFromMemory(
										const char						*inRemoteFilePath,
										const char						*inDataToStore,
										i32k							inDataLength,
										TTelemetrySubmitFlags	inFlags);


		bool					SubmitFile(
										const char			*inLocalFilePath,
										const char			*inRemoteFilePath,
										const char			*inHeaderData=NULL,
										i32k				inHeaderLength=0);
		virtual bool	SubmitLargeFile(
										const char			*inLocalFilePath,
										const char			*inRemoteFilePath,
										i32							inLocalFileOffset=0,
										const char			*inHintFileData=NULL,
										i32k				inHintFileDataLength=0,
										TTelemetrySubmitFlags	inFlags=k_tf_none);

		bool					SubmitTelemetryProducer(
										ITelemetryProducer		*inProducer,
										const char						*inRemoteFilePath,
										TSubmitResultCallback	inCallback=NULL,
										void									*inCallbackData=NULL,
										TTelemetrySubmitFlags inFlags=k_tf_defaultTelemetryProducer);

		void					SubmitChunkOfALargeFile(SLargeFileSubmitData *largeSubmitData);

		SLargeFileSubmitData *GetAvailableLargeFileSubmitData();
		SLargeFileSubmitData *FindLargeFileSubmitDataFromData(tukk pData);
		void UpdateLargeFileSubmitData();
		void					UpdateQueuedProducers();

		void					CreateEventStream();
		void					CloseEventStream();
		void					CreateStatoscopeStream();
		void					CloseStatoscopeStream();
		ILINE bool		CanLogEvent() const { return m_eventsStream!=NULL; }
		void					LogEvent(tukk eventName, float value);

		bool					AppendStringToFile(
									const char		*inRemoteFilePath,
									const char		*inDataToAppend);

		bool					AppendToFile(
									const char		*inRemoteFilePath,
									const char		*inDataToAppend,
									i32k			inDataLength);

		void					SetNewSessionId( bool includeMatchDetails );
		void					SetAbortedSessionId( tukk abortedSessionName, DrxSessionID& abortedSessionID );
		void					SetTestSessionId();
		void					OutputTelemetryServerHintFile();
		void					OutputMemoryUsage(tukk message, tukk newLevelName);
		string					GetSessionId();
		void					SetSessionId(
									string			inNewSessionId);

		virtual bool	AreTransfersInProgress();

		virtual void	GetMemoryUsage(IDrxSizer* pSizer) const;

		// ILevelSystemListener
		virtual void OnLevelNotFound(tukk levelName) {}
		virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) {}
		virtual void OnLoadingStart(ILevelInfo* pLevel);
		virtual void OnLoadingComplete(ILevelInfo* pLevel);
		virtual void OnLoadingError(ILevelInfo* pLevel, tukk error) {}
		virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) {}
		virtual void OnUnloadComplete(ILevelInfo* pLevel) {}
		//~ILevelSystemListener
};

////////////////////////////////////////////////////////////////////////////////
// Telemetry Buffer util class
////////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/RecordingBuffer.h>

enum ETelemetryPacketType
{
	// If you add enums to this list - you will also need to update the code in TelemetryCollector::FormatBufferData()
	eTPT_Performance = eRBPT_Custom,
	eTPT_Bandwidth,
	eTPT_Memory,
	eTPT_Sound,
};

typedef SRecording_Packet ITelemetryBufferData;

struct SPerformanceTelemetry : ITelemetryBufferData
{
	SPerformanceTelemetry()
		:	m_timeInSeconds(0.f)
		, m_numTicks(0)
		, m_gpuTime(0.f)
		, m_gpuLimited(0)
		, m_drawcalls(0)
		, m_drawcallOverbudget(0)
		, m_deltaTime(0.f)
		, m_renderThreadTime(0.f)
		, m_mainThreadTime(0.f)
		, m_waitForGPUTime(0.f)
	{
		size = sizeof(SPerformanceTelemetry);
		type = eTPT_Performance;
	}
	float	m_timeInSeconds;
	i32		m_numTicks;
	float m_gpuTime; //gpu frame time in secs
	i32	  m_gpuLimited; // num frames gpu limited
	i32	m_drawcalls;
	i32	m_drawcallOverbudget;
	float m_deltaTime;
	float m_renderThreadTime;
	float m_mainThreadTime;
	float m_waitForGPUTime;
};

struct SBandwidthTelemetry : ITelemetryBufferData
{
	SBandwidthTelemetry()
		: m_bandwidthSent(0)
		, m_bandwidthReceived(0)
		, m_timeInSeconds(0.f)
		, m_packetsSent(0)
		, m_deltaTime(0.f)
	{
		size = sizeof(SBandwidthTelemetry);
		type = eTPT_Bandwidth;
	}
	uint64		m_bandwidthSent;
	uint64		m_bandwidthReceived;
	float	m_timeInSeconds;
	i32		m_packetsSent;
	float m_deltaTime;
};

struct SMemoryTelemetry : ITelemetryBufferData
{
	SMemoryTelemetry()
		: m_timeInSeconds(0.0f)
		, m_cpuMemUsedInMB(0.0f)
		, m_gpuMemUsedInMB(0.0f)
	{
		size = sizeof(SMemoryTelemetry);
		type = eTPT_Memory;
	}
	float	m_timeInSeconds;
	float	m_cpuMemUsedInMB;
	float	m_gpuMemUsedInMB;
};

struct SSoundTelemetry : ITelemetryBufferData
{
	SSoundTelemetry()
	{
		size = sizeof(SSoundTelemetry);
		type = eTPT_Sound;
	}

	//SSoundMemoryInfo m_soundInfo;
};

#if USE_TELEMETRY_BUFFERS 
class CTelemetryBuffer
{
public:
	CTelemetryBuffer(i32 buffersize, ITelemetryCollector *collector, size_t structSize);
	~CTelemetryBuffer();

	void AddData(ITelemetryBufferData *dataStruct);
	void SubmitToServer(tukk filename);
	void DumpToFile(tukk filename);
	void Reset();
	static void FormatBufferData(ITelemetryBufferData* packet, DrxFixedStringT<1024> &outString);

protected:

	CRecordingBuffer *m_pBuffer;
	ITelemetryCollector *m_collector;

	size_t m_structSize;
};

class CTelemetryBufferProducer : public ITelemetryProducer
{
public:
	CTelemetryBufferProducer(CRecordingBuffer *pRecordingBuffer);
	virtual ~CTelemetryBufferProducer();

	virtual EResult ProduceTelemetry(
		char				*pOutBuffer,
		i32					inMinRequired,
		i32					inBufferSize,
		i32					*pOutWritten);

protected:
	u8* m_pBuffer;
	size_t m_length;
	size_t m_offset;
};
#endif //#if USE_TELEMETRY_BUFFERS 

class CTelemetryFileReader : public ITelemetryProducer
{
		protected:
			DrxFixedStringT<256>		m_localFilePath;
			i32											m_fileOffset;

		public:
															CTelemetryFileReader(
																const char	*pInLocalFile,
																i32					inFileOffset) :
																m_localFilePath(pInLocalFile),
																m_fileOffset(inFileOffset)
															{
															}

			virtual EResult					ProduceTelemetry(
																char				*pOutBuffer,
																i32					inMinRequired,
																i32					inBufferSize,
																i32					*pOutWritten);
};

class CTelemetryMemBufferProducer : public ITelemetryProducer
{
	protected:
		tukk 													m_pBuffer;
		i32																	m_bufferSize;
		i32																	m_dataSent;
		TTelemetryMemBufferDisposalCallback	m_pDisposerCallback;
		uk 																m_pDisposerCallbackData;

	public:
																	// passed memory will not be freed, however a disposal callback will be called if provided
																	CTelemetryMemBufferProducer(
																		tukk 													inBuffer,
																		i32																	inBufferSize,
																		TTelemetryMemBufferDisposalCallback	inDisposalCallback,
																		uk 																inDisposalCallbackData);

			virtual											~CTelemetryMemBufferProducer();

			virtual EResult							ProduceTelemetry(
																		tuk													pOutBuffer,
																		i32														inMinRequired,
																		i32														inBufferSize,
																		i32*													pOutWritten);
};

class CTelemetryStrProducer : public CTelemetryMemBufferProducer
{
	protected:
		const string											m_payload;

	public:
																			CTelemetryStrProducer(
																				const string			&inCopyMe) :
																				CTelemetryMemBufferProducer(inCopyMe.c_str(),inCopyMe.length(),NULL,0),
																				m_payload(inCopyMe)
																			{
																				// tech note : the CTelemetryMemBufferProducer() is taking the str buffer ptr from the inCopyMe rather than
																				// the m_payload str, but the DrxStringT constructor for m_payload will use the same ptr. if it duplicated
																				// the str contents to a new ptr, or did anything else to its internal ptr, then the ptr used for the
																				// CTelemetryMemBufferProducer would be the wrong one
																				DRX_ASSERT_MESSAGE(inCopyMe.c_str()==m_payload.c_str(),"CTelemetryStrProducer ptr mismatch - likely to corrupt data");
																			}
};

// this telemetry producer will write telemetry from the passed telemetry source to a local file, and then return it to the caller
// in effect it 'tees' the data to a file without changing it
class CTelemetrySaveToFile : public ITelemetryProducer
{
	protected:
		string												m_fileName;
		FILE													*m_pFile;
		ITelemetryProducer						*m_pSource;

	public:
																	CTelemetrySaveToFile(
																		ITelemetryProducer						*pInProducer,
																		string												inFileToSaveTo);
			virtual											~CTelemetrySaveToFile();

			virtual EResult							ProduceTelemetry(
																		tuk													pOutBuffer,
																		i32														inMinRequired,
																		i32														inBufferSize,
																		i32*													pOutWritten);
};

class CTelemetryCompressor : public ITelemetryProducer
{
	protected:
		ITelemetryProducer				*m_pSource;
		EResult										m_lastSourceResult;
		i32												m_lastUncompressedSize;
		IZLibDeflateStream				*m_zstream;
		bool											m_dataIncoming;

	public:
															// passed producer is adopted and will be deleted when this producer is
															CTelemetryCompressor(
																ITelemetryProducer		*inPSource);

		virtual										~CTelemetryCompressor();

		virtual EResult						ProduceTelemetry(
																char				*pOutBuffer,
																i32					inMinRequired,
																i32					inBufferSize,
																i32					*pOutWritten);
};

class CTelemetryCopier : public ITelemetryProducer
{
protected:
	static i32k k_blockSize;
	i32 m_sent;
	i32 m_size;
	std::vector<tuk> m_buffers;

public:
	explicit CTelemetryCopier(ITelemetryProducer *pIn);
	virtual ~CTelemetryCopier ();
	virtual EResult						ProduceTelemetry(
																char				*pOutBuffer,
																i32					inMinRequired,
																i32					inBufferSize,
																i32					*pOutWritten);
protected:
	void AddData(tukk src, i32 size);

private:
	CTelemetryCopier(const CTelemetryCopier& other);
	CTelemetryCopier& operator=(const CTelemetryCopier& rhs);
};

class CTelemetryStreamCipher : public ITelemetryProducer
{
	protected:
		ITelemetryProducer				*m_pSource;
		TCipher										m_cipher;

	public:
															// adopts the passed producer and will delete it when this is destructed
															CTelemetryStreamCipher(
																ITelemetryProducer	*pInSource,
																const char					*pInKey,
																i32									inKeyLen);

		virtual										~CTelemetryStreamCipher();

		virtual EResult						ProduceTelemetry(
																char				*pOutBuffer,
																i32					inMinRequired,
																i32					inBufferSize,
																i32					*pOutWritten);
};

// The proxy class exists because ITelemetryProducer is not reference counted and is owned by the telemetry collector
// It could be deleted at any time and is not safe to keep a pointer to it once it has been submitted
class CStreamedTelemetryProxy : public CMultiThreadRefCount
{
	friend class CStreamedTelemetryProducer;
public:
	CStreamedTelemetryProxy() : m_finished(false) {}
	~CStreamedTelemetryProxy() {}

	void WriteString(tukk string);
	void FormatString(tukk format, ...);
	void CloseStream() { m_finished = true; }

protected:
	string m_pendingData;
	bool m_finished;
};

class CStreamedTelemetryProducer : public ITelemetryProducer
{
public:
	CStreamedTelemetryProducer() { m_proxy.reset(new CStreamedTelemetryProxy()); }
	~CStreamedTelemetryProducer() {}

	virtual ITelemetryProducer::EResult	ProduceTelemetry(char *pOutBuffer, i32 inMinRequired, i32 inBufferSize, i32 *pOutWritten);
	CStreamedTelemetryProxy* GetProxy() { return m_proxy; }

protected:
	_smart_ptr<CStreamedTelemetryProxy> m_proxy;
};

#endif // __TELEMETRYCOLLECTOR_H__

