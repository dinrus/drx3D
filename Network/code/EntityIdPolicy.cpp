// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>
#include  <drx3D/Network/NetContext.h>
#include  <drx3D/Network/ContextView.h>
#include  <drx3D/Network/BoolCompress.h>
#include  <drx3D/Network/CTPEndpoint.h>
#include <drx3D/Entity/IEntitySystem.h>

class CEntityIdPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		return true;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		m_hasMemento = true;
		m_lastValue = in.GetTyped<SNetObjectID>();
		m_bool.ReadMemento(in);
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		out.PutTyped<SNetObjectID>() = m_lastValue;
		m_bool.WriteMemento(out);
		return true;
	}

	void NoMemento() const
	{
		m_hasMemento = false;
		m_lastValue = SNetObjectID();
		m_bool.NoMemento();
	}
#endif

#if USE_ARITHSTREAM
	#if RESERVE_UNBOUND_ENTITIES
	bool ReadValue(CCommInputStream& in, u16& value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID = SNetObjectID();
		ReadValue(in, netID, pModel, age);
		value = netID.id;
		return true;
	}
	bool WriteValue(CCommOutputStream& out, u16 value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID(value, 0);
		return WriteValue(out, netID, pModel, age);
	}
	#endif
	bool ReadValue(CCommInputStream& in, EntityId& value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID = SNetObjectID();
		ReadValue(in, netID, pModel, age);
		//const SContextObject * pObj = pModel->GetNetContext()->GetContextObject( netID );
		SContextObjectRef obj = pModel->GetNetContextState()->GetContextObject(netID);
		if (!obj.main)
		{
	#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel)
			{
				NetLog("Valid network id (%s) sent, but no valid entity id available (probably that entity hasn't yet been bound)", netID.GetText());
				NetLog("  processing message %s", CCTPEndpoint::GetCurrentProcessingMessageDescription());
			}
	#endif
			value = 0;
	#if RESERVE_UNBOUND_ENTITIES
			value = pModel->GetNetContextState()->AddReservedUnboundEntityMapEntry(netID.id, 0);
		#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel > 1)
			{
				NetLog("Valid network id (%s) mapped to reserved unbound EntityId %d", netID.GetText(), value);
			}
		#endif
	#endif
		}
		else
			value = obj.main->userID;
		return true;
	}
	bool WriteValue(CCommOutputStream& out, EntityId value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID = pModel->GetNetContextState()->GetNetID(value, false);
	#if ENABLE_UNBOUND_ENTITY_DEBUGGING
		if (!netID && value)
			NetWarning("Entity id %.8x is not known in the network system - probably has not been bound", value);
	#endif
		return WriteValue(out, netID, pModel, age);
	}
	bool ReadValue(CCommInputStream& in, SNetObjectID& value, CArithModel* pModel, u32 age) const
	{
		if (m_hasMemento)
		{
			bool same = m_bool.ReadValue(in);
			if (!same)
			{
				value = pModel->ReadNetId(in);
			}
			else
			{
				value = m_lastValue;
				pModel->GetNetContextState()->Resaltify(value);
			}
		}
		else
		{
			value = pModel->ReadNetId(in);
		}
		m_lastValue = value;

		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pModel->GetNetContextState()->GetEntityID(value));
		NetLogPacketDebug("CEntityIdPolicy::ReadValue %d:%d %s (%f)", value.id, value.salt, pEntity ? pEntity->GetName() : "<no name>", in.GetBitSize());

		return true;
	}
	bool WriteValue(CCommOutputStream& out, SNetObjectID value, CArithModel* pModel, u32 age) const
	{
		if (m_hasMemento)
		{
			m_bool.WriteValue(out, m_lastValue == value);
			if (m_lastValue != value)
				pModel->WriteNetId(out, value);
		}
		else
		{
			pModel->WriteNetId(out, value);
		}
		m_lastValue = value;
		return true;
	}
	bool ReadValue(CCommInputStream& in, XmlNodeRef& value, CArithModel* pModel, u32 age) const
	{
		assert(false);
		return false;
	}
	bool WriteValue(CCommOutputStream& out, XmlNodeRef value, CArithModel* pModel, u32 age) const
	{
		assert(false);
		return false;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
#else
	#if RESERVE_UNBOUND_ENTITIES
	bool ReadValue(CNetInputSerializeImpl* in, u16& value, u32 age) const
	{
		SNetObjectID netID = SNetObjectID();
		ReadValue(in, netID, age);
		value = netID.id;
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, u16 value, u32 age) const
	{
		SNetObjectID netID(value, 0);
		return WriteValue(out, netID, age);
	}
	#endif
	bool ReadValue(CNetInputSerializeImpl* in, EntityId& value, u32 age) const
	{
		SNetObjectID netID;

		ReadValue(in, netID, age);

		SContextObjectRef obj = in->GetNetChannel()->GetContextView()->ContextState()->GetContextObject(netID);

		if (!obj.main)
		{
	#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel)
			{
				NetLog("Valid network id (%s) sent, but no valid entity id available (probably that entity hasn't yet been bound)", netID.GetText());
				NetLog("  processing message %s", CCTPEndpoint::GetCurrentProcessingMessageDescription());
			}
	#endif
			value = 0;
	#if RESERVE_UNBOUND_ENTITIES
			value = in->GetNetChannel()->GetContextView()->ContextState()->AddReservedUnboundEntityMapEntry(netID.id, 0);
		#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel > 1)
			{
				NetLog("Valid network id (%s) mapped to reserved unbound EntityId %d", netID.GetText(), value);
			}
		#endif
	#endif
		}
		else
		{
			value = obj.main->userID;
		}

		return true;
	}

	bool WriteValue(CNetOutputSerializeImpl* out, EntityId value, u32 age) const
	{
		SNetObjectID netID = out->GetNetChannel()->GetContextView()->ContextState()->GetNetID(value, false);

	#if ENABLE_UNBOUND_ENTITY_DEBUGGING
		if (!netID && value)
		{
			NetWarning("Entity id %.8x is not known in the network system - probably has not been bound", value);
		}
	#endif

		return WriteValue(out, netID, age);
	}

	bool ReadValue(CNetInputSerializeImpl* in, SNetObjectID& value, u32 age) const
	{
		value = in->ReadNetId();

		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(in->GetNetChannel()->GetContextView()->ContextState()->GetEntityID(value));
		NetLogPacketDebug("CEntityIdPolicy::ReadValue %d:%d %s (%f)", value.id, value.salt, pEntity ? pEntity->GetName() : "<no name>", in->GetBitSize());

		return true;
	}

	bool WriteValue(CNetOutputSerializeImpl* out, SNetObjectID value, u32 age) const
	{
		out->WriteNetId(value);

		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}

	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CEntityIdPolicy");
		pSizer->Add(*this);
	}
