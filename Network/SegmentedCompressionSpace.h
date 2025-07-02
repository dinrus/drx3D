// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SEGMENTEDCOMPRESSIONSPACE_H__
#define __SEGMENTEDCOMPRESSIONSPACE_H__

#pragma once

#include <drx3D/Network/Config.h>

#if USE_MEMENTO_PREDICTORS

	#include <drx3D/Network/CommStream.h>

class CSegmentedCompressionSpace
{
public:
	CSegmentedCompressionSpace();
	bool Load(tukk filename);

	void Encode(CCommOutputStream& stm, i32k* pValue, i32 dim) const;
	bool CanEncode(i32k* pValue, i32 dim) const;
	void Decode(CCommInputStream& stm, i32* pValue, i32 dim) const;
	i32  GetBitCount() const { return m_bits; }

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CSegmentedCompressionSpace");

		if (countingThis)
			pSizer->Add(*this);
		pSizer->AddContainer(m_data);
	}

private:
	static i32k    MAX_DIMENSION = 4;

	i32                 m_steps;
	i32                 m_dimensions;
	i32               m_center[MAX_DIMENSION];
	i32                 m_outerSizeSteps;
	i32               m_outerSizeSpace;
	i32                 m_chunkSize;
	i32                 m_bits;
	std::vector<u32> m_data;

	ILINE u32 GetTot(u32 chunk) const
	{
		return m_data[(chunk + 1) * m_chunkSize - 3] + m_data[(chunk + 1) * m_chunkSize - 2];
	}
	ILINE u32 GetLow(u32 chunk, u32 ofs) const
	{
		return m_data[chunk * m_chunkSize + ofs * 3 + 0];
	}
	ILINE u32 GetSym(u32 chunk, u32 ofs) const
	{
		return m_data[chunk * m_chunkSize + ofs * 3 + 1];
	}
	ILINE u32 GetChild(u32 chunk, u32 ofs) const
	{
		return m_data[chunk * m_chunkSize + ofs * 3 + 2];
	}
};

#endif

#endif
