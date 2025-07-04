// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLCPB_AttrReader.h>
#include <drx3D/Act/XMLCPB_Reader.h>

using namespace XMLCPB;

//////////////////////////////////////////////////////////////////////////
// format in mem: look at CAttrLive::Compact()

CAttrReader::CAttrReader(CReader& Reader)
	: m_Reader(Reader)
	, m_addr(XMLCPB_INVALID_FLATADDR)
	, m_nameId(XMLCPB_INVALID_ID)
	, m_type(DT_INVALID)
{
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::InitFromCompact(FlatAddr addr, u16 header)
{
	assert(addr != XMLCPB_INVALID_FLATADDR);

	m_type = eAttrDataType(header & MASK_TYPEID);
	m_nameId = header >> BITS_TYPEID;  // nameStringId is always in the upper part, typeID in the lower part
	m_addr = addr;
}

//////////////////////////////////////////////////////////////////////////

FlatAddr CAttrReader::GetAddrNextAttr() const
{
	if (m_type != DT_RAWDATA)
		return GetDataAddr() + GetDataTypeSize(m_type);

	// Read in the size of the raw data which should be stored first at our location
	u32 size;
	FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), size);
	return addr + size;
}

//////////////////////////////////////////////////////////////////////////

tukk CAttrReader::GetName() const
{
	return m_Reader.GetAttrNamesTable().GetString(m_nameId);
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::Get(i32& val) const
{
	if (m_type >= DT_0 && m_type <= DT_10)
	{
		val = m_type - DT_0;
		return;
	}

	switch (m_type)
	{
	case DT_POS8:
		{
			u8 rawVal;
			m_Reader.ReadFromBuffer(GetDataAddr(), rawVal);
			val = rawVal;
			break;
		}

	case DT_POS16:
		{
			u16 rawVal;
			m_Reader.ReadFromBufferEndianAware(GetDataAddr(), rawVal);
			val = rawVal;
			break;
		}

	case DT_255:
		{
			val = 255;
			break;
		}

	case DT_INT32:
		{
			m_Reader.ReadFromBufferEndianAware(GetDataAddr(), val);
			break;
		}

	case DT_NEG16:
		{
			u16 rawVal;
			m_Reader.ReadFromBufferEndianAware(GetDataAddr(), rawVal);
			val = -i32(rawVal);
			break;
		}

	case DT_NEG8:
		{
			u8 rawVal;
			m_Reader.ReadFromBuffer(GetDataAddr(), rawVal);
			val = -i32(rawVal);
			break;
		}

	case DT_STR:
		{
			tukk pStr = NULL;
			Get(pStr);
			if (pStr)
				val = atoi(pStr);
			break;
		}

	case DT_0:
		{
			val = 0;
			break;
		}

	default:
		{
			assert(false);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::Get(int64& val) const
{
	switch (m_type)
	{
	case DT_INT64:
		m_Reader.ReadFromBufferEndianAware(GetDataAddr(), val);
		break;

	default:
		{
			i32 int32Val;
			Get(int32Val);
			val = int32Val;
			break;
		}
	}
}

void CAttrReader::Get(uint64& val) const
{
	int64 valInt64;
	Get(valInt64);
	val = uint64(valInt64);
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::Get(uint& val) const
{
	i32 valInt;
	Get(valInt);
	val = uint(valInt);
}

void CAttrReader::Get(i16& val) const
{
	i32 valInt;
	Get(valInt);
	val = valInt;
}

void CAttrReader::Get(int8& val) const
{
	i32 valInt;
	Get(valInt);
	val = valInt;
}

void CAttrReader::Get(u8& val) const
{
	i32 valInt;
	Get(valInt);
	val = valInt;
}

void CAttrReader::Get(u16& val) const
{
	i32 valInt;
	Get(valInt);
	val = valInt;
}

void CAttrReader::Get(bool& val) const
{
	i32 valInt;
	Get(valInt);
	val = valInt != 0;
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::Get(tukk & pStr) const
{
	if (m_type == DT_RAWDATA) // very big strings are stored as raw data
	{
		u32 size;
		FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), size);
		pStr = (tukk)(m_Reader.GetPointerFromFlatAddr(addr));
		assert(pStr[size - 1] == 0);
		return;
	}
	if (IsTypeStrConstant(m_type))
	{
		assert(m_type >= DT_FIRST_CONST_STR && m_type <= DT_LAST_CONST_STR);
		pStr = GetConstantString(m_type - DT_FIRST_CONST_STR);
		return;
	}

	if (m_type != DT_STR)
	{
		assert(false);
		pStr = "";
		return;
	}

	u16 rawVal;
	m_Reader.ReadFromBufferEndianAware(GetDataAddr(), rawVal);
	StringID stringId = rawVal;
	pStr = m_Reader.GetStrDataTable().GetString(stringId);
}

//////////////////////////////////////////////////////////////////////////

void CAttrReader::Get(float& val) const
{
	switch (m_type)
	{
	case DT_F1:
		{
			m_Reader.ReadFromBuffer(GetDataAddr(), val);
			break;
		}

	case DT_STR:
		{
			tukk pStr = NULL;
			Get(pStr);
			if (pStr)
				val = float(atof(pStr));
			break;
		}

	case DT_F1_1DEC:
		{
			int8 byteVal;
			m_Reader.ReadFromBuffer(GetDataAddr(), byteVal);
			val = byteVal;
			val /= 10;
			break;
		}

	default:
		{
			i32 intVal;
			Get(intVal);
			val = float(intVal);
			break;
		}
	}
}

void CAttrReader::Get(Vec2& _val) const
{
	Vec3 val;
	Get(val);
	_val.x = val.x;
	_val.y = val.y;
}

void CAttrReader::Get(Ang3& _val) const
{
	Vec3 val;
	Get(val);
	_val.x = val.x;
	_val.y = val.y;
	_val.z = val.z;
}

//////////////////////////////////////////////////////////////////////////
// look at CAttrWriter::PackFloatInSemiConstType() for some explanation.

float CAttrReader::UnpackFloatInSemiConstType(u8 mask, u32 ind, FlatAddr& addr) const
{
	u32 shift = ind * 2;

	u32 type = (mask >> shift) & 3;

	switch (type)
	{
	case PFSC_0:
		return 0;
	case PFSC_1:
		return 1;
	case PFSC_N1:
		return -1;
	case PFSC_VAL:
		{
			float val;
			addr = m_Reader.ReadFromBuffer(addr, val);
			return val;
		}
	default:
		{
			assert(false);
			return 0;
		}
	}
}

void CAttrReader::Get(Vec3& val) const
{
	switch (m_type)
	{
	case DT_F3:
		{
			FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), val.x);
			addr = m_Reader.ReadFromBuffer(addr, val.y);
			m_Reader.ReadFromBuffer(addr, val.z);
			break;
		}

	case DT_F3_1CONST:
	case DT_F3_2CONST:
	case DT_F3_3CONST:
		{
			u8 mask;
			FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), mask);
			val.x = UnpackFloatInSemiConstType(mask, 0, addr);
			val.y = UnpackFloatInSemiConstType(mask, 1, addr);
			val.z = UnpackFloatInSemiConstType(mask, 2, addr);
			break;
		}

	case DT_F3_100:
		{
			val.x = 1;
			val.y = 0;
			val.z = 0;
			break;
		}

	case DT_F3_010:
		{
			val.x = 0;
			val.y = 1;
			val.z = 0;
			break;
		}

	case DT_F3_001:
		{
			val.x = 0;
			val.y = 0;
			val.z = 1;
			break;
		}

	case DT_0:
		{
			val.x = val.y = val.z = 0;
			break;
		}

	default:
		{
			assert(false);
			break;
		}
	}
}

