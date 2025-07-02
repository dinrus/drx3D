// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  provides utility classes for compressing strings of symbols
               (not necessarily strings of bytes!)
   -------------------------------------------------------------------------
   История:
   - 05/08/2004   10:34 : Created by Craig Tiller
*************************************************************************/

#ifndef __ARITHALPHABET_H__
#define __ARITHALPHABET_H__

#pragma once

#include <drx3D/Network/Config.h>

#if USE_ARITHSTREAM

	#include <drx3D/Network/CommStream.h>
	#include <drx3D/Network/Network.h>
	#include <drx3D/CoreX/Containers/VectorMap.h>

//   This file contains probability-modellers for alphabet encoding.
// An alphabet is any set of symbols that we can define, but we
// must know how many symbols are in the alphabet before we begin.
// Examples:
//  - Bits: {01} form a 2 symbol alphabet
//  - Decimal Numbers: {0123456789} form a 10 symbol alphabet
//  - English Characters: {a-zA-Z} form a 52 symbol alphabet
//  - Bytes: {0x00-0xff} form a 256 symbol alphabet
// These can be fed into an arithmetic encoder, and used to
// compress data based on statistics obtained by earlier sequences,
// and they can be decoded in a similar fashion.
//   A modeller can be characterized by its "Order" - an order-0
// modeller bases all of its statistics based only on the symbol
// that it is currently presented with, whilst an order-1 includes
// the preceding symbol in combination with the current symbol.

// arithmetic "move-to-front" modeller - essentially an order-0
// modeller, but it's much closer to optimal if we have clustered symbols
// (ie: aaaabbbbbbbcbbb)
// uses linear memory, and a linear update time
class CArithAlphabetMTF
{
public:
	CArithAlphabetMTF(u16 nSymbols)
	{
		// 258 == 65535/254
		// which means that we'll never overflow m_nTot
		NET_ASSERT(nSymbols <= 258);
		m_nSymbols = nSymbols;
		m_pData = new u8[GetDataSize()];

		u16* pArray = (u16*)(m_pData + 0 * m_nSymbols);
		u16* pLow = (u16*)(m_pData + 2 * m_nSymbols);
		u8* pSym = m_pData + 4 * m_nSymbols;

		for (u16 i = 0; i < nSymbols; i++)
		{
			pArray[i] = i;
			pLow[i] = i;
			pSym[i] = 0;
		}
		m_nTot = nSymbols;
	}

	~CArithAlphabetMTF()
	{
		delete[] m_pData;
	}

	CArithAlphabetMTF(const CArithAlphabetMTF& cp)
	{
		m_nSymbols = cp.m_nSymbols;
		m_pData = new u8[GetDataSize()];
		m_nTot = cp.m_nTot;
		memcpy(m_pData, cp.m_pData, GetDataSize());
	}

	void Swap(CArithAlphabetMTF& other)
	{
		std::swap(m_nSymbols, other.m_nSymbols);
		std::swap(m_pData, other.m_pData);
		std::swap(m_nTot, other.m_nTot);
	}

	CArithAlphabetMTF& operator=(const CArithAlphabetMTF& cp)
	{
		if (m_nSymbols == cp.m_nSymbols)
		{
			m_nTot = cp.m_nTot;
			memcpy(m_pData, cp.m_pData, GetDataSize());
		}
		else
		{
			CArithAlphabetMTF temp(cp);
			Swap(temp);
		}
		return *this;
	}

	void   WriteSymbol(CCommOutputStream& stm, u16 nSymbol);

	float  EstimateSymbolSizeInBits(u16 nSymbol) const;

	u16 ReadSymbol(CCommInputStream& stm);

	// to preserve interface with other modellers
	void   RecalculateProbabilities() {}
	size_t GetSize()
	{
		return sizeof(*this) + GetDataSize();
	}

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CArithAlphabetMTF");

		if (countingThis)
			pSizer->Add(*this);
		pSizer->Add(m_pData, GetDataSize());
	}

private:
	u8* m_pData;
	u16 m_nSymbols;
	u16 m_nTot;

	size_t GetDataSize() const
	{
		return 5 * m_nSymbols;
	}

	u16 MoveSymbolToFront(u16 nSymbol)
	{
		u16* pArray = (u16*)(m_pData + 0 * m_nSymbols);
		u16 i;
		for (i = 0; pArray[i] != nSymbol && i != m_nSymbols; i++)
			;
		NET_ASSERT(i != m_nSymbols);
		u16 out = i;
		for (; i != 0; --i)
			pArray[i] = pArray[i - 1];
		pArray[0] = nSymbol;
		return out;
	}

	void GetFrequency(u16 nSymbol, u16& rnTot, u16& rnLow, u16& rnSym)
	{
		u16* pLow = (u16*)(m_pData + 2 * m_nSymbols);
		u8* pSym = m_pData + 4 * m_nSymbols;

		rnTot = m_nTot;
		rnLow = pLow[nSymbol];
		rnSym = pSym[nSymbol] + 1;

		IncSymbol(nSymbol);
	}

	void IncSymbol(u16 nSymbol)
	{
		u16* pLow = (u16*)(m_pData + 2 * m_nSymbols);
		u8* pSym = m_pData + 4 * m_nSymbols;

		if (255 == ++pSym[nSymbol])
		{
			m_nTot = 0;
			for (u16 i = 0; i < m_nSymbols; i++)
			{
				pSym[i] >>= 1;
				pLow[i] = m_nTot;
				m_nTot += pSym[i] + 1;
			}
		}
		else
		{
			for (u16 i = nSymbol + 1; i < m_nSymbols; i++)
				pLow[i]++;
			m_nTot++;
		}
	}
};

