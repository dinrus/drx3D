// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/TypeID.h>

#include <functional>

//! Globally Unique Identifier binary compatible with standard 128bit GUID
struct DrxGUID
{
	uint64 hipart;
	uint64 lopart;

	//////////////////////////////////////////////////////////////////////////
	// METHODS
	//////////////////////////////////////////////////////////////////////////
	//remark: this custom constructor will turn DrxGUID into a non-aggregate
	constexpr DrxGUID() : hipart(0),lopart(0) {}

	constexpr DrxGUID(const DrxGUID& rhs) : hipart(rhs.hipart),lopart(rhs.lopart) {}

	constexpr DrxGUID(const uint64& hipart_, const uint64& lopart_) : hipart(hipart_),lopart(lopart_) {}

	constexpr static DrxGUID Construct(const uint64& hipart, const uint64& lopart)
	{
		return DrxGUID{ hipart, lopart };
	}

	constexpr static DrxGUID Construct(u32 d1, u16 d2, u16 d3, u8 d4[8])
	{
		return DrxGUID(
			(((uint64)d3) << 48) | (((uint64)d2) << 32) | ((uint64)d1),  //high part
			*((uint64*)d4)  //low part
		 );
	}

	constexpr static DrxGUID Construct(u32 d1, u16 d2, u16 d3,
		u8 d4_0, u8 d4_1, u8 d4_2, u8 d4_3, u8 d4_4, u8 d4_5, u8 d4_6, u8 d4_7)
	{
		return DrxGUID(
			(((uint64)d3) << 48) | (((uint64)d2) << 32) | ((uint64)d1),  //high part
			(((uint64)d4_7) << 56) | (((uint64)d4_6) << 48) | (((uint64)d4_5) << 40) | (((uint64)d4_4) << 32) | (((uint64)d4_3) << 24) | (((uint64)d4_2) << 16) | (((uint64)d4_1) << 8) | (uint64)d4_0   //low part
		);
	}

	inline static DrxGUID Create();

	constexpr static DrxGUID Null()
	{
		return DrxGUID::Construct( 0, 0 );
	}

	constexpr bool IsNull() const { return hipart == 0 && lopart == 0; }

	constexpr bool operator==(const DrxGUID& rhs) const { return hipart == rhs.hipart && lopart == rhs.lopart; }
	constexpr bool operator!=(const DrxGUID& rhs) const { return hipart != rhs.hipart || lopart != rhs.lopart; }
	constexpr bool operator<(const DrxGUID& rhs) const  { return hipart < rhs.hipart || (hipart == rhs.hipart && lopart < rhs.lopart); }
	constexpr bool operator>(const DrxGUID& rhs) const  { return hipart > rhs.hipart || (hipart == rhs.hipart && lopart > rhs.lopart); }
	constexpr bool operator<=(const DrxGUID& rhs) const { return hipart < rhs.hipart || (hipart == rhs.hipart && lopart <= rhs.lopart); }
	constexpr bool operator>=(const DrxGUID& rhs) const { return hipart > rhs.hipart || (hipart == rhs.hipart && lopart >= rhs.lopart); }

	struct StringUtils
	{
		// Convert hexadecimal character to u8.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u8 HexCharToUInt8(char x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}

		// Convert hexadecimal character to unsigned short.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u16 HexCharToUInt16(char x)
		{
			return static_cast<u16>(HexCharToUInt8(x));
		}

		// Convert hexadecimal character to u64.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u32 HexCharToUInt32(char x)
		{
			return static_cast<u32>(HexCharToUInt8(x));
		}

		// Convert hexadecimal string to u8.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u8 HexStringToUInt8(tukk szInput)
		{
			return (HexCharToUInt8(szInput[0]) << 4) +
				HexCharToUInt8(szInput[1]);
		}

		// Convert hexadecimal string to unsigned short.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u16 HexStringToUInt16(tukk szInput)
		{
			return (HexCharToUInt16(szInput[0]) << 12) +
				(HexCharToUInt16(szInput[1]) << 8) +
				(HexCharToUInt16(szInput[2]) << 4) +
				HexCharToUInt16(szInput[3]);
		}

