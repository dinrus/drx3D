// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/PlayerInput.h>
#include <drx3D/Game/PlayerMovementController.h>
#include <drx3D/Game/Nodes/G2FlowBaseNode.h>

class CFlowPlayerStagingNode : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowPlayerStagingNode( SActivationInfo * pActInfo )
	{
	}

	~CFlowPlayerStagingNode()
	{
	}

	/*
	IFlowNodePtr Clone( SActivationInfo * pActInfo )
	{
		return new CFlowPlayerStagingNode(pActInfo);
	}
	*/

	enum EInputPorts
	{
		EIP_Trigger = 0,
		EIP_LimitDir,
		EIP_LocalSpace,
		EIP_LimitYaw,
		EIP_LimitPitch,
		EIP_Lock,
		EIP_Stance,
	};

	enum EOutputPorts
	{
		EOP_Done = 0,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void  ("Trigger", _HELP("Trigger")),
			InputPortConfig<Vec3> ("ViewLimitDir", _HELP("Center direction from where the player is limited by yaw/pitch. If (0,0,0) there will be no yaw/pitch limit.")),
			InputPortConfig<bool> ("InLocalSpace", true, _HELP("ViewLimitDir Vector is in Local Space or World Space. \nFor example, in Local Space the value (0,1,0) would be same direction than current player direction, while in World Space the value (0,1,0) would be along world Y axis.")),
			InputPortConfig<float>("ViewLimitYaw", _HELP("ViewLimitYaw, in degrees. 0 = no limit, -1 = total lock")),
			InputPortConfig<float>("ViewLimitPitch", _HELP("ViewLimitPitch, in degrees. 0 = no limit, -1 = total lock")),
			InputPortConfig<bool> ("LockPlayer", false, _HELP("Lock the player's position")),
			InputPortConfig<i32>( "TryStance", -1, _HELP("Try to set Stance on Locking [works only if Player was linked beforehand]"), 0, _UICONFIG("enum_int:<ignore>=-1,Stand=0,Crouch=1,Prone=2,Relaxed=3,Stealth=4,Swim=5,ZeroG=6")),
			{0}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void ("Done", _HELP("Trigger for Chaining")),
			{0}
		};
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Staging of the Player - ViewLimits and Position Lock");
		config.SetCategory(EFLN_OBSOLETE);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			break;
		case eFE_Activate:
			if (IsPortActive(pActInfo, EIP_Trigger))
			{
				const Vec3& dir = GetPortVec3(pActInfo, EIP_LimitDir);
				const bool localSpace = GetPortBool(pActInfo, EIP_LocalSpace);
				float rangeH = GetPortFloat(pActInfo, EIP_LimitYaw);
				float rangeV = GetPortFloat(pActInfo, EIP_LimitPitch);
				if (rangeH<0)
					rangeH = 0.001f;
				if (rangeV<0)
					rangeV = 0.001f;
				CActor *pPlayerActor = static_cast<CActor *>(gEnv->pGame->GetIGameFramework()->GetClientActor());
				if (pPlayerActor)
				{
					CPlayer::SStagingParams stagingParams;
					CPlayer* pPlayer = static_cast<CPlayer*> (pPlayerActor);
					if (dir.len2()>0.01f)
					{
						if (localSpace)
						{
							const Quat& viewQuat = pPlayer->GetViewQuatFinal(); // WC
							Vec3 dirWC = viewQuat * dir;
							stagingParams.vLimitDir = dirWC.GetNormalizedSafe(ZERO);
						}
						else
							stagingParams.vLimitDir = dir.GetNormalizedSafe(ZERO);
					}

					stagingParams.vLimitRangeH = DEG2RAD(rangeH);
					stagingParams.vLimitRangeV = DEG2RAD(rangeV);
					stagingParams.bLocked = GetPortBool(pActInfo, EIP_Lock);
					i32 stance = GetPortInt(pActInfo, EIP_Stance);
					if (stance < STANCE_NULL || stance >= STANCE_LAST)
					{
						stance = STANCE_NULL;
						GameWarning("[flow] PlayerStaging: stance=%d invalid", stance);
					}
					stagingParams.stance = (EStance) stance;

					bool bActive = (stagingParams.bLocked ||
						(!stagingParams.vLimitDir.IsZero() && (!iszero(stagingParams.vLimitRangeH) ||	!iszero(stagingParams.vLimitRangeV)) ));
					pPlayer->StagePlayer(bActive, &stagingParams);

					/*
					SActorParams* pActorParams = pPlayerActor->GetActorParams();
					if (pActorParams)
					{
						CPlayer* pPlayer = static_cast<CPlayer*> (pPlayerActor);
						if (dir.len2()>0.01f)
						{
							if (localSpace)
							{
								const Quat& viewQuat = pPlayer->GetViewQuatFinal(); // WC
								Vec3 dirWC = viewQuat * dir;
								pActorParams->vLimitDir = dirWC.GetNormalizedSafe(ZERO);
							}
							else
								pActorParams->vLimitDir = dir.GetNormalizedSafe(ZERO);
						}
						else 
							pActorParams->vLimitDir.zero();

						pActorParams->vLimitRangeH = DEG2RAD(rangeH);
						pActorParams->vLimitRangeV = DEG2RAD(rangeV);
						const bool bLock = GetPortBool(pActInfo, EIP_Lock);

						// AlexL 23/01/2007: disable this until we have a working solution to lock player movement (action filter)
						// SPlayerStats* pActorStats = static_cast<SPlayerStats*> (pPlayer->GetActorStats());
						// if (pActorStats)
						//	pActorStats->spectatorMode = bLock ? CActor::eASM_Cutscene : 0;
						IActionMapUpr* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapUpr();
						if (pAmMgr)
							pAmMgr->EnableFilter("no_move", bLock);

						if (bLock)
						{
							// CPlayerMovementController* pMC = static_cast<CPlayerMovementController*> (pPlayer->GetMovementController());
							// pMC->Reset();
							if(pPlayer->GetPlayerInput())
								pPlayer->GetPlayerInput()->Reset();
						}
					}
					*/
				}
				ActivateOutput(pActInfo, EOP_Done, false);
			}
			break;
		}
	}

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->Add(*this);
	}
};