// implements something that looks like an arithmetic alphabet, but just does pass-through writing
// for diagnosing problems quickly
class CArithAlphabetNull
{
public:
	CArithAlphabetNull(unsigned nSymbols)
	{
		m_nSymbols = nSymbols;
	}

	void Swap(CArithAlphabetNull& other)
	{
		std::swap(m_nSymbols, other.m_nSymbols);
	}

	void Resize(unsigned nSymbols)
	{
		m_nSymbols = nSymbols;
	}

	unsigned GetNumSymbols() const
	{
		return m_nSymbols;
	}

	size_t GetSize()
	{
		return sizeof(*this);
	}
	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CArithAlphabetNull");

		if (countingThis)
			pSizer->Add(*this);
	}

	ILINE size_t GetBufferSizeInBytes()
	{
		return 0;
	}

	void RecalculateProbabilities()
	{
	}

	// for compatibility with CArithAlphabetOrder1
	class CEstimator
	{
	public:
		CEstimator(const CArithAlphabetNull& alphabet) : m_alphabet(alphabet) {}

		float EstimateSymbol(unsigned nSymbol)
		{
			return log_tpl((float)m_alphabet.m_nSymbols) / log_tpl(2.f);
		}

	private:
		const CArithAlphabetNull& m_alphabet;
	};

	// return a (rough) estimate on how many bits that this symbol will
	// take in an output stream
	float EstimateSymbolSizeInBits(unsigned nSymbol) const
	{
		NET_ASSERT(nSymbol < m_nSymbols);
		return log_tpl((float)m_nSymbols) / log_tpl(2.f);
	}

	// write a symbol into an output stream
	void WriteSymbol(CCommOutputStream& stm, unsigned nSymbol)
	{
		stm.Encode(m_nSymbols, nSymbol, 1);
	}

	// read a symbol from an output stream
	unsigned ReadSymbol(CCommInputStream& stm)
	{
		unsigned out = stm.Decode(m_nSymbols);
		stm.Update(m_nSymbols, out, 1);
		return out;
	}

	/*
	   // debug aid: dump the contents of this model to a file
	   // (maybe we'll extend this to gather seed statistics as well
	   // one day!
	   void DumpCountsToFile( FILE * f ) const
	   {
	    fprintf(f,"COUNTS:\n");
	    for (unsigned i=0; i<m_nSymbols; i++)
	      fprintf(f, "%.4x ", GetCount(i));
	    fprintf(f, "\n");

	    fprintf(f,"LOW/SYM: (%.4x)\n", m_nTot);
	    for (unsigned i=0; i<m_nSymbols; i++)
	      fprintf(f, "{%.4x, %.4x} ", GetLow(i), GetSym(i));
	    fprintf(f, "\n");
	   }
	 */

private:
	unsigned m_nSymbols;
};

namespace
{
template<class U>
struct CArithAlphabetRow_InitHelper
{
	ILINE static void Init(U* p, i32 sz)
	{
		for (i32 i = 0; i < sz; i++)
			new(p + i)U();
	}
	ILINE static void Uninit(U* p, i32 sz)
	{
		for (i32 i = 0; i < sz; i++)
			(p + i)->~U();
	}
	ILINE static void Move(U* pDest, U* pSrc, i32 sz)
	{
		if (!sz) return;
		switch (sz % 4)
		{
		case 0:
			do
			{
				new(pDest++)U(*(pSrc++));
			case 3:
				new(pDest++)U(*(pSrc++));
			case 2:
				new(pDest++)U(*(pSrc++));
			case 1:
				new(pDest++)U(*(pSrc++));
			}
			while ((sz -= 4) > 0);
		}
	}
};
template<>
struct CArithAlphabetRow_InitHelper<u8>
{
	ILINE static void Init(u8* p, i32 sz)   {}
	ILINE static void Uninit(u8* p, i32 sz) {}
	ILINE static void Move(u8* pDest, u8* pSrc, i32 sz)
	{
		memcpy(pDest, pSrc, sz * sizeof(u8));
	}
};
template<>
struct CArithAlphabetRow_InitHelper<u16>
{
	ILINE static void Init(u16* p, i32 sz)   {}
	ILINE static void Uninit(u16* p, i32 sz) {}
	ILINE static void Move(u16* pDest, u16* pSrc, i32 sz)
	{
		memcpy(pDest, pSrc, sz * sizeof(u16));
	}
};
template<>
struct CArithAlphabetRow_InitHelper<u32>
{
	ILINE static void Init(u32* p, i32 sz)   {}
	ILINE static void Uninit(u32* p, i32 sz) {}
	ILINE static void Move(u32* pDest, u32* pSrc, i32 sz)
	{
		memcpy(pDest, pSrc, sz * sizeof(u32));
	}
};
}

template<class T, class C>
class CArithAlphabetRow
{
public:
	typedef T value_type;
	typedef C count_type;

