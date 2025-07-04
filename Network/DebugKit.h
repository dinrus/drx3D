// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DEBUGKIT_H__
#define __DEBUGKIT_H__

#pragma once

#include <drx3D/Network/Config.h>

#if ENABLE_DEBUG_KIT

	#include <drx3D/Network/INetwork.h>
	#include <drx3D/Network/DebugOutput.h>
	#include <drx3D/Network/Encoding.h>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CDebugKit : public IThread, public CDebugOutput, public CEncoding
{
public:
	static CDebugKit& Get()
	{
		if (!m_pThis)
		{
			DrxAutoCriticalSection lk(m_lockThis);
			if (!m_pThis)
			{
				m_pThis = new CDebugKit;
				if (!gEnv->pThreadUpr->SpawnThread(m_pThis, "NetworkDebugKit"))
				{
					DrxFatalError("Error spawning \"NetworkDebugKit\" thread.");
				}
			}
		}
		return *m_pThis;
	}

	static void Shutdown()
	{
		if (m_pThis)
		{
			m_pThis->SignalStopWork();
			gEnv->pThreadUpr->JoinThread(m_pThis, eJM_Join);
			SAFE_DELETE(m_pThis);
		}
	}

	void Update();

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

	//template<typename T>
	//void AddDataEntity(tukk name, u32 key, const T& data)
	//{
	//	if (m_curDataEnt.obj)
	//	{
	//		DrxAutoCriticalSection lk(m_lockThis);
	//		if (std::binary_search(m_servicesReq.begin(), m_servicesReq.end(), m_curDataEnt.key))
	//		{
	//			Write('d');
	//			Write(m_curDataEnt.obj);
	//			Write(string(name));
	//			Write(key);
	//			Write(data);
	//		}
	//	}
	//}

	void SetObject(EntityId obj)     { m_curDataEnt.obj = obj; }
	void SetValue(tukk value) { if (m_curDataEnt.obj) m_curDataEnt.value = value; }
	void SetKey(u32 key)          { m_curDataEnt.key = key; }

	void AddDataEnt(const Vec3& v);
	void AddDataEnt(const Ang3& v)       { AddDataEnt(Vec3(v)); }
	void AddDataEnt(SNetObjectID id)     { AddDataEnt(Vec3(id.id, id.salt, 0.0f)); }
	void AddDataEnt(float x)             { AddDataEnt(Vec3(x, 0.0f, 0.0f));    }
	void AddDataEnt(const Quat& q)       { Quat p = q.GetNormalized(); AddDataEnt(p.v);  }
	void AddDataEnt(const CTimeValue& t) { AddDataEnt(Vec3(t.GetSeconds(), 0.0f, 0.0f)); }
	void AddDataEnt(const string&)       { AddDataEnt(Vec3(0.0f, 0.0f, 0.0f)); }
	void AddDataEnt(ScriptAnyValue&)     { AddDataEnt(Vec3(0.0f, 0.0f, 0.0f)); }
	void AddDataEnt(i32 x)             { AddDataEnt(float(x)); }
	void AddDataEnt(int64 x)             { AddDataEnt(float(x)); }
	void AddDataEnt(u32 x)            { AddDataEnt(float(x)); }
	void AddDataEnt(uint64 x)            { AddDataEnt(float(x)); }

	// Only present to allow template instantiation for all SerializationTypes (see DrxCommon)
	void AddDataEnt(const XmlNodeRef& x)     { __debugbreak(); }
	void AddDataEnt(const ScriptAnyValue& x) { __debugbreak(); }

	void LogSnapping(const Vec3& witnessPos, const Vec3& witnessDir, const Vec3& entityPos0, const Vec3& entityPos1, const string& entityCls);

	void SCBegin(u32 tag);
	void SCPut(tukk);
	void SCPut(float);

	void SetSessionID(const CSessionID& session);

	void Lock()   { m_lockThis.Lock(); }
	void Unlock() { m_lockThis.Unlock(); }

private:
	CDebugKit();
	~CDebugKit();

	CDebugOutput* GetDebugOutput() { return this; }

	enum EState
	{
		eS_Starting,
		eS_Searching,
		eS_Running,
		eS_Complete
	};

	 bool              m_running;

	EState                     m_state;

	static DrxCriticalSection  m_lockThis;
	static CDebugKit*  m_pThis;

	DRXSOCKADDR_IN             m_server;
	std::vector<u32>        m_servicesReq;

	struct SDataEnt
	{
		EntityId obj;
		string   value;
		u32   key;
		Vec3     data;
	};
	SDataEnt m_curDataEnt;

	bool InitServer();
	void RunServer();
};

	#define DEBUGKIT_SET_OBJECT(o)   CDebugKit::Get().SetObject(o)
	#define DEBUGKIT_SET_VALUE(v)    CDebugKit::Get().SetValue(v)
	#define DEBUGKIT_SET_KEY(k)      CDebugKit::Get().SetKey(k)
	#define DEBUGKIT_ADD_DATA_ENT(d) CDebugKit::Get().AddDataEnt(d)
	#define DEBUGKIT_LOG_SNAPPING(witnessPos, witnessDir, entityPos0, entityPos1, entityCls) \
	  CDebugKit::Get().LogSnapping(witnessPos, witnessDir, entityPos0, entityPos1, entityCls)

//#define DEBUGKIT_ADD_DATA_ENTITY(name, key, data) CDebugKit::Get().AddDataEntity(name, key, data)

	#define DEBUGKIT_CODING(t, l, s) CDebugKit::Get().Coding(t, l, s)
	#define DEBUGKIT_ANNOTATION(x)   CDebugKit::Get().Annotation(x)

struct SDebugPacket
{
	SDebugPacket(TNetChannelID channel, bool isEncoding, u32 uniqueId)
	{
		m_bSent = false;
		CDebugKit::Get().BeginPacket(channel, isEncoding, uniqueId);
	}
	~SDebugPacket()
	{
		if (!m_bSent)
			CDebugKit::Get().EndPacket(false);
	}
	void Sent()
	{
		m_bSent = true;
		CDebugKit::Get().EndPacket(true);
	}

	bool m_bSent;
};

#else

	#define DEBUGKIT_SET_OBJECT(o)
	#define DEBUGKIT_SET_VALUE(v)
	#define DEBUGKIT_SET_KEY(k)
	#define DEBUGKIT_ADD_DATA_ENT(d)
	#define DEBUGKIT_LOG_SNAPPING(witnessPos, witnessDir, entityPos0, entityPos1, entityCls)

//#define DEBUGKIT_ADD_DATA_ENTITY(name, key, data) ((uk )0)

	#define DEBUGKIT_CODING(t, l, s) {}
	#define DEBUGKIT_ANNOTATION(x)

#endif

#endif
