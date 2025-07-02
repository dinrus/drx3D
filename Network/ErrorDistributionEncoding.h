// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Network/ArithStream.h>
#include <drx3D/CoreX/Serialization/Forward.h>

enum EDistributionType
{
	eDistributionType_Write,
	eDistributionType_Read,
	eDistributionType_Count,
	eDistributionType_NotSet
};

class CErrorDistribution
{
	//for calculation of CPU performance
	static CTimeValue m_lastOpEnd;

	enum EOpType
	{
		eOpType_Write = 0,
		eOpType_Read,
		eOpType_Idle,
		eOpType_Num
	};

	static int64 m_totalTime[eOpType_Num];

	void OnOpEnd(EOpType type) const;

public:
	CErrorDistribution();

	void Init(tukk szName, u32 channel, u32 symbolBits, u32 bitBucketSize, tukk szDir, EDistributionType type);

	void GetDebug(u32& symSize, u32& totalSize) const;

	void WriteMagic(CCommOutputStream* pStream) const;
	bool ReadMagic(tukk szTag, CCommInputStream* pStream) const;

	void AccumulateInit(CErrorDistribution& dist, tukk szPath) const;
	void Accumulate(CErrorDistribution& dist);

	bool LoadJson(tukk szPath);
	void SaveJson(tukk szPath) const;

	static void LogPerformance();
	void Serialize(Serialization::IArchive& ar);

	void Clear();
	void CountError(i32 error);
	void CountBits(u32 index);
	u32 GetNumBits(u32 index) const;

	bool IsTracked(i32 error) const;

	void InitProbability() const;
	
	string GetFileName(tukk szDirectory) const;

	i32 GenerateRandom(bool bLastZero) const;
	
	void GetAfterZeroProb(u32& prob, u32& total) const;

	bool ReadAfterZero(bool& bZero, CCommInputStream* pStream) const;
	bool WriteAfterZero(bool bZero, CCommOutputStream* pStream) const;

	bool WriteBitOutOfRange(CCommOutputStream* pStream) const;
	bool WriteOutOfRange(CCommOutputStream* pStream) const;

	bool WriteValue(i32 value, CCommOutputStream* pStream, bool bLastTimeZero) const;
	bool ReadValue(i32& value, CCommInputStream* pStream, bool bLastTimeZero) const;

	bool WriteBits(i32 value, CCommOutputStream* pStream) const;
	bool ReadBits(i32& value, CCommInputStream* pStream) const;

	bool WriteBitBucket(u32 bucket, CCommOutputStream* pStream) const;
	bool ReadBitBucket(u32& bucket, CCommInputStream* pStream) const;

	bool ReadSymbol(u32 symbol, CCommInputStream* pStream) const;

	bool FindSymbol(u32& symbol, u32 prob) const;
	bool GetSymbolDesc(u32 symbol, u32& symbolSize, u32& symbolBase, u32& total) const;

	void NotifyAfterZero(i32 error);

	void SmoothDistribution();

	bool IsEmpty() const { return m_totalCount == 0 && m_afterZero == 0; }
	unsigned GetTotalCount() const { return m_totalCount; }

	void PrepareForUse();
	void TrimToFirstZero();
	void PrepareBitBuckets();

	void AvoidEdgeProbabilities();

private:
	void TestMapping(i32 error) const;

	void CountIndex(u32 value);

	u32 ErrorToIndex(i32 error) const;
	i32 IndexToError(u32 index) const;

private:
	EDistributionType m_type;
	u32 m_channel;
	u32 m_maxSymbols;

	u32 m_symbolBits;
	u32 m_bitBucketSize;

	u32 m_afterZero;
	u32 m_zeroAfterZero;

	u32 m_bitBuckets[33];
	u32 m_bitBucketsSum[33];
	u32 m_bitBucketsTotal;
	u32 m_bitBucketLast;

	std::vector<u32> m_perBitsCount;

	mutable bool m_bProbabilityValid;
	mutable std::vector<u32> m_cumulativeDistribution;

	u32 m_totalCount;

	mutable u32 m_debugSymSize;
	mutable u32 m_debugTotalSize;

	STxt m_name;

	static const uint64 sm_maxProbability = CCommOutputStream::MaxProbabilityValue;

	std::vector<u32> m_valueCounter;
};

class CCommOutputStream;
class CCommInputStream;

class CErrorDistributionTest
{
	CErrorDistribution m_distribution;

	std::vector<i32> m_testValues;

	CCommOutputStream* m_outStream;
	CCommInputStream* m_inStream;

	std::vector<u8> m_outData;

public:
	CErrorDistributionTest(const string& name, const string& directory, u32 channel, u32 symbolNum, u32 testSize);

	void InitValues(u32 count);
	void TestRead();
	void Write();
};