	CArithAlphabetRow(i32 size)
	{
		if (size)
		{
			m_ptr = Alloc(size);
			Init(size);
		}
		else
		{
			m_ptr = 0;
		}
	}

	ILINE CArithAlphabetRow()
	{
		m_ptr = 0;
	}

	ILINE void Swap(CArithAlphabetRow& other)
	{
		u8* ptr = m_ptr;
		m_ptr = other.m_ptr;
		other.m_ptr = ptr;
	}

	ILINE CArithAlphabetRow(const CArithAlphabetRow& other) : m_ptr(other.m_ptr)
	{
		GetRefs().cnt++;
		// not NET_ASSERT due to massive performance overhead
		assert(GetRefs().cnt > 0);
	}

	ILINE CArithAlphabetRow& operator=(CArithAlphabetRow r)
	{
		Swap(r);
		return *this;
	}

	ILINE ~CArithAlphabetRow()
	{
		if (m_ptr && 0 == --GetRefs().cnt)
		{
			Uninit();
			Free(m_ptr, GetRefs().sz);
		}
	}

	void Update()
	{
		if (!m_ptr)
			return;
		uk curBuf = m_ptr;
		SRefCnt* pCurRC = (SRefCnt*)curBuf;
		if (pCurRC->cnt > 1)
		{
			u32 sz = Size();
			if (sz)
			{
				pCurRC->cnt--;
				T* oldBuf = (T*)(pCurRC + 1);
				m_ptr = Alloc(sz);
				uk pNewBuf = m_ptr;
				SRefCnt* pRC = (SRefCnt*) pNewBuf;
				T* pData = (T*)(pRC + 1);
				pRC->cnt = 1;
				pRC->flag = pCurRC->flag;
				pRC->sz = sz;
				CArithAlphabetRow_InitHelper<T>::Move(pData, oldBuf, sz);
			}
		}
	}

	void UpdateAndSetFlag(bool flag)
	{
		uk curBuf = m_ptr;
		SRefCnt* pCurRC = (SRefCnt*)curBuf;
		u32 sz = Size();
		bool needed;
		if (m_ptr)
			needed = pCurRC->cnt > 1 && (sz + (i32)flag);
		else
			needed = flag;
		if (needed)
		{
			if (pCurRC) pCurRC->cnt--;
			T* oldBuf = (T*)(pCurRC + 1);
			m_ptr = Alloc(sz);
			uk pNewBuf = m_ptr;
			SRefCnt* pRC = (SRefCnt*) pNewBuf;
			T* pData = (T*)(pRC + 1);
			pRC->cnt = 1;
			pRC->flag = flag;
			pRC->sz = sz;
			CArithAlphabetRow_InitHelper<T>::Move(pData, oldBuf, sz);
		}
	}

	ILINE T* Get()
	{
		return (T*)((u8*)m_ptr + sizeof(SRefCnt));
	}

	ILINE const T* Get() const
	{
		return (T*)((u8*)m_ptr + sizeof(SRefCnt));
	}

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CArithAlphabetRow");

		if (countingThis)
			pSizer->Add(*this);
		//MMM().AddHdlToSizer(m_hdl, pSizer);
	}

	ILINE u32 Size() const
	{
		return m_ptr ? GetRefs().sz : 0;
	}

	ILINE bool GetFlag() const { return m_ptr ? GetRefs().flag : false; }
	ILINE void SetFlag(bool flag)
	{
		//NET_ASSERT(GetRefs().cnt == 1);
		GetRefs().flag = flag;
	}

private:
	ILINE u8* Alloc(i32 size)
	{
		++g_objcnt.arithRow;
		return (u8*)MMM().AllocPtr(size * sizeof(T) + sizeof(SRefCnt));
	}

	u8* m_ptr;

	struct SRefCnt
	{
		C cnt;
		C sz: (sizeof(C) * 8 - 1);
		C flag : 1;
	};

	ILINE SRefCnt& GetRefs()
	{
		return *(SRefCnt*)m_ptr;
	}
	ILINE const SRefCnt& GetRefs() const
	{
		return *(const SRefCnt*)m_ptr;
	}

	ILINE void Init(C sz)
	{
		CArithAlphabetRow_InitHelper<T>::Init(Get(), sz);
		GetRefs().cnt = 1;
		GetRefs().flag = 0;
		GetRefs().sz = sz;
	}

	ILINE void Uninit()
	{
		CArithAlphabetRow_InitHelper<T>::Uninit(Get(), Size());
	}

	static ILINE void Free(u8* ptr, size_t size)
	{
		--g_objcnt.arithRow;
		MMM().FreePtr(ptr, size * sizeof(T) + sizeof(SRefCnt));
	}
};

// this class contains an order-1 modeller for compressing
// some alphabet (a set of symbols) arithmetically
// it uses O(n) memory, with n being the number of symbols
class CArithAlphabetOrder0
{
	typedef u16 TNum;
	static const TNum MaxCount = 65535;

public:
	CArithAlphabetOrder0(unsigned nSymbols) : m_counts(nSymbols * sizeof(TNum)), m_lowsym(2 * nSymbols * sizeof(TNum))
	{
		m_nSymbols = nSymbols;
		m_nMaxCount = MaxCount / m_nSymbols;
		for (unsigned i = 0; i < nSymbols; i++)
			GetCount(i) = 1;
		m_changed = true;
		RecalculateProbabilities();
	}

