// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

namespace detail
{

template<typename TValue, typename TByteType, typename TAllocator, const uint numHistograms>
void RadixSortTpl(u32* pRanks, const TValue* pValues, uint count, TAllocator& allocator)
{
	typedef typename TAllocator::template Array<u32, uint> Array;
	const uint histogramSize = 1 << (sizeof(TByteType) * 8);
	Array histogram(allocator, histogramSize * numHistograms);
	memset(histogram.data(), 0, sizeof(u32) * histogramSize * numHistograms);
	u32* h[numHistograms];
	for (uint i = 0; i < numHistograms; ++i)
		h[i] = &histogram[histogramSize * i];

	const TByteType* pBytes = reinterpret_cast<const TByteType*>(pValues);
	const TByteType* pEnd = reinterpret_cast<const TByteType*>(pValues + count);

	while (pBytes != pEnd)
	{
		for (uint i = 0; i < numHistograms; ++i)
			h[i][*pBytes++]++;
	}

	u32* pLink[histogramSize];
	Array rankTmp(allocator, count);
	memset(rankTmp.data(), 0, sizeof(u32) * count);
	memset(pRanks, 0, sizeof(u32) * count);
	u32* pRanks1 = pRanks;
	u32* pRanks2 = rankTmp.data();

	for (uint j = 0; j < numHistograms; ++j)
	{
		u32* curCount = &histogram[histogramSize * j];

		pLink[0] = pRanks2;
		for (uint i = 1; i < histogramSize; ++i)
			pLink[i] = pLink[i - 1] + curCount[i - 1];

		const TByteType* pInputBytes = reinterpret_cast<const TByteType*>(pValues);
		pInputBytes += j;
		if (j == 0)
		{
			for (uint i = 0; i < count; ++i)
				*pLink[pInputBytes[i * numHistograms]]++ = i;
		}
		else
		{
			u32* pIndices = pRanks1;
			u32* pIndicesEnd = pRanks1 + count;
			while (pIndices != pIndicesEnd)
			{
				u32 id = *pIndices++;
				*pLink[pInputBytes[id * numHistograms]]++ = id;
			}
		}

		std::swap(pRanks1, pRanks2);
	}
}

}

template<typename TValue, typename TAllocator>
void RadixSort(u32* pRanksBegin, u32* pRanksEnd, const TValue* pValuesBegin, const TValue* pValuesEnd, TAllocator& allocator)
{
	static_assert(
	  sizeof(TValue) == 4 || sizeof(TValue) == 8,
	  "Unsupported TValue type for RadixSort");
	const uint count = uint(pValuesEnd - pValuesBegin);
	DRX_ASSERT_MESSAGE(
	  uint(pRanksEnd - pRanksBegin) >= count,
	  "Ranks array needs to be large enough to hold the ranks for the sorted values array.");

	if (sizeof(TValue) == 4)
	{
		detail::RadixSortTpl<u32, u8, TAllocator, 4>(
		  pRanksBegin, reinterpret_cast<u32k*>(pValuesBegin), count, allocator);
	}
	else if (sizeof(TValue) == 8)
	{
		detail::RadixSortTpl<uint64, u8, TAllocator, 8>(
		  pRanksBegin, reinterpret_cast<const uint64*>(pValuesBegin), count, allocator);
	}
}
