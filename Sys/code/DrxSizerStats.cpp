// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Sys/DrxSizerImpl.h>
#include <drx3D/Sys/DrxSizerStats.h>
#include <drx3D/Sys/ITextModeConsole.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

DrxSizerStatsBuilder::DrxSizerStatsBuilder(DrxSizerImpl* pSizer, i32 nMinSubcomponentBytes) :
	m_pSizer(pSizer),
	m_nMinSubcomponentBytes(nMinSubcomponentBytes < 0 || nMinSubcomponentBytes > 0x10000000 ? 0 : nMinSubcomponentBytes),
	m_pStats(nullptr)
{

}

// creates the map of names from old (in the sizer Impl) to new (in the Stats)
void DrxSizerStatsBuilder::processNames()
{
	size_t numCompNames = m_pSizer->m_arrNames.size();
	m_pStats->m_arrComponents.reserve(numCompNames);
	m_pStats->m_arrComponents.clear();

	m_mapNames.resize(numCompNames, (size_t)-1);

	// add all root objects
	addNameSubtree(0, 0);
}

//////////////////////////////////////////////////////////////////////////
// given the name in the old system, adds the subtree of names to the
// name map and components. In case all the subtree is empty, returns false and
// adds nothing
size_t DrxSizerStatsBuilder::addNameSubtree(unsigned nDepth, size_t nName)
{
	assert((i32)nName < m_pSizer->m_arrNames.size());

	DrxSizerImpl::ComponentName& rCompName = m_pSizer->m_arrNames[nName];
	size_t sizeObjectsTotal = rCompName.sizeObjectsTotal;

	if (sizeObjectsTotal <= m_nMinSubcomponentBytes)
		return sizeObjectsTotal; // the subtree didn't pass

	// the index of the component in the stats object (sorted by the depth-first traverse order)
	size_t nNewName = m_pStats->m_arrComponents.size();
	m_pStats->m_arrComponents.resize(nNewName + 1);

	Component& rNewComp = m_pStats->m_arrComponents[nNewName];
	rNewComp.strName = rCompName.strName;
	rNewComp.nDepth = nDepth;
	rNewComp.numObjects = rCompName.numObjects;
	rNewComp.sizeBytes = rCompName.sizeObjects;
	rNewComp.sizeBytesTotal = sizeObjectsTotal;
	m_mapNames[nName] = nNewName;

	// find the immediate children and sort them by their total size
	typedef std::map<size_t, size_t> UintUintMap;
	UintUintMap mapSizeName; // total size -> child index (name in old indexation)

	for (i32 i = nName + 1; i < m_pSizer->m_arrNames.size(); ++i)
	{
		DrxSizerImpl::ComponentName& rChild = m_pSizer->m_arrNames[i];
		if (rChild.nParent == nName && rChild.sizeObjectsTotal > m_nMinSubcomponentBytes)
			mapSizeName.insert(UintUintMap::value_type(rChild.sizeObjectsTotal, i));
	}

	// add the sorted components
	/*
	   for (unsigned i = nName + 1; i < m_pSizer->m_arrNames.size(); ++i)
	   if (m_pSizer->m_arrNames[i].nParent == nName)
	    addNameSubtree(nDepth+1,i);
	 */

	for (UintUintMap::reverse_iterator it = mapSizeName.rbegin(); it != mapSizeName.rend(); ++it)
	{
		addNameSubtree(nDepth + 1, it->second);
	}

	return sizeObjectsTotal;
}

//////////////////////////////////////////////////////////////////////////
// creates the statistics out of the given DrxSizerImpl into the given DrxSizerStats
// Maps the old to new names according to the depth-walk tree rule
void DrxSizerStatsBuilder::build(DrxSizerStats* pStats)
{
	m_pStats = pStats;

	m_mapNames.clear();

	processNames();

	m_pSizer->clear();
	pStats->refresh();
	pStats->m_nAgeFrames = 0;
}

//////////////////////////////////////////////////////////////////////////
// constructs the statistics based on the given drx sizer
DrxSizerStats::DrxSizerStats(DrxSizerImpl* pDrxSizer)
{
	DrxSizerStatsBuilder builder(pDrxSizer);
	builder.build(this);
}