	~CArithAlphabetOrder0()
	{
	}

	CArithAlphabetOrder0(const CArithAlphabetOrder0& cp) : m_counts(cp.m_counts), m_lowsym(cp.m_lowsym)
	{
		m_nSymbols = cp.m_nSymbols;
		m_nMaxCount = cp.m_nMaxCount;
		m_nTot = cp.m_nTot;
		m_changed = cp.m_changed;
	}

	void Swap(CArithAlphabetOrder0& other)
	{
		std::swap(m_nSymbols, other.m_nSymbols);
		std::swap(m_nMaxCount, other.m_nMaxCount);
		std::swap(m_nTot, other.m_nTot);
		std::swap(m_changed, other.m_changed);
		m_counts.Swap(other.m_counts);
		m_lowsym.Swap(other.m_lowsym);
	}

	void Resize(unsigned nSymbols)
	{
		CArithAlphabetOrder0 temp(nSymbols);
		unsigned nCopy = min(temp.m_nSymbols, m_nSymbols);
		for (unsigned i = 0; i < nCopy; i++)
		{
			temp.GetSym(i) = min(GetSym(i), temp.m_nMaxCount);
		}
		temp.m_changed = true;
		temp.RecalculateProbabilities();
		Swap(temp);
	}

	unsigned GetNumSymbols() const
	{
		return m_nSymbols;
	}

	CArithAlphabetOrder0& operator=(const CArithAlphabetOrder0& cp)
	{
		CArithAlphabetOrder0 temp(cp);
		Swap(temp);
		return *this;
	}

	void RecalculateProbabilities()
	{
		if (!m_changed)
			return;

		m_lowsym.Update();
		m_nTot = 0;
		for (unsigned nCur = 0; nCur < m_nSymbols; nCur++)
		{
			NET_ASSERT(GetCount(nCur));
			GetLow(nCur) = m_nTot;
			GetSym(nCur) = GetCount(nCur);
			m_nTot += GetCount(nCur);
		}
		m_changed = false;
	}

	// for compatibility with CArithAlphabetOrder1
	class CEstimator
	{
	public:
		CEstimator(const CArithAlphabetOrder0& alphabet) : m_alphabet(alphabet) {}

		float EstimateSymbol(unsigned nSymbol)
		{
			return m_alphabet.EstimateSymbolSizeInBits(nSymbol);
		}

	private:
		const CArithAlphabetOrder0& m_alphabet;
	};

	// return a (rough) estimate on how many bits that this symbol will
	// take in an output stream
	float EstimateSymbolSizeInBits(unsigned nSymbol) const
	{
		NET_ASSERT(nSymbol < m_nSymbols);
		return CCommOutputStream::EstimateArithSizeInBits(m_nTot,
		                                                  GetSym(nSymbol));
	}

	// write a symbol into an output stream
	void WriteSymbol(CCommOutputStream& stm, unsigned nSymbol);

	// read a symbol from an output stream
	unsigned ReadSymbol(CCommInputStream& stm);

	/*
	   // debug aid: dump the contents of this model to a file
	   // (maybe we'll extend this to gather seed statistics as well
	   // one day!
	   void DumpCountsToFile( FILE * f ) const
	   {
	    fprintf(f,"COUNTS:\n");
	    for (unsigned i=0; i<m_nSymbols; i++)
	      fprintf(f, "%.4x ", GetCount(i));
	    fprintf(f, "\n");

	    fprintf(f,"LOW/SYM: (%.4x)\n", m_nTot);
	    for (unsigned i=0; i<m_nSymbols; i++)
	      fprintf(f, "{%.4x, %.4x} ", GetLow(i), GetSym(i));
	    fprintf(f, "\n");
	   }
	 */

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CArithAlphabetOrder0");

		if (countingThis)
			pSizer->Add(*this);

		m_counts.GetMemoryStatistics(pSizer);
		m_lowsym.GetMemoryStatistics(pSizer);
	}

	size_t GetSize() { return m_nSymbols * 3 * 2; }

private:
	unsigned                           m_nSymbols;
	CArithAlphabetRow<u8, unsigned> m_counts;
	CArithAlphabetRow<u8, unsigned> m_lowsym;
	TNum                               m_nMaxCount;
	TNum                               m_nTot;
	bool                               m_changed;

	// accessors to avoid needing to allocate multiple
	// buffers to implement this class
	ILINE TNum& GetCount(unsigned n)
	{
		return *(TNum*)(m_counts.Get() + sizeof(TNum) * n);
	}
	ILINE TNum GetCount(unsigned n) const
	{
		return *(TNum*)(m_counts.Get() + sizeof(TNum) * n);
	}
	ILINE TNum& GetSym(unsigned n)
	{
		return *(TNum*)(m_lowsym.Get() + sizeof(TNum) * n);
	}
	ILINE TNum GetSym(unsigned n) const
	{
		return *(TNum*)(m_lowsym.Get() + sizeof(TNum) * n);
	}
	ILINE TNum& GetLow(unsigned n)
	{
		return *(TNum*)(m_lowsym.Get() + sizeof(TNum) * (n + m_nSymbols));
	}
	ILINE TNum GetLow(unsigned n) const
	{
		return *(TNum*)(m_lowsym.Get() + sizeof(TNum) * (n + m_nSymbols));
	}

	// increment a count for a symbol safely - if we get to the highest
	// value available, halve all counts
	ILINE void IncCount(unsigned nSymbol)
	{
		m_changed = true;
		m_counts.Update();
		if (m_nMaxCount == ++GetCount(nSymbol))
			HalveCounts();
	}
	void HalveCounts()
	{
		RecalculateProbabilities();
		m_counts.Update();
		for (unsigned i = 0; i < m_nSymbols; i++)
			if (GetCount(i) > 1)
				GetCount(i) /= 2;
	}
};

