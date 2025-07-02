// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 09:05:2005   14:32 : Created by Carsten Wenzel

*************************************************************************/

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/SkyLightUpr.h>
#include <drx3D/Eng3D/SkyLightNishita.h>
#include <drx3D/CoreX/Renderer/RendElements/CRESky.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

DECLARE_JOB("SkyUpdate", TSkyJob, CSkyLightUpr::UpdateInternal);
static JobUpr::SJobState g_JobState;

CSkyLightUpr::CSkyLightUpr()
	: m_pSkyLightNishita(DrxAlignedNew<CSkyLightNishita>())
	, m_pSkyDomeMesh(0)
	, m_curSkyDomeCondition()
	, m_updatingSkyDomeCondition()
	, m_numSkyDomeColorsComputed(SSkyLightRenderParams::skyDomeTextureSize)
	, m_curBackBuffer(0)
	, m_curSkyHemiColor()
	, m_curHazeColor(0.0f, 0.0f, 0.0f)
	, m_curHazeColorMieNoPremul(0.0f, 0.0f, 0.0f)
	, m_curHazeColorRayleighNoPremul(0.0f, 0.0f, 0.0f)
	, m_skyHemiColorAccum()
	, m_hazeColorAccum(0.0f, 0.0f, 0.0f)
	, m_hazeColorMieNoPremulAccum(0.0f, 0.0f, 0.0f)
	, m_hazeColorRayleighNoPremulAccum(0.0f, 0.0f, 0.0f)
	, m_bFlushFullUpdate(false)
	, m_renderParams()
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "SkyLightUpr");

	if (GetRenderer())
	{
		InitSkyDomeMesh();
	}

	m_updateRequested[0] = m_updateRequested[1] = 0;

	// init textures with default data
	m_skyDomeTextureDataMie[0].resize(SSkyLightRenderParams::skyDomeTextureSize, DrxHalf4{ .0f,.0f,.0f,.0f });
	m_skyDomeTextureDataMie[1].resize(SSkyLightRenderParams::skyDomeTextureSize, DrxHalf4{ .0f,.0f,.0f,.0f });
	m_skyDomeTextureDataRayleigh[0].resize(SSkyLightRenderParams::skyDomeTextureSize, DrxHalf4{ .0f,.0f,.0f,.0f });
	m_skyDomeTextureDataRayleigh[1].resize(SSkyLightRenderParams::skyDomeTextureSize, DrxHalf4{ .0f,.0f,.0f,.0f });

	// init time stamps
	m_skyDomeTextureTimeStamp[0] = gEnv->nMainFrameID;
	m_skyDomeTextureTimeStamp[1] = gEnv->nMainFrameID;

	// init sky hemisphere colors and accumulators
	memset(m_curSkyHemiColor, 0, sizeof(m_curSkyHemiColor));
	memset(m_skyHemiColorAccum, 0, sizeof(m_skyHemiColorAccum));

	// set default render parameters
	UpdateRenderParams();
}

CSkyLightUpr::~CSkyLightUpr()
{
	DrxAlignedDelete(m_pSkyLightNishita);
}

inline static void Sync()
{
	gEnv->GetJobUpr()->WaitForJob(g_JobState);
}

void CSkyLightUpr::PushUpdateParams()
{
	//pushes the update parameters, explicite call since engine requests asynchronously
	memcpy(&m_reqSkyDomeCondition[0], &m_reqSkyDomeCondition[1], sizeof(SSkyDomeCondition));
	m_updateRequested[0] = m_updateRequested[1];
	m_updateRequested[1] = 0;
}

void CSkyLightUpr::SetSkyDomeCondition(const SSkyDomeCondition& skyDomeCondition)
{
	m_reqSkyDomeCondition[1] = skyDomeCondition;
	m_updateRequested[1] = 1;
}

void CSkyLightUpr::FullUpdate()
{
	Sync();
	PushUpdateParams();
	TSkyJob job(GetRenderer()->GetFrameID(false), SSkyLightRenderParams::skyDomeTextureSize, (i32)1);
	job.SetClassInstance(this);
	job.RegisterJobState(&g_JobState);
	job.Run();
	m_needRenderParamUpdate = true;
	m_bFlushFullUpdate = true;
}

