// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#include  <drx3D/Network/ErrorDistributionEncoding.h>
#include  <drx3D/Network/ArithStream.h>

#include <drx3D/CoreX/Math/Random.h>
#include  <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Sys/ArchiveHost.h>

//#define DISABLE_BIT_BUCKETS

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CErrorDistributionTest
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CErrorDistributionTest test("ori1", "A:\\distributions\\", 2, 10000, 1000000);

CErrorDistributionTest::CErrorDistributionTest(const string& name, const string& directory, u32 channel, u32 symbolNum, u32 testSize)
{
	m_outStream = nullptr;
	m_inStream = nullptr;

	m_distribution.Init(name.c_str(), channel, 10, 2, directory.c_str(), eDistributionType_Write);

	InitValues(testSize);
	Write();
	TestRead();
}

void CErrorDistributionTest::InitValues(u32 count)
{
	m_testValues.resize(count);

	bool bLastZero = false;
	for (auto& v : m_testValues)
	{
		v = m_distribution.GenerateRandom(bLastZero);

/*		if (drx_random<i32>(0, 1000) == 0)
			v = 0x0fffffff; //test out of bit range values*/

		bLastZero = (v == 0);
	}
}

void CErrorDistributionTest::TestRead()
{
#if USE_ARITHSTREAM
	m_inStream = new CCommInputStream(&m_outData[0], m_outStream->GetOutputSize());

	u32 errors = 0;

	bool bLastZero = false;
	for (u32 k = 0; k < m_testValues.size(); k++)
	{
		i32 value = 0;

		if (k == 99994)
			i32 i = 0;
		
		if (!m_distribution.ReadValue(value, m_inStream, bLastZero))
			value = (i32)m_inStream->ReadBits(32);

		bLastZero = (value == 0);

		i32 test = m_testValues[k];
		if (value != test)
			errors++;
	}

	char temp[200];
	drx_sprintf(temp, "CErrorDistributionTest::TestRead] Error count: %i / %i\n", errors, (u32)m_testValues.size());
	OutputDebugString(temp);
#endif
}

