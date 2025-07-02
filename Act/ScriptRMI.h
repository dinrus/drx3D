// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Provides remote method invocation to script

   -------------------------------------------------------------------------
   История:
   - 30:11:2004   11:30 : Created by Craig Tiller

*************************************************************************/
#ifndef __SCRIPTRMI_H__
#define __SCRIPTRMI_H__

#pragma once

#include "ScriptSerialize.h"
#include <drx3D/CoreX/Thread/DrxAtomics.h>

class CGameContext;

// this class is a singleton that handles remote method invocation for
// script objects
class CScriptRMI : private CScriptSerialize
{
public:
	CScriptRMI();

	bool            Init();
	void            UnloadLevel();

	i32             ExposeClass(IFunctionHandler* pFH);
	void            SetupEntity(EntityId id, IEntity* pEntity, bool client, bool server);
	void            RemoveEntity(EntityId id);
	INetAtSyncItem* HandleRMI(bool bClient, EntityId objID, u8 funcID, TSerialize ser, INetChannel* pChannel);
	bool            SerializeScript(TSerialize ser, IEntity* pEntity);
	void            OnInitClient(u16 channelId, IEntity* pEntity);
	void            OnPostInitClient(u16 channelId, IEntity* pEntity);
	void            GetMemoryStatistics(IDrxSizer* s);

	void            SetContext(CGameContext* pContext);

	static void     RegisterCVars();
	static void     UnregisterCVars();

private:
	enum EDispatchFlags
	{
		eDF_ToServer                = 0x01,
		eDF_ToClientOnChannel       = 0x02,
		eDF_ToClientOnOtherChannels = 0x04,
	};

	bool BuildDispatchTable(
	  SmartScriptTable methods,
	  SmartScriptTable methodTableFromCls,
	  SmartScriptTable cls,
	  tukk name);
	bool BuildSynchTable(
	  SmartScriptTable vars,
	  SmartScriptTable cls,
	  tukk name);

	void       AddProxyTable(IScriptTable* pEntityTable,
	                         ScriptHandle id, ScriptHandle flags, tukk name, SmartScriptTable dispatchTable);
	void       AddSynchedTable(IScriptTable* pEntityTable,
	                           ScriptHandle id, tukk name, SmartScriptTable metaTable);
	static i32 ProxyFunction(IFunctionHandler* pH, uk pBuffer, i32 nSize);
	static i32 SynchedNewIndexFunction(IFunctionHandler* pH);
	static i32 SynchedIndexFunction(IFunctionHandler* pH);
	static i32 SerializeFunction(IFunctionHandler* pH, uk pBuffer, i32 nSize);

	class CScriptMessage;
	class CCallHelper;
	static void DispatchRMI(INetChannel* pChannel, _smart_ptr<CScriptMessage> pMsg, bool bClient);

	CGameContext*      m_pParent;
	static CScriptRMI* m_pThis;

	struct SFunctionInfo;
	struct SSynchedPropertyInfo;

	static const size_t MaxRMIParameters = 31;             // must be a multiple of eight minus one
	static const size_t MaxSynchedPropertyNameLength = 31; // must be a multiple of eight minus one

	struct SFunctionDispatch
	{
		SFunctionDispatch()
		{
			format[0] = name[0] = 0;
		}
		void GetMemoryUsage(IDrxSizer* pSizer) const {}
		char format[MaxRMIParameters + 1];
		char name[MaxSynchedPropertyNameLength + 1];
	};

	typedef std::vector<SFunctionDispatch> SFunctionDispatchTable;
	struct SDispatch
	{
		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(server);
			pSizer->AddObject(client);
		}

		SFunctionDispatchTable server;
		SFunctionDispatchTable client;

		SmartScriptTable       m_serverDispatchScriptTable;
		SmartScriptTable       m_clientDispatchScriptTable;
	};

	bool ValidateDispatchTable(tukk clazz, SmartScriptTable dispatch, SmartScriptTable methods, bool bServerTable);

	// everything under here is protected by the following mutex, and is for multithreaded serialization
	DrxCriticalSection         m_dispatchMutex;
	std::map<string, size_t>   m_entityClassToEntityTypeID;
	std::map<EntityId, size_t> m_entities;
	std::vector<SDispatch>     m_dispatch;
};

#endif
