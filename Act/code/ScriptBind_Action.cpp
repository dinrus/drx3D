// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/ScriptBind_Action.h>
#include <drx3D/Act/XMLScriptLoader.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameServerChannel.h>
#include <drx3D/Act/GameServerNub.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/SignalTimers.h>
#include <drx3D/Act/RangeSignaling.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Act/AIProxy.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/Act/IEffectSystem.h>
#include <drx3D/Act/IGameplayRecorder.h>
#include <drx3D/Act/PersistantDebug.h>
#include <drx3D/AI/IAIObject.h>
#include <drx3D/AI/IAIObjectUpr.h>

//------------------------------------------------------------------------
CScriptBind_Action::CScriptBind_Action(CDrxAction* pDrxAction)
	: m_pDrxAction(pDrxAction)
{
	Init(gEnv->pScriptSystem, GetISystem());
	SetGlobalName("DinrusAction");

	RegisterGlobals();
	RegisterMethods();

	m_pPreviousView = NULL;
}

//------------------------------------------------------------------------
CScriptBind_Action::~CScriptBind_Action()
{
}

//------------------------------------------------------------------------
void CScriptBind_Action::RegisterGlobals()
{
	SCRIPT_REG_GLOBAL(eGE_DiscreetSample);
	SCRIPT_REG_GLOBAL(eGE_GameReset);
	SCRIPT_REG_GLOBAL(eGE_GameStarted);
	SCRIPT_REG_GLOBAL(eGE_SuddenDeath);
	SCRIPT_REG_GLOBAL(eGE_RoundEnd);
	SCRIPT_REG_GLOBAL(eGE_GameEnd);
	SCRIPT_REG_GLOBAL(eGE_Connected);
	SCRIPT_REG_GLOBAL(eGE_Disconnected);
	SCRIPT_REG_GLOBAL(eGE_Renamed);
	SCRIPT_REG_GLOBAL(eGE_ChangedTeam);
	SCRIPT_REG_GLOBAL(eGE_Death);
	SCRIPT_REG_GLOBAL(eGE_Scored);
	SCRIPT_REG_GLOBAL(eGE_Currency);
	SCRIPT_REG_GLOBAL(eGE_Rank);
	SCRIPT_REG_GLOBAL(eGE_Spectator);
	SCRIPT_REG_GLOBAL(eGE_ScoreReset);
	SCRIPT_REG_GLOBAL(eGE_Damage);
	SCRIPT_REG_GLOBAL(eGE_WeaponHit);

	gEnv->pScriptSystem->SetGlobalValue("QueryAimFromMovementController", CAIProxy::QueryAimFromMovementController);
	gEnv->pScriptSystem->SetGlobalValue("OverriddenAndAiming", CAIProxy::OverriddenAndAiming);
	gEnv->pScriptSystem->SetGlobalValue("OverriddenAndNotAiming", CAIProxy::OverriddenAndNotAiming);
}

