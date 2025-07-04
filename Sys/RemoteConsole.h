// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   RemoteConsole.h
//  Version:     v1.00
//  Created:     12/6/2012 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __RemoteConsole_H__
#define __RemoteConsole_H__

#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/Network/DrxSocks.h>

#if !defined(RELEASE) || defined(RELEASE_LOGGING) || defined(ENABLE_PROFILING_CODE)
	#define USE_REMOTE_CONSOLE
#endif

#include <drx3D/CoreX/Thread/IThreadUpr.h>

/////////////////////////////////////////////////////////////////////////////////////////////
// CRemoteConsole
//
// IRemoteConsole implementation
//
/////////////////////////////////////////////////////////////////////////////////////////////

struct SRemoteServer;
class CRemoteConsole : public IRemoteConsole
{
public:
	static CRemoteConsole* GetInst() { static CRemoteConsole inst; return &inst; }

	virtual void           RegisterConsoleVariables();
	virtual void           UnregisterConsoleVariables();

	virtual void           Start();
	virtual void           Stop();
	virtual bool           IsStarted() const;

	virtual void           AddLogMessage(tukk log);
	virtual void           AddLogWarning(tukk log);
	virtual void           AddLogError(tukk log);

	virtual void           Update();

	virtual void           RegisterListener(IRemoteConsoleListener* pListener, tukk name);
	virtual void           UnregisterListener(IRemoteConsoleListener* pListener);

private:
	CRemoteConsole();
	virtual ~CRemoteConsole();

#if 0 // currently no stroboscope support
	void SendThreadData();
	void SendStroboscopeResult();
#endif

private:
	SRemoteServer* m_pServer;
	 bool  m_running;
	typedef CListenerSet<IRemoteConsoleListener*> TListener;
	TListener      m_listener;
	ICVar*         m_pLogEnableRemoteConsole;
};

#ifdef USE_REMOTE_CONSOLE
/////////////////////////////////////////////////////////////////////////////////////////////
// IRemoteEvent
//
// remote events
// EConsoleEventType must be unique, also make sure order is same on clients!
// each package starts with ASCII ['0' + EConsoleEventType]
// not more supported than 256 - '0'!
//
/////////////////////////////////////////////////////////////////////////////////////////////

enum EConsoleEventType
{
	eCET_Noop = 0,
	eCET_Req,
	eCET_LogMessage,
	eCET_LogWarning,
	eCET_LogError,
	eCET_ConsoleCommand,
	eCET_AutoCompleteList,
	eCET_AutoCompleteListDone,

	eCET_Strobo_GetThreads,
	eCET_Strobo_ThreadAdd,
	eCET_Strobo_ThreadDone,

	eCET_Strobo_GetResult,
	eCET_Strobo_ResultStart,
	eCET_Strobo_ResultDone,

	eCET_Strobo_StatStart,
	eCET_Strobo_StatAdd,
	eCET_Strobo_ThreadInfoStart,
	eCET_Strobo_ThreadInfoAdd,
	eCET_Strobo_SymStart,
	eCET_Strobo_SymAdd,
	eCET_Strobo_CallstackStart,
	eCET_Strobo_CallstackAdd,

	eCET_GameplayEvent,

	eCET_Strobo_FrameInfoStart,
	eCET_Strobo_FrameInfoAdd,
};

