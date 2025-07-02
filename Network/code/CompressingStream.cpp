// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include <drx3D/Sys/IDrxPak.h>
#include  <drx3D/Network/CompressingStream.h>
#include  <drx3D/Network/ArithStream.h>
#include  <drx3D/Network/Config.h>

#if INCLUDE_DEMO_RECORDING

static const char ID[4] = { 'D', 'E', 'M', '1' };

/*
 * Output
 */

CCompressingOutputStream::CCompressingOutputStream(size_t bufsz)
	: CSimpleOutputStream(bufsz / sizeof(SStreamRecord))
	, m_file(0)
	, m_tempBuffer(0)
	, m_bufsz((bufsz / sizeof(SStreamRecord) + 1) * sizeof(SStreamRecord))
	#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
	, m_throwaway(nullptr)
	#endif
	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_WRITTEN
	, m_nLog(0)
	, m_file_txt(nullptr)
	#endif
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	, m_alphabet(257)
	#endif
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	, m_tempBW(0)
	#endif
{
}

CCompressingOutputStream::~CCompressingOutputStream()
{
	Sync();
	if (m_file)
		gEnv->pDrxPak->FClose(m_file);
	delete[] m_tempBuffer;
	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_WRITTEN
	if (m_file_txt)
		gEnv->pDrxPak->FClose(m_file_txt);
	#endif
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	delete[] m_tempBW;
	#endif
	#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
	delete[] m_throwaway;
	#endif
}

bool CCompressingOutputStream::Init(tukk filename)
{
	m_file = gEnv->pDrxPak->FOpen(filename, "wb");
	if (!m_file)
		return false;

	m_tempBuffer = new char[m_bufsz];

	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	m_tempBW = new tuk[2 * m_bufsz];
	#endif

	// write header
	gEnv->pDrxPak->FWrite(ID, sizeof(ID), 1, m_file);

	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_WRITTEN
	m_nLog = 0;
	m_file_txt = gEnv->pDrxPak->FOpen(filename + string(".w.txt"), "wb");
	assert(m_file_txt);
	#endif
	#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
	m_throwaway = new char[m_bufsz];
	#endif
	return true;
}

void CCompressingOutputStream::Flush(const SStreamRecord* pRecords, size_t numRecords)
{
	DRX_PROFILE_FUNCTION(PROFILE_NETWORK);

	tuk pBuffer = (tuk)pRecords;
	size_t nLength = numRecords * sizeof(SStreamRecord);

	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_WRITTEN
	//{
	//	DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.debug_write");
	//	char debug_fn[512];
	//	drx_sprintf( debug_fn, "write.%.6d.txt", m_nLog++ );
	//	FILE * debug_file = gEnv->pDrxPak->FOpen(debug_fn, "wb");
	//	if (debug_file)
	//	{
	//		gEnv->pDrxPak->FWrite( pBuffer, nLength, 1, debug_file );
	//		gEnv->pDrxPak->FClose( debug_file );
	//	}
	//}
	for (size_t i = 0; i < numRecords; ++i)
		gEnv->pDrxPak->FPrintf(m_file_txt, "%s\t%s\n", pRecords[i].key, pRecords[i].value);
	#endif

	if (m_file)
	{
		tuk input = pBuffer;
		tuk output = m_tempBuffer;

	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
		u8 flags = 0;
		size_t newLength;
		const size_t origLength = nLength;

		size_t nOffset;
		NBurrowsWheeler::Transform(input, input + nLength, output, nOffset, m_tempBW);
		#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.sanity_xform");
			NBurrowsWheeler::Untransform(output, output + nLength, m_throwaway, nOffset, m_tempBW);
			NET_ASSERT(0 == memcmp(input, m_throwaway, nLength));
		}
		#endif
		std::swap(input, output);
		NBurrowsWheeler::MoveToFrontTransform((u8*)input, (u8*)input + nLength, (u8*)output);
		#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.sanity_mtf");
			NBurrowsWheeler::MoveToFrontUntransform((u8*)output, (u8*)output + nLength, (u8*)m_throwaway);
			NET_ASSERT(0 == memcmp(input, m_throwaway, nLength));
		}
		#endif
		std::swap(input, output);

		if (newLength = NBurrowsWheeler::RLE((u8*)input, (u8*)input + nLength, (u8*)output))
		{
		#if COMPRESSING_STREAM_SANITY_CHECK_EVERYTHING
			{
				DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.sanity_rle");
				size_t delength = NBurrowsWheeler::UnRLE((u8*)output, (u8*)output + newLength, (u8*)m_throwaway);
				NET_ASSERT(delength == nLength);
				NET_ASSERT(0 == memcmp(input, m_throwaway, nLength));
			}
		#endif

			flags |= 0x01;
			std::swap(input, output);
			nLength = newLength;
		}

		CArithOutputStream cmp((u8*)output, m_bufsz, flags);

		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.encode");

			for (size_t i = 0; i < nLength; i++)
			{
				u8 symbol = input[i];
				m_alphabet.WriteSymbol(cmp, symbol);
			}
			m_alphabet.WriteSymbol(cmp, 256);
			cmp.WriteInt(nOffset, origLength);
			cmp.Flush();
		}

		{
			DRX_PROFILE_REGION(PROFILE_NETWORK, "CCompressingOutputStream::Flush.write");

			u32 len = cmp.GetOutputSize();
			gEnv->pDrxPak->FWrite(&len, sizeof(len), 1, m_file);
			gEnv->pDrxPak->FWrite(output, 1, cmp.GetOutputSize(), m_file);
		}
	#else // !COMPRESSING_STREAM_USE_BURROWS_WHEELER
		if (GetISystem()->CompressDataBlock(input, nLength, output, nLength))
		{
			gEnv->pDrxPak->FWrite(&nLength, sizeof(nLength), 1, m_file);
			gEnv->pDrxPak->FWrite(output, nLength, 1, m_file);
		}
	#endif
	}
}

