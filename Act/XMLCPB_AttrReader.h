// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_ATTRREADER_H
	#define XMLCPB_ATTRREADER_H

	#include <drx3D/Act/XMLCPB_Common.h>

namespace XMLCPB {

class CReader;

class CAttrReader
{
	friend class CReader;
public:

	CAttrReader(CReader& Reader);
	CAttrReader(const CAttrReader& other) : m_Reader(other.m_Reader) { *this = other; }
	~CAttrReader() {}

	CAttrReader& operator=(const CAttrReader& other)
	{
		if (this != &other)
		{
			DRX_ASSERT (&m_Reader == &other.m_Reader);
			m_type = other.m_type;
			m_nameId = other.m_nameId;
			m_addr = other.m_addr;
		}
		return *this;
	}

	void          InitFromCompact(FlatAddr offset, u16 header);

	FlatAddr      GetAddrNextAttr() const;
	tukk   GetName() const;
	eAttrDataType GetBasicDataType() const { return XMLCPB::GetBasicDataType(m_type); }

	void          Get(i32& val) const;
	void          Get(int64& val) const;
	void          Get(uint64& val) const;
	void          Get(i16& val) const;
	void          Get(int8& val) const;
	void          Get(u8& val) const;
	void          Get(u16& val) const;
	void          Get(uint& val) const;
	void          Get(bool& val) const;
	void          Get(float& val) const;
	void          Get(Vec2& val) const;
	void          Get(Ang3& val) const;
	void          Get(Vec3& val) const;
	void          Get(Quat& val) const;
	void          Get(tukk & pStr) const;
	void          Get(u8*& rdata, u32& outSize) const;

	void          GetValueAsString(string& str) const;

	#ifndef _RELEASE
	StringID GetNameId() const { return m_nameId; }    // debug help
	#endif

private:

	FlatAddr GetDataAddr() const { return m_addr;  }
	float    UnpackFloatInSemiConstType(u8 mask, u32 ind, FlatAddr& addr) const;

	template<class T>
	void ValueToString(string& str, tukk formatString) const;

	eAttrDataType m_type;
	StringID      m_nameId;
	FlatAddr      m_addr;
	CReader&      m_Reader;

};

template<class T>
void CAttrReader::ValueToString(string& str, tukk formatString) const
{
	T val;
	Get(val);
	str.Format(formatString, val);
}

} // end namespace
#endif
