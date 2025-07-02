// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <cstdio>
#include <vector>
#include <iterator>
#include <spirv.hpp>
#include <spirv_cross.hpp>

// Strips out Google's new SPIR-V reflection codes which are not supported by current GPU Drivers yet.
namespace VkSpvHelper
{
	static const uint32_t SpirvHeaderLength = 5;
	
	static tukk SpvExtGoogleDecorateString     = "SPV_GOOGLE_decorate_string";
	static tukk SpvExtGoogleHLSLFunctionality1 = "SPV_GOOGLE_hlsl_functionality1";

	struct SpirvBaseInstruction
	{
		i16 opcode;
		i16 length;
	};

	struct SpirvDectoratorInstruction : SpirvBaseInstruction
	{
		u32 target;
		u32 type;

		u32 GetDecorationId(i32 index) const { return (&type + 1)[index]; }
	};

	struct SpirvExtensionInstruction : SpirvBaseInstruction
	{
		tukk GetName() const { return name; } 

	private:
		char   name[1];
	};

	static constexpr i32   SpvDecoratorInstructionMinLength = sizeof(SpirvDectoratorInstruction) / sizeof(u32);
	static constexpr i32   SpvExtensionInstructionMinLength = sizeof(SpirvExtensionInstruction)  / sizeof(u32) + 1; // Requires at least one character for the extension string

	struct SpirvInstructionReader : SpirvBaseInstruction
	{
		SpirvInstructionReader(const uint32_t * pData, size_t dataSize)
			: m_pData(pData)
			, m_dataSize(dataSize)
			, m_bReadError(false)
			, m_currentOffset(0)
		{
		}
		
		bool good()
		{
			return !m_bReadError && m_currentOffset < m_dataSize && m_pData != nullptr;
		}

		const SpirvBaseInstruction* ReadBaseInstruction() const
		{
			m_bReadError = false;
			return reinterpret_cast<const SpirvBaseInstruction*>(m_pData + m_currentOffset);
		}
		
		const SpirvExtensionInstruction* ReadExtension() const
		{
			auto pInstruction = reinterpret_cast<const SpirvExtensionInstruction*>(ReadBaseInstruction());
			m_bReadError = (m_currentOffset + (SpvExtensionInstructionMinLength - 1)) >= m_dataSize;
			return pInstruction->opcode == spv::Op::OpExtension ? pInstruction : nullptr;
		}

		const SpirvDectoratorInstruction* ReadDecoration() const
		{
			auto pInstruction = reinterpret_cast<const SpirvDectoratorInstruction*>(ReadBaseInstruction());
			m_bReadError = m_currentOffset + SpvDecoratorInstructionMinLength - 1 >= m_dataSize;
			return pInstruction->opcode == spv::Op::OpDecorateId ? pInstruction : nullptr;
		}

		bool advance()
		{
			m_currentOffset += ReadBaseInstruction()->length;
			return good();
		}

		std::pair<u32k*, u32k*> GetInstructionRawData() const
		{
			auto result = std::make_pair(m_pData + m_currentOffset, m_pData + m_currentOffset + ReadBaseInstruction()->length);
			DRX_ASSERT(result.second <= m_pData + m_dataSize);
			return result;
		}

		u32k* m_pData;
		size_t        m_dataSize;
		size_t        m_currentOffset;
		mutable bool  m_bReadError;
	};

	static bool StripGoogleExtensionsFromShader(std::vector<u32>& spirvBinCode) {

		uint32_t spirvBinCodeLen = static_cast<uint32_t>(spirvBinCode.size());
		uint32_t* pSpirvBinData = spirvBinCode.data();

		i32 minGoogleExtStrSize = std::min(sizeof(SpvExtGoogleDecorateString), sizeof(SpvExtGoogleHLSLFunctionality1));

		// Make sure we at least have a header and the magic number is correct
		if (spirvBinCode.size() < SpirvHeaderLength || spirvBinCode[0] != spv::MagicNumber)
			return false;
		std::vector<uint32_t> StrippedOutSpv;
		StrippedOutSpv.reserve(spirvBinCode.size());

		// Copy the header
		StrippedOutSpv.insert(StrippedOutSpv.end(), spirvBinCode.begin(), spirvBinCode.begin() + SpirvHeaderLength);

		SpirvInstructionReader instructionReader(pSpirvBinData + SpirvHeaderLength, spirvBinCodeLen - SpirvHeaderLength);
		do
		{
			if (auto extensionInstruction = instructionReader.ReadExtension())
			{
				if (!instructionReader.good())
					return false;

				if (std::strcmp(extensionInstruction->GetName(), SpvExtGoogleDecorateString) == 0 ||
					std::strcmp(extensionInstruction->GetName(), SpvExtGoogleHLSLFunctionality1) == 0)
				{
					continue;
				}
			}
			else if (auto decorationInstruction = instructionReader.ReadDecoration())
			{
				if (!instructionReader.good())
					return false;

				if (decorationInstruction->type == spv::DecorationHlslCounterBufferGOOGLE)
				{
					continue;
				}
			}
			else
			{
				auto baseInstruction = instructionReader.ReadBaseInstruction();

				if (baseInstruction->opcode == spv::OpDecorateStringGOOGLE ||
					baseInstruction->opcode == spv::OpMemberDecorateStringGOOGLE) 
				{
					continue;
				}	
			}

			auto instructionRawData = instructionReader.GetInstructionRawData();
			StrippedOutSpv.insert(StrippedOutSpv.end(), instructionRawData.first, instructionRawData.second);

			if (!instructionReader.good())
				return false;

		} while (instructionReader.advance());

		spirvBinCode = StrippedOutSpv;

		return true;
	}
}