// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/LightningArc.h>
#include <drx3D/Game/ScriptBind_LightningArc.h>
#include <drx3D/Game/Effects/LightningGameEffect.h>
#include <drx3D/Game/Effects/RenderNodes/LightningNode.h>
#include <drx3D/Game/EntityUtility/EntityScriptCalls.h>



void CLightningArc::GetMemoryUsage(IDrxSizer *pSizer) const {}
void CLightningArc::PostInit( IGameObject * pGameObject ) {}
void CLightningArc::InitClient(i32 channelId) {}
void CLightningArc::PostInitClient(i32 channelId) {}
bool CLightningArc::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {return true;}
void CLightningArc::PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) {}
bool CLightningArc::GetEntityPoolSignature(TSerialize signature) {return true;}
void CLightningArc::FullSerialize(TSerialize ser) {}
bool CLightningArc::NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags) {return true;}
void CLightningArc::PostSerialize() {}
void CLightningArc::SerializeSpawnInfo(TSerialize ser) {}
ISerializableInfoPtr CLightningArc::GetSpawnInfo() {return ISerializableInfoPtr();}
void CLightningArc::HandleEvent( const SGameObjectEvent& event ) {}
void CLightningArc::SetChannelId(u16 id) {}
void CLightningArc::SetAuthority(bool auth ) {}
ukk CLightningArc::GetRMIBase() const {return 0;}
void CLightningArc::PostUpdate(float frameTime) {}
void CLightningArc::PostRemoteSpawn() {}



CLightningArc::CLightningArc()
	:	m_enabled(true)
	,	m_delay(5.0f)
	,	m_delayVariation(0.0f)
	,	m_timer(0.0f)
	,	m_inGameMode(!gEnv->IsEditor())
	, m_lightningPreset(NULL)
{
}



bool CLightningArc::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	g_pGame->GetLightningArcScriptBind()->AttachTo(this);

	ReadLuaParameters();

	return true;
}



void CLightningArc::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_LEVEL_LOADED:
			Enable(m_enabled);
			break;
		case ENTITY_EVENT_RESET:
			Reset(event.nParam[0]!=0);
			break;
	}
}



void CLightningArc::Update(SEntityUpdateContext& ctx, i32 updateSlot)
{
	if (!m_enabled || !m_inGameMode)
		return;

	m_timer -= ctx.fFrameTime;

	if (m_timer < 0.0f)
	{
		TriggerSpark();
		m_timer += m_delay + drx_random(0.5f, 1.0f) * m_delayVariation;
	}
}



void CLightningArc::TriggerSpark()
{
	tukk targetName = "Target";
	IEntity* pEntity = GetEntity();

	if (pEntity->GetMaterial() == 0)
	{
		GameWarning("Lightning arc '%s' has no Material, no sparks will trigger", pEntity->GetName());
		return;
	}

	IEntityLink* pLinks = pEntity->GetEntityLinks();
	i32 numLinks = 0;
	for (IEntityLink* link = pLinks; link; link = link->next)
	{
		if (strcmp(link->name, targetName) == 0)
			++numLinks;
	}

	if (numLinks == 0)
	{
		GameWarning("Lightning arc '%s' has no Targets, no sparks will trigger", pEntity->GetName());
		return;
	}

	i32 nextSpark = drx_random(0, numLinks - 1);
	IEntityLink* pNextSparkLink = pLinks;
	for (; nextSpark && pNextSparkLink; pNextSparkLink = pNextSparkLink->next)
	{
		if (strcmp(pNextSparkLink->name, targetName) == 0)
			--nextSpark;
	}

	assert(pNextSparkLink);
	PREFAST_ASSUME(pNextSparkLink);

	CLightningGameEffect::TIndex id = g_pGame->GetLightningGameEffect()->TriggerSpark(
		m_lightningPreset,
		pEntity->GetMaterial(),
		CLightningGameEffect::STarget(GetEntityId()),
		CLightningGameEffect::STarget(pNextSparkLink->entityId));
	float strikeTime = g_pGame->GetLightningGameEffect()->GetSparkRemainingTime(id);
	IScriptTable* pTargetScriptTable = 0;
	IEntity* pTarget = pNextSparkLink ? gEnv->pEntitySystem->GetEntity(pNextSparkLink->entityId) : NULL;
	if (pTarget)
		pTargetScriptTable = pTarget->GetScriptTable();

	EntityScripts::CallScriptFunction(pEntity, pEntity->GetScriptTable(), "OnStrike", strikeTime, pTargetScriptTable);
}



void CLightningArc::Enable(bool enable)
{
	if (m_enabled != enable)
		m_timer = 0.0f;
	m_enabled = enable;

	if (m_enabled && m_inGameMode)
		GetGameObject()->EnableUpdateSlot(this, 0);
	else
		GetGameObject()->DisableUpdateSlot(this, 0);
}



void CLightningArc::Reset(bool jumpingIntoGame)
{
	m_inGameMode = jumpingIntoGame;
	ReadLuaParameters();
}



void CLightningArc::ReadLuaParameters()
{
	SmartScriptTable pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable)
		return;

	SmartScriptTable pProperties;
	SmartScriptTable pTiming;
	SmartScriptTable pRender;
	if (!pScriptTable->GetValue("Properties", pProperties))
		return;
	if (!pProperties->GetValue("Timing", pTiming))
		return;
	if (!pProperties->GetValue("Render", pRender))
		return;

	pProperties->GetValue("bActive", m_enabled);
	Enable(m_enabled);

	pTiming->GetValue("fDelay", m_delay);
	pTiming->GetValue("fDelayVariation", m_delayVariation);

	pRender->GetValue("ArcPreset", m_lightningPreset);
}



void CLightningArc::Release()
{
	delete this;
}
