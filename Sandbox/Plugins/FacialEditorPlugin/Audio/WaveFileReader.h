// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __WAVEFILEREADER_H__
#define __WAVEFILEREADER_H__

#pragma once

#include <DrxSystem/IStreamEngine.h>
#include "Util/MemoryBlock.h"

class CWaveFileReader
{
public:

	CWaveFileReader();
	~CWaveFileReader(void);

	bool  LoadFile(tukk sFileName);
	i32 GetSample(u32 nPos);
	float GetSampleNormalized(u32 nPos);
	void  GetSamplesMinMax(i32 nPos, i32 nSamplesCount, float& vmin, float& vmax);
	//i32	GetLengthMs() const { return m_pSoundbufferInfo.nLengthInMs; };
	bool  IsLoaded()              { return m_bLoaded; };
	void  SetLoaded(bool bLoaded) { m_bLoaded = bLoaded; };

	//u32	GetSampleCount() { return m_pSoundbufferInfo.nLengthInSamples; }
	//u32	GetSamplesPerSec() { return m_pSoundbufferInfo.nBaseFreq; }
	//u32	GetBitDepth() { return m_pSoundbufferInfo.nBitsPerSample; }

	//bool GetSoundBufferInfo(SSoundBufferInfo* pSoundInfo);

protected:
	uk LoadAsSample(tukk AssetDataPtrOrName, i32 nLength);

	// Closes down Stream or frees memory of the Sample
	void DestroyData();

	CMemoryBlock m_MemBlock;
	bool         m_bLoaded;
	//SSoundBufferInfo	m_pSoundbufferInfo;
	u32       m_nVolume;
};

#endif

