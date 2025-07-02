// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Remove s_sizeTStringBufferSize and size_t functions, we should be using u32 and uint64 instead.
// #SchematycTODO : Remove Vec2 and Vec3 functions?.
// #SchematycTODO : Remove to string and stack_string functions?
// #SchematycTODO : Create append functions for each type?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/CoreX/String/DrxStringUtils.h>
#include <drx3D/Schema2/TemplateUtils_ArrayView.h>

#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/Deprecated/IStringPool.h>

namespace sxema2
{
	namespace StringUtils
	{
		static i32k s_invalidPos = -1;

		static const size_t s_boolStringBufferSize   = 2;
		static const size_t s_int8StringBufferSize   = 8;
		static const size_t s_uint8StringBufferSize  = 8;
		static const size_t s_int16StringBufferSize  = 16;
		static const size_t s_u16StringBufferSize = 16;
		static const size_t s_int32StringBufferSize  = 16;
		static const size_t s_uint32StringBufferSize = 16;
		static const size_t s_int64StringBufferSize  = 32;
		static const size_t s_uint64StringBufferSize = 32;
		static const size_t s_sizeTStringBufferSize  = 32;
		static const size_t s_floatStringBufferSize  = 50;
		static const size_t s_vec2StringBufferSize   = 102;
		static const size_t s_vec3StringBufferSize   = 154;
		static const size_t s_ang3StringBufferSize   = 154;
		static const size_t s_quatStringBufferSize	 = 208;

		// Convert hexadecimal character to u8.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr u8 HexCharToUnisgnedChar(char x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}

		// Convert hexadecimal character to unsigned short.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr unsigned short HexCharToUnisgnedShort(char x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}

		// Convert hexadecimal character to u64.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr u64 HexCharToUnisgnedLong(char x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}

		// Convert hexadecimal string to u8.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr u8 HexStringToUnsignedChar(tukk szInput)
		{
			return (HexCharToUnisgnedChar(szInput[0]) << 4) +
				HexCharToUnisgnedChar(szInput[1]);
		}

		// Convert hexadecimal string to unsigned short.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr unsigned short HexStringToUnsignedShort(tukk szInput)
		{
			return (HexCharToUnisgnedShort(szInput[0]) << 12) +
				(HexCharToUnisgnedShort(szInput[1]) << 8) +
				(HexCharToUnisgnedShort(szInput[2]) << 4) +
				HexCharToUnisgnedShort(szInput[3]);
		}

		// Convert hexadecimal string to u64.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		constexpr u64 HexStringToUnsignedLong(tukk szInput)
		{
			return (HexCharToUnisgnedLong(szInput[0]) << 28) +
				(HexCharToUnisgnedLong(szInput[1]) << 24) +
				(HexCharToUnisgnedLong(szInput[2]) << 20) +
				(HexCharToUnisgnedLong(szInput[3]) << 16) +
				(HexCharToUnisgnedLong(szInput[4]) << 12) +
				(HexCharToUnisgnedLong(szInput[5]) << 8) +
				(HexCharToUnisgnedLong(szInput[6]) << 4) +
				HexCharToUnisgnedLong(szInput[7]);
		}

		// Copy string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline i32 Copy(tukk szInput, const CharArrayView& output, i32 inputLength = s_invalidPos)
		{
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	outputSize = static_cast<i32>(output.size());
				DRX_ASSERT(outputSize > 0);
				if(inputLength < 0)
				{
					inputLength = strlen(szInput);
				}
				if(outputSize > inputLength)
				{
					memcpy(output.begin(), szInput, inputLength);
					output[inputLength] = '\0';
					return inputLength;
				}
			}
			return 0;
		}

		// Append string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline i32 Append(tukk szInput, const CharArrayView& output, i32 outputPos = s_invalidPos, i32 inputLength = s_invalidPos)
		{
			DRX_ASSERT(szInput);
			if(szInput)
			{
				if(outputPos == s_invalidPos)
				{
					outputPos = strlen(output.begin());
				}
				if(inputLength == s_invalidPos)
				{
					inputLength = strlen(szInput);
				}
				i32k	outputSize = static_cast<i32>(output.size());
				DRX_ASSERT(outputSize > outputPos);
				if(outputSize > (outputPos + inputLength))
				{
					memcpy(output.begin() + outputPos, szInput, inputLength);
					output[outputPos + inputLength] = '\0';
					return inputLength;
				}
			}
			return 0;
		}

