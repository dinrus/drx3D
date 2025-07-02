// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************

-------------------------------------------------------------------------
История:
- 28:05:2009   Created by Federico Rebora

*************************************************************************/

#pragma once

#ifndef MOCK_COLOR_GRADING_CONTROLLER_H_INCLUDED
#define MOCK_COLOR_GRADING_CONTROLLER_H_INCLUDED

#include <drx3D/Game/EngineFacade/3DEngine.h>

namespace GameTesting
{
	class CMockEngineColorGradingController : public EngineFacade::CNullEngineColorGradingController
	{
	public:
		CMockEngineColorGradingController();

		virtual i32 LoadColorChart(tukk colorChartFilePath) const;

		virtual void UnloadColorChart(i32 textureID) const;

		void SetLayers(const SColorChartLayer* layers, u32 numLayers);

		bool AnyChartsWereLoaded() const;

		i32 GetLastUnloadedTextureID() const;

		void ClearLoadedCharts();

		const string& GetPathOfLastLoadedColorChart() const;

		const string& GetPathOfLoadedColorChart(u32k index) const;

		bool LayersHaveBeenSet() const;

		u32 GetLayerCount() const;

		size_t GetNumberOfLayersSet() const;

		const SColorChartLayer& GetCurrentlySetLayerByIndex(u32k index) const;

		void SetFakeIDForLoadedTexture(i32k id);

		bool WasNullPointerSetOnLayers() const;

	private:
		mutable std::vector<string> m_loadedChartsPaths;
		bool m_wasNullPointerSetOnLayers;
		std::vector<SColorChartLayer> m_currentLayers;
		i32 m_fakeIDForLoadedTexture;
		mutable i32 m_lastUnloadedTextureID;
	};
}

#endif