//------------------------------------------------------------------------
void CScriptBind_Action::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Action::

	SCRIPT_REG_TEMPLFUNC(LoadXML, "definitionFile, dataFile");
	SCRIPT_REG_TEMPLFUNC(SaveXML, "definitionFile, dataFile, dataTable");
	SCRIPT_REG_TEMPLFUNC(IsServer, "");
	SCRIPT_REG_TEMPLFUNC(IsClient, "");
	SCRIPT_REG_TEMPLFUNC(IsGameStarted, "");
	SCRIPT_REG_TEMPLFUNC(IsRMIServer, "");
	SCRIPT_REG_TEMPLFUNC(IsGameObjectProbablyVisible, "entityId");
	SCRIPT_REG_TEMPLFUNC(GetPlayerList, "");
	SCRIPT_REG_TEMPLFUNC(ActivateEffect, "name");
	SCRIPT_REG_TEMPLFUNC(GetWaterInfo, "pos");
	SCRIPT_REG_TEMPLFUNC(GetServer, "number");
	SCRIPT_REG_TEMPLFUNC(ConnectToServer, "server");
	SCRIPT_REG_TEMPLFUNC(RefreshPings, "");
	SCRIPT_REG_TEMPLFUNC(GetServerTime, "");
	SCRIPT_REG_TEMPLFUNC(PauseGame, "pause");
	SCRIPT_REG_TEMPLFUNC(IsImmersivenessEnabled, "");
	SCRIPT_REG_TEMPLFUNC(IsChannelSpecial, "entityId/channelId");

	SCRIPT_REG_TEMPLFUNC(ForceGameObjectUpdate, "entityId, force");
	SCRIPT_REG_TEMPLFUNC(CreateGameObjectForEntity, "entityId");
	SCRIPT_REG_TEMPLFUNC(BindGameObjectToNetwork, "entityId");
	SCRIPT_REG_TEMPLFUNC(ActivateExtensionForGameObject, "entityId, extension, activate");
	SCRIPT_REG_TEMPLFUNC(SetNetworkParent, "entityId, parentId");
	SCRIPT_REG_TEMPLFUNC(IsChannelOnHold, "channelId");
	SCRIPT_REG_TEMPLFUNC(BanPlayer, "playerId, message");

	SCRIPT_REG_TEMPLFUNC(PersistantSphere, "pos, radius, color, name, timeout");
	SCRIPT_REG_TEMPLFUNC(PersistantLine, "start, end, color, name, timeout");
	SCRIPT_REG_TEMPLFUNC(PersistantArrow, "pos, radius, color, dir, name, timeout");
	SCRIPT_REG_TEMPLFUNC(Persistant2DText, "text, size, color, name, timeout");
	SCRIPT_REG_TEMPLFUNC(PersistantEntityTag, "entityId, text");
	SCRIPT_REG_TEMPLFUNC(ClearEntityTags, "entityId");
	SCRIPT_REG_TEMPLFUNC(ClearStaticTag, "entityId, staticId");

	SCRIPT_REG_TEMPLFUNC(SendGameplayEvent, "entityId, event, [desc], [value], [ID|ptr], [str]");

	SCRIPT_REG_TEMPLFUNC(CacheItemSound, "itemName");
	SCRIPT_REG_TEMPLFUNC(CacheItemGeometry, "itemName");

	SCRIPT_REG_TEMPLFUNC(DontSyncPhysics, "entityId");

	SCRIPT_REG_TEMPLFUNC(EnableSignalTimer, "entityId, text");
	SCRIPT_REG_TEMPLFUNC(DisableSignalTimer, "entityId, text");
	SCRIPT_REG_TEMPLFUNC(SetSignalTimerRate, "entityId, text, float, float");
	SCRIPT_REG_TEMPLFUNC(ResetSignalTimer, "entityId, text");

	SCRIPT_REG_TEMPLFUNC(EnableRangeSignaling, "entityId, bEnable");
	SCRIPT_REG_TEMPLFUNC(DestroyRangeSignaling, "entityId");
	SCRIPT_REG_TEMPLFUNC(ResetRangeSignaling, "entityId");

	SCRIPT_REG_TEMPLFUNC(AddRangeSignal, "entityId, float, float, text");
	SCRIPT_REG_TEMPLFUNC(AddTargetRangeSignal, "entityId, targetId, float, float, text");
	SCRIPT_REG_TEMPLFUNC(AddAngleSignal, "entityId, float, float, text");

	SCRIPT_REG_FUNC(SetViewCamera);
	SCRIPT_REG_FUNC(ResetToNormalCamera);

	SCRIPT_REG_FUNC(RegisterWithAI);
	SCRIPT_REG_TEMPLFUNC(HasAI, "entityId");

	SCRIPT_REG_TEMPLFUNC(GetClassName, "classId");
	SCRIPT_REG_TEMPLFUNC(SetAimQueryMode, "entityId, mode");

	SCRIPT_REG_TEMPLFUNC(PreLoadADB, "adbFileName");
}

i32 CScriptBind_Action::LoadXML(IFunctionHandler* pH, tukk definitionFile, tukk dataFile)
{
	return pH->EndFunction(XmlScriptLoad(definitionFile, dataFile));
}

i32 CScriptBind_Action::SaveXML(IFunctionHandler* pH, tukk definitionFile, tukk dataFile, SmartScriptTable dataTable)
{
	return pH->EndFunction(XmlScriptSave(definitionFile, dataFile, dataTable));
}

i32 CScriptBind_Action::IsGameStarted(IFunctionHandler* pH)
{
	return pH->EndFunction(m_pDrxAction->IsGameStarted());
}

i32 CScriptBind_Action::IsRMIServer(IFunctionHandler* pH)
{
	return pH->EndFunction(m_pDrxAction->GetGameServerNub() != 0);
}

i32 CScriptBind_Action::IsImmersivenessEnabled(IFunctionHandler* pH)
{
	i32 out = 0;
	if (!gEnv->bMultiplayer)
		out = 1;
	else if (CGameContext* pGC = CDrxAction::GetDrxAction()->GetGameContext())
		if (pGC->HasContextFlag(eGSF_ImmersiveMultiplayer))
			out = 1;
	return pH->EndFunction(out);
}

