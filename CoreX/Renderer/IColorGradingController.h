// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _I_COLOR_GRADING_CONTROLLER_H_
#define _I_COLOR_GRADING_CONTROLLER_H_

#pragma once

struct SColorChartLayer
{
	i32   m_texID;
	float m_blendAmount;

	SColorChartLayer()
		: m_texID(-1)
		, m_blendAmount(-1)
	{
	}

	SColorChartLayer(i32 texID, float blendAmount)
		: m_texID(texID)
		, m_blendAmount(blendAmount)
	{
	}

	SColorChartLayer(const SColorChartLayer& rhs)
		: m_texID(rhs.m_texID)
		, m_blendAmount(rhs.m_blendAmount)
	{
	}
};

struct IColorGradingController
{
public:
	// <interfuscator:shuffle>
	virtual ~IColorGradingController(){}
	virtual i32  LoadColorChart(tukk pChartFilePath) const = 0;
	virtual void UnloadColorChart(i32 texID) const = 0;

	virtual void SetLayers(const SColorChartLayer* pLayers, u32 numLayers) = 0;
	// </interfuscator:shuffle>
};

#endif //#ifndef _I_COLOR_GRADING_CONTROLLER_H_