#if NET_PROFILE_ENABLE
	i32 GetBitCount(EntityId value)
	{
		return BITCOUNT_NETID;
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif
private:
#if USE_MEMENTO_PREDICTORS
	mutable bool m_hasMemento;
	mutable SNetObjectID  m_lastValue;
	mutable CBoolCompress m_bool;
#endif
};

class CSimpleEntityIdPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		return true;
	}

#if USE_MEMENTO_PREDICTORS
	bool ReadMemento(CByteInputStream& in) const
	{
		return true;
	}

	bool WriteMemento(CByteOutputStream& out) const
	{
		return true;
	}

	void NoMemento() const
	{
	}
#endif

#if USE_ARITHSTREAM
	#if RESERVE_UNBOUND_ENTITIES
	bool ReadValue(CCommInputStream& in, u16& value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID = SNetObjectID();
		ReadValue(in, netID, pModel, age);
		value = netID.id;
		return true;
	}
	bool WriteValue(CCommOutputStream& out, u16 value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID(value, 0);
		return WriteValue(out, netID, pModel, age);
	}
	#endif
	bool ReadValue(CCommInputStream& in, EntityId& value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID;
		ReadValue(in, netID, pModel, age);
		//const SContextObject * pObj = pModel->GetNetContext()->GetContextObject( netID );
		SContextObjectRef obj = pModel->GetNetContextState()->GetContextObject(netID);
		if (!obj.main)
		{
	#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel)
			{
				NetLog("Valid network id (%s) sent, but no valid entity id available (probably that entity hasn't yet been bound)", netID.GetText());
				NetLog("  processing message %s", CCTPEndpoint::GetCurrentProcessingMessageDescription());
			}
	#endif
			value = 0;
	#if RESERVE_UNBOUND_ENTITIES
			value = pModel->GetNetContextState()->AddReservedUnboundEntityMapEntry(netID.id, 0);
		#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel > 1)
			{
				NetLog("Valid network id (%s) mapped to reserved unbound EntityId %d", netID.GetText(), value);
			}
		#endif
	#endif
		}
		else
			value = obj.main->userID;
		return true;
	}
	bool WriteValue(CCommOutputStream& out, EntityId value, CArithModel* pModel, u32 age) const
	{
		SNetObjectID netID = pModel->GetNetContextState()->GetNetID(value, false);
	#if ENABLE_UNBOUND_ENTITY_DEBUGGING
		if (!netID && value)
			NetWarning("Entity id %.8x is not known in the network system - probably has not been bound", value);
	#endif
		return WriteValue(out, netID, pModel, age);
	}
	bool ReadValue(CCommInputStream& in, SNetObjectID& value, CArithModel* pModel, u32 age) const
	{
		//value.id = in.ReadBits(16);
		//value.salt = in.ReadBits(16);
		value = pModel->ReadNetId(in);
		return true;
	}
	bool WriteValue(CCommOutputStream& out, SNetObjectID value, CArithModel* pModel, u32 age) const
	{
		//out.WriteBits(value.id, 16);
		//out.WriteBits(value.salt, 16);
		pModel->WriteNetId(out, value);
		return true;
	}

	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