DrxSizerStats::DrxSizerStats()
	: m_nStartRow(0)
	, m_nAgeFrames(0)
	, m_nMaxNameLength(0)
{
	for (i32 i = 0; i < g_numTimers; ++i)
	{
		m_fTime[i] = 0.0f;
	}
}

void DrxSizerStats::updateKeys()
{
	u32k statSize = size();
	//assume 10 pixels for font
	u32 height = gEnv->pRenderer->GetOverlayHeight() / 12;
	if (DrxGetAsyncKeyState(VK_UP))
	{
		if (m_nStartRow > 0)
			--m_nStartRow;
	}
	if (DrxGetAsyncKeyState(VK_DOWN))
	{
		if (statSize > height + m_nStartRow)
			++m_nStartRow;
	}
	if (DrxGetAsyncKeyState(VK_RIGHT) & 1)
	{
		//assume 10 pixels for font
		if (statSize > height)
			m_nStartRow = statSize - height;
	}
	if (DrxGetAsyncKeyState(VK_LEFT) & 1)
	{
		m_nStartRow = 0;
	}
}

// if there is already such name in the map, then just returns the index
// of the compoentn in the component array; otherwise adds an entry to themap
// and to the component array nad returns its index
DrxSizerStatsBuilder::Component& DrxSizerStatsBuilder::mapName(unsigned nName)
{
	assert(m_mapNames[nName] != -1);
	return m_pStats->m_arrComponents[m_mapNames[nName]];
	/*
	   IdToIdMap::iterator it = m_mapNames.find (nName);
	   if (it == m_mapNames.end())
	   {
	   unsigned nNewName = m_arrComponents.size();
	   m_mapNames.insert (IdToIdMap::value_type(nName, nNewName));
	   m_arrComponents.resize(nNewName + 1);
	   m_arrComponents[nNewName].strName.swap(m_pSizer->m_arrNames[nName]);
	   return m_arrComponents.back();
	   }
	   else
	   {
	   assert (it->second < m_arrComponents.size());
	   return m_arrComponents[it->second];
	   }
	 */
}

// refreshes the statistics built after the component array is built
void DrxSizerStats::refresh()
{
	m_nMaxNameLength = 0;
	for (size_t i = 0; i < m_arrComponents.size(); ++i)
	{
		size_t nLength = m_arrComponents[i].strName.length() + m_arrComponents[i].nDepth;
		if (nLength > m_nMaxNameLength)
			m_nMaxNameLength = nLength;
	}
}

bool DrxSizerStats::Component::GenericOrder::operator()(const Component& left, const Component& right) const
{
	return left.strName < right.strName;
}

DrxSizerStatsRenderer::DrxSizerStatsRenderer(ISystem* pSystem, DrxSizerStats* pStats, unsigned nMaxSubcomponentDepth, i32 nMinSubcomponentBytes) :
	m_pStats(pStats),
	m_pRenderer(pSystem->GetIRenderer()),
	m_pLog(pSystem->GetILog()),
	m_pTextModeConsole(pSystem->GetITextModeConsole()),
	m_nMinSubcomponentBytes(nMinSubcomponentBytes < 0 || nMinSubcomponentBytes > 0x10000000 ? 0x8000 : nMinSubcomponentBytes),
	m_nMaxSubcomponentDepth(nMaxSubcomponentDepth)
{

}


#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

static void DrawStatsText(float x, float y, float fScale, float color[4], tukk format, ...)
{
	va_list args;
	va_start(args, format);
	IRenderAuxText::DrawText(Vec3(x, y, 0.5f), fScale, color, eDrawText_2D | eDrawText_FixedSize | eDrawText_Monospace, format, args);
	va_end(args);
}

