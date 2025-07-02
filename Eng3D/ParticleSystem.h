// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Eng3D/ParticleCommon.h>
#include <drx3D/Eng3D/ParticleDebug.h>
#include <drx3D/Eng3D/ParticleJobUpr.h>
#include <drx3D/Eng3D/ParticleProfiler.h>
#include <drx3D/Eng3D/ParticleEmitter.h>
#include <drx3D/Eng3D/ParticleEffect.h>

namespace pfx2
{
class CParticleSystem : public DinrusX3dEngBase, public IParticleSystem, ISyncMainWithRenderListener
{
	DRXINTERFACE_SIMPLE(IParticleSystem)
	DRXGENERATE_SINGLETONCLASS_GUID(CParticleSystem, "DinrusX_ParticleSystem", "cd8d738d-54b4-46f7-82ba-23ba999cf2ac"_drx_guid)

	CParticleSystem();
	~CParticleSystem();

private:
	typedef std::unordered_map<string, _smart_ptr<CParticleEffect>, stl::hash_stricmp<string>, stl::hash_stricmp<string>> TEffectNameMap;

public:
	// IParticleSystem
	PParticleEffect         CreateEffect() override;
	PParticleEffect         ConvertEffect(const ::IParticleEffect* pOldEffect, bool bReplace) override;
	void                    RenameEffect(PParticleEffect pEffect, cstr name) override;
	PParticleEffect         FindEffect(cstr name, bool bAllowLoad = true) override;
	PParticleEmitter        CreateEmitter(PParticleEffect pEffect) override;
	uint                    GetNumFeatureParams() const override { return uint(GetFeatureParams().size()); }
	SParticleFeatureParams& GetFeatureParam(uint featureIdx) const override { return GetFeatureParams()[featureIdx]; }
	TParticleAttributesPtr  CreateParticleAttributes() const override;

	void                    OnFrameStart() override;
	void                    Update() override;
	void                    Reset() override;
	void                    ClearRenderResources() override;
	void                    FinishRenderTasks(const SRenderingPassInfo& passInfo) override;

	void                    Serialize(TSerialize ser) override;
	bool                    SerializeFeatures(IArchive& ar, TParticleFeatures& features, cstr name, cstr label) const override;

	void                    GetStats(SParticleStats& stats) override;
	void                    DisplayStats(Vec2& location, float lineHeight) override;
	void                    GetMemoryUsage(IDrxSizer* pSizer) const override;
	void                    SyncMainWithRender() override;
	// ~IParticleSystem

	struct SThreadData
	{
		TParticleHeap        memHeap;
		SParticleStats       statsCPU;
		SParticleStats       statsGPU;
		TElementCounts<uint> statsSync;
	};

	PParticleEffect          LoadEffect(cstr effectName);

	SThreadData&             GetThreadData() { return m_threadData[JobUpr::GetWorkerThreadId() + 1]; }
	SThreadData&             GetMainData()   { return m_threadData.front(); }
	SThreadData&             GetSumData()    { return m_threadData.back(); }
	CParticleJobUpr&     GetJobUpr() { return m_jobUpr; }
	CParticleProfiler&       GetProfiler()   { return m_profiler; }

	bool                     IsRuntime() const                { return m_numClears > 0 && m_numFrames > 1; }
	void                     CheckFileAccess(cstr filename = 0) const;

	static float             GetMaxAngularDensity(const CCamera& camera);
	QuatT                    GetLastCameraPose() const        { return m_lastCameraPose; }
	QuatT                    GetCameraMotion() const          { return m_cameraMotion; }
	const TParticleEmitters& GetActiveEmitters() const        { return m_emitters; }
	IMaterial*               GetFlareMaterial();

	typedef std::vector<SParticleFeatureParams> TFeatureParams;
	
	static TFeatureParams&               GetFeatureParams()                                    { static TFeatureParams params; return params; }
	static bool                          RegisterFeature(const SParticleFeatureParams& params) { GetFeatureParams().push_back(params); return true; }
	static const SParticleFeatureParams* FindFeatureParam(cstr groupName, cstr featureName);
	static const SParticleFeatureParams* GetDefaultFeatureParam(EFeatureType);

private:
	void              TrimEmitters(bool finished_only);

	// PFX1 to PFX2
	string ConvertPfx1Name(cstr oldEffectName);
	// ~PFX1 to PFX2

private:
	CParticleJobUpr      m_jobUpr;
	CParticleProfiler        m_profiler;
	TEffectNameMap           m_effects;
	TParticleEmitters        m_emitters;
	TParticleEmitters        m_newEmitters;
	std::vector<SThreadData> m_threadData;
	_smart_ptr<IMaterial>    m_pFlareMaterial;
	bool                     m_bResetEmitters = false;
	QuatT                    m_lastCameraPose = IDENTITY;
	QuatT                    m_cameraMotion   = ZERO;
	uint                     m_numClears      = 0;
	uint                     m_numFrames      = 0;
	uint                     m_nextEmitterId  = 0;
	ESystemConfigSpec        m_lastSysSpec    = END_CONFIG_SPEC_ENUM;
};

ILINE CParticleSystem* GetPSystem()
{
	static std::shared_ptr<IParticleSystem> pSystem(GetIParticleSystem());
	return static_cast<CParticleSystem*>(pSystem.get());
};

}