void CAttrReader::Get(Quat& val) const
{
	switch (m_type)
	{
	case DT_F4:
		{
			FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), val.v.x);
			addr = m_Reader.ReadFromBuffer(addr, val.v.y);
			addr = m_Reader.ReadFromBuffer(addr, val.v.z);
			m_Reader.ReadFromBuffer(addr, val.w);
			break;
		}

	case DT_F4_1CONST:
	case DT_F4_2CONST:
	case DT_F4_3CONST:
	case DT_F4_4CONST:
		{
			u8 mask;
			FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), mask);
			val.v.x = UnpackFloatInSemiConstType(mask, 0, addr);
			val.v.y = UnpackFloatInSemiConstType(mask, 1, addr);
			val.v.z = UnpackFloatInSemiConstType(mask, 2, addr);
			val.w = UnpackFloatInSemiConstType(mask, 3, addr);
			break;
		}

	case DT_0:
		{
			val.v.x = val.v.y = val.v.z = val.w = 0;
			break;
		}

	default:
		{
			assert(false);
			break;
		}
	}
}

void CAttrReader::Get(u8*& rdata, u32& outSize) const
{
	assert(m_type == DT_RAWDATA);

	// Read in the size of the raw chunk which follows
	u32 size;
	FlatAddr addr = m_Reader.ReadFromBuffer(GetDataAddr(), size);

	// Realloc the buffer so it's large enough to handle the new data's size
	rdata = (u8*)realloc((uk )rdata, size * sizeof(u8));
	m_Reader.ReadFromBuffer(addr, rdata, size);
	outSize = size;
}

//////////////////////////////////////////////////////////////////////////
// reads any attr value, converts to string and store it into an string
// is separated from the normal Get functions in purpose: This function should be used only for error handling or other special situations

void CAttrReader::GetValueAsString(string& str) const
{
	switch (XMLCPB::GetBasicDataType(m_type))
	{
	case DT_STR:
		ValueToString<tukk >(str, "%s");
		break;

	case DT_INT32:
		ValueToString<u32>(str, "%d");
		break;

	case DT_F1:
		ValueToString<float>(str, "%f");
		break;

	case DT_F3:
		{
			Vec3 val;
			Get(val);
			str.Format("%f,%f,%f", val.x, val.y, val.z);
			break;
		}

	case DT_F4:
		{
			Quat val;
			Get(val);
			str.Format("%f,%f,%f,%f", val.v.x, val.v.y, val.v.z, val.w);
			break;
		}

	case DT_INT64:
		ValueToString<uint64>(str, "%lu");
		break;

	case DT_RAWDATA:
		{
			u32 size;
			m_Reader.ReadFromBuffer(GetDataAddr(), size);
			str.Format("RawData %u", size);
			break;
		}

	default:
		str = "";
		assert(false);
		break;
	}
}
