// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 09:05:2005   14:32 : Created by Carsten Wenzel

*************************************************************************/

#ifndef _SKY_LIGHT_MANAGER_H_
#define _SKY_LIGHT_MANAGER_H_

#pragma once

#include <memory>
#include <vector>

#include <drx3D/CoreX/optional.h>

class CSkyLightNishita;
struct ITimer;

class DRX_ALIGN(128) CSkyLightUpr: public DinrusX3dEngBase
{
public:
	struct DRX_ALIGN(16) SSkyDomeCondition
	{
		SSkyDomeCondition()
			: m_sunIntensity(20.0f, 20.0f, 20.0f)
			  , m_Km(0.001f)
			  , m_Kr(0.00025f)
			  , m_g(-0.99f)
			  , m_rgbWaveLengths(650.0f, 570.0f, 475.0f)
			  , m_sunDirection(0.0f, 0.707106f, 0.707106f)
		{
		}

		Vec3 m_sunIntensity;
		float m_Km;
		float m_Kr;
		float m_g;
		Vec3 m_rgbWaveLengths;
		Vec3 m_sunDirection;
	};

public:
	CSkyLightUpr();
	~CSkyLightUpr();

	// sky dome condition
	void SetSkyDomeCondition(const SSkyDomeCondition &skyDomeCondition);
	void GetCurSkyDomeCondition(SSkyDomeCondition & skyCond) const;

	// controls updates
	void FullUpdate();
	void IncrementalUpdate(f32 updateRatioPerFrame, const SRenderingPassInfo &passInfo);
	void SetQuality(i32 quality);

	// rendering params
	const SSkyLightRenderParams* GetRenderParams() const;

	void GetMemoryUsage(IDrxSizer * pSizer) const;

	void InitSkyDomeMesh();
	void ReleaseSkyDomeMesh();

	void UpdateInternal(i32 newFrameID, i32 numUpdates, i32 callerIsFullUpdate = 0);
private:
	typedef std::vector<DrxHalf4> SkyDomeTextureData;

private:
	bool IsSkyDomeUpdateFinished() const;

	i32 GetFrontBuffer() const;
	i32 GetBackBuffer() const;
	void ToggleBuffer();
	void UpdateRenderParams();
	void PushUpdateParams();

private:
	SSkyDomeCondition m_curSkyDomeCondition;      //current sky dome conditions
	SSkyDomeCondition m_reqSkyDomeCondition[2];   //requested sky dome conditions, double buffered(engine writes async)
	SSkyDomeCondition m_updatingSkyDomeCondition; //sky dome conditions the update is currently processed with
	i32 m_updateRequested[2];                     //true if an update is requested, double buffered(engine writes async)
	CSkyLightNishita* m_pSkyLightNishita;

	SkyDomeTextureData m_skyDomeTextureDataMie[2];
	SkyDomeTextureData m_skyDomeTextureDataRayleigh[2];
	i32 m_skyDomeTextureTimeStamp[2];

	bool m_bFlushFullUpdate;

	_smart_ptr<IRenderMesh> m_pSkyDomeMesh;

	i32 m_numSkyDomeColorsComputed;
	i32 m_curBackBuffer;

	stl::optional<i32> m_lastFrameID;
	i32 m_needRenderParamUpdate;

	Vec3 m_curSkyHemiColor[5];
	Vec3 m_curHazeColor;
	Vec3 m_curHazeColorMieNoPremul;
	Vec3 m_curHazeColorRayleighNoPremul;

	Vec3 m_skyHemiColorAccum[5];
	Vec3 m_hazeColorAccum;
	Vec3 m_hazeColorMieNoPremulAccum;
	Vec3 m_hazeColorRayleighNoPremulAccum;

	DRX_ALIGN(16) SSkyLightRenderParams m_renderParams;
};

#endif // #ifndef _SKY_LIGHT_MANAGER_H_
