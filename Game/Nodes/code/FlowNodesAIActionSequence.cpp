// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowNodesAIActionSequence.cpp
//  Version:     v1.00
//  Created:     2014-05-12 by Christian Werle.
//  Описание: A bunch of game specific FlowNodes that are AI-Sequence-compatible.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>
#include <drx3D/AI/IAIActionSequence.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Game/G2FlowBaseNode.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Weapon.h>


////////////////////////////////////////////////////////////////////////// 
//
// DrxGame_AIActionSequenceFlowNodeBase
//
//////////////////////////////////////////////////////////////////////////
class DrxGame_AIActionSequenceFlowNodeBase	// prefixed with "DrxGame" to prevent naming clashes in monolithic builds
	: public CFlowBaseNode<eNCT_Instanced>
	, public AIActionSequence::SequenceActionBase
{
protected:
	void FinishSequenceActionAndActivateOutputPort(i32 port);
	void CancelSequenceAndActivateOutputPort(i32 port);

	SActivationInfo m_actInfo;
};

void DrxGame_AIActionSequenceFlowNodeBase::FinishSequenceActionAndActivateOutputPort(i32 port)
{
	gEnv->pAISystem->GetSequenceUpr()->ActionCompleted(GetAssignedSequenceId());
	ActivateOutput(&m_actInfo, port, true);
}

void DrxGame_AIActionSequenceFlowNodeBase::CancelSequenceAndActivateOutputPort(i32 port)
{
	gEnv->pAISystem->GetSequenceUpr()->CancelSequence(GetAssignedSequenceId());
	ActivateOutput(&m_actInfo, port, true);
}


//////////////////////////////////////////////////////////////////////////
//
// CFlowNode_AISequenceAction_WeaponDraw
//
//////////////////////////////////////////////////////////////////////////
class CFlowNode_AISequenceAction_WeaponDraw
	: public DrxGame_AIActionSequenceFlowNodeBase
{
public:
	CFlowNode_AISequenceAction_WeaponDraw(SActivationInfo* pActInfo);
	~CFlowNode_AISequenceAction_WeaponDraw();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo);
	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void GetMemoryUsage(IDrxSizer* sizer) const;
	// ~IFlowNode

	// AIActionSequence::SequenceActionBase
	virtual void HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent);
	// ~AIActionSequence::SequenceActionBase

private:
	enum
	{
		InputPort_Start,
		InputPort_SkipDrawAnimation,
	};

	enum
	{
		OutputPort_Done
	};
};

CFlowNode_AISequenceAction_WeaponDraw::CFlowNode_AISequenceAction_WeaponDraw(SActivationInfo* pActInfo)
{
}

CFlowNode_AISequenceAction_WeaponDraw::~CFlowNode_AISequenceAction_WeaponDraw()
{
}

IFlowNodePtr CFlowNode_AISequenceAction_WeaponDraw::Clone(SActivationInfo* pActInfo)
{
	return new CFlowNode_AISequenceAction_WeaponDraw(pActInfo);
}

void CFlowNode_AISequenceAction_WeaponDraw::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig inputPortConfig[] =
	{
		InputPortConfig_Void("Start"),
		InputPortConfig<bool>("SkipDrawAnimation", _HELP("If true, skip the draw animation")),
		{ 0 }
	};

	static const SOutputPortConfig outputPortConfig[] =
	{
		OutputPortConfig_Void("Done"),
		{ 0 }
	};

	config.pInputPorts = inputPortConfig;
	config.pOutputPorts = outputPortConfig;
	config.sDescription = _HELP("Makes the AI draw its holstered weapon");
	config.nFlags |= EFLN_TARGET_ENTITY | EFLN_AISEQUENCE_ACTION;
	config.SetCategory(EFLN_APPROVED);
}

void CFlowNode_AISequenceAction_WeaponDraw::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	if (!pActInfo->pEntity)
		return;

	switch(event)
	{
	case eFE_Activate:
		{
			m_actInfo = *pActInfo;

			if (IsPortActive(pActInfo, InputPort_Start))
			{
				if (const AIActionSequence::SequenceId assignedSequenceId = GetAssignedSequenceId())
				{
					gEnv->pAISystem->GetSequenceUpr()->RequestActionStart(assignedSequenceId, m_actInfo.myID);
				}
			}
		}
		break;
	}
}

void CFlowNode_AISequenceAction_WeaponDraw::GetMemoryUsage(IDrxSizer* sizer) const
{
	sizer->Add(*this);
}

