// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/CoreX/StlUtils.h> // TODO : Do we still need this include?
#include <drx3D/CoreX/Extension/DrxGUID.h>
#include <drx3D/CoreX/String/DrxStringUtils.h>

#include <drx3D/Schema2/BasicTypes.h>

namespace sxema2
{
	namespace StringUtils
	{
		static const size_t s_guidStringBufferSize = 38;

		// Convert ASCII character to number.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline u8 AsciiCharToNumber(char x)
		{
			if((x >= '0') && (x <= '9'))
			{
				return static_cast<u8>(x - '0');
			}
			else if((x >= 'a') && (x <= 'f'))
			{
				return static_cast<u8>(x - 'a') + 10;
			}
			else if((x >= 'A') && (x <= 'F'))
			{
				return static_cast<u8>(x - 'A') + 10;
			}
			else
			{
				return 0;
			}
		}

		// Verify that ASCII character is number.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline bool AsciiCharIsNumber(char x)
		{
			return (x == '0') || (AsciiCharToNumber(x) > 0);
		}

		// Validate system GUID string format.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline bool ValidateSysGUIDStringFormat(tukk szInput)
		{
			if(szInput)
			{
				bool bError = false;
				bError |= !AsciiCharIsNumber(szInput[ 0]);
				bError |= !AsciiCharIsNumber(szInput[ 1]);
				bError |= !AsciiCharIsNumber(szInput[ 2]);
				bError |= !AsciiCharIsNumber(szInput[ 3]);
				bError |= !AsciiCharIsNumber(szInput[ 4]);
				bError |= !AsciiCharIsNumber(szInput[ 5]);
				bError |= !AsciiCharIsNumber(szInput[ 6]);
				bError |= !AsciiCharIsNumber(szInput[ 7]);
				bError |= szInput[ 8] != '-';
				bError |= !AsciiCharIsNumber(szInput[ 9]);
				bError |= !AsciiCharIsNumber(szInput[10]);
				bError |= !AsciiCharIsNumber(szInput[11]);
				bError |= !AsciiCharIsNumber(szInput[12]);
				bError |= szInput[13] != '-';
				bError |= !AsciiCharIsNumber(szInput[14]);
				bError |= !AsciiCharIsNumber(szInput[15]);
				bError |= !AsciiCharIsNumber(szInput[16]);
				bError |= !AsciiCharIsNumber(szInput[17]);
				bError |= szInput[18] != '-';
				bError |= !AsciiCharIsNumber(szInput[19]);
				bError |= !AsciiCharIsNumber(szInput[20]);
				bError |= !AsciiCharIsNumber(szInput[21]);
				bError |= !AsciiCharIsNumber(szInput[22]);
				bError |= szInput[23] != '-';
				bError |= !AsciiCharIsNumber(szInput[24]);
				bError |= !AsciiCharIsNumber(szInput[25]);
				bError |= !AsciiCharIsNumber(szInput[26]);
				bError |= !AsciiCharIsNumber(szInput[27]);
				bError |= !AsciiCharIsNumber(szInput[28]);
				bError |= !AsciiCharIsNumber(szInput[29]);
				bError |= !AsciiCharIsNumber(szInput[30]);
				bError |= !AsciiCharIsNumber(szInput[31]);
				bError |= !AsciiCharIsNumber(szInput[32]);
				bError |= !AsciiCharIsNumber(szInput[33]);
				bError |= !AsciiCharIsNumber(szInput[34]);
				bError |= !AsciiCharIsNumber(szInput[35]);
				bError |= szInput[36] != '\0';
				return !bError;
			}
			return false;
		}

