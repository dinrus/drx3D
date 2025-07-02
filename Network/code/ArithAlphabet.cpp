// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   -------------------------------------------------------------------------
   История:
   - 02/11/2005   12:34 : Created by Jan Mueller
*************************************************************************/
#include <drx3D/Network/StdAfx.h>

#include  <drx3D/Network/Config.h>

#if USE_ARITHSTREAM

	#include  <drx3D/Network/ArithAlphabet.h>
	#include  <drx3D/Network/Serialize.h>

void CArithAlphabetMTF::WriteSymbol(CCommOutputStream& stm, u16 nSymbol)
{
	nSymbol = m_nSymbols - MoveSymbolToFront(nSymbol) - 1;
	u16 nTot, nLow, nSym;
	GetFrequency(nSymbol, nTot, nLow, nSym);
	stm.Encode(nTot, nLow, nSym);
}

float CArithAlphabetMTF::EstimateSymbolSizeInBits(u16 nSymbol) const
{
	u16* pArray = (u16*)(m_pData + 0 * m_nSymbols);
	u8* pSym = m_pData + 4 * m_nSymbols;
	u16 i;
	for (i = 0; pArray[i] != nSymbol; i++)
		;
	i = m_nSymbols - i - 1;
	return CCommOutputStream::EstimateArithSizeInBits(m_nTot, pSym[i] + 1);
}

u16 CArithAlphabetMTF::ReadSymbol(CCommInputStream& stm)
{
	u16 nProb = stm.Decode(m_nTot);

	u16* pArray = (u16*)(m_pData + 0 * m_nSymbols);
	u16* pLow = (u16*)(m_pData + 2 * m_nSymbols);
	u8* pSym = m_pData + 4 * m_nSymbols;

	u16 nPos;
	for (nPos = m_nSymbols - 1; pLow[nPos] > nProb; --nPos)
		;
	NET_ASSERT(pLow[nPos] <= nProb);
	NET_ASSERT(pLow[nPos] + pSym[nPos] + 1 > nProb);
	stm.Update(m_nTot, pLow[nPos], pSym[nPos] + 1);
	IncSymbol(nPos);

	nPos = m_nSymbols - nPos - 1;

	u16 nSymbol = pArray[nPos];
	for (; nPos != 0; --nPos)
		pArray[nPos] = pArray[nPos - 1];
	pArray[0] = nSymbol;

	return nSymbol;
}

void CArithAlphabetOrder0::WriteSymbol(CCommOutputStream& stm, unsigned nSymbol)
{
	stm.Encode(
	  m_nTot,
	  GetLow(nSymbol),
	  GetSym(nSymbol));
	IncCount(nSymbol);
}

unsigned CArithAlphabetOrder0::ReadSymbol(CCommInputStream& stm)
{
	unsigned nSymbol;
	unsigned nProb = stm.Decode(m_nTot);

	unsigned nBegin = 0;
	unsigned nEnd = m_nSymbols;
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

	stm.Update(
	  m_nTot,
	  GetLow(nSymbol),
	  GetSym(nSymbol));
	IncCount(nSymbol);

	return nSymbol;
}

#endif