		// Trim characters from right hand size of string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline i32 TrimRight(const CharArrayView& output, tukk szChars, i32 outputPos = s_invalidPos)
		{
			i32	trimLength = 0;
			DRX_ASSERT(szChars);
			if(szChars)
			{
				if(outputPos == s_invalidPos)
				{
					outputPos = strlen(output.begin());
				}
				while(outputPos > 0)
				{
					-- outputPos;
					bool	bTrim = false;
					for(tukk szCompare = szChars; *szCompare; ++ szCompare)
					{
						if(*szCompare == output[outputPos])
						{
							bTrim = true;
							break;
						}
					}
					if(bTrim)
					{
						output[outputPos] = '\0';
						++ trimLength;
					}
					else
					{
						break;
					}
				}
			}
			return trimLength;
		}
			
		// Write bool to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk BoolToString(bool input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_boolStringBufferSize);
			if(outputSize >= s_boolStringBufferSize)
			{
				output[0]	= input ? '1' : '0';
				output[1]	= '\0';
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write bool to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk BoolToString(bool input, stack_string& output)
		{
			output = input ? "1" : "0";
			return output.c_str();
		}

		// Write bool to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk BoolToString(bool input, string& output)
		{
			output = input ? "1" : "0";
			return output.c_str();
		}

		// Read bool from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline bool BoolFromString(tukk szInput, bool defaultOutput = false)
		{
			bool	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = atoi(szInput) != 0;
			}
			return output;
		}

