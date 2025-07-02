// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "WaveFileReader.h"
#include "WaveFile.h"
#include <DrxSystem/IStreamEngine.h>

//////////////////////////////////////////////////////////////////////////
CWaveFileReader::CWaveFileReader()
{
	//REINST("is this class still needed?");
	m_bLoaded = false;
}

//////////////////////////////////////////////////////////////////////////
CWaveFileReader::~CWaveFileReader()
{
	DestroyData();
}

bool CWaveFileReader::LoadFile(tukk sFileName)
{
	DestroyData();
	SetLoaded(false);

	CDrxFile file;
	if (!file.Open(sFileName, "rb "))
		return false;

	i32 filesize = file.GetLength();
	tuk buf = new char[filesize + 16];

	file.ReadRaw(buf, filesize);

	uk pSample = LoadAsSample(buf, filesize);
	if (!pSample)
	{
		delete[]buf;
		return false;
	}

	SetLoaded(true);

	return true;
}

//////////////////////////////////////////////////////////////////////////
uk CWaveFileReader::LoadAsSample(tukk AssetDataPtr, i32 nLength)
{
	ATG::WaveFileMemory MemWaveFile;
	ATG::WAVEFORMATEXTENSIBLE wfx;
	DWORD dwNewSize;
	//uk pSoundData = NULL;

	// Initialize the WaveFile Class and read all the chunks
	MemWaveFile.Init(AssetDataPtr, nLength);
	MemWaveFile.GetFormat(&wfx);

	// Create new memory for the sound data
	MemWaveFile.GetDuration(&dwNewSize);

	if (!m_MemBlock.Allocate(nLength))
		return NULL;

	// read the sound data from the file
	MemWaveFile.ReadSample(0, m_MemBlock.GetBuffer(), dwNewSize, &dwNewSize);

	// Pass some information to the SoundBuffer
	//m_pSoundbufferInfo.nBaseFreq = wfx.Format.nSamplesPerSec;
	//m_pSoundbufferInfo.nBitsPerSample = wfx.Format.wBitsPerSample;
	//m_pSoundbufferInfo.nChannels = wfx.Format.nChannels;
	//m_pSoundbufferInfo.nLengthInBytes = dwNewSize;

	//// Check for embedded loop points
	//DWORD dwLoopStart  = 0;
	//DWORD dwLoopLength = dwNewSize;
	//MemWaveFile.GetLoopRegionBytes( &dwLoopStart, &dwLoopLength );
	//// TODO enable to read the loop points from the file and store is somewhere

	//if (dwNewSize > 0)
	//{
	//	float fBaseFreqMs = m_pSoundbufferInfo.nBaseFreq/1000.0f;
	//	m_pSoundbufferInfo.nLengthInMs = (8*m_pSoundbufferInfo.nLengthInBytes)/(fBaseFreqMs*m_pSoundbufferInfo.nChannels*m_pSoundbufferInfo.nBitsPerSample);
	//	m_pSoundbufferInfo.nLengthInSamples = (8*m_pSoundbufferInfo.nLengthInBytes)/(m_pSoundbufferInfo.nChannels*m_pSoundbufferInfo.nBitsPerSample);
	//}

	return (m_MemBlock.GetBuffer());
}

//void SwapTyp( u16 &x )
//{
//	x = (x<<8) | (x>>8);
//}

i32 CWaveFileReader::GetSample(u32 nPos)
{
	i32 nData = 0;
	/*if (m_pSoundbufferInfo.nLengthInSamples > nPos)
	   {

	   u32 nOffset = nPos*m_pSoundbufferInfo.nChannels;

	   switch(m_pSoundbufferInfo.nBitsPerSample)
	   {
	    case 8:
	      {
	        u8 *pPointer = (u8*)m_MemBlock.GetBuffer();
	        u8 nData8 = pPointer[nOffset];
	        nData = (int8)nData8;
	        break;
	      }
	    case 16:
	      {
	        u16 *pPointer = (u16*)m_MemBlock.GetBuffer();
	        u16 nData16 = pPointer[nOffset];

	        nData = (i16)nData16;
	        break;
	      }
	    default:
	      assert(0);
	      break;
	   }

	   }*/
	return nData;
}

//////////////////////////////////////////////////////////////////////////
f32 CWaveFileReader::GetSampleNormalized(u32 nPos)
{

	i32 nData = GetSample(nPos);
	f32 fData = 0;

	/*switch(m_pSoundbufferInfo.nBitsPerSample)
	   {
	   case 8:
	   {
	    fData = nData/256.0f;
	    break;
	   }
	   case 16:
	   {
	    fData = nData/(float)(1<<15);
	    break;
	   }
	   default:
	   assert(0);
	   break;
	   }*/

	return fData;
}

//////////////////////////////////////////////////////////////////////////
void CWaveFileReader::GetSamplesMinMax(i32 nPos, i32 nSamplesCount, float& vmin, float& vmax)
{
	/*if (nPos < 0)
	   {
	   nSamplesCount -= nPos;
	   nPos = 0;
	   }
	   if (nPos >= m_pSoundbufferInfo.nLengthInSamples || nSamplesCount < 0)
	   {
	   vmin = 0;
	   vmax = 0;
	   return;
	   }
	   u32 nEnd = nPos+nSamplesCount;
	   if (nEnd > m_pSoundbufferInfo.nLengthInSamples)
	   {
	   nEnd = m_pSoundbufferInfo.nLengthInSamples;
	   }

	   float fMinValue = 0.0f;
	   float fMaxValue = 0.0f;

	   for (u32 i = nPos; i < nEnd; ++i)
	   {
	   float fValue = GetSampleNormalized(i);
	   fMinValue += min(fMinValue, fValue);
	   fMaxValue += max(fMaxValue, fValue);
	   fMinValue = fMinValue*0.5f;
	   fMaxValue = fMaxValue*0.5f;
	   }
	   vmin = fMinValue;
	   vmax = fMaxValue;*/
}

//bool CWaveFileReader::GetSoundBufferInfo(SSoundBufferInfo* pSoundInfo)
//{
//	if (pSoundInfo)
//	{
//		m_pSoundbufferInfo.nBaseFreq = pSoundInfo->nBaseFreq;
//		m_pSoundbufferInfo.nChannels = pSoundInfo->nChannels;
//		m_pSoundbufferInfo.nLengthInMs = pSoundInfo->nLengthInMs;
//		m_pSoundbufferInfo.nTimesUsed = pSoundInfo->nTimesUsed;
//		m_pSoundbufferInfo.nTimedOut = pSoundInfo->nTimedOut;
//		m_pSoundbufferInfo.nLengthInSamples = pSoundInfo->nLengthInSamples;
//		m_pSoundbufferInfo.nLengthInBytes = pSoundInfo->nLengthInBytes;
//
//		return true;
//	}
//
//	return false;
//}
//////////////////////////////////////////////////////////////////////////
void CWaveFileReader::DestroyData()
{
	m_MemBlock.Free();

	/*m_pSoundbufferInfo.nBaseFreq = 0;
	   m_pSoundbufferInfo.nBitsPerSample = 0;
	   m_pSoundbufferInfo.nChannels = 0;
	   m_pSoundbufferInfo.nTimesUsed = 0;
	   m_pSoundbufferInfo.nTimedOut = 0;
	   m_pSoundbufferInfo.nLengthInBytes = 0;
	   m_pSoundbufferInfo.nLengthInMs = 0;
	   m_pSoundbufferInfo.nLengthInSamples = 0;*/

	m_nVolume = 0;
	m_bLoaded = 0;
}