void CSkyLightUpr::IncrementalUpdate(f32 updateRatioPerFrame, const SRenderingPassInfo& passInfo)
{
	Sync();

	FUNCTION_PROFILER_3DENGINE;

	if (!m_lastFrameID)
	{
		FullUpdate();
		return;
	}

	// get current ID of "main" frame (no recursive rendering),
	// incremental update should only be processed once per frame
	if (m_lastFrameID.value() != passInfo.GetMainFrameID())
	{
		i32 numUpdate((i32) ((f32) SSkyLightRenderParams::skyDomeTextureSize * updateRatioPerFrame / 100.0f + 0.5f));
		numUpdate = clamp_tpl(numUpdate, 1, SSkyLightRenderParams::skyDomeTextureSize);
		if (m_needRenderParamUpdate)
			UpdateRenderParams();     // update render params

		PushUpdateParams();

		TSkyJob job(passInfo.GetMainFrameID(), numUpdate, (i32)0);
		job.SetClassInstance(this);
		job.RegisterJobState(&g_JobState);
		job.Run();
	}
}

void CSkyLightUpr::UpdateInternal(i32 newFrameID, i32 numUpdates, i32 callerIsFullUpdate)
{
	FUNCTION_PROFILER_3DENGINE;

	// update sky dome if requested -- requires last update request to be fully processed!
	i32 procUpdate = callerIsFullUpdate;
	procUpdate |= (i32)IsSkyDomeUpdateFinished();
	procUpdate &= m_updateRequested[0];
	if (procUpdate)
	{
		// set sky dome settings
		memcpy(&m_updatingSkyDomeCondition, &m_reqSkyDomeCondition[0], sizeof(SSkyDomeCondition));
		m_pSkyLightNishita->SetSunDirection(m_updatingSkyDomeCondition.m_sunDirection);
		m_pSkyLightNishita->SetRGBWaveLengths(m_updatingSkyDomeCondition.m_rgbWaveLengths);
		m_pSkyLightNishita->SetAtmosphericConditions(m_updatingSkyDomeCondition.m_sunIntensity,
		                                             1e-4f * m_updatingSkyDomeCondition.m_Km, 1e-4f * m_updatingSkyDomeCondition.m_Kr, m_updatingSkyDomeCondition.m_g); // scale mie and rayleigh scattering for more convenient editing in time of day dialog

		// update request has been accepted
		m_updateRequested[0] = 0;
		m_numSkyDomeColorsComputed = 0;

		// reset sky & haze color accumulator
		m_hazeColorAccum = Vec3(0.0f, 0.0f, 0.0f);
		m_hazeColorMieNoPremulAccum = Vec3(0.0f, 0.0f, 0.0f);
		m_hazeColorRayleighNoPremulAccum = Vec3(0.0f, 0.0f, 0.0f);
		memset(m_skyHemiColorAccum, 0, sizeof(m_skyHemiColorAccum));
	}

	// any work to do?
	if (false == IsSkyDomeUpdateFinished())
	{
		if (numUpdates <= 0)
		{
			// do a full update
			numUpdates = SSkyLightRenderParams::skyDomeTextureSize;
		}

		// find minimally required work load for this incremental update
		numUpdates = min(SSkyLightRenderParams::skyDomeTextureSize - m_numSkyDomeColorsComputed, numUpdates);

		// perform color computations
		SkyDomeTextureData& skyDomeTextureDataMie(m_skyDomeTextureDataMie[GetBackBuffer()]);
		SkyDomeTextureData& skyDomeTextureDataRayleigh(m_skyDomeTextureDataRayleigh[GetBackBuffer()]);

		i32 numSkyDomeColorsComputed(m_numSkyDomeColorsComputed);
		for (; numUpdates > 0; --numUpdates, ++numSkyDomeColorsComputed)
		{
			// calc latitude/longitude
			i32 lon(numSkyDomeColorsComputed / SSkyLightRenderParams::skyDomeTextureWidth);
			i32 lat(numSkyDomeColorsComputed % SSkyLightRenderParams::skyDomeTextureWidth);

			float lonArc(DEG2RAD((float) lon * 90.0f / (float) SSkyLightRenderParams::skyDomeTextureHeight));
			float latArc(DEG2RAD((float) lat * 360.0f / (float) SSkyLightRenderParams::skyDomeTextureWidth));

			float sinLon(0);
			float cosLon(0);
			sincos_tpl(lonArc, &sinLon, &cosLon);
			float sinLat(0);
			float cosLat(0);
			sincos_tpl(latArc, &sinLat, &cosLat);

			// calc sky direction for given update latitude/longitude (hemisphere)
			Vec3 skyDir(sinLon * cosLat, sinLon * sinLat, cosLon);

			// compute color
			//Vec3 skyColAtDir( 0.0, 0.0, 0.0 );
			Vec3 skyColAtDirMieNoPremul(0.0, 0.0, 0.0);
			Vec3 skyColAtDirRayleighNoPremul(0.0, 0.0, 0.0);
			Vec3 skyColAtDirRayleigh(0.0, 0.0, 0.0);

			m_pSkyLightNishita->ComputeSkyColor(skyDir, 0, &skyColAtDirMieNoPremul, &skyColAtDirRayleighNoPremul, &skyColAtDirRayleigh);

			// store color in texture
			skyDomeTextureDataMie[numSkyDomeColorsComputed] = DrxHalf4(skyColAtDirMieNoPremul.x, skyColAtDirMieNoPremul.y, skyColAtDirMieNoPremul.z, 1.0f);
			skyDomeTextureDataRayleigh[numSkyDomeColorsComputed] = DrxHalf4(skyColAtDirRayleighNoPremul.x, skyColAtDirRayleighNoPremul.y, skyColAtDirRayleighNoPremul.z, 1.0f);

			// update haze color accum (accumulate second last sample row)
			if (lon == SSkyLightRenderParams::skyDomeTextureHeight - 2)
			{
				m_hazeColorAccum += skyColAtDirRayleigh;
				m_hazeColorMieNoPremulAccum += skyColAtDirMieNoPremul;
				m_hazeColorRayleighNoPremulAccum += skyColAtDirRayleighNoPremul;
			}

			// update sky hemisphere color accumulator
			i32 y(lon >> SSkyLightRenderParams::skyDomeTextureHeightBy2Log);
			i32 x(((lat + SSkyLightRenderParams::skyDomeTextureWidthBy8) & (SSkyLightRenderParams::skyDomeTextureWidth - 1)) >> SSkyLightRenderParams::skyDomeTextureWidthBy4Log);
			i32 skyHemiColAccumIdx(x * y + y);
			assert(((u32)skyHemiColAccumIdx) < 5);
			m_skyHemiColorAccum[skyHemiColAccumIdx] += skyColAtDirRayleigh;
		}

		m_numSkyDomeColorsComputed = numSkyDomeColorsComputed;

		// sky dome update finished?
		if (false != IsSkyDomeUpdateFinished())
		{
			// update time stamp
			m_skyDomeTextureTimeStamp[GetBackBuffer()] = newFrameID;

			// get new haze color
			const float c_invNumHazeSamples(1.0f / (float) SSkyLightRenderParams::skyDomeTextureWidth);
			m_curHazeColor = m_hazeColorAccum * c_invNumHazeSamples;
			m_curHazeColorMieNoPremul = m_hazeColorMieNoPremulAccum * c_invNumHazeSamples;
			m_curHazeColorRayleighNoPremul = m_hazeColorRayleighNoPremulAccum * c_invNumHazeSamples;

			// get new sky hemisphere colors
			const float c_scaleHemiTop(2.0f / (SSkyLightRenderParams::skyDomeTextureWidth * SSkyLightRenderParams::skyDomeTextureHeight));
			const float c_scaleHemiSide(8.0f / (SSkyLightRenderParams::skyDomeTextureWidth * SSkyLightRenderParams::skyDomeTextureHeight));
			m_curSkyHemiColor[0] = m_skyHemiColorAccum[0] * c_scaleHemiTop;
			m_curSkyHemiColor[1] = m_skyHemiColorAccum[1] * c_scaleHemiSide;
			m_curSkyHemiColor[2] = m_skyHemiColorAccum[2] * c_scaleHemiSide;
			m_curSkyHemiColor[3] = m_skyHemiColorAccum[3] * c_scaleHemiSide;
			m_curSkyHemiColor[4] = m_skyHemiColorAccum[4] * c_scaleHemiSide;

			// toggle sky light buffers
			ToggleBuffer();
		}
	}

	// update frame ID
	m_lastFrameID = newFrameID;
}

