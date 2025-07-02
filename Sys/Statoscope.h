// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Statoscope.h
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Statoscope_h__
#define __Statoscope_h__
#pragma once

#include <drx3D/Sys/IStatoscope.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

#if ENABLE_STATOSCOPE

u32k STATOSCOPE_BINARY_VERSION = 2;

	#include <drx3D/CoreX/String/DrxName.h>
	#include <drx3D/Network/DrxSocks.h>

struct SPerfStatFrameProfilerRecord
{
	CFrameProfiler* m_pProfiler;
	i32             m_count;
	float           m_selfTime;
	float           m_variance;
	float           m_peak;
};

struct SPerfTexturePoolAllocationRecord
{
	i32    m_size;
	string m_path;
};

struct SParticleInfo
{
	string name;
	i32    numParticles;
};

struct SPhysInfo
{
	string name;
	float  time;
	i32    nCalls;
	Vec3   pos;
};

struct SUserMarker
{
	SUserMarker(const string path, const string name) : m_path(path), m_name(name) {}
	string m_path;
	string m_name;
};

struct SCallstack
{
	string             m_tag;
	std::vector<uk> m_addresses;

	SCallstack() {}

	SCallstack(uk * addresses, u32 numAddresses, tukk tag)
		: m_addresses(addresses, addresses + numAddresses), m_tag(tag)
	{}

	void swap(SCallstack& other)
	{
		m_tag.swap(other.m_tag);
		m_addresses.swap(other.m_addresses);
	}
};

class CDataWriter;

class CStatoscopeFrameRecordWriter : public IStatoscopeFrameRecord
{
public:
	explicit CStatoscopeFrameRecordWriter(CDataWriter* pDataWriter)
		: m_pDataWriter(pDataWriter)
		, m_nWrittenElements(0)
	{
	}

	virtual void AddValue(float f);
	virtual void AddValue(tukk s);
	virtual void AddValue(i32 i);

	inline void  ResetWrittenElementCount()
	{
		m_nWrittenElements = 0;
	}

	inline i32 GetWrittenElementCount() const
	{
		return m_nWrittenElements;
	}

private:
	CDataWriter* m_pDataWriter;
	i32          m_nWrittenElements;
};

namespace StatoscopeDataWriter
{
//The following enums must match in the statoscope tool

enum EFrameElementType
{
	None = 0,
	Float,
	Int,
	String,
	B64Texture,
	Int64
};

enum EEndian
{
	EE_LittleEndian = 0,
	EE_BigEndian
};

enum EEventId
{
	EI_DefineClass = 0,
	EI_BeginInterval,
	EI_EndInterval,
	EI_ModifyInterval,
	EI_ModifyIntervalBit
};

	#pragma pack(push)
	#pragma pack(1)

struct EventHeader
{
	u8  eventId;
	u8  sequence;
	u16 eventLengthInWords;
	uint64 timeStampUs;
};

struct EventDefineClass
{
	enum { EventId = EI_DefineClass };

	u32 classId;
	u32 numElements;

	// numElements {u8 of EFrameElementType, null terminated name}
};

struct EventBeginInterval
{
	enum { EventId = EI_BeginInterval };

	uint64 id;
	u32 classId;

	// numElements properties
};

struct EventEndInterval
{
	enum { EventId = EI_EndInterval };

	uint64 id;
};

struct EventModifyInterval
{
	enum { EventId = EI_ModifyInterval };

	static u32k FieldIdMask = 0x7fffffff;
	static u32k FieldSplitIntervalMask = 0x80000000;

	uint64              id;
	u32              classId;
	u32              field;

	// numElements properties
};

struct EventModifyIntervalBit
{
	enum { EventId = EI_ModifyIntervalBit };

	static u32k FieldIdMask = 0x7fffffff;
	static u32k FieldSplitIntervalMask = 0x80000000;

	uint64              id;
	u32              classId;
	u32              field;

	// property mask
	// property or
};