/*
   m_lowsym.Update();
   m_nTot = 0;
   for (unsigned nCur = 0; nCur < m_nSymbols; nCur++)
   {
   NET_ASSERT(GetCount(nCur));
   GetLow(nCur) = m_nTot;
   GetSym(nCur) = GetCount(nCur);
   m_nTot += GetCount(nCur);
   }
   m_changed = false;
 */

template<i32 N>
struct SConstDivideHelper
{
	static ILINE unsigned f(unsigned x)
	{
		return x / N;
	}
};
template<>
struct SConstDivideHelper<5>
{
	static ILINE unsigned f(unsigned n)
	{
		unsigned q, r;
		q = (n >> 1) + (n >> 2);
		q = q + (q >> 4);
		q = q + (q >> 8);
		q = q + (q >> 16);
		q = q >> 2;
		r = n - q * 5;
		return q + (7 * r >> 5);
	}
};
template<>
struct SConstDivideHelper<12>
{
	static ILINE unsigned f(unsigned n)
	{
		unsigned q, r;
		q = (n >> 1) + (n >> 3);
		q = q + (q >> 4);
		q = q + (q >> 8);
		q = q + (q >> 16);
		q = q >> 3;
		r = n - q * 12;
		return q + ((r + 4) >> 4);
	}
};
/*
   template <>
   struct SConstDivideHelper<60>
   {
   static ILINE unsigned f(unsigned n)
   {
    return SConstDivideHelper<12>::f(SConstDivideHelper<5>::f(n));
   }
   };
 */

// this class contains an order-1 modeller for compressing
// some large alphabet (a set of more than 1000 symbols) arithmetically
// it uses O(n) memory, with n being the number of symbols
template<i32 L1 = 30, i32 L2 = 28>
class CArithLargeAlphabetOrder0
{
public:
	CArithLargeAlphabetOrder0(unsigned nSymbols)
	{
		++g_objcnt.largeArithOrder0;

		m_pData = new(MMM().AllocPtr(sizeof(SData)))SData();
		m_pData->refs = 1;
		m_pData->nSymbols = nSymbols;

		unsigned nLev0 = nSymbols / (L1 * L2) + (nSymbols % (L1 * L2) != 0);
		m_pData->count = TSymLevel0(nLev0);
		m_pData->count.SetFlag(true);
		m_pData->low = TLowLevel0(nLev0);
		i32 nTot = 0;
		for (u32 i = 0; i < nLev0; i++)
		{
			unsigned nLeft = nSymbols - nTot;
			unsigned nLev1 = std::min(unsigned(L1), nLeft / L2 + (nLeft % L2 != 0));
			TSymLevel1& lev1_count = m_pData->count.Get()[i] = TSymLevel1(nLev1);
			TLowLevel1& lev1_low = m_pData->low.Get()[i].seg = TLowLevel1(nLev1);
			lev1_count.SetFlag(true);
			for (u32 j = 0; j < nLev1; j++)
			{
				unsigned nSeg = std::min(unsigned(L2), nLeft);
				(lev1_count.Get()[j] = TSymSegment(nSeg)).SetFlag(true);
				TSymCount* pCounts = lev1_count.Get()[j].Get();
				for (u32 k = 0; k < nSeg; k++)
					pCounts[k] = 1;
				lev1_low.Get()[j].seg = ConstructInitialSegment(nSeg, GetLowCache(), false);
				nTot += nSeg;
				nLeft -= nSeg;
			}
		}
		RecalculateProbabilities();
	}

	ILINE CArithLargeAlphabetOrder0(const CArithLargeAlphabetOrder0& other) : m_pData(other.m_pData)
	{
		m_pData->refs++;
		++g_objcnt.largeArithOrder0;
	}

	ILINE void Swap(CArithLargeAlphabetOrder0& other)
	{
		std::swap(m_pData, other.m_pData);
	}

	ILINE CArithLargeAlphabetOrder0& operator=(CArithLargeAlphabetOrder0 other)
	{
		Swap(other);
		return *this;
	}

	ILINE ~CArithLargeAlphabetOrder0()
	{
		--g_objcnt.largeArithOrder0;
		if (0 == --m_pData->refs)
		{
			m_pData->~SData();
			MMM().FreePtr(m_pData, sizeof(SData));
		}
	}

	void Resize(unsigned nSymbols)
	{
		Validate();
		if (nSymbols == m_pData->nSymbols)
			return;
		CArithLargeAlphabetOrder0<L1, L2> tmp(nSymbols);
		i32 nCopy = std::min(nSymbols, m_pData->nSymbols);
		tmp.CopyCountsFrom(nCopy, *this);
		*this = tmp;

		RecalculateProbabilities();
	}

