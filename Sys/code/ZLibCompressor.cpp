// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/******************************************************************************
** ZLibCompressor.cpp
** 24/06/10
******************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#include <drx/Core/lib/z/zlib.h>
#include <drx3D/Sys/ZLibCompressor.h>
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Plugins/md5/md5.h>

// keep these in sync with the enums in IZLibCompressor.h
static i32k k_stratMap[] = { Z_DEFAULT_STRATEGY, Z_FILTERED, Z_HUFFMAN_ONLY, Z_RLE };
static i32k k_methodMap[] = { Z_DEFLATED };
static i32k k_flushMap[] = { Z_NO_FLUSH, Z_PARTIAL_FLUSH, Z_SYNC_FLUSH, Z_FULL_FLUSH };

struct CZLibDeflateStream : public IZLibDeflateStream
{
protected:
	virtual ~CZLibDeflateStream();

protected:
	z_stream       m_compressStream;
	i32            m_zSize;
	i32            m_zPeak;
	i32            m_level;
	i32            m_windowBits;
	i32            m_memLevel;
	i32            m_zlibFlush;
	i32            m_bytesInput;
	i32            m_bytesOutput;
	EZLibStrategy  m_strategy;
	EZLibMethod    m_method;
	EZDeflateState m_curState;
	bool           m_streamOpened;

	static voidpf ZAlloc(
	  voidpf pInOpaque,
	  uInt inItems,
	  uInt inSize);
	static void ZFree(
	  voidpf pInOpaque,
	  voidpf pInAddress);

	EZDeflateState RunDeflate();

public:
	CZLibDeflateStream(
	  i32 inLevel,
	  EZLibMethod inMethod,
	  i32 inWindowBits,
	  i32 inMemLevel,
	  EZLibStrategy inStrategy,
	  EZLibFlush inFlushMethod);

	virtual void SetOutputBuffer(
	  tuk pInBuffer,
	  i32 inSize);
	virtual i32  GetBytesOutput();

	virtual void Input(
	  tukk pInSource,
	  i32 inSourceSize);
	virtual void           EndInput();

	virtual EZDeflateState GetState();

	virtual void           GetStats(
	  SStats* pOutStats);

	virtual void Release();
};

inline static i32 Lookup(i32 inIndex, i32k* pInValues, i32 inMaxValues)
{
	DRX_ASSERT_MESSAGE(inIndex >= 0 && inIndex < inMaxValues, "CZLibDeflateStream mapping invalid");
	return pInValues[inIndex];
}

IZLibDeflateStream* CZLibCompressor::CreateDeflateStream(i32 inLevel, EZLibMethod inMethod, i32 inWindowBits, i32 inMemLevel, EZLibStrategy inStrategy, EZLibFlush inFlushMethod)
{
	return new CZLibDeflateStream(inLevel, inMethod, inWindowBits, inMemLevel, inStrategy, inFlushMethod);
}

void CZLibCompressor::Release()
{
	delete this;
}

void CZLibCompressor::MD5Init(SMD5Context* pIOCtx)
{
	static_assert(sizeof(*pIOCtx) == sizeof(SMD5Context), "Invalid type size!");

	::MD5Init(pIOCtx);
}

void CZLibCompressor::MD5Update(SMD5Context* pIOCtx, u8k* pInBuff, u32 len)
{
	::MD5Update(pIOCtx, pInBuff, len);
}

void CZLibCompressor::MD5Final(SMD5Context* pIOCtx, u8 outDigest[16])
{
	::MD5Final(outDigest, pIOCtx);
}

CZLibCompressor::~CZLibCompressor()
{
}

CZLibDeflateStream::CZLibDeflateStream(
  i32 inLevel,
  EZLibMethod inMethod,
  i32 inWindowBits,
  i32 inMemLevel,
  EZLibStrategy inStrategy,
  EZLibFlush inFlushMethod) :
	m_zSize(0),
	m_zPeak(0),
	m_level(inLevel),
	m_windowBits(inWindowBits),
	m_memLevel(inMemLevel),
	m_bytesInput(0),
	m_bytesOutput(0),
	m_strategy(inStrategy),
	m_method(inMethod),
	m_curState(eZDefState_AwaitingInput),
	m_streamOpened(false)
{
	memset(&m_compressStream, 0, sizeof(m_compressStream));
	m_zlibFlush = Lookup(inFlushMethod, k_flushMap, DRX_ARRAY_COUNT(k_flushMap));
}

CZLibDeflateStream::~CZLibDeflateStream()
{
}

void CZLibDeflateStream::Release()
{
	if (m_streamOpened)
	{
		i32 err = deflateEnd(&m_compressStream);
		if (err != Z_OK)
		{
			DrxLog("zlib deflateEnd() error %d returned when closing stream", err);
		}
	}
	delete this;
}

void CZLibDeflateStream::SetOutputBuffer(
  tuk pInBuffer,
  i32 inSize)
{
	m_bytesOutput += m_compressStream.total_out;

	m_compressStream.next_out = (byte*)pInBuffer;
	m_compressStream.avail_out = inSize;
	m_compressStream.total_out = 0;
}

