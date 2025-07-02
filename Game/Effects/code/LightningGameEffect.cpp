// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/LightningGameEffect.h>
#include <drx3D/Game/Effects/RenderNodes/LightningNode.h>
#include <drx3D/Game/GameXmlParamReader.h>
#include <drx3D/Animation/IAttachment.h>



REGISTER_DATA_CALLBACKS(
	CLightningGameEffect::LoadStaticData,
	CLightningGameEffect::ReleaseStaticData,
	CLightningGameEffect::ReloadStaticData,
	LightningData);



namespace
{


	void DrawLightningStats(const SLightningStats& stats)
	{
		Vec2 textPos = Vec2(400.0f, 10.0f);
		float red[4] = {1,0,0,1};
		float location = 50.0f;
		const float step = 12.0f;
		gEnv->pRenderer->Draw2dLabel(textPos.x, textPos.y, 1.4f, red, false, "Lightning Stats: # (peak #)");
		textPos.y += step;
		gEnv->pRenderer->Draw2dLabel(textPos.x, textPos.y, 1.4f, red, false, "Active Sparks: %d (%d)", stats.m_activeSparks .GetCurrent(), stats.m_activeSparks.GetPeak());
		textPos.y += step;
		gEnv->pRenderer->Draw2dLabel(textPos.x, textPos.y, 1.4f, red, false, "Memory: %d (%d) bytes", stats.m_memory.GetCurrent(), stats.m_memory.GetPeak());
		textPos.y += step;
		gEnv->pRenderer->Draw2dLabel(textPos.x, textPos.y, 1.4f, red, false, "Tri Count: %d (%d)", stats.m_triCount.GetCurrent(), stats.m_triCount.GetPeak());
		textPos.y += step;
		gEnv->pRenderer->Draw2dLabel(textPos.x, textPos.y, 1.4f, red, false, "Branches: %d (%d)", stats.m_branches.GetCurrent(), stats.m_branches.GetPeak());
	}


}



SLightningParams::SLightningParams()
	:	m_lightningDeviation(0.05f)
	,	m_lightningFuzzyness(0.01f)
	,	m_branchMaxLevel(1)
	,	m_branchProbability(2.0f)
	,	m_lightningVelocity(0.5f)
	,	m_strikeTimeMin(0.25f)
	,	m_strikeTimeMax(0.25f)
	,	m_strikeFadeOut(1.0f)
	,	m_strikeNumSegments(6)
	,	m_strikeNumPoints(8)
	,	m_maxNumStrikes(6)
	,	m_beamSize(0.15f)
	,	m_beamTexTiling(0.75f)
	,	m_beamTexShift(0.2f)
	,	m_beamTexFrames(4.0f)
	,	m_beamTexFPS(15.0f)
{
}



void SLightningParams::Reset(XmlNodeRef node)
{
	CGameXmlParamReader reader(node);
	reader.ReadParamValue("lightningDeviation", m_lightningDeviation);
	reader.ReadParamValue("lightningFuzzyness", m_lightningFuzzyness);
	reader.ReadParamValue("branchMaxLevel", m_branchMaxLevel);
	reader.ReadParamValue("branchProbability", m_branchProbability);
	reader.ReadParamValue("lightningVelocity", m_lightningVelocity);
	reader.ReadParamValue("strikeTimeMin", m_strikeTimeMin);
	reader.ReadParamValue("strikeTimeMax", m_strikeTimeMax);
	reader.ReadParamValue("strikeFadeOut", m_strikeFadeOut);
	reader.ReadParamValue("strikeNumSegments", m_strikeNumSegments);
	reader.ReadParamValue("strikeNumPoints", m_strikeNumPoints);
	reader.ReadParamValue("maxNumStrikes", m_maxNumStrikes);
	reader.ReadParamValue("beamSize", m_beamSize);
	reader.ReadParamValue("beamTexTiling", m_beamTexTiling);
	reader.ReadParamValue("beamTexShift", m_beamTexShift);
	reader.ReadParamValue("beamTexFrames", m_beamTexFrames);
	reader.ReadParamValue("beamTexFPS", m_beamTexFPS);
}



CLightningGameEffect::STarget::STarget()
	:	m_position(ZERO)
	,	m_entityId(0)
	,	m_characterAttachmentSlot(-1)
	,	m_characterAttachmentNameCRC(0)
{
}



CLightningGameEffect::STarget::STarget(const Vec3& position)
	:	m_position(position)
	,	m_entityId(0)
	,	m_characterAttachmentSlot(-1)
	,	m_characterAttachmentNameCRC(0)
{
}