void DrxSizerStatsRenderer::render(bool bRefreshMark)
{
	if (!m_pStats->size())
		return;

	// left coordinate of the text
	unsigned nNameWidth = (unsigned)(m_pStats->getMaxNameLength() + 1);
	if (nNameWidth < 25)
		nNameWidth = 25;
	float fCharScaleX = 1.2f;
	float fLeft = 0;
	float fTop = 8;
	float fVStep = 9;

#ifdef _DEBUG
	tukk szCountStr1 = "count";
	tukk szCountStr2 = "_____";
#else // _DEBUG
	tukk szCountStr1 = "", * szCountStr2 = "";
#endif // _DEBUG

	float fTextColor[4] = { 0.9f, 0.85f, 1, 0.85f };
	DrawStatsText(fLeft, fTop, fCharScaleX, fTextColor,
	              "%-*s   total  partial  %s", nNameWidth, bRefreshMark ? "Memory usage (refresh*)" : "Memory usage (refresh )", szCountStr1);
	DrawStatsText(fLeft, fTop + fVStep * 0.25f, fCharScaleX, fTextColor,
	              "%*s   _____   _______  %s", nNameWidth, "", szCountStr2);

	unsigned nSubgroupDepth = 1;

	// different colors used to paint the statistics subgroups
	// a new statistic subgroup starts with a new subtree of depth <= specified
	float fGray = 0;//0.45f;
	float fLightGray = 0.5f;//0.8f;
	float fColors[] =
	{
		fLightGray, fLightGray, fGray, 1,
		1,          1,          1,     1,
		fGray,      1,          1,     1,
		1,          fGray,      1,     1,
		1,          1,          fGray, 1,
		fGray,      fLightGray, 1,     1,
		fGray,      1,          fGray, 1,
		1,          fGray,      fGray, 1
	};
	float* pColor = fColors;

	unsigned statSize = m_pStats->size();
	unsigned startRow = m_pStats->row();
	unsigned i = 0;
	for (; i < startRow; ++i)
	{
		const Component& rComp = (*m_pStats)[i];
		if (rComp.nDepth <= nSubgroupDepth)
		{
			//switch the color
			pColor += 4;
			if (pColor >= fColors + DRX_ARRAY_COUNT(fColors))
				pColor = fColors;

			fTop += fVStep * (0.333333f + (nSubgroupDepth - rComp.nDepth) * 0.15f);
		}
	}

	for (unsigned r = startRow; i < statSize; ++i)
	{
		const Component& rComp = (*m_pStats)[i];
		if (rComp.nDepth <= nSubgroupDepth)
		{
			//switch the color
			pColor += 4;
			if (pColor >= fColors + DRX_ARRAY_COUNT(fColors))
				pColor = fColors;

			fTop += fVStep * (0.333333f + (nSubgroupDepth - rComp.nDepth) * 0.15f);
		}

		if (rComp.sizeBytesTotal <= m_nMinSubcomponentBytes || rComp.nDepth > m_nMaxSubcomponentDepth)
			continue;

		fTop += fVStep;

		char szDepth[32] = " ..............................";
		if (rComp.nDepth < sizeof(szDepth))
			szDepth[rComp.nDepth] = '\0';

		char szSize[32];
		if (rComp.sizeBytes > 0)
		{
			if (rComp.sizeBytesTotal > rComp.sizeBytes)
				drx_sprintf(szSize, "%7.3f  %7.3f", rComp.getTotalSizeMBytes(), rComp.getSizeMBytes());
			else
				drx_sprintf(szSize, "         %7.3f", rComp.getSizeMBytes());
		}
		else
		{
			assert(rComp.sizeBytesTotal > 0);
			drx_sprintf(szSize, "%7.3f         ", rComp.getTotalSizeMBytes());
		}
		char szCount[16];
#ifdef _DEBUG
		if (rComp.numObjects)
			drx_sprintf(szCount, "%8" PRISIZE_T, rComp.numObjects);
		else
#endif
		szCount[0] = '\0';

		DrawStatsText(fLeft, fTop, fCharScaleX, pColor,
		              "%s%-*s:%s%s", szDepth, nNameWidth - rComp.nDepth, rComp.strName.c_str(), szSize, szCount);

		if (m_pTextModeConsole)
		{
			string text;
			text.Format("%s%-*s:%s%s", szDepth, nNameWidth - rComp.nDepth, rComp.strName.c_str(), szSize, szCount);
			m_pTextModeConsole->PutText(0, r++, text.c_str());
		}
	}

	float fLTGrayColor[4] = { fLightGray, fLightGray, fLightGray, 1.0f };
	fTop += 0.25f * fVStep;
	DrawStatsText(fLeft, fTop, fCharScaleX, fLTGrayColor,
	              "%-*s %s", nNameWidth, "___________________________", "________________");
	fTop += fVStep;

	tukk szOverheadNames[DrxSizerStats::g_numTimers] =
	{
		".Collection",
		".Transformation",
		".Cleanup"
	};
	bool bOverheadsHeaderPrinted = false;
	for (i = 0; i < DrxSizerStats::g_numTimers; ++i)
	{
		float fTime = m_pStats->getTime(i);
		if (fTime < 20)
			continue;
		// print the header
		if (!bOverheadsHeaderPrinted)
		{
			DrawStatsText(fLeft, fTop, fCharScaleX, fTextColor,
			              "%-*s", nNameWidth, "Overheads");
			fTop += fVStep;
			bOverheadsHeaderPrinted = true;
		}

		DrawStatsText(fLeft, fTop, fCharScaleX, fTextColor,
		              "%-*s:%7.1f ms", nNameWidth, szOverheadNames[i], fTime);
		fTop += fVStep;
	}
}

