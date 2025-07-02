// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 19:05:2009: Created by Federico Rebora
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameFlowBaseNode.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/EngineFacade/GameFacade.h>

void CGameFlowBaseNode::AddRef()
{
	++m_refs;
}

void CGameFlowBaseNode::Release()
{
	if (0 >= --m_refs)	delete this;
}

IFlowNodePtr CGameFlowBaseNode::Clone( SActivationInfo *pActInfo )
{
	return this;
}

bool CGameFlowBaseNode::SerializeXML( SActivationInfo *, const XmlNodeRef&, bool )
{
	return true;
}

void CGameFlowBaseNode::Serialize( SActivationInfo *, TSerialize ser )
{

}

void CGameFlowBaseNode::ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
{
	if (event == IFlowNode::eFE_Activate)
	{
		ProcessActivateEvent(pActInfo); 
	}
}

bool CGameFlowBaseNode::IsPortActive( SActivationInfo *pActInfo,i32 nPort ) const
{
	return pActInfo->pInputPorts[nPort].IsUserFlagSet();
}

bool CGameFlowBaseNode::IsBoolPortActive( SActivationInfo *pActInfo,i32 nPort ) const
{
	if (IsPortActive(pActInfo,nPort) && GetPortBool(pActInfo,nPort))
		return true;
	else
		return false;
}

EFlowDataTypes CGameFlowBaseNode::GetPortType( SActivationInfo *pActInfo,i32 nPort ) const
{
	return (EFlowDataTypes)pActInfo->pInputPorts[nPort].GetType();
}

const TFlowInputData& CGameFlowBaseNode::GetPortAny( SActivationInfo *pActInfo,i32 nPort ) const
{
	return pActInfo->pInputPorts[nPort];
}

bool CGameFlowBaseNode::GetPortBool( SActivationInfo *pActInfo,i32 nPort ) const
{
	bool* p_x = (pActInfo->pInputPorts[nPort].GetPtr<bool>());
	if (p_x != 0) return *p_x;
	SFlowNodeConfig config;
	const_cast<CGameFlowBaseNode*> (this)->GetConfiguration(config);
	GameWarning("CGameFlowBaseNode::GetPortBool: Node=%p Port=%d '%s' Tag=%d -> Not a bool tag!", this, nPort,
		config.pInputPorts[nPort].name,
		pActInfo->pInputPorts[nPort].GetTag());
	return false;
}

i32 CGameFlowBaseNode::GetPortInt( SActivationInfo *pActInfo,i32 nPort ) const
{
	i32 x = *(pActInfo->pInputPorts[nPort].GetPtr<i32>());
	return x;
}

EntityId CGameFlowBaseNode::GetPortEntityId( SActivationInfo *pActInfo,i32 nPort )
{
	EntityId x = *(pActInfo->pInputPorts[nPort].GetPtr<EntityId>());
	return x;
}

EntityId CGameFlowBaseNode::GetPortEntityId( SActivationInfo *pActInfo,i32 nPort ) const
{
	EntityId x = *(pActInfo->pInputPorts[nPort].GetPtr<EntityId>());
	return x;
}
float CGameFlowBaseNode::GetPortFloat( SActivationInfo *pActInfo,i32 nPort ) const
{
	float x = *(pActInfo->pInputPorts[nPort].GetPtr<float>());
	return x;
}

Vec3 CGameFlowBaseNode::GetPortVec3( SActivationInfo *pActInfo,i32 nPort ) const
{
	Vec3 x = *(pActInfo->pInputPorts[nPort].GetPtr<Vec3>());
	return x;
}

const string& CGameFlowBaseNode::GetPortString( SActivationInfo *pActInfo,i32 nPort ) const
{
	const string* p_x = (pActInfo->pInputPorts[nPort].GetPtr<string>());
	if (p_x != 0) return *p_x;
	const static string empty ("");
	SFlowNodeConfig config;
	const_cast<CGameFlowBaseNode*> (this)->GetConfiguration(config);
	GameWarning("CGameFlowBaseNode::GetPortString: Node=%p Port=%d '%s' Tag=%d -> Not a string tag!", this, nPort,
		config.pInputPorts[nPort].name,
		pActInfo->pInputPorts[nPort].GetTag());
	return empty;
}

// In single player, when the input entity is NULL, it returns true,  for backward compatibility
bool CGameFlowBaseNode::InputEntityIsLocalPlayer( const SActivationInfo* const pActInfo ) const
{
	bool bRet = true;

	if (pActInfo->pEntity) 
	{
		IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor( pActInfo->pEntity->GetId() );
		if (pActor!=gEnv->pGame->GetIGameFramework()->GetClientActor())
			bRet = false;
	}
	else
	{
		if (gEnv->bMultiplayer)
			bRet = false;
	}

	return bRet;
}


CGameFlowBaseNode::~CGameFlowBaseNode()
{

}

bool CGameFlowBaseNode::IsOutputConnected( SActivationInfo *pActInfo,i32 nPort ) const
{
	SFlowAddress addr( pActInfo->myID, nPort, true );
	return pActInfo->pGraph->IsOutputConnected( addr );
}

void CGameFlowBaseNode::ProcessActivateEvent(SActivationInfo* activationInfo)
{

}

CGameFlowBaseNode::CGameFlowBaseNode() : m_refs(0)
{

}

CG2AutoRegFlowNodeBase::CG2AutoRegFlowNodeBase(tukk className)
: m_className(className)
, m_next(0)
{
	if (!m_last)
	{
		m_first = this;
	}
	else
	{
		m_last->m_next = this;
	}

	m_last = this;
}

void CG2AutoRegFlowNodeBase::AddRef()
{

}

void CG2AutoRegFlowNodeBase::Release()
{

}