void CFlowNode_AISequenceAction_WeaponDraw::HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent)
{
	switch(sequenceEvent)
	{
	case AIActionSequence::StartAction:
		{
			DRX_ASSERT_MESSAGE(m_actInfo.pEntity, "entity has magically gone");
			if (!m_actInfo.pEntity)
			{
				// the entity has gone for some reason, at least make sure the action gets finished properly and the FG continues
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			assert(gEnv && gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetIActorSystem());
			IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_actInfo.pEntity->GetId());
			if (!pActor)
			{
				DRX_ASSERT_MESSAGE(0, "Provided entity must be an IActor");
				DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_WARNING, "Provided entity %s must be an IActor", m_actInfo.pEntity->GetName());
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			const bool skipDrawAnimation = GetPortBool(&m_actInfo, InputPort_SkipDrawAnimation);
			pActor->HolsterItem(false, !skipDrawAnimation);
			FinishSequenceActionAndActivateOutputPort(OutputPort_Done);
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// CFlowNode_AISequenceAction_WeaponHolster
//
//////////////////////////////////////////////////////////////////////////
class CFlowNode_AISequenceAction_WeaponHolster
	: public DrxGame_AIActionSequenceFlowNodeBase
{
public:
	CFlowNode_AISequenceAction_WeaponHolster(SActivationInfo* pActInfo);
	~CFlowNode_AISequenceAction_WeaponHolster();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo);
	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void GetMemoryUsage(IDrxSizer* sizer) const;
	// ~IFlowNode

	// AIActionSequence::SequenceActionBase
	virtual void HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent);
	// ~AIActionSequence::SequenceActionBase

private:
	enum
	{
		InputPort_Start,
		InputPort_SkipHolsterAnimation,
	};

	enum
	{
		OutputPort_Done
	};
};

CFlowNode_AISequenceAction_WeaponHolster::CFlowNode_AISequenceAction_WeaponHolster(SActivationInfo* pActInfo)
{
}

CFlowNode_AISequenceAction_WeaponHolster::~CFlowNode_AISequenceAction_WeaponHolster()
{
}

IFlowNodePtr CFlowNode_AISequenceAction_WeaponHolster::Clone(SActivationInfo* pActInfo)
{
	return new CFlowNode_AISequenceAction_WeaponHolster(pActInfo);
}

void CFlowNode_AISequenceAction_WeaponHolster::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig inputPortConfig[] =
	{
		InputPortConfig_Void("Start"),
		InputPortConfig<bool>("SkipHolsterAnimation", _HELP("If true, skip the holster animation")),
		{ 0 }
	};

	static const SOutputPortConfig outputPortConfig[] =
	{
		OutputPortConfig_Void("Done"),
		{ 0 }
	};

	config.pInputPorts = inputPortConfig;
	config.pOutputPorts = outputPortConfig;
	config.sDescription = _HELP("Makes the AI holster its drawn weapon");
	config.nFlags |= EFLN_TARGET_ENTITY | EFLN_AISEQUENCE_ACTION;
	config.SetCategory(EFLN_APPROVED);
}

void CFlowNode_AISequenceAction_WeaponHolster::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	if (!pActInfo->pEntity)
		return;

	switch(event)
	{
	case eFE_Activate:
		{
			m_actInfo = *pActInfo;

			if (IsPortActive(pActInfo, InputPort_Start))
			{
				if (const AIActionSequence::SequenceId assignedSequenceId = GetAssignedSequenceId())
				{
					gEnv->pAISystem->GetSequenceUpr()->RequestActionStart(assignedSequenceId, m_actInfo.myID);
				}
			}
		}
		break;
	}
}

void CFlowNode_AISequenceAction_WeaponHolster::GetMemoryUsage(IDrxSizer* sizer) const
{
	sizer->Add(*this);
}