void DrxSizerStatsRenderer::dump(bool bUseKB)
{
	if (!m_pStats->size())
		return;

	unsigned nNameWidth = (unsigned)(m_pStats->getMaxNameLength() + 1);

	// left coordinate of the text
	m_pLog->LogToFile("Memory Statistics: %s", bUseKB ? "KB" : "MB");
	m_pLog->LogToFile("%-*s   TOTAL   partial  count", nNameWidth, "");

	// different colors used to paint the statistics subgroups
	// a new statistic subgroup starts with a new subtree of depth <= specified

	for (unsigned i = 0; i < m_pStats->size(); ++i)
	{
		const Component& rComp = (*m_pStats)[i];

		if (rComp.sizeBytesTotal <= m_nMinSubcomponentBytes || rComp.nDepth > m_nMaxSubcomponentDepth)
			continue;

		char szDepth[32] = " ..............................";
		if (rComp.nDepth < sizeof(szDepth))
			szDepth[rComp.nDepth] = '\0';

		char szSize[32];
		if (rComp.sizeBytes > 0)
		{
			if (rComp.sizeBytesTotal > rComp.sizeBytes)
				drx_sprintf(szSize, bUseKB ? "%7.2f  %7.2f" : "%7.3f  %7.3f", bUseKB ? rComp.getTotalSizeKBytes() : rComp.getTotalSizeMBytes(), bUseKB ? rComp.getSizeKBytes() : rComp.getSizeMBytes());
			else
				drx_sprintf(szSize, bUseKB ? "         %7.2f" : "         %7.3f", bUseKB ? rComp.getSizeKBytes() : rComp.getSizeMBytes());
		}
		else
		{
			assert(rComp.sizeBytesTotal > 0);
			drx_sprintf(szSize, bUseKB ? "%7.2f         " : "%7.3f         ", bUseKB ? rComp.getTotalSizeKBytes() : rComp.getTotalSizeMBytes());
		}
		char szCount[16];

		if (rComp.numObjects)
			drx_sprintf(szCount, "%8u", (u32)rComp.numObjects);
		else
			szCount[0] = '\0';

		m_pLog->LogToFile("%s%-*s:%s%s", szDepth, nNameWidth - rComp.nDepth, rComp.strName.c_str(), szSize, szCount);
	}
}

void DrxSizerStats::startTimer(unsigned nTimer, ITimer* pTimer)
{
	assert(nTimer < g_numTimers);
	m_fTime[nTimer] = pTimer->GetAsyncCurTime();
}

void DrxSizerStats::stopTimer(unsigned nTimer, ITimer* pTimer)
{
	assert(nTimer < g_numTimers);
	m_fTime[nTimer] = 1000 * (pTimer->GetAsyncCurTime() - m_fTime[nTimer]);
}