	unsigned GetNumSymbols() const { return m_pData->nSymbols; }

	void     RecalculateProbabilities()
	{
		if (!m_pData->count.GetFlag())
			return;

		BeginUpdate();

		TSymLevel0& count = m_pData->count;
		TLowLevel0& low = m_pData->low;

		count.UpdateAndSetFlag(false);
		low.Update();
		i32 lev0_count_sz = count.Size();
		TSymLevel1* lev0_count_elems = count.Get();
		TLowLevel0Elem* lev0_low_elems = low.Get();
		TLowValue lev0_tot = 0;
		for (i32 i = 0; i < lev0_count_sz; i++)
		{
			TSymLevel1& lev1_count = lev0_count_elems[i];
			TLowLevel0Elem& lev1_low = lev0_low_elems[i];

			if (lev1_count.GetFlag())
			{
				lev1_count.UpdateAndSetFlag(false);
				lev1_low.seg.Update();
				i32 lev1_count_sz = lev1_count.Size();
				TSymSegment* lev1_count_elems = lev1_count.Get();
				TLowLevel1Elem* lev1_low_elems = lev1_low.seg.Get();
				TLowValue lev1_tot = 0;
				for (i32 j = 0; j < lev1_count_sz; j++)
				{
					TSymSegment& seg_count = lev1_count_elems[j];
					TLowLevel1Elem& seg_low = lev1_low_elems[j];

					if (seg_count.GetFlag())
					{
						seg_count.UpdateAndSetFlag(false);
						seg_low.seg.Update();
						i32 seg_count_sz = seg_count.Size();
						const TSymCount* seg_count_elems = seg_count.Get();
						TLowValue* seg_low_elems = seg_low.seg.Get();
						TLowValue seg_tot = 0;
						TSymCount bits_set = 0;
						if (seg_count_sz)
							switch (seg_count_sz % 4)
							{
							case 0:
								do
								{
									*seg_low_elems++ = seg_tot;
									bits_set |= *seg_count_elems;
									seg_tot += *seg_count_elems++;
								case 3:
									*seg_low_elems++ = seg_tot;
									bits_set |= *seg_count_elems;
									seg_tot += *seg_count_elems++;
								case 2:
									*seg_low_elems++ = seg_tot;
									bits_set |= *seg_count_elems;
									seg_tot += *seg_count_elems++;
								case 1:
									*seg_low_elems++ = seg_tot;
									bits_set |= *seg_count_elems;
									seg_tot += *seg_count_elems++;
								}
								while ((seg_count_sz -= 4) > 0);
							}

						if (bits_set == 1)
							seg_count = ConstructInitialSegment(seg_count.Size(), GetSymCache(), true);

						seg_low.localTot = seg_tot;
					}

					seg_low.localLow = lev1_tot;
					lev1_tot += seg_low.localTot;
				}

				lev1_low.localTot = lev1_tot;
			}

			lev1_low.localLow = lev0_tot;
			lev0_tot += lev1_low.localTot;
		}

		m_pData->sym = count;
		m_pData->tot = lev0_tot;
		Validate();
	}

	// write a symbol into an output stream
	void WriteSymbol(CCommOutputStream& stm, unsigned nSymbol)
	{
		stm.Encode(m_pData->tot, GetLow(nSymbol), GetSym(nSymbol));
		IncCount(nSymbol);
	}

	// read a symbol from an output stream
	unsigned ReadSymbol(CCommInputStream& stm)
	{
		unsigned nSymbol;
		unsigned nProb = stm.Decode(m_pData->tot);

		unsigned nBegin = 0;
		unsigned nEnd = m_pData->nSymbols;
		while (true)
		{
			nSymbol = (nBegin + nEnd) / 2;

			if (nProb < GetLow(nSymbol))
				nEnd = nSymbol;
			else if (nProb >= unsigned(GetLow(nSymbol) + GetSym(nSymbol)))
				nBegin = nSymbol + 1;
			else
				break;
		}

		stm.Update(m_pData->tot, GetLow(nSymbol), GetSym(nSymbol));
		IncCount(nSymbol);

		return nSymbol;
	}

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
	}

	#if DEBUG_ENDPOINT_LOGIC
	void DumpCountsToFile(FILE* f) const
	{
		/*u16 * pArray = (u16*)( m_pData + 0 * m_nSymbols );
		   u16 * pLow = (u16*)( m_pData + 2 * m_nSymbols );
		   u8 * pSym = m_pData + 4 * m_nSymbols;

		   fprintf(f,"ORDER:\n");
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.2x ", pArray[i]);
		   fprintf(f, "\n");

		   fprintf(f,"COUNTS:\n");
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.2x ", pSym[i]);
		   fprintf(f, "\n");

		   fprintf(f,"LOW: (%.4x)\n", m_nTot);
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.4x ", pLow[i]);
		   fprintf(f, "\n");*/
	}
	#endif

