// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IArchive.h"
#include "Serializer.h"

template<typename TCRCRef>
class CRCRefSerializer
	: public Serialization::IString
{
public:
	CRCRefSerializer(TCRCRef& crcRef)
		: m_crcRef(crcRef)
	{
	}

	virtual void set(tukk value)
	{
		m_crcRef.SetByString(value);
	}

	virtual tukk get() const
	{
		return m_crcRef.c_str();
	}

	ukk handle() const
	{
		return &m_crcRef;
	}

	Serialization::TypeID type() const
	{
		return Serialization::TypeID::get<TCRCRef>();
	}

	TCRCRef& m_crcRef;
};

template<u32 StoreStrings, typename THash>
class CCRCRefSerializerNoStrings
{
public:
	CCRCRefSerializerNoStrings(struct SCRCRef<StoreStrings, THash>& crcRef)
		: crc(crcRef.crc)
	{
	}

	bool Serialize(Serialization::IArchive& ar)
	{
		return ar(crc, "CRC", "CRC");
	}

	typedef typename THash::TInt TInt;
	TInt& crc;
};

template<u32 StoreStrings, typename THash>
bool Serialize(Serialization::IArchive& ar, struct SCRCRef<StoreStrings, THash>& crcRef, tukk name, tukk label)
{
	if (StoreStrings == 0)
	{
		if (ar.isInput())
		{
			SCRCRef<StoreStrings, THash> crcCopy;
			ar(CCRCRefSerializerNoStrings<StoreStrings, THash>(crcCopy), name, label);
			if (crcCopy.crc != THash::INVALID)
			{
				crcRef = crcCopy;
				return true;
			}
		}
		else if (ar.isOutput())
		{
			return ar(CCRCRefSerializerNoStrings<StoreStrings, THash>(crcRef), name, label);
		}
	}

	CRCRefSerializer<SCRCRef<StoreStrings, THash>> crcRefSerializer(crcRef);
	return ar(static_cast<Serialization::IString&>(crcRefSerializer), name, label);
}
