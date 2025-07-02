// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DrxName.h"
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Serializer.h>

class DrxNameSerializer : public Serialization::IString
{
public:
	DrxNameSerializer(CDrxName& s)
		: m_s(s)
	{
	}

	virtual void set(tukk value)
	{
		m_s = value;
	}

	virtual tukk get() const
	{
		return m_s.c_str();
	}

	virtual ukk handle() const
	{
		return &m_s;
	}

	virtual Serialization::TypeID type() const
	{
		return Serialization::TypeID::get<CDrxName>();
	}

	CDrxName& m_s;
};

inline bool Serialize(Serialization::IArchive& ar, CDrxName& drxName, tukk name, tukk label)
{
	DrxNameSerializer serializer(drxName);
	return ar(static_cast<Serialization::IString&>(serializer), name, label);
}