class CFlowPlayerLinkNode : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowPlayerLinkNode( SActivationInfo * pActInfo )
	{
	}

	~CFlowPlayerLinkNode()
	{
	}

	enum EInputPorts
	{
		EIP_Link = 0,
		EIP_Unlink,
		EIP_Target,
		EIP_DrawPlayer,
		EIP_KeepTransform,
	};

	enum EOutputPorts
	{
		EOP_Linked = 0,
		EOP_Unlinked,
	};

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inputs[] = {
			InputPortConfig_Void  ("Link", _HELP("Link the Player to Target Entity")),
			InputPortConfig_Void  ("Unlink", _HELP("Unlink the Player (from any Entity)")),
			InputPortConfig<EntityId> ("Target", _HELP("Target Entity Id") ),
			InputPortConfig<i32>  ("DrawPlayer", 0, _HELP("Draw the Player"), 0, _UICONFIG("enum_int:NoChange=0,Hide=-1,Show=1") ),
			InputPortConfig<bool> ("KeepTransfromDetach", true, _HELP("Keep Transformation on Detach")),
			{0}
		};
		static const SOutputPortConfig outputs[] = {
			OutputPortConfig_Void ("Linked", _HELP("Trigger if Linked")),
			OutputPortConfig_Void ("Unlinked", _HELP("Trigger if Unlinked")),
			{0}
		};
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Linking the Player to an Entity (with FreeLook)");
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Activate:
			i32k nDrawPlayer = GetPortInt(pActInfo, EIP_DrawPlayer);
			if (IsPortActive(pActInfo, EIP_Link))
			{
				IEntity* pEntity = gEnv->pEntitySystem->GetEntity(GetPortEntityId(pActInfo, EIP_Target));
				if (pEntity)
				{
					CActor *pPlayerActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
					if (pPlayerActor)
					{
						SActorStats* pActorStats = pPlayerActor->GetActorStats();
						if (pActorStats)
						{
							if (nDrawPlayer == -1)
								pActorStats->isHidden = true;
							else if (nDrawPlayer == 1)
								pActorStats->isHidden = false;
						}
						pPlayerActor->LinkToEntity(pEntity->GetId());
						ActivateOutput(pActInfo, EOP_Linked, true);
					}
				}
			}
			if (IsPortActive(pActInfo, EIP_Unlink))
			{
				CActor *pPlayerActor = static_cast<CActor*>(gEnv->pGame->GetIGameFramework()->GetClientActor());
				if (pPlayerActor)
				{
					SActorStats* pActorStats = pPlayerActor->GetActorStats();
					if (pActorStats)
					{
						if (nDrawPlayer == -1)
							pActorStats->isHidden = true;
						else if (nDrawPlayer == 1)
							pActorStats->isHidden = false;
					}
					pPlayerActor->LinkToEntity(0, GetPortBool(pActInfo, EIP_KeepTransform));
					ActivateOutput(pActInfo, EOP_Unlinked, true);
				}
			}
			break;
		}
	}

	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->Add(*this);
	}
};

REGISTER_FLOW_NODE("Game:PlayerStaging", CFlowPlayerStagingNode);
REGISTER_FLOW_NODE("Game:PlayerLink", CFlowPlayerLinkNode);