void CErrorDistributionTest::Write()
{
#if USE_ARITHSTREAM
	m_outData.resize(m_testValues.size() * 20, 0);

	m_outStream = new CCommOutputStream(&m_outData[0], m_outData.size());

	bool bLastZero = false;

#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
	float s = -m_outStream->GetBitSize();
#endif

	for (u32 k = 0; k < m_testValues.size(); k++)
	{
		if (!m_distribution.WriteValue(m_testValues[k], m_outStream, bLastZero))
		{
			m_outStream->WriteBits(m_testValues[k], 32);
		}

		bLastZero = (m_testValues[k] == 0);
	}

	m_outStream->Flush();

#if ENABLE_ACCURATE_BANDWIDTH_PROFILING
	s += m_outStream->GetBitSize();

	char temp[200];
	drx_sprintf(temp, "Data size: %i writen: %3.3f ratio: %3.3f bits per value: %3.3f\n", (i32)(m_testValues.size() * 32), s, s / (float)(m_testValues.size() * 32), s / (float)m_testValues.size());
	OutputDebugString(temp);
#endif
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CErrorDistribution
/////////////////////////////////////////////////////////////////////////////////////////////////////////
CTimeValue CErrorDistribution::m_lastOpEnd;

int64 CErrorDistribution::m_totalTime[CErrorDistribution::eOpType_Num] = { 0 };

CErrorDistribution::CErrorDistribution()
{
	m_lastOpEnd = gEnv->pTimer->GetAsyncTime();

	m_debugSymSize = 0xffffffff;
	m_debugTotalSize = 0xffffffff;

	m_afterZero = 0;
	m_zeroAfterZero = 0;

	m_type = eDistributionType_NotSet;

	m_bProbabilityValid = false;
	m_channel = 0;
	m_totalCount = 0;

	m_bitBucketLast = 0;
	m_bitBucketsTotal = 0;

	m_perBitsCount.clear();
	m_perBitsCount.resize(33, 0);
}

void CErrorDistribution::LogPerformance()
{
	int64 total = 1; //in case of div by 0
	for (auto t : m_totalTime)
		total += t;

	float wt = (float)m_totalTime[eOpType_Write] / (float)total;
	float rt = (float)m_totalTime[eOpType_Read] / (float)total;

	NetLogAlways("Distribution times - w: %f r: %f", wt, rt);

	for (auto& t : m_totalTime)
		t = 0;
}

void CErrorDistribution::OnOpEnd(EOpType type) const
{
	CTimeValue now = gEnv->pTimer->GetAsyncTime();

	int64 diff = now.GetMicroSecondsAsInt64() - m_lastOpEnd.GetMicroSecondsAsInt64();
	m_lastOpEnd = now;

	m_totalTime[type] += diff;
}

void CErrorDistribution::InitProbability() const
{
	m_bProbabilityValid = true;
	m_cumulativeDistribution.resize(m_valueCounter.size());

	u32 sum = 0;
	for (u32 i = 0; i < m_valueCounter.size(); i++)
	{
		m_cumulativeDistribution[i] = sum;
		sum += m_valueCounter[i];
	}

	DRX_ASSERT(m_totalCount == sum);
}

u32 CErrorDistribution::GetNumBits(u32 index) const
{
	if (!index)
		return 0;

	u32 test = IntegerLog2_RoundUp(index + 1);

	return test;
}

void CErrorDistribution::CountBits(u32 index)
{
	m_perBitsCount[GetNumBits(index)]++;
}

void CErrorDistribution::CountError(i32 error)
{
	m_bProbabilityValid = false;

	u32 value = ErrorToIndex(error);
	CountBits(value);
	if (value > m_maxSymbols)
		CountIndex(m_maxSymbols);
	else
		CountIndex(value);
}

bool CErrorDistribution::IsTracked(i32 error) const
{
	return ErrorToIndex(error) < m_maxSymbols;
}

void CErrorDistribution::TestMapping(i32 error) const
{
	DRX_ASSERT(IndexToError(ErrorToIndex(error)) == error);
}

void CErrorDistribution::CountIndex(u32 value)
{
	m_totalCount++;

	if (m_valueCounter.size() <= value)
		m_valueCounter.resize(value + 1, 0);

	m_valueCounter[value]++;
}

u32 CErrorDistribution::ErrorToIndex(i32 error) const
{
	if (error < 0)
		return ((u32)(-error - 1)) * 2 + 1;
	else
		return ((u32)(error)) * 2;
}

i32 CErrorDistribution::IndexToError(u32 index) const
{
	if (index & 1)
		return (-(i32)((index - 1) / 2)) - 1;
	else
		return (i32)(index / 2);
}

i32 CErrorDistribution::GenerateRandom(bool bLastZero) const
{
	//could be faster with binary search in m_cumulativeDistribution

	if (bLastZero)
	{
		u32 zero = drx_random<u32>(0, m_afterZero);
		if (zero < m_zeroAfterZero)
			return 0;
	}

	u32 value = drx_random<u32>(0, m_totalCount); 

	for (u32 index = 0; index < m_valueCounter.size(); index++)
	{
		if (m_valueCounter[index] > value)
			return IndexToError(index);

		value -= m_valueCounter[index];
	}

	return IndexToError(0);
}

bool CErrorDistribution::WriteBitOutOfRange(CCommOutputStream* pStream) const
{
	return WriteBitBucket(m_bitBucketLast, pStream);
}

bool CErrorDistribution::WriteOutOfRange(CCommOutputStream* pStream) const
{
	u32 size = 0;
	u32 base = 0;
	u32 total = 0;

	if (!GetSymbolDesc(m_maxSymbols, size, base, total))
	{
		DRX_ASSERT(false);

		return false;
	}

	pStream->Encode(total, base, size);

	return true;
}

void CErrorDistribution::GetAfterZeroProb(u32& prob, u32& total) const
{
	if (m_afterZero == 0 || m_zeroAfterZero == 0)
	{
		prob = 1;
		total = 2;
	}
	else
	{
		prob = m_zeroAfterZero;
		total = m_afterZero;

		//some distributions have 100% zero after zero probability. This would break in case some would write non zero.
		u32 min = m_afterZero / 500;
		if (min == 0)
			min = 1;

		if (prob > total - min)
			prob = total - min;
	}
}

bool CErrorDistribution::ReadAfterZero(bool& zero, CCommInputStream* pStream) const
{
	u32 prob = 0;
	u32 total = 0;

	GetAfterZeroProb(prob, total);

	u32 readed = pStream->Decode(total);
	if (readed >= prob)
	{//not zero
		pStream->Update(total, prob, total - prob);
		zero = false;
	}
	else
	{//zero
		pStream->Update(total, 0, prob);
		zero = true;
	}

	return true;
}

bool CErrorDistribution::WriteAfterZero(bool bZero, CCommOutputStream* pStream) const
{
	u32 prob = 0;
	u32 total = 0;

	GetAfterZeroProb(prob, total);

	if (bZero)
	{
		pStream->Encode(total, 0, prob);
	}
	else
	{
		pStream->Encode(total, prob, total - prob);
	}

	return true;
}

bool CErrorDistribution::WriteValue(i32 value, CCommOutputStream* pStream, bool lastTimeZero) const
{
	OnOpEnd(eOpType_Idle);
	if (lastTimeZero)
	{
		if (!WriteAfterZero(value == 0, pStream))
		{
			OnOpEnd(eOpType_Write);
			return false;
		}

		if (value == 0)
		{
			OnOpEnd(eOpType_Write);
			return true;
		}
	}

	u32 size = 0;
	u32 base = 0;
	u32 total = 0;

	m_debugSymSize = 0xffffffff;
	m_debugTotalSize = m_totalCount;

	if (!IsTracked(value) || !GetSymbolDesc(ErrorToIndex(value), size, base, total))
	{
		WriteOutOfRange(pStream);

		OnOpEnd(eOpType_Write);

#ifdef DISABLE_BIT_BUCKETS
		return false;
#else
		bool ret = WriteBits(value, pStream);

		OnOpEnd(eOpType_Write);
		return ret;
#endif
	}

	m_debugSymSize = size;
	m_debugTotalSize = total;

	pStream->Encode(total, base, size);

	OnOpEnd(eOpType_Write);
	return true;
}

bool CErrorDistribution::ReadSymbol(u32 symbol, CCommInputStream* pStream) const
{
	u32 size = 0;
	u32 base = 0;
	u32 total = 0;

	if (!GetSymbolDesc(symbol, size, base, total))
		return false;

	pStream->Update(total, base, size);
	return true;
}

void CErrorDistribution::WriteMagic(CCommOutputStream* pStream) const
{
	pStream->WriteBits(876, 10);
}

bool CErrorDistribution::ReadMagic(tukk szTag, CCommInputStream* pStream) const
{
	unsigned magic = pStream->ReadBits(10);

	if (magic != 876)
	{
		DrxLogAlways("read magic: %i name: %s_%i tag: %s\n", magic, m_name.c_str(), m_channel, szTag);
		return false;
	}

	return true;
}

bool CErrorDistribution::ReadValue(i32& value, CCommInputStream* pStream, bool bLastTimeZero) const
{
	OnOpEnd(eOpType_Idle);

	if (bLastTimeZero)
	{
		bool bZero = false;
		if (!ReadAfterZero(bZero, pStream))
		{
			OnOpEnd(eOpType_Read);
			return false;
		}

		if (bZero)
		{
			OnOpEnd(eOpType_Read);
			value = 0;
			return true;
		}
	}

	u32 total = m_totalCount;

	u32 prob = pStream->Decode(total);

	u32 symbol = 0;
	bool bFound = FindSymbol(symbol, prob);

	if (!ReadSymbol(symbol, pStream))
	{
		OnOpEnd(eOpType_Read);
		return false;
	}

	if (!bFound)
	{
#ifdef DISABLE_BIT_BUCKETS
		return false;
#else
		bool ret = ReadBits(value, pStream);

		OnOpEnd(eOpType_Read);
		return ret;
#endif
	}
	else
		value = IndexToError(symbol);

	OnOpEnd(eOpType_Read);
	return true;
}

bool CErrorDistribution::FindSymbol(u32& symbol, u32 prob) const
{
	if (!m_bProbabilityValid)
		InitProbability();

	u32 rangeMin = 0;

	u32 rangeMax = 1;

	while (rangeMax < m_cumulativeDistribution.size() - 1)
	{
		if (m_cumulativeDistribution[rangeMax] > prob)
			break;

		rangeMin = rangeMax;
		rangeMax *= 2;
	}

	if (rangeMax >= m_cumulativeDistribution.size())
		rangeMax = m_cumulativeDistribution.size() - 1;

	while (rangeMin != rangeMax)
	{
		u32 mid = (rangeMin + rangeMax) >> 1;

		if (m_cumulativeDistribution[mid] < prob)
		{
			if (rangeMin == mid)
				break;

			rangeMin = mid;
		}
		else
		{
			if (rangeMax == mid)
				break;

			rangeMax = mid;
		}
	}

	for (i32 r = (i32)rangeMax; r >= (i32)rangeMin && r >= 0; r--)
	{
		if (m_cumulativeDistribution[r] <= prob)
		{
			u32 size = m_valueCounter[r];
			while (size == 0)
			{
				r++;
				size = m_valueCounter[r];
			}

			symbol = r;
			return symbol != m_maxSymbols;
		}
	}

	symbol = m_maxSymbols;
	return false;
}

bool CErrorDistribution::WriteBitBucket(u32 bucket, CCommOutputStream* pStream) const
{
	u32 total = m_bitBucketsTotal;
	u32 base = m_bitBucketsSum[bucket];
	u32 size = m_bitBuckets[bucket];

	bool bRet = true;

	if (size == 0)
	{
		base = m_bitBucketsSum[m_bitBucketLast];
		size = m_bitBuckets[m_bitBucketLast];

		bRet = false;
	}
	else if (bucket == m_bitBucketLast)
		bRet = false;

	pStream->Encode(total, base, size);

	return bRet;
}

bool CErrorDistribution::ReadBitBucket(u32& bucket, CCommInputStream* pStream) const
{
	u32 total = m_bitBucketsTotal;
	u32 prob = pStream->Decode(total);

	i32 found = -1;
	for (i32 b = m_bitBucketLast; b >= 0; b--)
	{
		if (m_bitBucketsSum[b] <= prob)
		{
			found = b;
			break;
		}
	}

	DRX_ASSERT(found != -1);

	if (found == -1)
	{
		return false;
	}

	u32 base = m_bitBucketsSum[found];
	u32 size = m_bitBuckets[found];

	DRX_ASSERT(base + size > prob);
	if (base + size <= prob)
	{
		return false;
	}

	pStream->Update(total, base, size);

	bucket = found;

	return found < m_bitBucketLast;
}

bool CErrorDistribution::WriteBits(i32 value, CCommOutputStream* pStream) const
{
	u32 index = ErrorToIndex(value);
	u32 bits = GetNumBits(index);
	u32 bucket = (bits - 1) / m_bitBucketSize;

	if (!WriteBitBucket(bucket, pStream))
		return false;

	pStream->WriteBits(index, (bucket + 1) * m_bitBucketSize);
	return true;
}

bool CErrorDistribution::ReadBits(i32& value, CCommInputStream* pStream) const
{
	u32 bucket = 0;
	if (!ReadBitBucket(bucket, pStream))
		return false;

	u32 index = pStream->ReadBits((bucket + 1) * m_bitBucketSize);
	value = IndexToError(index);

	return true;
}

bool CErrorDistribution::GetSymbolDesc(u32 symbol, u32& symbolSize, u32& symbolBase, u32& total) const
{
	if (!m_bProbabilityValid)
		InitProbability();

	if (symbol >= m_cumulativeDistribution.size())
		return false;

	symbolSize = m_valueCounter[symbol];
	symbolBase = m_cumulativeDistribution[symbol];
	total = m_totalCount;

	return true;
}

void CErrorDistribution::NotifyAfterZero(i32 error)
{
	m_afterZero++;
	if (error == 0)
		m_zeroAfterZero++;
}

void CErrorDistribution::GetDebug(u32& symSize, u32& totalSize) const
{
	symSize = m_debugSymSize;
	totalSize = m_debugTotalSize;
}

string CErrorDistribution::GetFileName(tukk szDirectory) const
{
	string str;
	bool bServer = gEnv ? gEnv->IsDedicated() : true;

	if (m_type == eDistributionType_Read)
		bServer = !bServer; //when we want to read, we need peer's distribution

	str.Format("%s%s_%i_%s.log", szDirectory, m_name.c_str(), m_channel, bServer ? "S" : "C");

	return str;
}

void CErrorDistribution::Init(tukk szName, u32 channel, u32 symbolBits, u32 bitBucketSize, tukk szDir, EDistributionType type)
{
	m_type = type;
	m_name = szName;
	m_channel = channel;
	m_maxSymbols = 1 << symbolBits;

	m_symbolBits = symbolBits;
	m_bitBucketSize = bitBucketSize;

	string file = GetFileName(szDir);

	LoadJson(file.c_str());

	if (type != eDistributionType_Count)
		PrepareForUse();
}

void CErrorDistribution::SmoothDistribution()
{
	//this is just for quick test, will be changed to better smoothing in future
	std::vector<u32> smoothed(m_valueCounter.size(), 0);

	for (i32 k = 0; k < 10; k++)
		smoothed[k] = m_valueCounter[k] * 3;

	for (i32 k = 10; k < (i32)m_valueCounter.size() - 4; k++)
	{
		i32 sum = m_valueCounter[k] + m_valueCounter[k - 2] + m_valueCounter[k + 2];
		smoothed[k] = sum;
	}

	i32 start = std::max((i32)m_valueCounter.size() - 4, 0);

	for (i32 k = start; k < m_valueCounter.size(); k++)
		smoothed[k] = m_valueCounter[k] * 3;

	m_valueCounter = smoothed;

	m_totalCount = 0;
	for (auto c : m_valueCounter)
		m_totalCount += c;
}

void CErrorDistribution::TrimToFirstZero()
{
	u32 lastCount = 0;
	i32 firstZero = -1;
	for (u32 k = 0; k < m_valueCounter.size(); k++)
	{
		if (firstZero == -1)
		{
			if (m_valueCounter[k] == 0)
				firstZero = k;
		}
		else
			lastCount += m_valueCounter[k];
	}

	if (firstZero == -1)
		return;

	if (firstZero == 0)
	{
		m_valueCounter.resize(2);
		m_valueCounter[0] = 1;
		m_valueCounter[1] = 1;

		m_maxSymbols = 1;
		m_totalCount = 2;
	}
	else
	{
		m_valueCounter.resize(firstZero + 1);
		m_valueCounter[firstZero] = lastCount;
		m_maxSymbols = firstZero;
	}
}

void CErrorDistribution::PrepareBitBuckets()
{
	memset(m_bitBuckets, 0, sizeof(m_bitBuckets));
	memset(m_bitBucketsSum, 0, sizeof(m_bitBucketsSum));
	for (u32 bit = 1; bit < 33; bit++)
	{
		u32 bucket = (bit - 1) / m_bitBucketSize;
		m_bitBuckets[bucket] += m_perBitsCount[bit];
	}

	for (unsigned k = 0; k < m_maxSymbols; k++)
	{
		u32 bits = GetNumBits(k);
		if (bits == 0)
			continue;

		u32 bucket = (bits - 1) / m_bitBucketSize;
		if (m_bitBuckets[bucket] >= m_valueCounter[k])
			m_bitBuckets[bucket] -= m_valueCounter[k];
		else
			m_bitBuckets[bucket] = 0;
	}

	m_bitBucketLast = 0;
	for (i32 k = 0; k < 33; k++)
	{
		if (m_bitBuckets[k] == 0)
			continue;

		m_bitBucketLast = k;
	}

	if (m_bitBucketLast < 32)
		m_bitBucketLast++;

	for (i32 k = 0; k <= m_bitBucketLast; k++)
	{
		if (m_bitBuckets[k] < 10)
			m_bitBuckets[k] = 10;
	}

	m_bitBucketsTotal = 0;
	for (u32 i = 0; i <= m_bitBucketLast; i++)
	{
		m_bitBucketsSum[i] = m_bitBucketsTotal;
		m_bitBucketsTotal += m_bitBuckets[i];
	}
}

void CErrorDistribution::PrepareForUse()
{
	SmoothDistribution();
	SmoothDistribution();

	TrimToFirstZero();
	PrepareBitBuckets();
}

void CErrorDistribution::Clear()
{
	m_bProbabilityValid = false;

	m_valueCounter.clear();

	m_perBitsCount.clear();
	m_perBitsCount.resize(33, 0);

	m_totalCount = 0;
	m_zeroAfterZero = 0;
	m_afterZero = 0;
}

void CErrorDistribution::Serialize(Serialization::IArchive& ar)
{
	if (ar.isInput())
		Clear();

	ar(m_afterZero, "AfterZero");
	ar(m_zeroAfterZero, "ZeroAfterZero");
	ar(m_perBitsCount, "BitCounts");

	m_perBitsCount.resize(33, 0);
	ar(m_valueCounter, "ValueCounts");
}

void CErrorDistribution::AvoidEdgeProbabilities()
{
	if (m_totalCount == 0)
		CountIndex(0);

	if (m_valueCounter.size() <= m_maxSymbols)
		CountIndex(m_maxSymbols);
}

void CErrorDistribution::AccumulateInit(CErrorDistribution& dist, tukk szPath) const
{
	dist.Init(m_name.c_str(), m_channel, m_symbolBits, m_bitBucketSize, szPath, eDistributionType_Count);
}

void CErrorDistribution::Accumulate(CErrorDistribution& dist)
{
	if (dist.m_valueCounter.size() < m_valueCounter.size())
		dist.m_valueCounter.resize(m_valueCounter.size(), 0);

	for (i32 k = 0; k < m_valueCounter.size() && k < dist.m_valueCounter.size(); k++)
		dist.m_valueCounter[k] += m_valueCounter[k];

	for (i32 k = 0; k < 33; k++)
		dist.m_perBitsCount[k] += m_perBitsCount[k];

	dist.m_afterZero += m_afterZero;
	dist.m_zeroAfterZero += m_zeroAfterZero;
	dist.m_totalCount += m_totalCount;

	Clear();
}

bool CErrorDistribution::LoadJson(tukk szPath)
{
	bool bRet = Serialization::LoadJsonFile(*this, szPath);
	if (!bRet)
		Clear();

	m_totalCount = 0;
	for (auto v : m_valueCounter)
		m_totalCount += v;

	AvoidEdgeProbabilities();

	return bRet;
}

void CErrorDistribution::SaveJson(tukk szPath) const
{
	Serialization::SaveJsonFile(GetFileName(szPath).c_str(), *this);
}