		// Write int8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int8ToString(int8 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_int8StringBufferSize);
			if(outputSize >= s_int8StringBufferSize)
			{
				itoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write int8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int8ToString(int8 input, stack_string& output)
		{
			char	temp[s_int8StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write int8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int8ToString(int8 input, string& output)
		{
			char	temp[s_int8StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read int8 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline int8 Int8FromString(tukk szInput, int8 defaultOutput = 0)
		{
			int8	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = static_cast<int8>(clamp_tpl<i32>(atoi(szInput), -std::numeric_limits<int8>::max(), std::numeric_limits<int8>::max()));
			}
			return output;
		}

		// Write u8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt8ToString(u8 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_uint8StringBufferSize);
			if(outputSize >= s_uint8StringBufferSize)
			{
				ltoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write u8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt8ToString(u8 input, stack_string& output)
		{
			char	temp[s_uint8StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write u8 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt8ToString(u8 input, string& output)
		{
			char	temp[s_uint8StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read u8 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline u8 UInt8FromString(tukk szInput, u8 defaultOutput = 0)
		{
			u8	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = std::min<u8>(static_cast<u8>(atol(szInput)), std::numeric_limits<u8>::max());
			}
			return output;
		}

		// Write i16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int16ToString(i16 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_int16StringBufferSize);
			if(outputSize >= s_int16StringBufferSize)
			{
				itoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write i16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int16ToString(i16 input, stack_string& output)
		{
			char	temp[s_int16StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write i16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int16ToString(i16 input, string& output)
		{
			char	temp[s_int16StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read i16 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline i16 Int16FromString(tukk szInput, i16 defaultOutput = 0)
		{
			i16	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = static_cast<i16>(clamp_tpl<i32>(atoi(szInput), -std::numeric_limits<i16>::max(), std::numeric_limits<i16>::max()));
			}
			return output;
		}

		// Write u16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt16ToString(u16 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_u16StringBufferSize);
			if(outputSize >= s_u16StringBufferSize)
			{
				ltoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write u16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt16ToString(u16 input, stack_string& output)
		{
			char	temp[s_u16StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write u16 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt16ToString(u16 input, string& output)
		{
			char	temp[s_u16StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read u16 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline u16 UInt16FromString(tukk szInput, u16 defaultOutput = 0)
		{
			u16	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = std::min<u16>(static_cast<u16>(atol(szInput)), std::numeric_limits<u16>::max());
			}
			return output;
		}

		// Write i32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int32ToString(i32 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_int32StringBufferSize);
			if(outputSize >= s_int32StringBufferSize)
			{
				itoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write i32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int32ToString(i32 input, stack_string& output)
		{
			char	temp[s_int32StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write i32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int32ToString(i32 input, string& output)
		{
			char	temp[s_int32StringBufferSize] = "";
			itoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read i32 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline i32 Int32FromString(tukk szInput, i32 defaultOutput = 0)
		{
			i32	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = atoi(szInput);
			}
			return output;
		}

		// Write u32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt32ToString(u32 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_uint32StringBufferSize);
			if(outputSize >= s_uint32StringBufferSize)
			{
				ltoa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write u32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt32ToString(u32 input, stack_string& output)
		{
			char	temp[s_uint32StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write u32 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt32ToString(u32 input, string& output)
		{
			char	temp[s_uint32StringBufferSize] = "";
			ltoa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read u32 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline u32 UInt32FromString(tukk szInput, u32 defaultOutput = 0)
		{
			u32	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				output = atol(szInput);
			}
			return output;
		}

		// Write int64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int64ToString(int64 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_int64StringBufferSize);
			if(outputSize >= s_int64StringBufferSize)
			{
				SXEMA2_I64TOA(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write int64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int64ToString(int64 input, stack_string& output)
		{
			char	temp[s_int64StringBufferSize] = "";
			SXEMA2_I64TOA(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write int64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Int64ToString(int64 input, string& output)
		{
			char	temp[s_int64StringBufferSize] = "";
			SXEMA2_I64TOA(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read int64 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline int64 Int64FromString(tukk szInput, int64 defaultOutput = 0)
		{
			int64	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
#ifdef _MSC_VER
				output = _strtoi64(szInput, 0, 10);
#else
				output = strtoll(szInput, 0, 10);
#endif
			}
			return output;
		}

		// Write uint64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt64ToString(uint64 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_uint64StringBufferSize);
			if(outputSize >= s_uint64StringBufferSize)
			{
				_ui64toa(input, output.begin(), 10);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write uint64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt64ToString(uint64 input, stack_string& output)
		{
			char	temp[s_uint64StringBufferSize] = "";
			_ui64toa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Write uint64 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk UInt64ToString(uint64 input, string& output)
		{
			char	temp[s_uint64StringBufferSize] = "";
			_ui64toa(input, temp, 10);
			output = temp;
			return output.c_str();
		}

		// Read uint64 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline uint64 UInt64FromString(tukk szInput, uint64 defaultOutput = 0)
		{
			uint64	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
#ifdef _MSC_VER
				output = _strtoui64(szInput, 0, 10);
#else
				output = strtoull(szInput, 0, 10);
#endif
			}
			return output;
		}

		// Write size_t to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk SizeTToString(size_t input, const CharArrayView& output)
		{
			return UInt64ToString(static_cast<uint64>(input), output);
		}

		// Write size_t to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk SizeTToString(size_t input, stack_string& output)
		{
			return UInt64ToString(static_cast<uint64>(input), output);
		}

		// Write size_t to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk SizeTToString(size_t input, string& output)
		{
			return UInt64ToString(static_cast<uint64>(input), output);
		}

		// Read size_t from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline size_t SizeTFromString(tukk szInput, size_t defaultOutput = 0)
		{
			return static_cast<size_t>(UInt64FromString(szInput, defaultOutput));
		}

		// Write float to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk FloatToString(float input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_floatStringBufferSize);
			if(outputSize >= s_floatStringBufferSize)
			{
				drx_sprintf(output.begin(), outputSize, "%.8f", input);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write float to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk FloatToString(float input, stack_string& output)
		{
			char temp[s_floatStringBufferSize] = "";
			drx_sprintf(temp, "%.8f", input);
			output = temp;
			return output.c_str();
		}

		// Write float to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk FloatToString(float input, string& output)
		{
			char temp[s_floatStringBufferSize] = "";
			drx_sprintf(temp, "%.8f", input);
			output = temp;
			return output.c_str();
		}

		// Read float from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline float FloatFromString(tukk szInput, float defaultOutput = 0.0f)
		{
			float	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	itemCount = sscanf(szInput, "%f", &output);
				DRX_ASSERT(itemCount == 1);
			}
			return output;
		}

		// Write Vec2 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec2ToString(Vec2 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_vec2StringBufferSize);
			if(outputSize >= s_vec2StringBufferSize)
			{
				drx_sprintf(output.begin(), outputSize, "%.8f, %.8f", input.x, input.y);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write Vec2 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec2ToString(Vec2& input, stack_string& output)
		{
			char temp[s_vec2StringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f", input.x, input.y);
			output = temp;
			return output.c_str();
		}

		// Write Vec2 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec2ToString(Vec2& input, string& output)
		{
			char temp[s_vec2StringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f", input.x, input.y);
			output = temp;
			return output.c_str();
		}

		// Read Vec2 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline Vec2 Vec2FromString(tukk szInput, const Vec2& defaultOutput = ZERO)
		{
			Vec2	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	itemCount = sscanf(szInput, "%f, %f", &output.x, &output.y);
				DRX_ASSERT(itemCount == 2);
			}
			return output;
		}

		// Write Vec3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec3ToString(Vec3 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_vec3StringBufferSize);
			if(outputSize >= s_vec3StringBufferSize)
			{
				drx_sprintf(output.begin(), outputSize, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write Vec3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec3ToString(Vec3& input, stack_string& output)
		{
			char temp[s_vec3StringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			output = temp;
			return output.c_str();
		}

		// Write Vec3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Vec3ToString(Vec3& input, string& output)
		{
			char temp[s_vec3StringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			output = temp;
			return output.c_str();
		}

		// Read Vec3 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline Vec3 Vec3FromString(tukk szInput, const Vec3& defaultOutput)
		{
			Vec3	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	itemCount = sscanf(szInput, "%f, %f, %f", &output.x, &output.y, &output.z);
				DRX_ASSERT(itemCount == 3);
			}
			return output;
		}

		// Write Ang3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Ang3ToString(Ang3 input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_ang3StringBufferSize);
			if(outputSize >= s_ang3StringBufferSize)
			{
				drx_sprintf(output.begin(), outputSize, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write Ang3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Ang3ToString(Ang3& input, stack_string& output)
		{
			char	temp[s_ang3StringBufferSize];
			drx_sprintf(temp, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			output = temp;
			return output.c_str();
		}

		// Write Ang3 to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk Ang3ToString(Ang3& input, string& output)
		{
			char	temp[s_ang3StringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f, %.8f", input.x, input.y, input.z);
			output = temp;
			return output.c_str();
		}

		// Read Ang3 from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline Ang3 Ang3FromString(tukk szInput, const Ang3& defaultOutput)
		{
			Ang3	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	itemCount = sscanf(szInput, "%f, %f, %f", &output.x, &output.y, &output.z);
				DRX_ASSERT(itemCount == 3);
			}
			return output;
		}

		// Write Quat to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk QuatToString(Quat input, const CharArrayView& output)
		{
			i32k	outputSize = static_cast<i32>(output.size());
			DRX_ASSERT(outputSize >= s_quatStringBufferSize);
			if(outputSize >= s_quatStringBufferSize)
			{
				drx_sprintf(output.begin(), outputSize, "%.8f, %.8f, %.8f, %.8f", input.v.x, input.v.y, input.v.z, input.w);
			}
			else if(outputSize > 0)
			{
				output[0] = '\0';
			}
			return output.begin();
		}

		// Write Quat to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk QuatToString(Quat& input, stack_string& output)
		{
			char	temp[s_quatStringBufferSize];
			drx_sprintf(temp, "%.8f, %.8f, %.8f, %.8f", input.v.x, input.v.y, input.v.z, input.w);
			output = temp;
			return output.c_str();
		}

		// Write Quat to string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline tukk QuatToString(Quat& input, string& output)
		{
			char	temp[s_quatStringBufferSize] = "";
			drx_sprintf(temp, "%.8f, %.8f, %.8f, %.8f", input.v.x, input.v.y, input.v.z, input.w);
			output = temp;
			return output.c_str();
		}

		// Read Quat from string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline Quat QuatFromString(tukk szInput, const Quat& defaultOutput)
		{
			Quat	output = defaultOutput;
			DRX_ASSERT(szInput);
			if(szInput)
			{
				i32k	itemCount = sscanf(szInput, "%f, %f, %f, %f", &output.v.x, &output.v.y, &output.v.z, &output.w);
				DRX_ASSERT(itemCount == 4);
			}
			return output;
		}

		// Filter string.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline bool FilterString(string input, string filter)
		{
			// Convert inputs to lower case.
			input.MakeLower();
			filter.MakeLower();
			// Tokenize filter and ensure input contains all words in filter.
			string::size_type	pos = 0;
			string::size_type	filterEnd = -1;
			do
			{
				const string::size_type	filterStart = filterEnd + 1;
				filterEnd	= filter.find(' ', filterStart);
				pos				= input.find(filter.substr(filterStart, filterEnd - filterStart), pos);
				if(pos == string::npos)
				{
					return false;			
				}
				pos	+= filterEnd - filterStart;
			} while(filterEnd != string::npos);
			return true;
		}

		// Separate type name and namespace.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline void SeparateTypeNameAndNamespace(tukk szDeclaration, string& typeName, string& output)
		{
			DRX_ASSERT(szDeclaration);
			if(szDeclaration)
			{
				const size_t	length = strlen(szDeclaration);
				size_t				pos = length - 1;
				size_t				templateDepth = 0;
				do
				{
					if(szDeclaration[pos] == '>')
					{
						++ templateDepth;
					}
					else if(szDeclaration[pos] == '<')
					{
						-- templateDepth;
					}
					else if((templateDepth == 0) && (szDeclaration[pos] == ':') && (szDeclaration[pos - 1] == ':'))
					{
						break;
					}
					-- pos;
				} while(pos > 1);
				if(pos > 1)
				{
					typeName	= string(szDeclaration + pos + 1, length - pos);
					output		= string(szDeclaration, pos - 1);
				}
				else
				{
					typeName	= szDeclaration;
					output		= "";
				}
			}
		}

		// Make project relative file name.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		inline void MakeProjectRelativeFileName(tukk szFileName, tukk szProjectDir, string& output)
		{
			DRX_ASSERT(szFileName && szProjectDir);
			if(szFileName && szProjectDir)
			{
				output = szFileName;
				const string::size_type	projectDirPos = output.find(szProjectDir);
				if(projectDirPos != string::npos)
				{
					output.erase(0, projectDirPos);
					output.TrimLeft("\\/");
				}
			}
		}
	}
}