i32 CScriptBind_Action::IsChannelSpecial(IFunctionHandler* pH)
{
	INetChannel* pChannel = 0;

	if (pH->GetParamCount() > 0)
	{
		if (pH->GetParamType(1) == svtNumber)
		{
			i32 channelId = 0;
			if (pH->GetParam(1, channelId))
			{
				if (CGameServerChannel* pGameChannel = CDrxAction::GetDrxAction()->GetGameServerNub()->GetChannel(channelId))
					pChannel = pGameChannel->GetNetChannel();
			}
		}
		else if (pH->GetParamType(1) == svtPointer)
		{
			ScriptHandle entityId;
			if (pH->GetParam(1, entityId))
			{
				if (IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor((EntityId)entityId.n))
				{
					i32 channelId = pActor->GetChannelId();
					if (CGameServerChannel* pGameChannel = CDrxAction::GetDrxAction()->GetGameServerNub()->GetChannel(channelId))
						pChannel = pGameChannel->GetNetChannel();
				}
			}
		}

		if (pChannel && pChannel->IsPreordered())
			return pH->EndFunction(true);
	}

	return pH->EndFunction();
}

i32 CScriptBind_Action::IsClient(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->IsClient());
}

i32 CScriptBind_Action::IsServer(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->bServer);
}

i32 CScriptBind_Action::GetPlayerList(IFunctionHandler* pH)
{
	CGameServerNub* pNub = m_pDrxAction->GetGameServerNub();
	if (!pNub)
	{
		GameWarning("No game server nub");
		return pH->EndFunction();
	}
	TServerChannelMap* playerMap = m_pDrxAction->GetGameServerNub()->GetServerChannelMap();
	if (!playerMap)
		return pH->EndFunction();

	IEntitySystem* pES = gEnv->pEntitySystem;

	i32 k = 1;
	SmartScriptTable playerList(m_pSS);

	for (TServerChannelMap::iterator it = playerMap->begin(); it != playerMap->end(); ++it)
	{
		EntityId playerId = it->second->GetPlayerId();

		if (!playerId)
			continue;

		IEntity* pPlayer = pES->GetEntity(playerId);
		if (!pPlayer)
			continue;
		if (pPlayer->GetScriptTable())
			playerList->SetAt(k++, pPlayer->GetScriptTable());
	}

	return pH->EndFunction(*playerList);
}

i32 CScriptBind_Action::IsGameObjectProbablyVisible(IFunctionHandler* pH, ScriptHandle gameObject)
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity((EntityId)gameObject.n);
	if (pEntity)
	{
		CGameObject* pGameObject = static_cast<CGameObject*>(pEntity->GetProxy(ENTITY_PROXY_USER));
		if (pGameObject && pGameObject->IsProbablyVisible())
			return pH->EndFunction(1);
	}
	return pH->EndFunction();
}

i32 CScriptBind_Action::ActivateEffect(IFunctionHandler* pH, tukk name)
{
	i32 i = CDrxAction::GetDrxAction()->GetIEffectSystem()->GetEffectId(name);
	CDrxAction::GetDrxAction()->GetIEffectSystem()->Activate(i);
	return pH->EndFunction();
}