/*
 * input
 */

CCompressingInputStream::CCompressingInputStream(size_t bufsz) :
	CSimpleInputStream(bufsz / sizeof(SStreamRecord)),
	m_file(0),
	m_tempBuffer(new char[bufsz]),
	m_nCapacity(bufsz)
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	,
	m_tempBW(new tuk[bufsz]),
	m_alphabet(257)
	#endif
{
	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
	m_nBlock = 0;
	m_file_txt = 0;
	#endif
}

CCompressingInputStream::~CCompressingInputStream()
{
	if (m_file)
		gEnv->pDrxPak->FClose(m_file);
	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
	if (m_file_txt)
		gEnv->pDrxPak->FClose(m_file_txt);
	#endif
	delete[] m_tempBuffer;
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
	delete[] m_tempBW;
	#endif
}

bool CCompressingInputStream::Init(tukk filename)
{
	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
	m_file_txt = gEnv->pDrxPak->FOpen(filename + string(".r.txt"), "wb");
	assert(m_file_txt);
	#endif
	m_file = gEnv->pDrxPak->FOpen(filename, "rb");
	if (m_file)
	{
		char temp[sizeof(ID)];
		gEnv->pDrxPak->FReadRaw(temp, sizeof(temp), 1, m_file);
		if (0 != memcmp(temp, ID, sizeof(temp)))
		{
			gEnv->pDrxPak->FClose(m_file);
			return false;
		}
		return true;
	}
	return false;
}

void CCompressingInputStream::Underflow(SStreamRecord* pStream, size_t& count)
{
	#define DOERROR { count = 0; return; }
	//	char * buffer = (char *)pStream;

	size_t nLength;

	if (m_file)
	{
	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
		tuk input = buffer;
		tuk output = m_tempBuffer;
	#else
		tuk input = m_tempBuffer;
	#endif

		u32 buf;
		if (1 != gEnv->pDrxPak->FReadRaw(&buf, sizeof(buf), 1, m_file))
			DOERROR;
		if (buf > m_nCapacity || buf > count * sizeof(SStreamRecord))
			DOERROR;
		if (1 != gEnv->pDrxPak->FReadRaw(input, buf, 1, m_file))
			DOERROR;

	#if COMPRESSING_STREAM_USE_BURROWS_WHEELER
		CArithInputStream stm((u8*)input, buf);
		nLength = 0;
		while (true)
		{
			unsigned sym = m_alphabet.ReadSymbol(stm);
			if (sym == 256)
				break;
			if (sym > 256)
				DOERROR;
			if (nLength == m_nCapacity)
				DOERROR;
			output[nLength++] = char(sym);
		}

		if (stm.GetBonus() & 0x01)
		{
			std::swap(input, output);
			nLength = NBurrowsWheeler::UnRLE((u8*)input, (u8*)input + nLength, (u8*)output);
		}

		if (nLength % sizeof(SStreamRecord))
			DOERROR;

		u32 offset = stm.ReadInt(nLength);
		if (offset >= nLength)
			DOERROR;
		std::swap(input, output);
		NBurrowsWheeler::MoveToFrontUntransform((u8*)input, (u8*)input + nLength, (u8*)output);
		std::swap(input, output);
		NBurrowsWheeler::Untransform(input, input + nLength, output, offset, m_tempBW);
		if (output != buffer)
		{
			memcpy(buffer, output, nLength);
		}
	#else //COMPRESSING_STREAM_USE_BURROWS_WHEELER
		nLength = count * sizeof(SStreamRecord);
		if (!GetISystem()->DecompressDataBlock(input, buf, pStream, nLength))
			DOERROR;
	#endif

		//#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
		//char debug_fn[512];
		//drx_sprintf( debug_fn, "read.%.6d.txt", m_nBlock++ );
		//FILE * debug_file = gEnv->pDrxPak->FOpen(debug_fn, "wb");
		//if (debug_file)
		//{
		//	gEnv->pDrxPak->FWrite( buffer, nLength, 1, debug_file );
		//	gEnv->pDrxPak->FClose( debug_file );
		//}
		//#endif

		count = nLength / sizeof(SStreamRecord);

	#if COMPRESSING_STREAM_DEBUG_WHAT_GETS_READ
		for (size_t i = 0; i < count; ++i)
			gEnv->pDrxPak->FPrintf(m_file_txt, "%s\t%s\n", pStream[i].key, pStream[i].value);
	#endif

		return;
	}
	else
		DOERROR;
}

#endif
