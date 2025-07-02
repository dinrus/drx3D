// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************

-------------------------------------------------------------------------
История:
- 28:05:2009   Created by Federico Rebora

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/MockColorGradingController.h>

#include <drx3D/CoreX/Renderer/IColorGradingController.h>

namespace GameTesting
{
	CMockEngineColorGradingController::CMockEngineColorGradingController() : m_wasNullPointerSetOnLayers(false)
		, m_fakeIDForLoadedTexture(0)
		, m_lastUnloadedTextureID(-1)
	{

	}

	i32 CMockEngineColorGradingController::LoadColorChart( tukk colorChartFilePath ) const
	{
		m_loadedChartsPaths.push_back(colorChartFilePath);

		return m_fakeIDForLoadedTexture;
	}

	void CMockEngineColorGradingController::UnloadColorChart( i32 textureID ) const
	{
		m_lastUnloadedTextureID = textureID;
	}

	void CMockEngineColorGradingController::SetLayers( const SColorChartLayer* layers, u32 numLayers )
	{
		if (layers == 0)
		{
			m_wasNullPointerSetOnLayers = true;
		}

		m_currentLayers.clear();

		for (u32 layerIndex = 0; layerIndex < numLayers; ++layerIndex)
		{
			m_currentLayers.push_back(layers[layerIndex]);
		}
	}

	bool CMockEngineColorGradingController::AnyChartsWereLoaded() const
	{
		return !m_loadedChartsPaths.empty();
	}

	i32 CMockEngineColorGradingController::GetLastUnloadedTextureID() const
	{
		return m_lastUnloadedTextureID;
	}

	void CMockEngineColorGradingController::ClearLoadedCharts()
	{
		m_loadedChartsPaths.clear();
	}

	const string& CMockEngineColorGradingController::GetPathOfLastLoadedColorChart() const
	{
		if (!AnyChartsWereLoaded())
		{
			static const string emptyString("");
			return emptyString;
		}

		return m_loadedChartsPaths.back();
	}

	const string& CMockEngineColorGradingController::GetPathOfLoadedColorChart( u32k index ) const
	{
		if (index >= m_loadedChartsPaths.size())
		{
			static const string emptyString("");
			return emptyString;
		}
		return m_loadedChartsPaths[index];
	}

	bool CMockEngineColorGradingController::LayersHaveBeenSet() const
	{
		return !m_currentLayers.empty();
	}

	u32 CMockEngineColorGradingController::GetLayerCount() const
	{
		return m_currentLayers.size();
	}

	size_t CMockEngineColorGradingController::GetNumberOfLayersSet() const
	{
		return m_currentLayers.size();
	}

	const SColorChartLayer& CMockEngineColorGradingController::GetCurrentlySetLayerByIndex( u32k index ) const
	{
		return m_currentLayers[index];
	}

	void CMockEngineColorGradingController::SetFakeIDForLoadedTexture( i32k id )
	{
		m_fakeIDForLoadedTexture = id;
	}

	bool CMockEngineColorGradingController::WasNullPointerSetOnLayers() const
	{
		return m_wasNullPointerSetOnLayers;
	}
}
