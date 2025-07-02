// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Util/GdiUtil.h"

class PLUGIN_API CImageHistogramCtrl : public CWnd
{
	DECLARE_DYNAMIC(CImageHistogramCtrl)

public:

	static i32k kNumChannels = 4;
	static i32k kNumColorLevels = 256;

	enum EImageFormat
	{
		eImageFormat_8BPP,
		eImageFormat_24BPP_RGB,
		eImageFormat_24BPP_BGR,
		eImageFormat_32BPP_RGBA,
		eImageFormat_32BPP_BGRA,
		eImageFormat_32BPP_ARGB,
		eImageFormat_32BPP_ABGR
	};

	enum EDrawMode
	{
		eDrawMode_Luminosity,
		eDrawMode_OverlappedRGB,
		eDrawMode_SplitRGB,
		eDrawMode_RedChannel,
		eDrawMode_GreenChannel,
		eDrawMode_BlueChannel,
		eDrawMode_AlphaChannel
	};

	CImageHistogramCtrl();
	virtual ~CImageHistogramCtrl();

	// Description:
	//	Compute the histogram of an image
	// Arguments:
	//	pImageData - the image data
	//	aWidth - the width of the image in pixels
	//	aHeight - the height of the image in pixels
	//	aBitsPerPixel - the number of bits per pixel, currently supported: 8 (monochrome), 24 (RGB) and 32 (RGBA)
	void ComputeHistogram(BYTE* pImageData, UINT aWidth, UINT aHeight, EImageFormat aFormat = eImageFormat_32BPP_RGBA);
	void ClearHistogram();
	void CopyComputedDataFrom(CImageHistogramCtrl* pCtrl);

protected:

	void ComputeStatisticsForChannel(i32 aIndex);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

	CGdiCanvas   m_canvas;
	i32          m_graphMargin;
	float        m_graphHeightPercent;
	UINT         m_count[kNumChannels][kNumColorLevels];
	UINT         m_lumCount[kNumColorLevels];
	UINT         m_maxCount[kNumChannels];
	UINT         m_maxLumCount;
	float        m_mean[kNumChannels], m_stdDev[kNumChannels], m_median[kNumChannels];
	float        m_meanAvg, m_stdDevAvg, m_medianAvg;
	EDrawMode    m_drawMode;
	EImageFormat m_imageFormat;
	COLORREF     m_backColor;

	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

