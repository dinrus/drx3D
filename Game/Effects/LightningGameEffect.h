// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIGHTNING_GAME_EFFECT_GAME_EFFECT_
#define _LIGHTNING_GAME_EFFECT_GAME_EFFECT_

#pragma once

#include <drx3D/Game/Effects/GameEffect.h>


class CLightningRenderNode;



struct SLightningStats
{
	struct SStat
	{
		SStat() : m_current(0), m_peak(0) {}
		void SetCurrent(i32 current)
		{
			m_current = current;
			m_peak = max(m_peak, current);
		}
		void Increment(i32 plus=1) {SetCurrent(m_current + plus);}
		i32 GetCurrent() const {return m_current;}
		i32 GetPeak() const {return m_peak;}
	private:
		i32 m_current;
		i32 m_peak;
	};

	void Restart()
	{
		m_activeSparks.SetCurrent(0);
		m_memory.SetCurrent(0);
		m_triCount.SetCurrent(0);
		m_branches.SetCurrent(0);
	}

	SStat m_activeSparks;
	SStat m_memory;
	SStat m_triCount;
	SStat m_branches;
};



struct SLightningParams
{
	SLightningParams();

	void Reset(XmlNodeRef node);

	u32 m_nameCRC;

	float m_strikeTimeMin;
	float m_strikeTimeMax;
	float m_strikeFadeOut;
	i32 m_strikeNumSegments;
	i32 m_strikeNumPoints;

	float m_lightningDeviation;
	float m_lightningFuzzyness;
	float m_lightningVelocity;

	float m_branchProbability;
	i32 m_branchMaxLevel;
	i32 m_maxNumStrikes;

	float m_beamSize;
	float m_beamTexTiling;
	float m_beamTexShift;
	float m_beamTexFrames;
	float m_beamTexFPS;
};



class CLightningGameEffect : public CGameEffect
{
public:
	typedef i32 TIndex;

	struct STarget
	{
		STarget();
		explicit STarget(const Vec3& position);
		explicit STarget(EntityId targetEntity);
		STarget(EntityId targetEntity, i32 slot, tukk attachment);

		Vec3 m_position;
		EntityId m_entityId;
		i32 m_characterAttachmentSlot;
		u32 m_characterAttachmentNameCRC;
	};

private:
	static i32k maxNumSparks = 24;

	struct SLightningSpark
	{
		CLightningRenderNode* m_renderNode;
		STarget m_emitter;
		STarget m_receiver;
		float m_timer;
	};

public:
	CLightningGameEffect();
	virtual ~CLightningGameEffect();

	virtual void	Initialise(const SGameEffectParams* gameEffectParams = NULL) override;
	virtual tukk GetName() const override;
	virtual void Update(float frameTime) override;
	virtual void ResetRenderParameters() override { }
	void ClearSparks();

	static void LoadStaticData(IItemParamsNode* rootNode);
	static void ReloadStaticData(IItemParamsNode* rootNode);
	static void ReleaseStaticData();
	void LoadData();

	TIndex TriggerSpark(tukk presetName, IMaterial* pMaterial, const STarget& emitter, const STarget& receiver);
	void RemoveSpark(const TIndex spark);
	void SetEmitter(const TIndex spark, const STarget& target);
	void SetReceiver(const TIndex spark, const STarget& target);
	float GetSparkRemainingTime(const TIndex spark) const;
	void SetSparkDeviationMult(const TIndex spark, float deviationMult);

private:
	void UnloadData();
	i32 FindEmptySlot() const;
	i32 FindPreset(tukk name) const;
	Vec3 ComputeTargetPosition(const STarget& target);

	std::vector<SLightningParams> m_lightningParams;
	SLightningSpark m_sparks[maxNumSparks];
	SLightningStats m_stats;
};


#endif