CLightningGameEffect::STarget::STarget(EntityId targetEntity)
	:	m_position(ZERO)
	,	m_entityId(targetEntity)
	,	m_characterAttachmentSlot(-1)
	,	m_characterAttachmentNameCRC(0)
{
}



CLightningGameEffect::STarget::STarget(EntityId targetEntity, i32 slot, tukk attachment)
	:	m_position(ZERO)
	,	m_entityId(targetEntity)
	,	m_characterAttachmentSlot(slot)
	,	m_characterAttachmentNameCRC(CCrc32::Compute(attachment))
{
}


CLightningGameEffect::CLightningGameEffect()
{
	for (i32 i = 0; i < maxNumSparks; ++i)
	{
		m_sparks[i].m_renderNode = 0;
		m_sparks[i].m_timer = 0;
	}

	SetFlag(GAME_EFFECT_AUTO_UPDATES_WHEN_ACTIVE, true);
}



CLightningGameEffect::~CLightningGameEffect()
{
	ClearSparks();
}



void CLightningGameEffect::LoadStaticData(IItemParamsNode*)
{
	g_pGame->GetLightningGameEffect()->LoadData();
}



void CLightningGameEffect::ReloadStaticData(IItemParamsNode*)
{
	g_pGame->GetLightningGameEffect()->LoadData();
}



void CLightningGameEffect::ReleaseStaticData()
{
	g_pGame->GetLightningGameEffect()->UnloadData();
}



void CLightningGameEffect::LoadData()
{
	tukk fileLocation = "Libs/LightningArc/LightningArcEffects.xml";
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(fileLocation);

	if(!rootNode)
	{
		GameWarning("Could not load lightning data. Invalid XML file '%s'! ", fileLocation);
		return;
	}

	m_lightningParams.clear();

	i32 numPresets = rootNode->getChildCount();
	m_lightningParams.resize(numPresets);

	for (i32 i = 0; i < numPresets; ++i)
	{
		XmlNodeRef preset = rootNode->getChild(i);
		tukk presetName = preset->getAttr("name");
		m_lightningParams[i].m_nameCRC = CCrc32::Compute(presetName);
		m_lightningParams[i].Reset(preset);
	}
}



void CLightningGameEffect::UnloadData()
{
	ClearSparks();
	m_lightningParams.clear();
	stl::free_container(m_lightningParams);
}



void CLightningGameEffect::Initialise(const SGameEffectParams* gameEffectParams)
{
	CGameEffect::Initialise(gameEffectParams);
	LoadData();
}



tukk CLightningGameEffect::GetName() const
{
	return "Lightning";
}



void CLightningGameEffect::Update(float frameTime)
{
	CGameEffect::Update(frameTime);

	bool keepUpdating = false;

	m_stats.Restart();

	for (i32 i = 0; i < maxNumSparks; ++i)
	{
		if (m_sparks[i].m_renderNode)
			m_sparks[i].m_renderNode->AddStats(&m_stats);
		if (m_sparks[i].m_timer <= 0.0f)
			continue;
		m_sparks[i].m_timer -= frameTime;
		m_sparks[i].m_emitter.m_position = ComputeTargetPosition(m_sparks[i].m_emitter);
		m_sparks[i].m_receiver.m_position = ComputeTargetPosition(m_sparks[i].m_receiver);
		m_sparks[i].m_renderNode->SetEmiterPosition(m_sparks[i].m_emitter.m_position);
		m_sparks[i].m_renderNode->SetReceiverPosition(m_sparks[i].m_receiver.m_position);
		if (m_sparks[i].m_timer <= 0.0f)
			gEnv->p3DEngine->UnRegisterEntityDirect(m_sparks[i].m_renderNode);
		keepUpdating = true;

		m_stats.m_activeSparks.Increment();
	}

	if (g_pGameCVars->g_gameFXLightningProfile)
		DrawLightningStats(m_stats);

	SetActive(keepUpdating);
}



CLightningGameEffect::TIndex CLightningGameEffect::TriggerSpark(tukk presetName, IMaterial* pMaterial, const STarget& emitter, const STarget& receiver)
{
	i32 preset = FindPreset(presetName);
	if (preset == -1)
	{
		GameWarning("Could not find lightning preset '%s'", presetName);
		return -1;
	}

	i32 idx = FindEmptySlot();

	if (!m_sparks[idx].m_renderNode)
		m_sparks[idx].m_renderNode = new CLightningRenderNode();
	gEnv->p3DEngine->UnRegisterEntityDirect(m_sparks[idx].m_renderNode);
	m_sparks[idx].m_renderNode->Reset();
	m_sparks[idx].m_renderNode->SetMaterial(pMaterial);
	m_sparks[idx].m_renderNode->SetLightningParams(&m_lightningParams[preset]);
	SetEmitter(TIndex(idx), emitter);
	SetReceiver(TIndex(idx), receiver);
	m_sparks[idx].m_timer = m_sparks[idx].m_renderNode->TriggerSpark();
	gEnv->p3DEngine->RegisterEntity(m_sparks[idx].m_renderNode);

	SetActive(true);

	return TIndex(idx);
}