struct SRemoteEventFactory;
struct IRemoteEvent
{
	virtual ~IRemoteEvent() {}
	IRemoteEvent(EConsoleEventType type) : m_type(type) {}
	EConsoleEventType                     GetType() const { return m_type; }
	virtual std::unique_ptr<IRemoteEvent> Clone() const = 0;

protected:
	friend struct SRemoteEventFactory;
	virtual void                          WriteToBuffer(tuk buffer, size_t& size, size_t maxsize) const = 0;
	virtual std::unique_ptr<IRemoteEvent> CreateFromBuffer(tukk buffer, size_t size) const = 0;

private:
	const EConsoleEventType m_type;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Event implementations
//
// implementations for simple data packages
//
/////////////////////////////////////////////////////////////////////////////////////////////
template<EConsoleEventType T>
struct SNoDataEvent final : IRemoteEvent
{
	SNoDataEvent() : IRemoteEvent(T) {}
	virtual std::unique_ptr<IRemoteEvent> Clone() const override { return std::unique_ptr<IRemoteEvent>(new SNoDataEvent<T>()); }

protected:
	virtual void                          WriteToBuffer(tuk buffer, size_t& size, size_t maxsize) const override { size = 0; }
	virtual std::unique_ptr<IRemoteEvent> CreateFromBuffer(tukk buffer, size_t size) const override         { return Clone(); }
};

/////////////////////////////////////////////////////////////////////////////////////////////
template<EConsoleEventType T>
struct SStringEvent final : IRemoteEvent
{
	SStringEvent(tukk string) : IRemoteEvent(T), m_data(string) {}
	virtual std::unique_ptr<IRemoteEvent> Clone() const override { return std::unique_ptr<IRemoteEvent>(new SStringEvent<T>(GetData())); }
	tukk                           GetData() const        { return m_data.c_str(); }

protected:
	virtual void WriteToBuffer(tuk buffer, size_t& size, size_t maxsize) const override
	{
		tukk data = GetData();
		size = std::min(m_data.size(), maxsize);
		memcpy(buffer, data, size);
	}

	virtual std::unique_ptr<IRemoteEvent> CreateFromBuffer(tukk buffer, size_t size) const override
	{
		return std::unique_ptr<IRemoteEvent>(new SStringEvent<T>(buffer));
	}

private:
	const string m_data;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// SRemoteEventFactory
//
// remote event factory
//
/////////////////////////////////////////////////////////////////////////////////////////////
struct SRemoteEventFactory
{
	static SRemoteEventFactory*   GetInst() { static SRemoteEventFactory inst; return &inst; }
	std::unique_ptr<IRemoteEvent> CreateEventFromBuffer(tukk buffer, size_t size);
	void                          WriteToBuffer(const IRemoteEvent* pEvent, tuk buffer, size_t& size);

private:
	SRemoteEventFactory();
	~SRemoteEventFactory() {}

	void RegisterEvent(std::unique_ptr<IRemoteEvent> pEvent);
private:
	typedef std::map<EConsoleEventType, std::unique_ptr<IRemoteEvent>> TPrototypes;
	TPrototypes m_prototypes;
};

typedef std::list<std::unique_ptr<IRemoteEvent>> TEventBuffer;

/////////////////////////////////////////////////////////////////////////////////////////////
// SRemoteServer
//
// Server thread
//
/////////////////////////////////////////////////////////////////////////////////////////////
struct SRemoteClient;
struct SRemoteServer : public IThread
{
	SRemoteServer() : m_socket(DRX_INVALID_SOCKET), m_bAcceptClients(false) {}

	void StartServer();
	void StopServer();

	void AddEvent(std::unique_ptr<IRemoteEvent> pEvent);
	void GetEvents(TEventBuffer& buffer);

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

private:
	bool WriteBuffer(SRemoteClient* pClient, tuk buffer, size_t& size);
	bool ReadBuffer(tukk buffer, size_t data);

private:
	struct SRemoteClientInfo
	{
		SRemoteClientInfo(std::unique_ptr<SRemoteClient> client) : pClient(std::move(client)), pEvents(new TEventBuffer) {}
		std::unique_ptr<SRemoteClient> pClient;
		std::unique_ptr<TEventBuffer>  pEvents;
	};

	typedef std::vector<SRemoteClientInfo> TClients;
	TClients      m_clients;
	DRXSOCKET     m_socket;
	DrxMutex      m_lock;
	TEventBuffer  m_eventBuffer;
	 bool m_bAcceptClients;
	friend struct SRemoteClient;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// SRemoteClient
//
// Client thread
//
/////////////////////////////////////////////////////////////////////////////////////////////
struct SRemoteClient : public IThread
{
	SRemoteClient(SRemoteServer* pServer) : m_pServer(pServer), m_socket(DRX_INVALID_SOCKET), m_bRun(true){}

	void StartClient(DRXSOCKET socket);

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

private:
	bool RecvPackage(tuk buffer, size_t& size);
	bool SendPackage(tukk buffer, size_t size);
	void FillAutoCompleteList(std::vector<string>& list);

private:
	SRemoteServer* m_pServer;
	DRXSOCKET      m_socket;
	 bool  m_bRun;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#endif //#ifdef USE_REMOTE_CONSOLE

#endif //#ifndef __RemoteConsole_H__