void CSkyLightUpr::SetQuality(i32 quality)
{
	if (quality != m_pSkyLightNishita->GetInScatteringIntegralStepSize())
	{
		Sync();
		// when setting new quality we need to start sky dome update from scratch...
		// ... to avoid "artifacts" in the resulting texture
		m_numSkyDomeColorsComputed = 0;
		m_pSkyLightNishita->SetInScatteringIntegralStepSize(quality);
	}
}

const SSkyLightRenderParams* CSkyLightUpr::GetRenderParams() const
{
	return &m_renderParams;
}

void CSkyLightUpr::UpdateRenderParams()
{
	// sky dome mesh data
	m_renderParams.m_pSkyDomeMesh = m_pSkyDomeMesh;

	// sky dome texture access
	m_renderParams.m_skyDomeTextureTimeStamp = m_skyDomeTextureTimeStamp[GetFrontBuffer()];
	m_renderParams.m_pSkyDomeTextureDataMie = (ukk ) &m_skyDomeTextureDataMie[GetFrontBuffer()][0];
	m_renderParams.m_pSkyDomeTextureDataRayleigh = (ukk ) &m_skyDomeTextureDataRayleigh[GetFrontBuffer()][0];
	m_renderParams.m_skyDomeTexturePitch = SSkyLightRenderParams::skyDomeTextureWidth * sizeof(DrxHalf4);

	// shader constants for final per-pixel phase computation
	m_renderParams.m_partialMieInScatteringConst = m_pSkyLightNishita->GetPartialMieInScatteringConst();
	m_renderParams.m_partialRayleighInScatteringConst = m_pSkyLightNishita->GetPartialRayleighInScatteringConst();
	Vec3 sunDir(m_pSkyLightNishita->GetSunDirection());
	m_renderParams.m_sunDirection = Vec4(sunDir.x, sunDir.y, sunDir.z, 0.0f);
	m_renderParams.m_phaseFunctionConsts = m_pSkyLightNishita->GetPhaseFunctionConsts();
	m_renderParams.m_hazeColor = Vec4(m_curHazeColor.x, m_curHazeColor.y, m_curHazeColor.z, 0);
	m_renderParams.m_hazeColorMieNoPremul = Vec4(m_curHazeColorMieNoPremul.x, m_curHazeColorMieNoPremul.y, m_curHazeColorMieNoPremul.z, 0);
	m_renderParams.m_hazeColorRayleighNoPremul = Vec4(m_curHazeColorRayleighNoPremul.x, m_curHazeColorRayleighNoPremul.y, m_curHazeColorRayleighNoPremul.z, 0);

	// set sky hemisphere colors
	m_renderParams.m_skyColorTop = m_curSkyHemiColor[0];
	m_renderParams.m_skyColorNorth = m_curSkyHemiColor[3];
	m_renderParams.m_skyColorWest = m_curSkyHemiColor[4];
	m_renderParams.m_skyColorSouth = m_curSkyHemiColor[1];
	m_renderParams.m_skyColorEast = m_curSkyHemiColor[2];

	// copy sky dome condition params
	m_curSkyDomeCondition = m_updatingSkyDomeCondition;

	m_needRenderParamUpdate = 0;
}

