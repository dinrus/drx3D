// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StatsUpr.cpp
//  Created:     15/09/2008 by Matthew
//  Описание: Stores global AI statistics and provides display/output methods
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/StatsUpr.h>
#include <drx3D/AI/DebugDrawContext.h>

// Useful formatting strings
// 30 characters total are allowed for this in the standard view!
// 20 (max) of those are usually used for the short description
const static tuk sFmtDefault = "%s: %3.1f";

void CStatsUpr::InitDescriptions()
{
	// The list of statistics metadata
	static const SStatMetadata sMetadata[] = {
		{ "Enabled AI Actors",   eSF_FrameReset | eSF_Integer, 0 },
		{ "AI full updates",     eSF_FrameReset,               0 },
		{ "TPS queries (sync)",  eSF_FrameReset,               0 },
		{ "TPS queries (async)", eSF_FrameReset,               0 },
	};

	// Make sure that the number of descriptions never varies from the size of the enum
	// That should be enough to avoid descriptions and enums from getting out of step
	PREFAST_SUPPRESS_WARNING(6326) static_assert(DRX_ARRAY_COUNT(sMetadata) == eStat_Last, "Unexpected array size!");

	m_sMetadata = sMetadata;
}

CStatsUpr::CStatsUpr(void)
{
	m_fValues = new float[eStat_Last];

	InitDescriptions();

	Reset(eStatReset_All);
}

CStatsUpr::~CStatsUpr(void)
{
	delete[] m_fValues;
}

void CStatsUpr::Reset(EStatsReset eReset)
{
	// Reset some or all of the stats values
	for (u32 i = 0; i < eStat_Last; i++)
	{
		switch (eReset)
		{
		case eStatReset_Frame:
			if (!(m_sMetadata[i].flags & eSF_FrameReset))
				break;
		// Otherwise, fall-through
		case eStatReset_All:
			m_fValues[i] = 0.0f;
		}
	}
}

void CStatsUpr::Render()
{
	CDebugDrawContext dc;

	static float xPos = 78.0f;
	static float yPos = 45.0f;
	static float yStep = 2.0;

	if (gAIEnv.CVars.StatsDisplayMode == 0)
		return;

	float yOffset = 0.0f;

	const char sTitle[30] = "------- AI Statistics -------";
	dc->TextToScreen(xPos, yPos, sTitle);
	yOffset += yStep;

	for (u32 i = 0; i < eStat_Last; i++, yOffset += yStep)
	{
		const SStatMetadata& metadata = m_sMetadata[i];
		tukk sFormSpec = metadata.format ? metadata.format : sFmtDefault;
		dc->TextToScreen(xPos, yPos + yOffset, sFormSpec, metadata.description, m_fValues[i]);
	}
}