private:
	typedef u8  TSymCount;
	typedef u32 TLowValue;

	static i32k MaxCount = 255;

	typedef CArithAlphabetRow<TSymCount, unsigned>   TSymSegment;
	typedef CArithAlphabetRow<TLowValue, TLowValue>  TLowSegment;

	typedef CArithAlphabetRow<TSymSegment, unsigned> TSymLevel1;
	typedef CArithAlphabetRow<TSymLevel1, unsigned>  TSymLevel0;

	template<class Row>
	struct SLowLevelElem
	{
		ILINE SLowLevelElem() {}
		ILINE SLowLevelElem(const SLowLevelElem& other) : seg(other.seg), localLow(other.localLow), localTot(other.localTot) {}
		SLowLevelElem& operator=(const SLowLevelElem&);
		ILINE ~SLowLevelElem() {}
		Row       seg;
		TLowValue localLow;
		TLowValue localTot;
	};
	typedef SLowLevelElem<TLowSegment>                  TLowLevel1Elem;
	typedef CArithAlphabetRow<TLowLevel1Elem, unsigned> TLowLevel1;
	typedef SLowLevelElem<TLowLevel1>                   TLowLevel0Elem;
	typedef CArithAlphabetRow<TLowLevel0Elem, unsigned> TLowLevel0;

	struct SData
	{
		i32        refs;
		TSymLevel0 count;
		TSymLevel0 sym;
		TLowLevel0 low;
		TLowValue  tot;
		unsigned   nSymbols;
	};

	SData* m_pData;

	template<class T>
	T ConstructInitialSegment(unsigned sz, VectorMap<unsigned, T>& cache, bool setup)
	{
		typename VectorMap<unsigned, T>::iterator it = cache.find(sz);
		if (it == cache.end())
		{
			it = cache.insert(std::make_pair(sz, T(sz))).first;
			if (setup)
			{
				typedef typename T::value_type VT;
				VT* v = it->second.Get();
				for (u32 i = 0; i < sz; i++)
					v[i] = 1;
			}
		}
		NET_ASSERT(!it->second.GetFlag());
		return it->second;
	}

	template<class T>
	class CMementoCacheThing : public IMementoManagedThing, public VectorMap<unsigned, T>
	{
		void Release() { delete this; }
	};

	template<class T>
	VectorMap<unsigned, T>& GetCacheInSlot(i32 slot)
	{
		if (!MMM().pThings[slot])
			MMM().pThings[slot] = new CMementoCacheThing<T>();
		return *(CMementoCacheThing<T>*)MMM().pThings[slot];
	}

	VectorMap<unsigned, TSymSegment>& GetSymCache()
	{
		return GetCacheInSlot<TSymSegment>(0);
	}
	VectorMap<unsigned, TLowSegment>& GetLowCache()
	{
		return GetCacheInSlot<TLowSegment>(1);
	}

	TSymCount GetSym(unsigned sym, bool validate = true) const
	{
		if (validate)
			Validate();
		i32 lev0idx = SConstDivideHelper<L1* L2>::f(sym);
		const TSymLevel1& lev1 = m_pData->sym.Get()[lev0idx];
		sym -= lev0idx * L1 * L2;
		i32 lev1idx = SConstDivideHelper<L2>::f(sym);
		const TSymSegment& seg = lev1.Get()[lev1idx];
		return seg.Get()[sym - lev1idx * L2];
	}

	TLowValue GetLow(unsigned sym, bool validate = true) const
	{
		if (validate)
			Validate();
		i32 lev0idx = SConstDivideHelper<L1* L2>::f(sym);
		const TLowLevel0Elem& lev1 = m_pData->low.Get()[lev0idx];
		sym -= lev0idx * L1 * L2;
		i32 lev1idx = SConstDivideHelper<L2>::f(sym);
		const TLowLevel1Elem& seg = lev1.seg.Get()[lev1idx];
		return seg.seg.Get()[sym - lev1idx * L2] + seg.localLow + lev1.localLow;
	}

	void IncCount(unsigned sym)
	{
		BeginUpdate();
		Validate();
		i32 lev0idx = SConstDivideHelper<L1* L2>::f(sym);
		m_pData->count.UpdateAndSetFlag(true);
		TSymLevel1& lev1 = m_pData->count.Get()[lev0idx];
		sym -= lev0idx * L1 * L2;
		i32 lev1idx = SConstDivideHelper<L2>::f(sym);
		lev1.UpdateAndSetFlag(true);
		TSymSegment& seg = lev1.Get()[lev1idx];
		seg.UpdateAndSetFlag(true);
		sym -= lev1idx * L2;
		TSymCount& cnt = seg.Get()[sym];
		if (++cnt == MaxCount)
		{
			RecalculateProbabilities();
			HalveCounts();
		}
		else
		{
			assert(cnt);
		}
		Validate();
	}

	void HalveCounts()
	{
		BeginUpdate();
		Validate();
		m_pData->count.UpdateAndSetFlag(true);
		i32 lev0_count_sz = m_pData->count.Size();
		TSymLevel1* lev0_count_elems = m_pData->count.Get();
		for (i32 i = 0; i < lev0_count_sz; i++)
		{
			TSymLevel1& lev1_count = lev0_count_elems[i];

			lev1_count.UpdateAndSetFlag(true);
			i32 lev1_count_sz = lev1_count.Size();
			TSymSegment* lev1_count_elems = lev1_count.Get();
			for (i32 j = 0; j < lev1_count_sz; j++)
			{
				TSymSegment& seg_count = lev1_count_elems[j];

				seg_count.UpdateAndSetFlag(true);
				i32 seg_count_sz = seg_count.Size();
				TSymCount* seg_count_elems = seg_count.Get();
				for (i32 k = 0; k < seg_count_sz; k++)
				{
					seg_count_elems[k] >>= i32(seg_count_elems[k] > 1);
					NET_ASSERT(seg_count_elems[k]);
				}
			}
		}
		Validate();
	}

	void CopyCountsFrom(unsigned nCounts, CArithLargeAlphabetOrder0& other)
	{
		BeginUpdate();
		m_pData->count.Update();
		m_pData->count.SetFlag(true);
		i32 lev0_count_sz = m_pData->count.Size();
		TSymLevel1* lev0_count_elems = m_pData->count.Get();
		TSymLevel1* lev0_count_elems_other = other.m_pData->count.Get();
		for (i32 i = 0; i < lev0_count_sz; i++)
		{
			TSymLevel1& lev1_count = lev0_count_elems[i];
			TSymLevel1& lev1_count_other = lev0_count_elems_other[i];

			lev1_count.Update();
			lev1_count.SetFlag(true);
			i32 lev1_count_sz = lev1_count.Size();
			TSymSegment* lev1_count_elems = lev1_count.Get();
			TSymSegment* lev1_count_elems_other = lev1_count_other.Get();
			for (i32 j = 0; j < lev1_count_sz; j++)
			{
				TSymSegment& seg_count = lev1_count_elems[j];
				TSymSegment& seg_count_other = lev1_count_elems_other[j];

				seg_count.Update();
				seg_count.SetFlag(true);
				i32 seg_count_sz = seg_count.Size();
				TSymCount* seg_count_elems = seg_count.Get();
				TSymCount* seg_count_elems_other = seg_count_other.Get();
				bool all_ones = true;
				for (i32 k = 0; k < seg_count_sz; k++)
				{
					seg_count_elems[k] = seg_count_elems_other[k];
					all_ones &= (seg_count_elems[k] == 1);
					if (0 == --nCounts)
						return; // !! early out
				}
			}
		}
	}

	void BeginUpdate()
	{
		if (m_pData->refs != 1)
		{
			SData* pOld = m_pData;
			m_pData = new(MMM().AllocPtr(sizeof(SData)))SData(*pOld);
			pOld->refs--;
			m_pData->refs = 1;
		}
	}

	void Validate() const
	{
		/*
		    u32 tot = 0;
		    for (i32 i=0; i<GetNumSymbols(); i++)
		    {
		      NET_ASSERT(GetLow(i, false) == tot);
		      TSymCount sym = GetSym(i, false);
		      NET_ASSERT(sym);
		      tot += sym;
		    }
		    NET_ASSERT(tot == m_pData->tot);
		   //*/
	}
};

