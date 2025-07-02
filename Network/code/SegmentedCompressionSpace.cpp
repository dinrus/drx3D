// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SegmentedCompressionSpace.h>

#if USE_MEMENTO_PREDICTORS
static void ParseLine(string text, std::vector<u32>& parsed)
{
	i32 curPos = 0;
	// [AlexMcC|27/08/09] If this text coming from binaryxml, it might be
	// shared data. Make a new string to avoid smashing it
	string val = text.Tokenize(",", curPos);
	while (!val.empty())
	{
		parsed.push_back(atoi(val.c_str()));
		val = text.Tokenize(",", curPos);
	}
}

CSegmentedCompressionSpace::CSegmentedCompressionSpace()
	: m_steps(0)
	, m_dimensions(0)
	, m_outerSizeSteps(0)
	, m_outerSizeSpace(0)
	, m_chunkSize(0)
	, m_bits(0)
{
	ZeroMemory(&m_center, sizeof(m_center));
}

bool CSegmentedCompressionSpace::Load(tukk filename)
{
	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile(filename);
	if (!root || !root->getAttr("d", m_dimensions) || !root->getAttr("s", m_steps) || !root->getAttr("b", m_bits) || !root->getChildCount())
		return false;

	i32 expectSize = 1;
	for (i32 i = 0; i < m_dimensions; i++, expectSize *= m_steps)
		;
	m_chunkSize = expectSize * 3;

	std::vector<u32> c;
	for (i32 i = 0; i < root->getChildCount(); i++)
	{
		XmlNodeRef row = root->getChild(i);

		if (i == 0)
		{
			if (!row->getAttr("oss", m_outerSizeSpace))
				return false;
			if (m_outerSizeSpace % m_steps)
				return false;
			i32 x = 1, n = 0;
			for (; x < m_outerSizeSpace; n++, x *= m_steps)
				;
			m_outerSizeSteps = n;

			c.resize(0);
			ParseLine(row->getAttr("c"), c);
			if (c.size() != m_dimensions)
				return false;
			for (i32 k = 0; k < m_dimensions; k++)
				m_center[k] = c[k];
		}

		XmlNodeRef counts = row->findChild("c");
		XmlNodeRef links = row->findChild("l");
		if (!counts || !links)
			return false;

		std::vector<u32> vCounts, vLinks;
		ParseLine(counts->getContent(), vCounts);
		ParseLine(links->getContent(), vLinks);

		if (vCounts.size() != expectSize || vLinks.size() != expectSize)
			return false;

		u32 low = 0;
		for (i32 n = 0; n < expectSize; n++)
		{
			m_data.push_back(low);
			m_data.push_back(vCounts[n]);
			low += vCounts[n];
			m_data.push_back(vLinks[n]);
		}
	}

	return true;
}

bool CSegmentedCompressionSpace::CanEncode(i32k* pValue, i32 dim) const
{
	i32 centered[MAX_DIMENSION];
	bool inCompressibleSpace = true;
	for (i32 i = 0; i < dim; i++)
	{
		centered[i] = pValue[i] - m_center[i];
		inCompressibleSpace &= (centered[i] < 1 + m_outerSizeSpace / 2) && (centered[i] > -m_outerSizeSpace / 2);
	}
	return inCompressibleSpace;
}

void CSegmentedCompressionSpace::Encode(CCommOutputStream& stm, i32k* pValue, i32 dim) const
{
	NET_ASSERT(dim <= MAX_DIMENSION);
	NET_ASSERT(dim == m_dimensions);

	i32 centered[MAX_DIMENSION];
	bool inCompressibleSpace = true;
	for (i32 i = 0; i < dim; i++)
	{
		centered[i] = pValue[i] - m_center[i];
		inCompressibleSpace &= (centered[i] < 1 + m_outerSizeSpace / 2) && (centered[i] > -m_outerSizeSpace / 2);
	}

	NET_ASSERT(inCompressibleSpace);

	u32 chunk = 0;
	u32k numChunks = m_data.size() / m_chunkSize;
	i32 chunkSpaceSize = m_outerSizeSpace;

	i32 extremaSmall[MAX_DIMENSION];
	for (i32 i = 0; i < dim; i++)
		extremaSmall[i] = -chunkSpaceSize / 2;

	while (chunk < numChunks)
	{
		u32 ofs = 0;
		u32 ofsFactor = 1;
		NET_ASSERT(chunkSpaceSize % m_steps == 0);
		chunkSpaceSize /= m_steps;
		for (i32 i = 0; i < dim; i++)
		{
			for (i32 j = 0; j < m_steps; j++)
			{
				if (centered[i] < extremaSmall[i] + chunkSpaceSize * (j + 1))
				{
					extremaSmall[i] += chunkSpaceSize * j;
					ofs += ofsFactor * j;
					break;
				}
			}
			ofsFactor *= m_steps;
		}
		stm.Encode(GetTot(chunk), GetLow(chunk, ofs), GetSym(chunk, ofs));
		chunk = GetChild(chunk, ofs);
	}

	// encode residual
	if (chunkSpaceSize > 1)
	{
		for (i32 i = 0; i < dim; i++)
			stm.Encode(chunkSpaceSize, centered[i] - extremaSmall[i], 1);
	}
}

void CSegmentedCompressionSpace::Decode(CCommInputStream& stm, i32* pValue, i32 dim) const
{
	NET_ASSERT(dim <= MAX_DIMENSION);
	NET_ASSERT(dim == m_dimensions);

	u32 chunk = 0;
	u32k numChunks = m_data.size() / m_chunkSize;
	i32 chunkSpaceSize = m_outerSizeSpace;

	i32 extremaSmall[MAX_DIMENSION];
	for (i32 i = 0; i < dim; i++)
		extremaSmall[i] = -chunkSpaceSize / 2;

	while (chunk < numChunks)
	{
		u32 ofsDec = stm.Decode(GetTot(chunk));
		u32 ofs;
		for (ofs = m_chunkSize / 3 - 1; GetLow(chunk, ofs) > ofsDec; ofs--)
			;
		stm.Update(GetTot(chunk), GetLow(chunk, ofs), GetSym(chunk, ofs));
		chunkSpaceSize /= m_steps;
		u32 ofsFactor = 1;
		for (i32 i = 0; i < dim; i++)
		{
			u32 bigOfsFactor = ofsFactor * m_steps;
			u32 step = (ofs % bigOfsFactor) / ofsFactor;
			extremaSmall[i] += chunkSpaceSize * step;
			ofsFactor = bigOfsFactor;
		}
		chunk = GetChild(chunk, ofs);
	}

	if (chunkSpaceSize > 1)
	{
		for (i32 i = 0; i < dim; i++)
		{
			i32 x = stm.Decode(chunkSpaceSize);
			stm.Update(chunkSpaceSize, x, 1);
			extremaSmall[i] += x;
		}
	}

	for (i32 i = 0; i < dim; i++)
		pValue[i] = extremaSmall[i] + m_center[i];
}
#endif