void CLightningGameEffect::RemoveSpark(const TIndex spark)
{
	IF_UNLIKELY(spark == -1)
	{
		return;
	}

	IF_UNLIKELY( (spark < 0) || (spark >= maxNumSparks))
	{
		assert(false);
		return;
	}

	if (m_sparks[spark].m_timer > 0.0f)
	{
		gEnv->p3DEngine->UnRegisterEntityDirect(m_sparks[spark].m_renderNode);
		m_sparks[spark].m_timer = 0.0f;
	}
}



void CLightningGameEffect::SetEmitter(TIndex spark, const STarget& target)
{
	if (spark != -1 && m_sparks[spark].m_renderNode != 0)
	{
		m_sparks[spark].m_emitter = target;
		Vec3 targetPos = ComputeTargetPosition(target);
		m_sparks[spark].m_emitter.m_position = targetPos;
		m_sparks[spark].m_renderNode->SetEmiterPosition(targetPos);
	}
}



void CLightningGameEffect::SetReceiver(TIndex spark, const STarget& target)
{
	if (spark != -1 && m_sparks[spark].m_renderNode != 0)
	{
		m_sparks[spark].m_receiver = target;
		Vec3 targetPos = ComputeTargetPosition(target);
		m_sparks[spark].m_receiver.m_position = targetPos;
		m_sparks[spark].m_renderNode->SetReceiverPosition(targetPos);
	}
}



float CLightningGameEffect::GetSparkRemainingTime(TIndex spark) const
{
	if (spark != -1)
		return max(m_sparks[spark].m_timer, 0.0f);
	return 0.0f;
}



void CLightningGameEffect::SetSparkDeviationMult(const TIndex spark, float deviationMult)
{
	if (spark != -1 && m_sparks[spark].m_renderNode != 0)
	{
		m_sparks[spark].m_renderNode->SetSparkDeviationMult(deviationMult);
	}
}



i32 CLightningGameEffect::FindEmptySlot() const
{
	float lowestTime = m_sparks[0].m_timer;
	i32 bestIdex = 0;
	for (i32 i = 0; i < maxNumSparks; ++i)
	{
		if (m_sparks[i].m_timer <= 0.0f)
			return i;
		if (m_sparks[i].m_timer <= lowestTime)
		{
			lowestTime = m_sparks[i].m_timer;
			bestIdex = i;
		}
	}
	return bestIdex;
}



i32 CLightningGameEffect::FindPreset(tukk name) const
{
	u32 crc = CCrc32::Compute(name);
	for (size_t i = 0; i < m_lightningParams.size(); ++i)
		if (m_lightningParams[i].m_nameCRC == crc)
			return i32(i);
	return -1;
}



Vec3 CLightningGameEffect::ComputeTargetPosition(const STarget& target)
{
	Vec3 result = target.m_position;
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(target.m_entityId);

	if (pEntity)
	{
		AABB worldBounds;
		pEntity->GetWorldBounds(worldBounds);
		result = worldBounds.GetCenter();

		if (target.m_characterAttachmentSlot != -1)
		{
			ICharacterInstance* pCharacter = pEntity->GetCharacter(target.m_characterAttachmentSlot);
			if (pCharacter)
			{
				IAttachmentUpr* pAttachmentUpr = pCharacter->GetIAttachmentUpr();
				IAttachment* pAttachment = pAttachmentUpr->GetInterfaceByNameCRC(target.m_characterAttachmentNameCRC);
				if (pAttachment)
					result = pAttachment->GetAttWorldAbsolute().t;
			}
		}
	}
	return result;
}



void CLightningGameEffect::ClearSparks()
{
	for (i32 i = 0; i < maxNumSparks; ++i)
	{
		if (m_sparks[i].m_renderNode)
		{
			gEnv->p3DEngine->UnRegisterEntityDirect(m_sparks[i].m_renderNode);
			m_sparks[i].m_timer = 0.0f;
			delete m_sparks[i].m_renderNode;
			m_sparks[i].m_renderNode = 0;
		}
	}
}