// this class contains an order-2 modeller for compressing
// some alphabet (a set of symbols) arithmetically
// it uses O(n^2) memory, with n being the number of symbols
template<class TAlphabetOrder0>
class CArithAlphabetOrder1
{
public:
	CArithAlphabetOrder1(unsigned nSymbols)
	{
		m_nSymbols = nSymbols;
		TAlphabetOrder0 defVal(nSymbols);
		m_rows.resize(nSymbols, defVal);
		m_nPrevSymbol = 0;
	}

	void RecalculateProbabilities()
	{
		for (typename DynArray<TAlphabetOrder0>::iterator it = m_rows.begin(); it != m_rows.end(); ++it)
		{
			it->RecalculateProbabilities();
		}
	}

	void WriteSymbol(CCommOutputStream& stm, unsigned nSymbol)
	{
		m_rows[m_nPrevSymbol].WriteSymbol(stm, nSymbol);
		m_nPrevSymbol = nSymbol;
	}

	unsigned ReadSymbol(CCommInputStream& stm)
	{
		return m_nPrevSymbol = m_rows[m_nPrevSymbol].ReadSymbol(stm);
	}

	void GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CArithAlphabetOrder1");

		if (countingThis)
			pSizer->Add(*this);

		for (typename DynArray<TAlphabetOrder0>::size_type i = 0; i < m_rows.size(); ++i)
		{
			m_rows[i].GetMemoryStatistics(pSizer, true);
		}
	}

	u32 GetSize() { return 0; }

	#if DEBUG_ENDPOINT_LOGIC
	void DumpCountsToFile(FILE* f) const
	{
		/*u16 * pArray = (u16*)( m_pData + 0 * m_nSymbols );
		   u16 * pLow = (u16*)( m_pData + 2 * m_nSymbols );
		   u8 * pSym = m_pData + 4 * m_nSymbols;

		   fprintf(f,"ORDER:\n");
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.2x ", pArray[i]);
		   fprintf(f, "\n");

		   fprintf(f,"COUNTS:\n");
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.2x ", pSym[i]);
		   fprintf(f, "\n");

		   fprintf(f,"LOW: (%.4x)\n", m_nTot);
		   for (unsigned i=0; i<m_nSymbols; i++)
		   fprintf(f, "%.4x ", pLow[i]);
		   fprintf(f, "\n");*/
	}
	#endif

private:
	unsigned                  m_nSymbols;
	unsigned                  m_nPrevSymbol;
	DynArray<TAlphabetOrder0> m_rows;
};

#endif

#endif
