// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ProceduralContextMovementControlMethod.h>


DRXREGISTER_CLASS( CProceduralContextMovementControlMethod );


//////////////////////////////////////////////////////////////////////////
CProceduralContextMovementControlMethod::CProceduralContextMovementControlMethod()
{
}


//////////////////////////////////////////////////////////////////////////
CProceduralContextMovementControlMethod::~CProceduralContextMovementControlMethod()
{
}


//////////////////////////////////////////////////////////////////////////
void CProceduralContextMovementControlMethod::Update( float timePassedSeconds )
{
	IAnimatedCharacter* pAnimatedCharacter = GetAnimatedCharacter();
	IF_UNLIKELY ( pAnimatedCharacter == NULL )
	{
		return;
	}

	EMovementControlMethod horizontal = eMCM_Undefined;
	EMovementControlMethod vertical = eMCM_Undefined;

	const size_t requestCount = m_requestList.GetCount();
	for ( size_t i = 0; i < requestCount; ++i )
	{
		const SMCMRequest& request = m_requestList.GetRequest( i );
		horizontal = ( request.horizontal != eMCM_Undefined ) ? request.horizontal : horizontal;
		vertical = ( request.vertical != eMCM_Undefined ) ? request.vertical : vertical;
	}

	pAnimatedCharacter->SetMovementControlMethods( eMCMSlot_Animation, horizontal, vertical, "ProcContextUpdate" );
}


//////////////////////////////////////////////////////////////////////////
u32 CProceduralContextMovementControlMethod::RequestMovementControlMethod( const EMovementControlMethod horizontal, const EMovementControlMethod vertical )
{
	SMCMRequest request;
	request.horizontal = horizontal;
	request.vertical = vertical;

	return m_requestList.AddRequest( request );
}


//////////////////////////////////////////////////////////////////////////
void CProceduralContextMovementControlMethod::CancelRequest( u32k cancelRequestId )
{
	m_requestList.RemoveRequest( cancelRequestId );
}


//////////////////////////////////////////////////////////////////////////
IAnimatedCharacter* CProceduralContextMovementControlMethod::GetAnimatedCharacter() const
{
	const EntityId entityId = m_entity->GetId();
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor( entityId );
	IAnimatedCharacter* pAnimatedCharacter = ( pActor != NULL ) ? pActor->GetAnimatedCharacter() : NULL;
	return pAnimatedCharacter;
}