void CSkyLightUpr::GetCurSkyDomeCondition(SSkyDomeCondition& skyCond) const
{
	skyCond = m_curSkyDomeCondition;
}

bool CSkyLightUpr::IsSkyDomeUpdateFinished() const
{
	return(SSkyLightRenderParams::skyDomeTextureSize == m_numSkyDomeColorsComputed);
}

void CSkyLightUpr::InitSkyDomeMesh()
{
	ReleaseSkyDomeMesh();

	if (!GetRenderer())
		return;

#if DRX_PLATFORM_MOBILE
	u32k c_numRings(10);
	u32k c_numSections(10);
#else
	u32k c_numRings(20);
	u32k c_numSections(20);
#endif
	u32k c_numSkyDomeVertices((c_numRings + 1) * (c_numSections + 1));
	u32k c_numSkyDomeTriangles(2 * c_numRings * c_numSections);
	u32k c_numSkyDomeIndices(c_numSkyDomeTriangles * 3);

	std::vector<vtx_idx> skyDomeIndices;
	std::vector<SVF_P3F_C4B_T2F> skyDomeVertices;

	// setup buffers with source data
	skyDomeVertices.reserve(c_numSkyDomeVertices);
	skyDomeIndices.reserve(c_numSkyDomeIndices);

	// calculate vertices
	float sectionSlice(DEG2RAD(360.0f / (float) c_numSections));
	float ringSlice(DEG2RAD(180.0f / (float) c_numRings));
	for (u32 a(0); a <= c_numRings; ++a)
	{
		float w(sinf(a * ringSlice));
		float z(cosf(a * ringSlice));

		for (u32 i(0); i <= c_numSections; ++i)
		{
			SVF_P3F_C4B_T2F v;

			float ii(i - a * 0.5f);   // Gives better tessellation, requires texture address mode to be "wrap"
			                          // for u when rendering (see v.st[ 0 ] below). Otherwise set ii = i;
			v.xyz = Vec3(cosf(ii * sectionSlice) * w, sinf(ii * sectionSlice) * w, z);
			assert(fabs(v.xyz.GetLengthSquared() - 1.0) < 1e-2 /*1e-4*/);       // because of FP-16 precision
			v.st = Vec2(ii / (float) c_numSections, 2.0f * (float) a / (float) c_numRings);
			skyDomeVertices.push_back(v);
		}
	}

	// build faces
	for (u32 a(0); a < c_numRings; ++a)
	{
		for (u32 i(0); i < c_numSections; ++i)
		{
			skyDomeIndices.push_back((vtx_idx) (a * (c_numSections + 1) + i + 1));
			skyDomeIndices.push_back((vtx_idx) (a * (c_numSections + 1) + i));
			skyDomeIndices.push_back((vtx_idx) ((a + 1) * (c_numSections + 1) + i + 1));

			skyDomeIndices.push_back((vtx_idx) ((a + 1) * (c_numSections + 1) + i));
			skyDomeIndices.push_back((vtx_idx) ((a + 1) * (c_numSections + 1) + i + 1));
			skyDomeIndices.push_back((vtx_idx) (a * (c_numSections + 1) + i));
		}
	}

	// sanity checks
	assert(skyDomeVertices.size() == c_numSkyDomeVertices);
	assert(skyDomeIndices.size() == c_numSkyDomeIndices);

	// create static buffers in renderer
	m_pSkyDomeMesh = gEnv->pRenderer->CreateRenderMeshInitialized(&skyDomeVertices[0], c_numSkyDomeVertices, EDefaultInputLayouts::P3F_C4B_T2F,
	                                                              &skyDomeIndices[0], c_numSkyDomeIndices, prtTriangleList, "SkyHDR", "SkyHDR");

	m_lastFrameID = stl::nullopt;
	m_needRenderParamUpdate = true;
}