	#pragma pack(pop)
}

class CStatoscopeEventWriter;

class CStatoscopeDataClass
{
public:
	struct BinDataElement
	{
		StatoscopeDataWriter::EFrameElementType type;
		string name;
	};

public:
	CStatoscopeDataClass(tukk format);

	size_t                GetNumElements() const          { return m_numDataElements; }
	size_t                GetNumBinElements() const       { return m_binElements.size(); }
	const BinDataElement& GetBinElement(size_t idx) const { return m_binElements[idx]; }

	tukk           GetFormat() const               { return m_format; }
	tukk           GetPath() const                 { return m_path.c_str(); }

private:
	void ProcessNewBinDataElem(tukk pStart, tukk pEnd);

private:
	tukk                 m_format;

	string                      m_path;
	u32                      m_numDataElements;
	std::vector<BinDataElement> m_binElements;
};

class CStatoscopeDataGroup
{
public:
	CStatoscopeDataGroup(const IStatoscopeDataGroup::SDescription& desc, IStatoscopeDataGroup* pCallback)
		: m_id(desc.key)
		, m_name(desc.name)
		, m_dataClass(desc.format)
		, m_pCallback(pCallback)
	{
	}

	char                  GetId() const       { return m_id; }
	tukk           GetName() const     { return m_name; }
	IStatoscopeDataGroup* GetCallback() const { return m_pCallback; }

	void                  WriteHeader(CDataWriter* pDataWriter);
	size_t                GetNumElements() const { return m_dataClass.GetNumElements(); }

private:
	const char            m_id;
	tukk           m_name;
	CStatoscopeDataClass  m_dataClass;
	IStatoscopeDataGroup* m_pCallback;
};

class CStatoscopeIntervalGroup
{
public:
	CStatoscopeIntervalGroup(u32 id, tukk name, tukk format);
	virtual ~CStatoscopeIntervalGroup() {}

	u32      GetId() const          { return m_id; }
	tukk GetName() const        { return m_name; }

	size_t      GetNumElements() const { return m_dataClass.GetNumElements(); }

	void        Enable(CStatoscopeEventWriter* pWriter);
	void        Disable();

	size_t      GetDescEventLength() const;
	void        WriteDescEvent(uk p) const;

protected:
	size_t GetValueLength(tukk pStr) const
	{
		assert(pStr);
		return (strlen(pStr) + 4) & ~3;
	}

	size_t GetValueLength(float f) { return sizeof(f); }
	size_t GetValueLength(i32 i)   { return sizeof(i); }
	size_t GetValueLength(int64 i) { return sizeof(i); }

	void   Align(tuk& p)
	{
		p = reinterpret_cast<tuk>((reinterpret_cast<UINT_PTR>(p) + 3) & ~3);
	}

	void WriteValue(tuk& p, tukk pStr)
	{
		assert(pStr);
		strcpy(p, pStr);
		p += strlen(pStr) + 1;
		Align(p);
	}

	void WriteValue(tuk& p, float f)
	{
		*alias_cast<float*>(p) = f;
		p += sizeof(float);
	}

	void WriteValue(tuk& p, int64 i)
	{
		*alias_cast<int64*>(p) = i;
		p += sizeof(int64);
	}

	void WriteValue(tuk& p, i32 i)
	{
		*alias_cast<i32*>(p) = i;
		p += sizeof(i32);
	}

	CStatoscopeEventWriter* GetWriter() const { return m_pWriter; }

private:
	virtual void Enable_Impl() = 0;
	virtual void Disable_Impl() = 0;

private:
	u32                  m_id;
	tukk             m_name;
	CStatoscopeDataClass    m_dataClass;
	size_t                  m_instLength;

	CStatoscopeEventWriter* m_pWriter;
};

class CStatoscope;

class CStatoscopeServer
{
public:
	CStatoscopeServer(CStatoscope* pStatoscope);

	bool  IsConnected() { return m_isConnected; }
	void  StartListening();
	void  CheckForConnection();
	void  CloseConnection();
	void  SetBlockingState(bool block);