		// Convert hexadecimal string to u64.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr static u32 HexStringToUInt32(tukk szInput)
		{
			return (HexCharToUInt32(szInput[0]) << 28) +
				(HexCharToUInt32(szInput[1]) << 24) +
				(HexCharToUInt32(szInput[2]) << 20) +
				(HexCharToUInt32(szInput[3]) << 16) +
				(HexCharToUInt32(szInput[4]) << 12) +
				(HexCharToUInt32(szInput[5]) << 8) +
				(HexCharToUInt32(szInput[6]) << 4) +
				HexCharToUInt32(szInput[7]);
		}
		struct SGuidStringSerializer : public Serialization::IString
		{
			char buffer[40];

			SGuidStringSerializer() { memset(buffer,0,sizeof(buffer)); }
			void        set(tukk value) { drx_strcpy(buffer,value); }
			tukk get() const { return buffer; }
			ukk handle() const { return &buffer; }
			Serialization::TypeID      type() const { return Serialization::TypeID::get<SGuidStringSerializer>(); }
		};
		struct GUID_mapped
		{
			u32 Data1;
			u16 Data2;
			u16 Data3;
			u8  Data4[8];
		};
	};

	//////////////////////////////////////////////////////////////////////////
	// Create GUID from string.
	// GUID must be in form of this example string (case insensitive): "296708CE-F570-4263-B067-C6D8B15990BD"
	////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr static DrxGUID FromStringInternal(tukk szInput)
	{
		return DrxGUID::Construct(
			StringUtils::HexStringToUInt32(szInput),        // u32 data1
			StringUtils::HexStringToUInt16(szInput + 9),   // u16 data2
			StringUtils::HexStringToUInt16(szInput + 14),  // u16 data3
			StringUtils::HexStringToUInt8(szInput + 19),   // u8  data4[8]
			StringUtils::HexStringToUInt8(szInput + 21),
			StringUtils::HexStringToUInt8(szInput + 24),
			StringUtils::HexStringToUInt8(szInput + 26),
			StringUtils::HexStringToUInt8(szInput + 28),
			StringUtils::HexStringToUInt8(szInput + 30),
			StringUtils::HexStringToUInt8(szInput + 32),
			StringUtils::HexStringToUInt8(szInput + 34)
		);
	};