void CFlowNode_AISequenceAction_WeaponHolster::HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent)
{
	switch(sequenceEvent)
	{
	case AIActionSequence::StartAction:
		{
			DRX_ASSERT_MESSAGE(m_actInfo.pEntity, "entity has magically gone");
			if (!m_actInfo.pEntity)
			{
				// the entity has gone for some reason, at least make sure the action gets finished properly and the FG continues
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			assert(gEnv && gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetIActorSystem());
			IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_actInfo.pEntity->GetId());
			if (!pActor)
			{
				DRX_ASSERT_MESSAGE(0, "Provided entity must be an IActor");
				DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_WARNING, "Provided entity %s must be an IActor", m_actInfo.pEntity->GetName());
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			const bool skipHolsterAnimation = GetPortBool(&m_actInfo, InputPort_SkipHolsterAnimation);
			pActor->HolsterItem(true, !skipHolsterAnimation);
			FinishSequenceActionAndActivateOutputPort(OutputPort_Done);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//
// CFlowNode_AISequenceAction_WeaponDrawFromInventory
//
//////////////////////////////////////////////////////////////////////////
class CFlowNode_AISequenceAction_WeaponDrawFromInventory
	: public DrxGame_AIActionSequenceFlowNodeBase
{
public:
	CFlowNode_AISequenceAction_WeaponDrawFromInventory(SActivationInfo* pActInfo);
	~CFlowNode_AISequenceAction_WeaponDrawFromInventory();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo);
	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void GetMemoryUsage(IDrxSizer* sizer) const;
	// ~IFlowNode

	// AIActionSequence::SequenceActionBase
	virtual void HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent);
	// ~AIActionSequence::SequenceActionBase

private:
	enum
	{
		InputPort_Start,
		InputPort_WeaponName,
	};

	enum
	{
		OutputPort_Done
	};
};

CFlowNode_AISequenceAction_WeaponDrawFromInventory::CFlowNode_AISequenceAction_WeaponDrawFromInventory(SActivationInfo* pActInfo)
{
}

CFlowNode_AISequenceAction_WeaponDrawFromInventory::~CFlowNode_AISequenceAction_WeaponDrawFromInventory()
{
}

IFlowNodePtr CFlowNode_AISequenceAction_WeaponDrawFromInventory::Clone(SActivationInfo* pActInfo)
{
	return new CFlowNode_AISequenceAction_WeaponDrawFromInventory(pActInfo);
}

void CFlowNode_AISequenceAction_WeaponDrawFromInventory::GetConfiguration(SFlowNodeConfig& config)
{
	static const SInputPortConfig inputPortConfig[] =
	{
		InputPortConfig_Void("Start"),
		InputPortConfig<string>("Weapon", _HELP("Name of the weapon to be selected from the AI's inventory.\nNotice: this is a list of all weapons available in the game, so, be sure to only select one actually available in the AI's inventory."), NULL, _UICONFIG("enum_global:weapon")),
		{ 0 }
	};

	static const SOutputPortConfig outputPortConfig[] =
	{
		OutputPortConfig_Void("Done"),
		{ 0 }
	};

	config.pInputPorts = inputPortConfig;
	config.pOutputPorts = outputPortConfig;
	config.sDescription = _HELP("Makes the AI draw a specific weapon from its inventory");
	config.nFlags |= EFLN_TARGET_ENTITY | EFLN_AISEQUENCE_ACTION;
	config.SetCategory(EFLN_APPROVED);
}

void CFlowNode_AISequenceAction_WeaponDrawFromInventory::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	if (!pActInfo->pEntity)
		return;

	switch(event)
	{
	case eFE_Activate:
		{
			m_actInfo = *pActInfo;

			if (IsPortActive(pActInfo, InputPort_Start))
			{
				if (const AIActionSequence::SequenceId assignedSequenceId = GetAssignedSequenceId())
				{
					gEnv->pAISystem->GetSequenceUpr()->RequestActionStart(assignedSequenceId, m_actInfo.myID);
				}
			}
		}
		break;
	}
}

void CFlowNode_AISequenceAction_WeaponDrawFromInventory::GetMemoryUsage(IDrxSizer* sizer) const
{
	sizer->Add(*this);
}

void CFlowNode_AISequenceAction_WeaponDrawFromInventory::HandleSequenceEvent(AIActionSequence::SequenceEvent sequenceEvent)
{
	switch(sequenceEvent)
	{
	case AIActionSequence::StartAction:
		{
			DRX_ASSERT_MESSAGE(m_actInfo.pEntity, "entity has magically gone");
			if (!m_actInfo.pEntity)
			{
				// the entity has gone for some reason, at least make sure the action gets finished properly and the FG continues
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			assert(gEnv && gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetIActorSystem());
			IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_actInfo.pEntity->GetId());
			if (!pActor)
			{
				DRX_ASSERT_MESSAGE(0, "Provided entity must be an IActor");
				DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_WARNING, "Provided entity %s must be an IActor", m_actInfo.pEntity->GetName());
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			assert(gEnv && gEnv->pGame && gEnv->pGame->GetIGameFramework() && gEnv->pGame->GetIGameFramework()->GetIItemSystem());
			IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();

			IInventory* pInventory = pActor->GetInventory();
			if (!pInventory)
			{
				DRX_ASSERT_MESSAGE(0, "Actor has no inventory");
				DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_WARNING, "Actor %s has no inventory", m_actInfo.pEntity->GetName());
				CancelSequenceAndActivateOutputPort(OutputPort_Done);
				return;
			}

			pInventory->SetHolsteredItem(EntityId(0));	// otherwise trying to holster the new weapon later on will not work (i. e. will do nothing)

			// draw the weapon
			const string& weaponName = GetPortString(&m_actInfo, InputPort_WeaponName);
			pItemSystem->SetActorItem(pActor, weaponName.c_str(), false);

			FinishSequenceActionAndActivateOutputPort(OutputPort_Done);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_FLOW_NODE("AISequence:WeaponDraw", CFlowNode_AISequenceAction_WeaponDraw)
REGISTER_FLOW_NODE("AISequence:WeaponHolster", CFlowNode_AISequenceAction_WeaponHolster)
REGISTER_FLOW_NODE("AISequence:WeaponDrawFromInventory", CFlowNode_AISequenceAction_WeaponDrawFromInventory)