	i32 ReceiveData(uk buffer, i32 bufferSize);    // returns num bytes received or -1 if an error occurred

	//called from data thread
	void SendData(tukk buffer, i32 bufferSize);

protected:

	bool CheckError(i32 err, tukk tag);          // returns true if err is a DrxSock error and also handles the error (prints it and closes the connection)
	void HandleError(tukk tag);

	DRXSOCKET    m_socket;
	bool         m_isConnected;
	CStatoscope* m_pStatoscope;
};

class CStatoscopeIOThread : public IThread
{
public:
	CStatoscopeIOThread();
	virtual ~CStatoscopeIOThread();

	void     QueueSendData(tukk pBuffer, i32 nBytes);

	threadID GetThreadID() const                     { return m_threadID; }

	void     SetDataWriter(CDataWriter* pDataWriter) { m_pDataWriter = pDataWriter; }

	void     Flush();

	void     Clear()
	{
		DrxMT::queue<SendJob>::AutoLock lock(m_sendJobs.get_lock());
		m_sendJobs.clear();
		m_numBytesInQueue = 0;
	}

	u32 GetReadBounds(tukk & pStart, tukk & pEnd)
	{
		DrxMT::queue<SendJob>::AutoLock lock(m_sendJobs.get_lock());
		if (m_sendJobs.size() > 0)
		{
			const SendJob* currentJob = &m_sendJobs.front();
			pStart = currentJob->pBuffer;

			currentJob = &m_sendJobs.back();
			pEnd = currentJob->pBuffer + currentJob->nBytes;
		}
		else
		{
			pStart = NULL;
			pEnd = NULL;
		}
		return m_numBytesInQueue;
	}

protected:
	// Start accepting work on thread
	virtual void ThreadEntry();

	struct SendJob
	{
		tukk pBuffer;
		i32         nBytes;
	};

	DrxMT::queue<SendJob> m_sendJobs;
	u32                m_numBytesInQueue;
	threadID              m_threadID;
	CDataWriter*          m_pDataWriter;
	 bool         m_bRun;
};

//Base data writer class
class CDataWriter
{
public:

	CDataWriter(bool bUseStringPool, float writeTimeout);
	virtual ~CDataWriter() {}

	virtual bool Open() = 0;
	virtual void Close();
	virtual void Flush();
	virtual void SendData(tukk pBuffer, i32 nBytes) = 0;

	void         ResetForNewLog();

	//write to internal buffer, common to all IO Writers
	void WriteData(ukk pData, i32 Size);

	template<typename T>
	void WriteData(T data)
	{
		WriteData((uk )&data, sizeof(T));
	}

	void WriteDataStr(tukk pStr)
	{
		if (m_bUseStringPool)
		{
			u32 crc = CCrc32::Compute(pStr);
			WriteData(crc);

			if (m_GlobalStringPoolHashes.find(crc) == m_GlobalStringPoolHashes.end())
			{
				m_GlobalStringPoolHashes.insert(crc);
				WriteDataStr_Raw(pStr);
			}
		}
		else
		{
			WriteDataStr_Raw(pStr);
		}
	}

	void Pad4();

	void FlushIOThread();

	bool IsUsingStringPool()
	{
		return m_bUseStringPool;
	}

	void TimeOut()
	{
		m_bTimedOut = true;
	}

	bool HasTimedOut()
	{
		return m_bTimedOut;
	}

	float GetWriteTimeout()
	{
		return m_writeTimeout;
	}

	bool m_bShouldOutputLogTopHeader;
	bool m_bHaveOutputModuleInformation;

protected:

	void WriteDataStr_Raw(tukk pStr)
	{
		i32 size = strlen(pStr);
		WriteData(size);
		WriteData(pStr, size);
	}

	enum
	{
		FlushLength = 4096,
	};

protected:
	CStatoscopeIOThread                              m_DataThread;