#else
	#if RESERVE_UNBOUND_ENTITIES
	bool ReadValue(CNetInputSerializeImpl* in, u16& value, u32 age) const
	{
		SNetObjectID netID = SNetObjectID();
		ReadValue(in, netID, age);
		value = netID.id;
		return true;
	}
	bool WriteValue(CNetOutputSerializeImpl* out, u16 value, u32 age) const
	{
		SNetObjectID netID(value, 0);
		return WriteValue(out, netID, age);
	}
	#endif
	bool ReadValue(CNetInputSerializeImpl* in, EntityId& value, u32 age) const
	{
		SNetObjectID netID;

		ReadValue(in, netID, age);

		SContextObjectRef obj = in->GetNetChannel()->GetContextView()->ContextState()->GetContextObject(netID);

		if (!obj.main)
		{
	#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel)
			{
				NetLog("Valid network id (%s) sent, but no valid entity id available (probably that entity hasn't yet been bound)", netID.GetText());
				NetLog("  processing message %s", CCTPEndpoint::GetCurrentProcessingMessageDescription());
			}
	#endif
			value = 0;
	#if RESERVE_UNBOUND_ENTITIES
			value = in->GetNetChannel()->GetContextView()->ContextState()->AddReservedUnboundEntityMapEntry(netID.id, 0);
		#if LOG_ENTITYID_ERRORS
			if (netID && CNetCVars::Get().LogLevel > 1)
			{
				NetLog("Valid network id (%s) mapped to reserved unbound EntityId %d", netID.GetText(), value);
			}
		#endif
	#endif
		}
		else
		{
			value = obj.main->userID;
		}

		return true;
	}

	bool WriteValue(CNetOutputSerializeImpl* out, EntityId value, u32 age) const
	{
		SNetObjectID netID = out->GetNetChannel()->GetContextView()->ContextState()->GetNetID(value, false);

	#if ENABLE_UNBOUND_ENTITY_DEBUGGING
		if (!netID && value)
		{
			NetWarning("Entity id %.8x is not known in the network system - probably has not been bound", value);
		}
	#endif

		return WriteValue(out, netID, age);
	}

	bool ReadValue(CNetInputSerializeImpl* in, SNetObjectID& value, u32 age) const
	{
		value.id = in->ReadBits(16);
		value.salt = in->ReadBits(16);
		return true;
	}

	bool WriteValue(CNetOutputSerializeImpl* out, SNetObjectID value, u32 age) const
	{
		out->WriteBits(value.id, 16);
		out->WriteBits(value.salt, 16);
		return true;
	}

	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}

	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		NetWarning("EntityIdPolicy: not implemented for generic types");
		return false;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CSimpleEntityIdPolicy");
		pSizer->Add(*this);
	}
#if NET_PROFILE_ENABLE
	i32 GetBitCount(EntityId value)
	{
		return 32;
	}

	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif
};

REGISTER_COMPRESSION_POLICY(CEntityIdPolicy, "EntityId");
REGISTER_COMPRESSION_POLICY(CSimpleEntityIdPolicy, "SimpleEntityId");