		// Read system GUID from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline SysGUID SysGUIDFromString(tukk szInput)
		{
			SysGUID sysGUID = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0} };
			if(ValidateSysGUIDStringFormat(szInput))
			{
				sysGUID.Data1 = (u64)(AsciiCharToNumber(szInput[0])) * 0x10000000
					+ (u64)(AsciiCharToNumber(szInput[1])) * 0x01000000
					+ (u64)(AsciiCharToNumber(szInput[2])) * 0x00100000
					+ (u64)(AsciiCharToNumber(szInput[3])) * 0x00010000
					+ (u64)(AsciiCharToNumber(szInput[4])) * 0x00001000
					+ (u64)(AsciiCharToNumber(szInput[5])) * 0x00000100
					+ (u64)(AsciiCharToNumber(szInput[6])) * 0x00000010
					+ (u64)(AsciiCharToNumber(szInput[7])) * 0x00000001;

				sysGUID.Data2 = static_cast<unsigned short>(AsciiCharToNumber(szInput[ 9])) * 0x00001000
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[10])) * 0x00000100
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[11])) * 0x00000010
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[12])) * 0x00000001;

				sysGUID.Data3 = static_cast<unsigned short>(AsciiCharToNumber(szInput[14])) * 0x00001000
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[15])) * 0x00000100
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[16])) * 0x00000010
					+ static_cast<unsigned short>(AsciiCharToNumber(szInput[17])) * 0x00000001;

				sysGUID.Data4[0] = AsciiCharToNumber(szInput[19]) * 0x00000010
					+ AsciiCharToNumber(szInput[20]) * 0x00000001;

				sysGUID.Data4[1] = AsciiCharToNumber(szInput[21]) * 0x00000010
					+ AsciiCharToNumber(szInput[22]) * 0x00000001;

				sysGUID.Data4[2] = AsciiCharToNumber(szInput[24]) * 0x00000010
					+ AsciiCharToNumber(szInput[25]) * 0x00000001;

				sysGUID.Data4[3]= AsciiCharToNumber(szInput[26]) * 0x00000010
					+ AsciiCharToNumber(szInput[27]) * 0x00000001;

				sysGUID.Data4[4] = AsciiCharToNumber(szInput[28]) * 0x00000010
					+ AsciiCharToNumber(szInput[29]) * 0x00000001;

				sysGUID.Data4[5] = AsciiCharToNumber(szInput[30]) * 0x00000010
					+ AsciiCharToNumber(szInput[31]) * 0x00000001;

				sysGUID.Data4[6] = AsciiCharToNumber(szInput[32]) * 0x00000010
					+ AsciiCharToNumber(szInput[33]) * 0x00000001;

				sysGUID.Data4[7] = AsciiCharToNumber(szInput[34]) * 0x00000010
					+ AsciiCharToNumber(szInput[35]) * 0x00000001;
			}
			return sysGUID;
		}

		// Write system GUID to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tuk SysGUIDToString(const SysGUID& sysGUID, const CharArrayView& output)
		{
			const size_t outputSize = output.size();
			DRX_ASSERT(outputSize >= s_guidStringBufferSize);
			if(outputSize >= s_guidStringBufferSize)
			{
				u32 Data1;
				u32 Data2;
				u32 Data3;
				u32 Data4[8];
				Data1    = static_cast<u32>(sysGUID.Data1);
				Data2    = static_cast<u32>(sysGUID.Data2);
				Data3    = static_cast<u32>(sysGUID.Data3);
				Data4[0] = static_cast<u32>(sysGUID.Data4[0]);
				Data4[1] = static_cast<u32>(sysGUID.Data4[1]);
				Data4[2] = static_cast<u32>(sysGUID.Data4[2]);
				Data4[3] = static_cast<u32>(sysGUID.Data4[3]);
				Data4[4] = static_cast<u32>(sysGUID.Data4[4]);
				Data4[5] = static_cast<u32>(sysGUID.Data4[5]);
				Data4[6] = static_cast<u32>(sysGUID.Data4[6]);
				Data4[7] = static_cast<u32>(sysGUID.Data4[7]);
				drx_sprintf(output.begin(), outputSize, "%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x", Data1, Data2, Data3, Data4[0], Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write system GUID to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk SysGUIDToString(const SysGUID& input, stack_string& output)
		{
			char temp[s_guidStringBufferSize] = "";
			SysGUIDToString(input, temp);
			output = temp;
			return output.c_str();
		}

		// Write system GUID to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk SysGUIDToString(const SysGUID& input, string& output)
		{
			char temp[s_guidStringBufferSize] = "";
			SysGUIDToString(input, temp);
			output = temp;
			return output.c_str();
		}
	}

	// GUID structure.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	struct SGUID
	{
		inline SGUID()
		{
			drxGUID.hipart = 0;
			drxGUID.lopart = 0;
		}

		inline SGUID(const DrxGUID& rhs)
			: drxGUID(rhs)
		{}

		inline SGUID(const SysGUID& rhs)
			: sysGUID(rhs)
		{}

		/*explicit*/ inline SGUID(tukk szString)
		{
			sysGUID = StringUtils::SysGUIDFromString(szString);
		}

		inline SGUID(const SGUID& rhs)
			: drxGUID(rhs.drxGUID)
		{}

		inline operator bool () const
		{
			return (drxGUID.hipart != 0) || (drxGUID.lopart != 0);
		}

		inline bool operator == (const SGUID& rhs) const
		{
			return (drxGUID.hipart == rhs.drxGUID.hipart) && (drxGUID.lopart == rhs.drxGUID.lopart);
		}

		inline bool operator != (const SGUID& rhs) const
		{
			return (drxGUID.hipart != rhs.drxGUID.hipart) || (drxGUID.lopart != rhs.drxGUID.lopart);
		}

		inline bool operator < (const SGUID& rhs) const
		{
			return (drxGUID.hipart < rhs.drxGUID.hipart) || ((drxGUID.hipart == rhs.drxGUID.hipart) && (drxGUID.lopart < rhs.drxGUID.lopart));
		}

		inline bool operator > (const SGUID& rhs) const
		{
			return (drxGUID.hipart > rhs.drxGUID.hipart) || ((drxGUID.hipart == rhs.drxGUID.hipart) && (drxGUID.lopart > rhs.drxGUID.lopart));
		}

		inline bool operator <= (const SGUID& rhs) const
		{
			return (drxGUID.hipart < rhs.drxGUID.hipart) || ((drxGUID.hipart == rhs.drxGUID.hipart) && (drxGUID.lopart <= rhs.drxGUID.lopart));
		}

		inline bool operator >= (const SGUID& rhs) const
		{
			return (drxGUID.hipart > rhs.drxGUID.hipart) || ((drxGUID.hipart == rhs.drxGUID.hipart) && (drxGUID.lopart >= rhs.drxGUID.lopart));
		}

		union
		{
			DrxGUID drxGUID;
			SysGUID sysGUID;
		};
	};

	inline bool Serialize(Serialization::IArchive& archive, SGUID& value, tukk szName, tukk szLabel)
	{
		string temp;
		if(archive.isInput())
		{
			archive(temp, szName, szLabel);
			if(!temp.empty())
			{
				value.sysGUID = sxema2::StringUtils::SysGUIDFromString(temp);
			}
			else
			{
				value = SGUID();
			}
		}
		else if(archive.isOutput())
		{
			if(value)
			{
				sxema2::StringUtils::SysGUIDToString(value.sysGUID, temp);
			}
			archive(temp, szName, szLabel);
		}
		return true;
	}

	namespace GUID
	{
		inline SGUID Empty()
		{
			return SGUID();
		}

		inline bool IsEmpty(const SGUID& guid)
		{
			return (guid == Empty());
		}
	} //endns GUID
}

// Enable use of SGUID structure as key in std::unordered containers.
namespace std
{
	template <> struct hash<sxema2::SGUID>
	{
		inline size_t operator () (const sxema2::SGUID& rhs) const
		{
			return static_cast<size_t>(rhs.drxGUID.hipart ^ rhs.drxGUID.lopart);
		}
	};
}