i32 CScriptBind_Action::GetWaterInfo(IFunctionHandler* pH, Vec3 pos)
{
	i32k mb = 8;
	pe_params_buoyancy buoyancy[mb];
	Vec3 gravity;

	if (i32 n = gEnv->pPhysicalWorld->CheckAreas(pos, gravity, buoyancy, mb))
	{
		for (i32 i = 0; i < n; i++)
		{
			if (buoyancy[i].iMedium == 0) // 0==water
			{
				return pH->EndFunction(buoyancy[i].waterPlane.origin.z,
				                       Script::SetCachedVector(buoyancy[i].waterPlane.n, pH, 2), Script::SetCachedVector(buoyancy[i].waterFlow, pH, 2));
			}
		}
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::GetServer(IFunctionHandler* pFH, i32 number)
{
	tuk server = 0;
	i32 ping = 9999;
	tuk data = 0;

	ILanQueryListener* pLanQueryListener = m_pDrxAction->GetILanQueryListener();
	IGameQueryListener* pGameQueryListener = NULL;
	if (pLanQueryListener)
		pGameQueryListener = pLanQueryListener->GetGameQueryListener();
	if (pGameQueryListener)
		pGameQueryListener->GetServer(number, &server, &data, ping);

	return pFH->EndFunction(server, data, ping);
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::RefreshPings(IFunctionHandler* pFH)
{
	ILanQueryListener* pLanQueryListener = m_pDrxAction->GetILanQueryListener();
	IGameQueryListener* pGameQueryListener = NULL;
	if (pLanQueryListener)
		pGameQueryListener = pLanQueryListener->GetGameQueryListener();
	if (pGameQueryListener)
		pGameQueryListener->RefreshPings();
	return pFH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::ConnectToServer(IFunctionHandler* pFH, tuk server)
{
	ILanQueryListener* pLanQueryListener = m_pDrxAction->GetILanQueryListener();
	if (pLanQueryListener)
		pLanQueryListener->GetGameQueryListener()->ConnectToServer(server);
	return pFH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::GetServerTime(IFunctionHandler* pFH)
{
	return pFH->EndFunction(m_pDrxAction->GetServerTime().GetSeconds());
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::ForceGameObjectUpdate(IFunctionHandler* pH, ScriptHandle entityId, bool force)
{
	if (IGameObject* pGameObject = CDrxAction::GetDrxAction()->GetGameObject((EntityId)entityId.n))
		pGameObject->ForceUpdate(force);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::CreateGameObjectForEntity(IFunctionHandler* pH, ScriptHandle entityId)
{
	CDrxAction::GetDrxAction()->GetIGameObjectSystem()->CreateGameObjectForEntity((EntityId)entityId.n);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::BindGameObjectToNetwork(IFunctionHandler* pH, ScriptHandle entityId)
{
	if (IGameObject* pGameObject = CDrxAction::GetDrxAction()->GetGameObject((EntityId)entityId.n))
		pGameObject->BindToNetwork();

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::ActivateExtensionForGameObject(IFunctionHandler* pH, ScriptHandle entityId, tukk extension, bool activate)
{
	if (IGameObject* pGameObject = CDrxAction::GetDrxAction()->GetGameObject((EntityId)entityId.n))
	{
		if (activate)
			pGameObject->ActivateExtension(extension);
		else
			pGameObject->DeactivateExtension(extension);
	}

	return pH->EndFunction();

}

//------------------------------------------------------------------------
i32 CScriptBind_Action::SetNetworkParent(IFunctionHandler* pH, ScriptHandle entityId, ScriptHandle parentId)
{
	if (IGameObject* pGameObject = CDrxAction::GetDrxAction()->GetGameObject((EntityId)entityId.n))
		pGameObject->SetNetworkParent((EntityId)parentId.n);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::IsChannelOnHold(IFunctionHandler* pH, i32 channelId)
{
	if (CGameServerChannel* pServerChannel = CDrxAction::GetDrxAction()->GetGameServerNub()->GetChannel(channelId))
		return pH->EndFunction(pServerChannel->IsOnHold());
	return pH->EndFunction();
}

i32 CScriptBind_Action::BanPlayer(IFunctionHandler* pH, ScriptHandle entityId, tukk message)
{
	if (IActor* pAct = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor((EntityId)entityId.n))
	{
		CDrxAction::GetDrxAction()->GetGameServerNub()->BanPlayer(pAct->GetChannelId(), message);
	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::PauseGame(IFunctionHandler* pH, bool pause)
{
	bool forced = false;

	if (pH->GetParamCount() > 1)
	{
		pH->GetParam(2, forced);
	}
	CDrxAction::GetDrxAction()->PauseGame(pause, forced);

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Action::SetViewCamera(IFunctionHandler* pH)
{

	// save previous valid view
	IView* pView = m_pDrxAction->GetIViewSystem()->GetActiveView();
	if (pView && !m_pPreviousView)
		m_pPreviousView = pView;

	// override view with our camera settings.
	pView = NULL;
	m_pDrxAction->GetIViewSystem()->SetActiveView(pView);

	Vec3 vPos(0, 0, 0);
	Vec3 vDir(0, 0, 0);

	pH->GetParam(1, vPos);
	pH->GetParam(2, vDir);

	CCamera camera(GetISystem()->GetViewCamera());
	float fRoll(camera.GetAngles().z);

	camera.SetMatrix(CCamera::CreateOrientationYPR(CCamera::CreateAnglesYPR(vDir, fRoll)));
	camera.SetPosition(vPos);
	GetISystem()->SetViewCamera(camera);

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Action::ResetToNormalCamera(IFunctionHandler* pH)
{
	if (m_pPreviousView)
	{
		m_pDrxAction->GetIViewSystem()->SetActiveView(m_pPreviousView);
		m_pPreviousView = nullptr;
	}
	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::PersistantSphere(IFunctionHandler* pH, Vec3 pos, float radius, Vec3 color, tukk name, float timeout)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();

	pPD->Begin(name, false);
	pPD->AddSphere(pos, radius, ColorF(color, 1.f), timeout);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::PersistantLine(IFunctionHandler* pH, Vec3 start, Vec3 end, Vec3 color, tukk name, float timeout)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();

	pPD->Begin(name, false);
	pPD->AddLine(start, end, ColorF(color, 1.f), timeout);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::PersistantArrow(IFunctionHandler* pH, Vec3 pos, float radius, Vec3 dir, Vec3 color, tukk name, float timeout)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();

	pPD->Begin(name, false);
	pPD->AddDirection(pos, radius, dir, ColorF(color, 1.f), timeout);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::Persistant2DText(IFunctionHandler* pH, tukk text, float size, Vec3 color, tukk name, float timeout)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();

	pPD->Begin(name, false);
	pPD->Add2DText(text, size, ColorF(color, 1.f), timeout);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
// Required Params: entityId, tukk text
// Optional Params: float size, Vec3 color, float visibleTime, float fadeTime, float viewDistance, tukk staticId, i32 columnNum, tukk contextTag
i32 CScriptBind_Action::PersistantEntityTag(IFunctionHandler* pH, ScriptHandle entityId, tukk text)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();

	SEntityTagParams params;
	params.entity = (EntityId)entityId.n;
	params.text = text;
	params.tagContext = "scriptbind";

	// Optional params
	if (pH->GetParamType(3) != svtNull) // Size
		pH->GetParam(3, params.size);
	if (pH->GetParamType(4) != svtNull) // Color
	{
		Vec3 color;
		pH->GetParam(4, color);
		params.color = ColorF(color, 1.f);
	}
	if (pH->GetParamType(5) != svtNull) // Visible Time
		pH->GetParam(5, params.visibleTime);
	if (pH->GetParamType(6) != svtNull) // Fade Time
		pH->GetParam(6, params.fadeTime);
	if (pH->GetParamType(7) != svtNull) // View Distance
		pH->GetParam(7, params.viewDistance);
	if (pH->GetParamType(8) != svtNull) // Static ID
	{
		tukk staticId;
		pH->GetParam(8, staticId);
		params.staticId = staticId;
	}
	if (pH->GetParamType(9) != svtNull) // Column Num
		pH->GetParam(9, params.column);
	if (pH->GetParamType(10) != svtNull) // Context Tag
	{
		tukk tagContext;
		pH->GetParam(10, tagContext);
		params.tagContext = tagContext; // overrides default one set above
	}

	pPD->AddEntityTag(params);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::ClearEntityTags(IFunctionHandler* pH, ScriptHandle entityId)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();
	pPD->ClearEntityTags((EntityId)entityId.n);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::ClearStaticTag(IFunctionHandler* pH, ScriptHandle entityId, tukk staticId)
{
	IPersistantDebug* pPD = CDrxAction::GetDrxAction()->GetIPersistantDebug();
	pPD->ClearStaticTag((EntityId)entityId.n, staticId);

	return pH->EndFunction();
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::EnableSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText)
{
	bool bRet = CSignalTimer::ref().EnablePersonalUpr((EntityId)entityId.n, sText);
	return pH->EndFunction(bRet);
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::DisableSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText)
{
	bool bRet = CSignalTimer::ref().DisablePersonalSignalTimer((EntityId)entityId.n, sText);
	return pH->EndFunction(bRet);
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::SetSignalTimerRate(IFunctionHandler* pH, ScriptHandle entityId, tukk sText, float fRateMin, float fRateMax)
{
	bool bRet = CSignalTimer::ref().SetTurnRate((EntityId)entityId.n, sText, fRateMin, fRateMax);
	return pH->EndFunction(bRet);
}

//-------------------------------------------------------------------------
i32 CScriptBind_Action::ResetSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText)
{
	bool bRet = CSignalTimer::ref().ResetPersonalTimer((EntityId)entityId.n, sText);
	return pH->EndFunction(bRet);
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::SendGameplayEvent(IFunctionHandler* pH, ScriptHandle entityId, i32 event)
{
	tukk desc = 0;
	float value = 0.0f;
	ScriptHandle hdl;

	tukk str_data = 0;

	if (pH->GetParamCount() > 2 && pH->GetParamType(3) == svtString)
		pH->GetParam(3, desc);
	if (pH->GetParamCount() > 3 && pH->GetParamType(4) == svtNumber)
		pH->GetParam(4, value);
	if (pH->GetParamCount() > 4 && pH->GetParamType(5) == svtPointer)
		pH->GetParam(5, hdl);
	if (pH->GetParamCount() > 5 && pH->GetParamType(6) == svtString)
		pH->GetParam(6, str_data);

	IEntity* pEntity = gEnv->pEntitySystem->GetEntity((EntityId)entityId.n);
	CDrxAction::GetDrxAction()->GetIGameplayRecorder()->Event(pEntity, GameplayEvent(event, desc, value, hdl.ptr, str_data));

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::CacheItemSound(IFunctionHandler* pH, tukk itemName)
{
	CDrxAction::GetDrxAction()->GetIItemSystem()->CacheItemSound(itemName);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::CacheItemGeometry(IFunctionHandler* pH, tukk itemName)
{
	CDrxAction::GetDrxAction()->GetIItemSystem()->CacheItemGeometry(itemName);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::EnableRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId, bool bEnable)
{
	CDrxAction::GetDrxAction()->GetRangeSignaling()->EnablePersonalRangeSignaling((EntityId)entityId.n, bEnable);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::DestroyRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId)
{
	CDrxAction::GetDrxAction()->GetRangeSignaling()->DestroyPersonalRangeSignaling((EntityId)entityId.n);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::ResetRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId)
{
	CDrxAction::GetDrxAction()->GetRangeSignaling()->ResetPersonalRangeSignaling((EntityId)entityId.n);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::AddRangeSignal(IFunctionHandler* pH, ScriptHandle entityId, float fRadius, float fFlexibleBoundary, tukk sSignal)
{
	if (gEnv->pAISystem)
	{
		// Get optional signal data
		IAISignalExtraData* pData = NULL;
		if (pH->GetParamCount() > 4)
		{
			SmartScriptTable dataTable;
			if (pH->GetParam(5, dataTable))
			{
				pData = gEnv->pAISystem->CreateSignalExtraData();
				DRX_ASSERT(pData);
				pData->FromScriptTable(dataTable);
			}
		}

		CDrxAction::GetDrxAction()->GetRangeSignaling()->AddRangeSignal((EntityId)entityId.n, fRadius, fFlexibleBoundary, sSignal, pData);
		gEnv->pAISystem->FreeSignalExtraData(pData);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::AddTargetRangeSignal(IFunctionHandler* pH, ScriptHandle entityId, ScriptHandle targetId, float fRadius, float fFlexibleBoundary, tukk sSignal)
{
	if (gEnv->pAISystem)
	{
		// Get optional signal data
		IAISignalExtraData* pData = NULL;
		if (pH->GetParamCount() > 5)
		{
			SmartScriptTable dataTable;
			if (pH->GetParam(6, dataTable))
			{
				pData = gEnv->pAISystem->CreateSignalExtraData();
				DRX_ASSERT(pData);
				pData->FromScriptTable(dataTable);
			}
		}

		CDrxAction::GetDrxAction()->GetRangeSignaling()->AddTargetRangeSignal((EntityId)entityId.n, (EntityId)targetId.n, fRadius, fFlexibleBoundary, sSignal, pData);
		gEnv->pAISystem->FreeSignalExtraData(pData);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::AddAngleSignal(IFunctionHandler* pH, ScriptHandle entityId, float fAngle, float fFlexibleBoundary, tukk sSignal)
{
	if (gEnv->pAISystem)
	{
		// Get optional signal data
		IAISignalExtraData* pData = NULL;
		if (pH->GetParamCount() > 4)
		{
			SmartScriptTable dataTable;
			if (pH->GetParam(5, dataTable))
			{
				pData = gEnv->pAISystem->CreateSignalExtraData();
				DRX_ASSERT(pData);
				pData->FromScriptTable(dataTable);
			}
		}

		CDrxAction::GetDrxAction()->GetRangeSignaling()->AddAngleSignal((EntityId)entityId.n, fAngle, fFlexibleBoundary, sSignal, pData);
		gEnv->pAISystem->FreeSignalExtraData(pData);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::DontSyncPhysics(IFunctionHandler* pH, ScriptHandle entityId)
{
	if (CGameObject* pGO = static_cast<CGameObject*>(CDrxAction::GetDrxAction()->GetIGameObjectSystem()->CreateGameObjectForEntity((EntityId)entityId.n)))
		pGO->DontSyncPhysics();
	else
		GameWarning("DontSyncPhysics: Unable to find entity %" PRISIZE_T, entityId.n);
	return pH->EndFunction();
}

//
//-----------------------------------------------------------------------------------------------------------
// (MATT) Moved here from Scriptbind_AI when that was moved to the AI system {2008/02/15:15:23:16}
i32 CScriptBind_Action::RegisterWithAI(IFunctionHandler* pH)
{
	if (gEnv->bMultiplayer && !gEnv->bServer)
		return pH->EndFunction();

	i32 type;
	ScriptHandle hdl;

	if (!pH->GetParams(hdl, type))
		return pH->EndFunction();

	EntityId entityID = (EntityId)hdl.n;
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityID);

	if (!pEntity)
	{
		GameWarning("RegisterWithAI: Tried to set register with AI nonExisting entity with id [%d]. ", entityID);
		return pH->EndFunction();
	}

	// Apparently we can't assume that there is just one IGameObject to an entity, because we choose between (at least) Actor and Vehicle objects.
	// (MATT) Do we really need to check on the actor system here? {2008/02/15:18:38:34}
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	IVehicleSystem* pVSystem = pGameFramework->GetIVehicleSystem();
	IActorSystem* pASystem = pGameFramework->GetIActorSystem();
	if (!pASystem)
	{
		GameWarning("RegisterWithAI: no ActorSystem for %s.", pEntity->GetName());
		return pH->EndFunction();
	}

	AIObjectParams params(type, 0, entityID);
	bool autoDisable(true);

	// For most types, we need to parse the tables
	// For others we leave them blank
	switch (type)
	{
	case AIOBJECT_ACTOR:
	case AIOBJECT_2D_FLY:
	case AIOBJECT_BOAT:
	case AIOBJECT_CAR:
	case AIOBJECT_HELICOPTER:

	case AIOBJECT_INFECTED:
	case AIOBJECT_ALIENTICK:

	case AIOBJECT_HELICOPTERDRXSIS2:
		if (gEnv->pAISystem && !gEnv->pAISystem->ParseTables(3, true, pH, params, autoDisable))
			return pH->EndFunction();
	default:
		;
	}

	// Most types check these, so just get them in advance
	IActor* pActor = pASystem->GetActor(pEntity->GetId());

	IVehicle* pVehicle = NULL;
	if (pVSystem)
		pVehicle = pVSystem->GetVehicle(pEntity->GetId());

	// Set this if we've found something to create a proxy from
	IGameObject* pGameObject = NULL;

	switch (type)
	{
	case AIOBJECT_ACTOR:
	case AIOBJECT_2D_FLY:

	case AIOBJECT_INFECTED:
	case AIOBJECT_ALIENTICK:
		{
			// (MATT) The pActor/pVehicle test below - is it basically trying to distiguish between the two cases above? If so, separate them! {2008/02/15:19:38:08}
			if (!pActor)
			{
				GameWarning("RegisterWithAI: no Actor for %s.", pEntity->GetName());
				return pH->EndFunction();
			}
			pGameObject = pActor->GetGameObject();
		}
		break;
	case AIOBJECT_BOAT:
	case AIOBJECT_CAR:
		{
			if (!pVehicle)
			{
				GameWarning("RegisterWithAI: no Vehicle for %s (Id %i).", pEntity->GetName(), pEntity->GetId());
				return pH->EndFunction();
			}
			pGameObject = pVehicle->GetGameObject();
		}
		break;

	case AIOBJECT_HELICOPTER:
	case AIOBJECT_HELICOPTERDRXSIS2:
		{
			if (!pVehicle)
			{
				GameWarning("RegisterWithAI: no Vehicle for %s (Id %i).", pEntity->GetName(), pEntity->GetId());
				return pH->EndFunction();
			}
			pGameObject = pVehicle->GetGameObject();
			params.m_moveAbility.b3DMove = true;
		}
		break;
	case AIOBJECT_PLAYER:
		{
			if (IsDemoPlayback())
				return pH->EndFunction();

			SmartScriptTable pTable;

			if (pH->GetParamCount() > 2)
				pH->GetParam(3, pTable);
			else
				return pH->EndFunction();

			pGameObject = pActor->GetGameObject();

			pTable->GetValue("groupid", params.m_sParamStruct.m_nGroup);

			tukk faction = 0;
			if (pTable->GetValue("esFaction", faction) && gEnv->pAISystem)
			{
				params.m_sParamStruct.factionID = gEnv->pAISystem->GetFactionMap().GetFactionID(faction);
				if (faction && *faction && (params.m_sParamStruct.factionID == IFactionMap::InvalidFactionID))
				{
					GameWarning("Unknown faction '%s' being set...", faction);
				}
			}
			else
			{
				// Márcio: backwards compatibility
				i32 species = -1;
				if (!pTable->GetValue("eiSpecies", species))
					pTable->GetValue("species", species);

				if (species > -1)
					params.m_sParamStruct.factionID = species;
			}

			pTable->GetValue("commrange", params.m_sParamStruct.m_fCommRange); //Luciano - added to use GROUPONLY signals

			SmartScriptTable pPerceptionTable;
			if (pTable->GetValue("Perception", pPerceptionTable))
			{
				pPerceptionTable->GetValue("sightrange", params.m_sParamStruct.m_PerceptionParams.sightRange);
			}
		}
		break;
	case AIOBJECT_SNDSUPRESSOR:
		{
			// (MATT) This doesn't need a proxy? {2008/02/15:19:45:58}
			SmartScriptTable pTable;
			// Properties table
			if (pH->GetParamCount() > 2)
				pH->GetParam(3, pTable);
			else
				return pH->EndFunction();
			if (!pTable->GetValue("radius", params.m_moveAbility.pathRadius))
				params.m_moveAbility.pathRadius = 10.f;
			break;
		}
	case AIOBJECT_WAYPOINT:
		break;
		/*
		   // this block is commented out since params.m_sParamStruct is currently ignored in pEntity->RegisterInAISystem()
		   // instead of setting the group id here, it will be set from the script right after registering
		   default:
		   // try to get groupid settings for anchors
		   params.m_sParamStruct.m_nGroup = -1;
		   params.m_sParamStruct.m_nSpecies = -1;
		   {
		   SmartScriptTable pTable;
		   if ( pH->GetParamCount() > 2 )
		   pH->GetParam( 3, pTable );
		   if ( *pTable )
		   pTable->GetValue( "groupid", params.m_sParamStruct.m_nGroup );
		   }
		   break;
		 */
	}

	params.name = pEntity->GetName();

	// Register in AI to get a new AI object, deregistering the old one in the process
	gEnv->pAISystem->GetAIObjectUpr()->CreateAIObject(params);

	// (MATT) ? {2008/02/15:19:46:29}
	// AI object was not created (possibly AI System is disabled)
	if (IAIObject* aiObject = pEntity->GetAI())
	{
		if (type == AIOBJECT_SNDSUPRESSOR)
			aiObject->SetRadius(params.m_moveAbility.pathRadius);
		else if (type >= AIANCHOR_FIRST) // if anchor - set radius
		{
			SmartScriptTable pTable;
			// Properties table
			if (pH->GetParamCount() > 2)
				pH->GetParam(3, pTable);
			else
				return pH->EndFunction();
			float radius(0.f);
			pTable->GetValue("radius", radius);
			i32 groupId = -1;
			pTable->GetValue("groupid", groupId);
			aiObject->SetGroupId(groupId);
			aiObject->SetRadius(radius);
		}

		if (IAIActorProxy* proxy = aiObject->GetProxy())
			proxy->UpdateMeAlways(!autoDisable);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::HasAI(IFunctionHandler* pH, ScriptHandle entityId)
{
	bool bResult = false;

	const EntityId id = (EntityId)entityId.n;
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (pEntity)
	{
		bResult = (pEntity->GetAI() != 0);
	}

	return pH->EndFunction(bResult);
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::GetClassName(IFunctionHandler* pH, i32 classId)
{
	char className[128];
	bool retrieved = CDrxAction::GetDrxAction()->GetNetworkSafeClassName(className, sizeof(className), classId);

	if (retrieved)
	{
		return pH->EndFunction(className);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_Action::SetAimQueryMode(IFunctionHandler* pH, ScriptHandle entityId, i32 mode)
{
	IEntity* entity = gEnv->pEntitySystem->GetEntity(static_cast<EntityId>(entityId.n));
	IAIObject* ai = entity ? entity->GetAI() : NULL;
	CAIProxy* proxy = ai ? static_cast<CAIProxy*>(ai->GetProxy()) : NULL;

	if (proxy)
		proxy->SetAimQueryMode(static_cast<CAIProxy::AimQueryMode>(mode));

	return pH->EndFunction();
}

// ============================================================================
//	Pre-load a mannequin ADB file.
//
//	In:		The function handler (NULL is invalid!)
//	In:		The name of the ADB file.
//
//	Returns:	A default result code (in Lua: void).
//
i32 CScriptBind_Action::PreLoadADB(IFunctionHandler* pH, tukk adbFileName)
{
	IF_LIKELY (adbFileName != NULL)
	{
		IMannequin& mannequinInterface = gEnv->pGameFramework->GetMannequinInterface();
		if (mannequinInterface.GetAnimationDatabaseUpr().Load(adbFileName) == NULL)
		{
			GameWarning("PreLoadADB(): Failed to pre-load '%s'!", adbFileName);
		}
	}

	return pH->EndFunction();
}