void CZLibDeflateStream::GetStats(
  IZLibDeflateStream::SStats* pOutStats)
{
	pOutStats->bytesInput = m_bytesInput;
	pOutStats->bytesOutput = m_bytesOutput + m_compressStream.total_out;
	pOutStats->curMemoryUsed = m_zSize;
	pOutStats->peakMemoryUsed = m_zPeak;
}

i32 CZLibDeflateStream::GetBytesOutput()
{
	return m_compressStream.total_out;
}

void CZLibDeflateStream::Input(
  tukk pInSource,
  i32 inSourceSize)
{
	DRX_ASSERT_MESSAGE(m_curState == eZDefState_AwaitingInput, "CZLibDeflateStream::Input() called when stream is not awaiting input");

	m_compressStream.next_in = (Bytef*)pInSource;
	m_compressStream.avail_in = inSourceSize;
	m_bytesInput += inSourceSize;
}

void CZLibDeflateStream::EndInput()
{
	DRX_ASSERT_MESSAGE(m_curState == eZDefState_AwaitingInput, "CZLibDeflateStream::EndInput() called when stream is not awaiting input");

	m_zlibFlush = Z_FINISH;
}

voidpf CZLibDeflateStream::ZAlloc(
  voidpf pInOpaque,
  uInt inItems,
  uInt inSize)
{
	CZLibDeflateStream* pStr = reinterpret_cast<CZLibDeflateStream*>(pInOpaque);

	i32 size = inItems * inSize;

	i32* ptr = (i32*) malloc(sizeof(i32) + size);
	if (ptr)
	{
		*ptr = inItems * inSize;
		ptr += 1;

		i32 newSize = pStr->m_zSize + size;
		pStr->m_zSize = newSize;
		if (newSize > pStr->m_zPeak)
		{
			pStr->m_zPeak = newSize;
		}
	}
	return ptr;
}

void CZLibDeflateStream::ZFree(
  voidpf pInOpaque,
  voidpf pInAddress)
{
	i32* pPtr = reinterpret_cast<i32*>(pInAddress);
	if (pPtr)
	{
		CZLibDeflateStream* pStr = reinterpret_cast<CZLibDeflateStream*>(pInOpaque);
		pStr->m_zSize -= pPtr[-1];
		free(pPtr - 1);
	}
}

EZDeflateState CZLibDeflateStream::RunDeflate()
{
	bool runDeflate = false;
	bool inputAvailable = (m_compressStream.avail_in > 0) || (m_zlibFlush == Z_FINISH);
	bool outputAvailable = (m_compressStream.avail_out > 0);

	switch (m_curState)
	{
	case eZDefState_AwaitingInput:
	case eZDefState_ConsumeOutput:
		if (inputAvailable && outputAvailable)
		{
			runDeflate = true;
		}
		else if (inputAvailable || !outputAvailable)
		{
			m_curState = eZDefState_ConsumeOutput;
		}
		else
		{
			m_curState = eZDefState_AwaitingInput;
		}
		break;

	case eZDefState_Finished:
		break;

	case eZDefState_Deflating:
		DRX_ASSERT_MESSAGE(0, "Shouldn't be trying to run deflate whilst a deflate is in progress");
		break;

	case eZDefState_Error:
		break;

	default:
		DRX_ASSERT_MESSAGE(0, "unknown state");
		break;
	}

	if (runDeflate)
	{
		if (!m_streamOpened)
		{
			m_streamOpened = true;

			// initialising with deflateInit2 requires that the next_in be initialised
			m_compressStream.zalloc = &CZLibDeflateStream::ZAlloc;
			m_compressStream.zfree = &CZLibDeflateStream::ZFree;
			m_compressStream.opaque = this;

			i32 error = deflateInit2(&m_compressStream, m_level, Lookup(m_method, k_methodMap, DRX_ARRAY_COUNT(k_methodMap)), m_windowBits, m_memLevel, Lookup(m_strategy, k_stratMap, DRX_ARRAY_COUNT(k_stratMap)));
			if (error != Z_OK)
			{
				m_curState = eZDefState_Error;
				DrxLog("zlib deflateInit2() error, err %d", error);
			}
		}

		if (m_curState != eZDefState_Error)
		{
			i32 error = deflate(&m_compressStream, m_zlibFlush);

			if (error == Z_STREAM_END)
			{
				// end of stream has been generated, only produced if we pass Z_FINISH into deflate
				m_curState = eZDefState_Finished;
			}
			else if ((error == Z_OK && m_compressStream.avail_out == 0) || (error == Z_BUF_ERROR && m_compressStream.avail_out == 0))
			{
				// output buffer has been filled
				// data should be available for consumption by caller
				m_curState = eZDefState_ConsumeOutput;
			}
			else if (m_compressStream.avail_in == 0)
			{
				// ran out of input data
				// data may be available for consumption - but we need more input right now
				m_curState = eZDefState_AwaitingInput;
			}
			else
			{
				// some sort of error has occurred
				m_curState = eZDefState_Error;
				DrxLog("zlib deflate() error, err %d", error);
			}
		}
	}

	return m_curState;
}

EZDeflateState CZLibDeflateStream::GetState()
{
	return RunDeflate();
}
