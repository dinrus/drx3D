// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef D3D_COLOR_GRADING_CONTROLLER_H
#define D3D_COLOR_GRADING_CONTROLLER_H

#pragma once

#include <drx3D/Render/IColorGradingControllerInt.h>

class CD3D9Renderer;

struct SColorGradingMergeParams
{
	Vec4   pColorMatrix[3];
	Vec4   pLevels[2];
	Vec4   pFilterColor;
	Vec4   pSelectiveColor[2];
	uint64 nFlagsShaderRT;
};

class CColorGradingController : public IColorGradingControllerInt
{
	friend class CColorGradingStage;

public:
	// IColorGradingController interface
	virtual i32  LoadColorChart(tukk pChartFilePath) const;
	virtual void UnloadColorChart(i32 texID) const;

	virtual void SetLayers(const SColorChartLayer* pLayers, u32 numLayers);

public:
	CColorGradingController();
	virtual ~CColorGradingController();

	bool            Update(const SColorGradingMergeParams* pMergeParams = 0);
	CTexture*       GetColorChart() const;
	void            DrawDebugInfo() const;

	bool            LoadStaticColorChart(tukk pChartFilePath);
	const CTexture* GetStaticColorChart() const;

	void            FreeMemory();

	i32             GetColorChartSize() const;

	DrxCriticalSectionNonRecursive& GetLayersLock() { return m_layersLock; };

private:
	typedef std::vector<SColorChartLayer> Layers;

private:
	bool      ValidateColorChart(const CTexture* pChart) const;
	CTexture* LoadColorChartInt(tukk pChartFilePath) const;
	bool      InitResources();
	void      DrawLayer(float x, float y, float w, float h, CTexture* pChart, float blendAmount, tukk pLayerName) const;

private:
	Layers                         m_layers;
	DrxCriticalSectionNonRecursive m_layersLock;

	// TODO: remove once we don't need r_graphicspipeline=0 anymore
	CTexture*                    m_pChartIdentity;
	CTexture*                    m_pChartStatic;
	CTexture*                    m_pChartToUse;
	CTexture*                    m_pMergeLayers[2];
};

#endif // #ifndef D3D_COLOR_GRADING_CONTROLLER_H