	//! Write GUID to zero terminated character array.
	//! Require 36 bytes
	inline void ToString(char *output,size_t const output_size_in_bytes) const
	{
		static_assert(sizeof(StringUtils::GUID_mapped) == sizeof(DrxGUID), "GUID_mapped and DrxGUID must be the same size");

		const StringUtils::GUID_mapped* pMappedGUID = alias_cast<StringUtils::GUID_mapped*>(this);

		u32 Data1;
		u32 Data2;
		u32 Data3;
		u32 Data4[8];

		Data1 = static_cast<u32>(pMappedGUID->Data1);
		Data2 = static_cast<u32>(pMappedGUID->Data2);
		Data3 = static_cast<u32>(pMappedGUID->Data3);

		Data4[0] = static_cast<u32>(pMappedGUID->Data4[0]);
		Data4[1] = static_cast<u32>(pMappedGUID->Data4[1]);
		Data4[2] = static_cast<u32>(pMappedGUID->Data4[2]);
		Data4[3] = static_cast<u32>(pMappedGUID->Data4[3]);
		Data4[4] = static_cast<u32>(pMappedGUID->Data4[4]);
		Data4[5] = static_cast<u32>(pMappedGUID->Data4[5]);
		Data4[6] = static_cast<u32>(pMappedGUID->Data4[6]);
		Data4[7] = static_cast<u32>(pMappedGUID->Data4[7]);

		drx_sprintf(output,output_size_in_bytes,"%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x", Data1, Data2, Data3, Data4[0], Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]);
	}

	//////////////////////////////////////////////////////////////////////////
	// Create GUID from string.
	// GUID must be in form of this example string (case insensitive): "296708CE-F570-4263-B067-C6D8B15990BD"
	// GUID can also be with curly brackets: "{296708CE-F570-4263-B067-C6D8B15990BD}"
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static DrxGUID FromString(tukk szGuidAsString)
	{
		DRX_ASSERT(szGuidAsString);
		DrxGUID guid;
		if (szGuidAsString)
		{
			const size_t len = strlen(szGuidAsString);
			if (szGuidAsString[0] == '{' && len >= DRX_ARRAY_COUNT("{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}") - 1)
			{
				guid = FromStringInternal(szGuidAsString + 1);
			}
			else if (szGuidAsString[0] != '{' && len >= DRX_ARRAY_COUNT("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX") - 1)
			{
				guid = FromStringInternal(szGuidAsString);
			}
			else if (len <= sizeof(uint64) * 2 && std::all_of(szGuidAsString, szGuidAsString + len, ::isxdigit)) // Convert if it still coming from the old 64bit GUID system.
			{
				guid.hipart = std::strtoull(szGuidAsString, 0, 16);
			}
			else
			{
				DRX_ASSERT_MESSAGE(false, "GUID string is invalid: %s", szGuidAsString);
			}
		}
		return guid;
	};

	template<std::size_t N>
	void ToString(char(&ar)[N]) const
	{
		static_assert(N >= DRX_ARRAY_COUNT("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX") - 1, "GUID require buffer of at least 36 bytes");
		ToString((tuk)ar, N);
	}

	string ToString() const
	{
		char temp[DRX_ARRAY_COUNT("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")];
		ToString(temp);
		return string(temp);
	}

	//! Returns a character string used for Debugger Visualization or log messages.
	//! Do not use this method in runtime code.
	tukk ToDebugString() const
	{
		static char temp[DRX_ARRAY_COUNT("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")];
		ToString(temp);
		return temp;
	}

	//! Serialize GUID as a two 64bit unsigned integers
	static bool SerializeAsNumber(Serialization::IArchive& ar,DrxGUID &guid)
	{
		bool bResult = false;
		if (ar.isInput())
		{
			u32 dwords[4];
			bResult = ar(dwords, "guid");
			guid.lopart = (((uint64)dwords[1]) << 32) | (uint64)dwords[0];
			guid.hipart = (((uint64)dwords[3]) << 32) | (uint64)dwords[2];
		}
		else
		{
			u32 g[4] = {
				(u32)(guid.lopart & 0xFFFFFFFF), (u32)((guid.lopart >> 32) & 0xFFFFFFFF),
				(u32)(guid.hipart & 0xFFFFFFFF), (u32)((guid.hipart >> 32) & 0xFFFFFFFF)
			};
			bResult = ar(g, "guid");
		}
		return bResult;
	}

	//! Serialize GUID as a string in form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
	static bool SerializeAsString(Serialization::IArchive& archive, DrxGUID& guid, tukk szName, tukk szLabel)
	{
		StringUtils::SGuidStringSerializer strSerializer;
		if (archive.isInput())
		{
			if (archive(strSerializer, szName, szLabel))
			{
				guid = DrxGUID::FromStringInternal(strSerializer.buffer);
			}
			else
			{
				guid = DrxGUID::Null();
				return false;
			}
		}
		else if (archive.isOutput())
		{
			if (!guid.IsNull())
			{
				guid.ToString(strSerializer.buffer);
			}
			archive(strSerializer, szName, szLabel);
		}
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////
//! GUID string literal.
//! Example: "e397e62c-5c7f-4fab-9195-12032f670c9f"_drx_guid
//! creates a valid GUID usable in static initialization
////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr DrxGUID operator"" _drx_guid(tukk szInput, size_t)
{
	// Can accept guids in both forms as
	// BAC0332E-AB02-4276-9731-AB48865E0411 or as {BAC0332E-AB02-4276-9731-AB48865E0411}
	return (szInput[0] == '{') ? DrxGUID::FromStringInternal(szInput+1) : DrxGUID::FromStringInternal(szInput);
}

//! Global yasli serialization override for the DrxGUID
inline bool Serialize(Serialization::IArchive& ar, DrxGUID &guid,tukk szName, tukk szLabel = nullptr)
{
	return DrxGUID::SerializeAsString(ar, guid, szName, szLabel);
}

namespace std
{
template<> struct hash<DrxGUID>
{
public:
	size_t operator()(const DrxGUID& guid) const
	{
		std::hash<uint64> hasher;
		return hasher(guid.lopart) ^ hasher(guid.hipart);
	}
};
}

inline bool Serialize(Serialization::IArchive& ar, DrxGUID::StringUtils::SGuidStringSerializer& value, tukk name, tukk label)
{
	return ar(static_cast<Serialization::IString&>(value), name, label);
}