	std::vector<char, stl::STLGlobalAllocator<char>> m_buffer;  //circular buffer for data
	std::vector<char, stl::STLGlobalAllocator<char>> m_formatBuffer;
	tuk            m_pWritePtr;
	tuk            m_pFlushStartPtr;

	std::set<u32> m_GlobalStringPoolHashes;

	float            m_writeTimeout;
	bool             m_bUseStringPool;
	 bool    m_bTimedOut;
};

class CFileDataWriter : public CDataWriter
{
public:
	CFileDataWriter(const string& fileName, float writeTimeout);
	~CFileDataWriter();

	virtual bool Open();
	virtual void Close();
	virtual void Flush();

	virtual void SendData(tukk pBuffer, i32 nBytes);

private:
	// don't want m_pFile being pinched
	CFileDataWriter(const CFileDataWriter&);
	CFileDataWriter& operator=(const CFileDataWriter&);

protected:
	string m_fileName;
	FILE*  m_pFile;
	bool   m_bAppend;
};

class CSocketDataWriter : public CDataWriter
{
public:
	CSocketDataWriter(CStatoscopeServer* pStatoscopeServer, float writeTimeout);
	~CSocketDataWriter() { Close(); }

	virtual bool Open();
	virtual void SendData(tukk pBuffer, i32 nBytes);

protected:
	CStatoscopeServer* m_pStatoscopeServer;
};

class CStatoscopeEventWriter
{
public:
	CStatoscopeEventWriter();

	void BeginBlock()
	{
		m_eventStreamLock.Lock();
		CTimeValue tv = gEnv->pTimer->GetAsyncTime();
		uint64 timeStamp = (uint64)tv.GetMicroSecondsAsInt64();
		if (timeStamp < m_lastTimestampUs)
			__debugbreak();
		m_lastTimestampUs = timeStamp;
	}

	void EndBlock()
	{
		m_eventStreamLock.Unlock();
	}

	template<typename T>
	T* BeginEvent(size_t additional = 0)
	{
		BeginBlock();
		return BeginBlockEvent<T>(additional);
	}

	void EndEvent()
	{
		EndBlockEvent();
		EndBlock();
	}

	template<typename T>
	T* BeginBlockEvent(size_t additional = 0)
	{
		using namespace StatoscopeDataWriter;

		u8 id = (u8)T::EventId;
		size_t req = ((sizeof(EventHeader) + sizeof(T) + additional) + 3) & ~3;
		size_t len = m_eventStream.size();
		m_eventStream.resize(len + req);
		EventHeader* pHdr = (EventHeader*)&m_eventStream[len];
		pHdr->eventId = id;
		pHdr->sequence = m_eventNextSequence++;
		pHdr->eventLengthInWords = static_cast<u16>(req / 4);
		pHdr->timeStampUs = m_lastTimestampUs;
		T* pEv = (T*)&m_eventStream[len + sizeof(EventHeader)];
		return pEv;
	}

	void EndBlockEvent()
	{
	}

	void Flush(CDataWriter* pWriter);
	void Reset();

private:
	DrxCriticalSectionNonRecursive                   m_eventStreamLock;
	std::vector<char, stl::STLGlobalAllocator<char>> m_eventStream;
	std::vector<char, stl::STLGlobalAllocator<char>> m_eventStreamTmp;
	u8  m_eventNextSequence;
	uint64 m_lastTimestampUs;
};

class CTelemetryDataWriter : public CDataWriter
{
public:
	CTelemetryDataWriter(tukk postHeader, tukk hostname, i32 port, float writeTimeout, float connectTimeout);
	~CTelemetryDataWriter() { Close(); }

	virtual bool Open();
	virtual void Close();
	virtual void SendData(tukk pBuffer, i32 nBytes);

protected:
	void CheckSocketError(DrxSock::eDrxSockError sockErr, tukk description);

private:
	void SendToSocket(tukk pData, size_t nSize, tukk sDescription);

