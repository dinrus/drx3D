// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   ZlibDecompressor.cpp
//  Created:     30/8/2012 by Axel Gneiting
//  Описание: zlib inflate wrapper
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>

#include <drx/Core/lib/z/zlib.h>
#include <drx3D/Sys/ZLibDecompressor.h>

class CZLibInflateStream : public IZLibInflateStream
{
public:
	CZLibInflateStream()
		: m_bStreamOpened(false)
		, m_zlibFlush(0)
		, m_currentState(eZInfState_AwaitingInput)
		, m_bytesInput(0)
		, m_bytesOutput(0)
		, m_zSize(0)
		, m_zPeak(0) {}

	virtual void           Release();

	virtual void           SetOutputBuffer(tuk pInBuffer, u32 inSize);
	virtual u32   GetBytesOutput();
	virtual void           Input(tukk pInSource, u32 inSourceSize);
	virtual void           EndInput();
	virtual EZInflateState GetState();
	virtual void           GetStats(IZLibInflateStream::SStats* pOutStats);

private:
	virtual ~CZLibInflateStream() {}

	EZInflateState RunInflate();

	static voidpf  ZAlloc(voidpf pInOpaque, uInt inItems, uInt inSize);
	static void    ZFree(voidpf pInOpaque, voidpf pInAddress);

	z_stream       m_decompressStream;
	bool           m_bStreamOpened;
	i32            m_zlibFlush;
	EZInflateState m_currentState;
	u32   m_bytesInput;
	u32   m_bytesOutput;
	u32   m_zSize;
	u32   m_zPeak;
};

IZLibInflateStream* CZLibDecompressor::CreateInflateStream()
{
	return new CZLibInflateStream();
}

void CZLibDecompressor::Release()
{
	delete this;
}

void CZLibInflateStream::Release()
{
	if (m_bStreamOpened)
	{
		i32 err = inflateEnd(&m_decompressStream);
		if (err != Z_OK)
		{
			DrxLog("zlib inflateEnd() error %d returned when closing stream", err);
		}
	}

	delete this;
}

void CZLibInflateStream::SetOutputBuffer(tuk pInBuffer, u32 inSize)
{
	m_bytesOutput += m_decompressStream.total_out;

	m_decompressStream.next_out = (byte*)pInBuffer;
	m_decompressStream.avail_out = inSize;
	m_decompressStream.total_out = 0;
}

void CZLibInflateStream::GetStats(IZLibInflateStream::SStats* pOutStats)
{
	pOutStats->bytesInput = m_bytesInput;
	pOutStats->bytesOutput = m_bytesOutput + m_decompressStream.total_out;
	pOutStats->curMemoryUsed = m_zSize;
	pOutStats->peakMemoryUsed = m_zPeak;
}

u32 CZLibInflateStream::GetBytesOutput()
{
	return m_decompressStream.total_out;
}

void CZLibInflateStream::Input(tukk pInSource, u32 inSourceSize)
{
	DRX_ASSERT_MESSAGE(m_currentState == eZInfState_AwaitingInput, "CZLibInflateStream::Input() called when stream is not awaiting input or has finished");

	m_decompressStream.next_in = (Bytef*)pInSource;
	m_decompressStream.avail_in = inSourceSize;
	m_bytesInput += inSourceSize;
}

void CZLibInflateStream::EndInput()
{
	DRX_ASSERT_MESSAGE(m_currentState == eZInfState_AwaitingInput, "CZLibInflateStream::EndInput() called when stream is not awaiting input");

	m_zlibFlush = Z_FINISH;
}

voidpf CZLibInflateStream::ZAlloc(voidpf pInOpaque, uInt inItems, uInt inSize)
{
	CZLibInflateStream* pStr = reinterpret_cast<CZLibInflateStream*>(pInOpaque);

	u32k size = inItems * inSize;

	i32* pPtr = (i32*)malloc(sizeof(i32) + size);

	if (pPtr)
	{
		*pPtr = inItems * inSize;
		pPtr += 1;

		u32k newSize = pStr->m_zSize + size;
		pStr->m_zSize = newSize;
		if (newSize > pStr->m_zPeak)
		{
			pStr->m_zPeak = newSize;
		}
	}

	return pPtr;
}

void CZLibInflateStream::ZFree(voidpf pInOpaque, voidpf pInAddress)
{
	i32* pPtr = reinterpret_cast<i32*>(pInAddress);

	if (pPtr)
	{
		CZLibInflateStream* pStr = reinterpret_cast<CZLibInflateStream*>(pInOpaque);
		pStr->m_zSize -= pPtr[-1];
		free(pPtr - 1);
	}
}

EZInflateState CZLibInflateStream::RunInflate()
{
	bool runInflate = false;
	bool inputAvailable = (m_decompressStream.avail_in > 0) || (m_zlibFlush == Z_FINISH);
	bool outputAvailable = (m_decompressStream.avail_out > 0);

	switch (m_currentState)
	{
	case eZInfState_AwaitingInput:
	case eZInfState_ConsumeOutput:
		if (inputAvailable && outputAvailable)
		{
			runInflate = true;
		}
		else if (inputAvailable || !outputAvailable)
		{
			m_currentState = eZInfState_ConsumeOutput;
		}
		else
		{
			m_currentState = eZInfState_AwaitingInput;
		}
		break;

	case eZInfState_Inflating:
		DRX_ASSERT_MESSAGE(false, "Shouldn't be trying to run inflate whilst a inflate is in progress");
		break;

	case eZInfState_Error:
		break;

	default:
		DRX_ASSERT_MESSAGE(false, "unknown state");
		break;
	}

	if (runInflate)
	{
		if (!m_bStreamOpened)
		{
			m_bStreamOpened = true;

			// initializing with inflateInit2 requires that the next_in be initialized
			m_decompressStream.zalloc = &CZLibInflateStream::ZAlloc;
			m_decompressStream.zfree = &CZLibInflateStream::ZFree;
			m_decompressStream.opaque = this;

			i32k error = inflateInit2(&m_decompressStream, -MAX_WBITS);
			if (error != Z_OK)
			{
				m_currentState = eZInfState_Error;
				DrxLog("zlib inflateInit2() error, err %d", error);
			}
		}

		if (m_currentState != eZInfState_Error)
		{
			i32 error = inflate(&m_decompressStream, m_zlibFlush);

			if (error == Z_STREAM_END)
			{
				// end of stream has been generated, only produced if we pass Z_FINISH into inflate
				m_currentState = eZInfState_Finished;
			}
			else if ((error == Z_OK && m_decompressStream.avail_out == 0) || (error == Z_BUF_ERROR && m_decompressStream.avail_out == 0))
			{
				// output buffer has been filled
				// data should be available for consumption by caller
				m_currentState = eZInfState_ConsumeOutput;
			}
			else if (m_decompressStream.avail_in == 0)
			{
				// ran out of input data
				// data may be available for consumption - but we need more input right now
				m_currentState = eZInfState_AwaitingInput;
			}
			else
			{
				// some sort of error has occurred
				m_currentState = eZInfState_Error;
				DrxLog("zlib inflate() error, err %d", error);
			}
		}
	}

	return m_currentState;
}

EZInflateState CZLibInflateStream::GetState()
{
	return RunInflate();
}