void CSkyLightUpr::ReleaseSkyDomeMesh()
{
	m_renderParams.m_pSkyDomeMesh = NULL;
	m_pSkyDomeMesh = NULL;
}

i32 CSkyLightUpr::GetFrontBuffer() const
{
	assert(m_curBackBuffer >= 0 && m_curBackBuffer <= 1);
	return((m_curBackBuffer + 1) & 1);
}

i32 CSkyLightUpr::GetBackBuffer() const
{
	assert(m_curBackBuffer >= 0 && m_curBackBuffer <= 1);
	return(m_curBackBuffer);
}

void CSkyLightUpr::ToggleBuffer()
{
	assert(m_curBackBuffer >= 0 && m_curBackBuffer <= 1);
	//better enforce cache flushing then making PPU wait til job has been finished
	m_curBackBuffer = (m_curBackBuffer + 1) & 1;
	m_needRenderParamUpdate = 1;
}

void CSkyLightUpr::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_pSkyLightNishita);
	pSizer->AddObject(m_skyDomeTextureDataMie[0]);
	pSizer->AddObject(m_skyDomeTextureDataMie[1]);
	pSizer->AddObject(m_skyDomeTextureDataRayleigh[0]);
	pSizer->AddObject(m_skyDomeTextureDataRayleigh[1]);
}
