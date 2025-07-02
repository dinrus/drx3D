// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  compression policy that doesn't send (nor change) anything
   -------------------------------------------------------------------------
   История:
   - 02/11/2006   12:34 : Created by Craig Tiller

   TODO : We don't actually define any values for this policy, so its using
   the defaults of 0 - The policy itself is used only as a hack to get around
   a problem when switching profiles (see Actor.cpp ('skip')) - AKA we should
   fix this and remove the policy (at least when non ARITHSTREAM mode)

*************************************************************************/

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include  <drx3D/Network/ArithModel.h>

class CNoSendPolicy
{
public:
	bool Load(XmlNodeRef node, const string& filename)
	{
		for (i32 i = 0; i < NENT; i++)
			m_values[i] = 0;

		if (XmlNodeRef params = node->findChild("Params"))
		{
			if (params->haveAttr("default"))
			{
				params->getAttr("default", m_values[0]);
				for (i32 i = 1; i < NENT; i++)
					m_values[i] = m_values[0];
			}
			else
			{
				params->getAttr("x", m_values[0]);
				params->getAttr("y", m_values[1]);
				params->getAttr("z", m_values[2]);
				params->getAttr("w", m_values[3]);
			}
		}

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
	template<class T>
	bool ReadValue(CCommInputStream& in, T& value, CArithModel* pModel, u32 age) const
	{
		value = T(m_values[0]);
		return true;
	}
	bool ReadValue(CCommInputStream& in, bool& value, CArithModel* pModel, u32 age) const
	{
		value = (fabsf(m_values[0]) > 1e-6f);
		return true;
	}
	bool ReadValue(CCommInputStream& in, Vec2& value, CArithModel* pModel, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		return true;
	}
	bool ReadValue(CCommInputStream& in, Vec3& value, CArithModel* pModel, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		value[2] = m_values[2];
		return true;
	}
	bool ReadValue(CCommInputStream& in, Ang3& value, CArithModel* pModel, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		value[2] = m_values[2];
		return true;
	}
	bool ReadValue(CCommInputStream& in, Quat& value, CArithModel* pModel, u32 age) const
	{
		value.w = m_values[0];
		value.v.x = m_values[1];
		value.v.y = m_values[2];
		value.v.z = m_values[3];
		return true;
	}
	bool ReadValue(CCommInputStream& in, SSerializeString& value, CArithModel* pModel, u32 age) const
	{
		value.resize(0);
		return true;
	}
	bool ReadValue(CCommInputStream& in, SNetObjectID& value, CArithModel* pModel, u32 age) const
	{
		value = SNetObjectID();
		return true;
	}
	bool ReadValue(CCommInputStream& in, XmlNodeRef& value, CArithModel* pModel, u32 age) const
	{
		assert(false);
		return false;
	}
	template<class T>
	bool WriteValue(CCommOutputStream& out, T value, CArithModel* pModel, u32 age) const
	{
		return true;
	}
#else
	template<class T>
	bool ReadValue(CNetInputSerializeImpl* in, T& value, u32 age) const
	{
		value = T(m_values[0]);
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, bool& value, u32 age) const
	{
		value = (fabsf(m_values[0]) > 1e-6f);
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, Vec2& value, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, Vec3& value, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		value[2] = m_values[2];
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, Ang3& value, u32 age) const
	{
		value[0] = m_values[0];
		value[1] = m_values[1];
		value[2] = m_values[2];
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, Quat& value, u32 age) const
	{
		value.w = m_values[0];
		value.v.x = m_values[1];
		value.v.y = m_values[2];
		value.v.z = m_values[3];
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, SSerializeString& value, u32 age) const
	{
		value.resize(0);
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, SNetObjectID& value, u32 age) const
	{
		value = SNetObjectID();
		return true;
	}
	bool ReadValue(CNetInputSerializeImpl* in, XmlNodeRef& value, u32 age) const
	{
		assert(false);
		return false;
	}
	template<class T>
	bool WriteValue(CNetOutputSerializeImpl* out, T value, u32 age) const
	{
		return true;
	}
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CNoSendPolicy");
		pSizer->Add(*this);
	}
#if NET_PROFILE_ENABLE
	template<class T>
	i32 GetBitCount(T value)
	{
		return 0;
	}
#endif
private:
	static i32k NENT = 4;
	float            m_values[NENT];
};

REGISTER_COMPRESSION_POLICY(CNoSendPolicy, "NoSend");