	DrxStringLocal m_postHeader;
	DrxStringLocal m_hostname;
	float          m_connectTimeout;
	i32            m_port;
	DRXSOCKET      m_socket;
	bool           m_hasSentHeader;
	bool           m_socketErrorTriggered;
};

struct SParticleProfilersDG;
struct SPhysEntityProfilersDG;
struct SFrameProfilersDG;
struct STexturePoolBucketsDG;
struct SUserMarkerDG;
struct SCallstacksDG;

// Statoscope implementation, access IStatoscope through gEnv->pStatoscope
class CStatoscope : public IStatoscope, public ISystemEventListener, ICaptureFrameListener
{
public:
	CStatoscope();
	~CStatoscope();

	virtual bool        RegisterDataGroup(IStatoscopeDataGroup* pDG);
	virtual void        UnregisterDataGroup(IStatoscopeDataGroup* pDG);

	virtual void        Tick();
	virtual void        AddUserMarker(tukk path, tukk name);
	virtual void        AddUserMarkerFmt(tukk path, tukk fmt, ...);
	virtual void        LogCallstack(tukk tag);
	virtual void        LogCallstackFormat(tukk tagFormat, ...);
	virtual void        SetCurrentProfilerRecords(const std::vector<CFrameProfiler*>* profilers);
	virtual void        Flush();
	virtual bool        IsLoggingForTelemetry();
	virtual bool        RequiresParticleStats(bool& bEffectStats);
	virtual void        AddParticleEffect(tukk pEffectName, i32 count);
	virtual void        AddPhysEntity(const phys_profile_info* pInfo);
	virtual tukk GetLogFileName() { return m_logFilename.c_str(); }
	virtual void        CreateTelemetryStream(tukk postHeader, tukk hostname, i32 port);
	virtual void        CloseTelemetryStream();

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);

	//protected:
	enum EScreenShotCaptureState
	{
		eSSCS_Idle = 0,
		eSSCS_RequestCapture,
		eSSCS_AwaitingBufferRequest,
		eSSCS_AwaitingCapture,
		eSSCS_DataReceived
	};

	enum { SCREENSHOT_SCALING_FACTOR = 8 };

	enum ELogDestination
	{
		// these values must match the help string for e_StatoscopeLogDestination
		eLD_File      = 0,
		eLD_Socket    = 1,
		eLD_Telemetry = 2
	};

	void AddFrameRecord(bool bOutputHeader);
	void SetLogFilename();
	void SetDataGroups(uint64 enableDGs, uint64 disableDGs);
	void OutputLoadedModuleInformation(CDataWriter* pDataWriter);
	void StoreCallstack(tukk tag, uk * callstack, u32 callstackLength);

	template<typename T, typename A>
	static tukk GetDataGroupsCVarHelpString(const std::vector<T*, A>& dgs)
	{
		tukk helpStr = "Which data groups are recorded each frame: flags+ enables, flags- disables\n"
		                      " 0 = none\n"
		                      " 1 = all\n";

		u32 sizeNeeded = strlen(helpStr) + 1;  // +1 for the null terminator

		for (u32 i = 0; i < dgs.size(); i++)
			sizeNeeded += 7 + strlen(dgs[i]->GetName());

		static string helpString;
		helpString.reserve(sizeNeeded);
		helpString.append(helpStr);

		string tmp;
		for (u32 i = 0; i < dgs.size(); i++)
		{
			const T* dg = dgs[i];
			tmp.Format(" %c+ = %s\n", dg->GetId(), dg->GetName());
			helpString += tmp;
		}

		return helpString.c_str();
	}

	static void ConsoleAddUserMarker(IConsoleCmdArgs* pParams);
	static void OnLogDestinationCVarChange(ICVar* pVar);
	static void OnTagCVarChange(ICVar* pVar);

	//Screenshot capturing
	virtual bool OnNeedFrameData(u8*& pConvertedTextureBuf);
	virtual void OnFrameCaptured(void);
	virtual i32  OnGetFrameWidth(void);
	virtual i32  OnGetFrameHeight(void);
	virtual i32  OnCaptureFrameBegin(i32* pTexHandle);

	//Setup statoscope for fps captures
	virtual void SetupFPSCaptureCVars();

	//Request statoscope screen shot, may not succeed if screem shot is already in progress
	virtual bool RequestScreenShot();

	void         PrepareScreenShot();
	u8*       ProcessScreenShot();

	std::vector<CStatoscopeDataGroup*>             m_activeDataGroups;
	std::vector<CStatoscopeDataGroup*>             m_allDataGroups;

	ICVar*                                         m_pStatoscopeEnabledCVar;
	ICVar*                                         m_pStatoscopeDumpAllCVar;
	ICVar*                                         m_pStatoscopeDataGroupsCVar;
	ICVar*                                         m_pStatoscopeIvDataGroupsCVar;
	ICVar*                                         m_pStatoscopeLogDestinationCVar;
	ICVar*                                         m_pStatoscopeScreenshotCapturePeriodCVar;
	ICVar*                                         m_pStatoscopeFilenameUseBuildInfoCVar;
	ICVar*                                         m_pStatoscopeFilenameUseMapCVar;
	ICVar*                                         m_pStatoscopeFilenameUseTagCvar;
	ICVar*                                         m_pStatoscopeFilenameUseTimeCVar;
	ICVar*                                         m_pStatoscopeFilenameUseDatagroupsCVar;
	ICVar*                                         m_pStatoscopeMinFuncLengthMsCVar;
	ICVar*                                         m_pStatoscopeMaxNumFuncsPerFrameCVar;
	ICVar*                                         m_pStatoscopeCreateLogFilePerLevelCVar;
	ICVar*                                         m_pStatoscopeWriteTimeout;
	ICVar*                                         m_pStatoscopeConnectTimeout;
	ICVar*                                         m_pGameRulesCVar;
	ICVar*                                         m_pStatoscopeAllowFPSOverrideCVar;

	string                                         m_currentMap;

	float                                          m_lastDumpTime;
	float                                          m_screenshotLastCaptureTime;
	i32                                            m_lastScreenWidth;
	i32                                            m_lastScreenHeight;
	bool                                           m_groupMaskInitialized;
	bool                                           m_bLevelStarted;
	uint64                                         m_activeDataGroupMask;
	uint64                                         m_activeIvDataGroupMask;
	u32                                         m_logNum;

	std::vector<std::pair<CFrameProfiler*, int64>> m_perfStatDumpProfilers;   // only used locally in SetCurrentProfilerRecords() but kept to avoid reallocation

	string                                         m_logFilename;

	u8*                                         m_pScreenShotBuffer; //Buffer for the render thread to asynchronously push screenshots to
	 EScreenShotCaptureState               m_ScreenShotState;

	CStatoscopeServer*                             m_pServer;

	CDataWriter*                                   m_pDataWriter;

private:
	typedef std::vector<CStatoscopeIntervalGroup*, stl::STLGlobalAllocator<CStatoscopeIntervalGroup*>> IntervalGroupVec;

private:
	CStatoscope(const CStatoscope&);
	CStatoscope& operator=(const CStatoscope&);

private:
	void RegisterBuiltInDataGroups();
	void RegisterBuiltInIvDataGroups();

	void CreateDataWriter();
	void WriteIntervalClassEvents();

private:
	IntervalGroupVec       m_intervalGroups;
	CStatoscopeEventWriter m_eventWriter;

	// Built in data groups
	SParticleProfilersDG*   m_pParticleProfilers;
	SPhysEntityProfilersDG* m_pPhysEntityProfilers;
	SFrameProfilersDG*      m_pFrameProfilers;
	SUserMarkerDG*          m_pUserMarkers;
	SCallstacksDG*          m_pCallstacks;
};

#endif // ENABLE_STATOSCOPE

#endif  // __Statoscope_h__
