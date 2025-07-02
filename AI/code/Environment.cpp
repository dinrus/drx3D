// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AIEnvironment.cpp
//  Created:     29/02/2008 by Matthew
//  Описание: Simple global struct for accessing major module pointers
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/Environment.h>

#include <drx3D/AI/ObjectContainer.h>

#include <drx3D/AI/GoalOpFactory.h>
#include <drx3D/AI/StatsUpr.h>
#include <drx3D/AI/TacticalPointSystem.h>
#include <drx3D/AI/TargetTrackUpr.h>
#include <drx3D/AI/NullAIDebugRenderer.h>
#include <drx3D/AI/NavigationSystem.h>
#include <drx3D/AI/BehaviorTreeGraft.h>
#include <drx3D/AI/FormationUpr.h>

static CNullAIDebugRenderer nullAIRenderer;

SAIEnvironment::SAIEnvironment()
	: pActorLookUp(NULL)
	, pGoalOpFactory(NULL)
	, pObjectContainer(NULL)
	, pTacticalPointSystem(NULL)
	, pTargetTrackUpr(NULL)
	, pStatsUpr(NULL)
	, pPipeUpr(NULL)
	, pAIActionUpr(NULL)
	, pSmartObjectUpr(NULL)
	, pCommunicationUpr(NULL)
	, pCoverSystem(NULL)
	, pNavigationSystem(NULL)
	, pBehaviorTreeUpr(NULL)
	, pGraftUpr(NULL)
	, pAuditionMap(NULL)
	, pVisionMap(NULL)
	, pFactionMap(NULL)
	, pFactionSystem(NULL)
	, pGroupUpr(NULL)
	, pCollisionAvoidanceSystem(NULL)
	, pRayCaster(NULL)
	, pIntersectionTester(NULL)
	, pMovementSystem(NULL)
	, pSequenceUpr(NULL)
	, pAIObjectUpr(NULL)
	, pPathfinderNavigationSystemUser(NULL)
	, pMNMPathfinder(NULL)
	, pNavigation(NULL)
	, pClusterDetector(NULL)
	, pFormationUpr(NULL)
	, pWorld(NULL)
{
	SetDebugRenderer(0);
	SetNetworkDebugRenderer(0);

#ifdef DRXAISYS_DEBUG
	pRecorder = NULL;
	pBubblesSystem = NULL;
#endif //DRXAISYS_DEBUG
}

SAIEnvironment::~SAIEnvironment()
{
}

void SAIEnvironment::ShutDown()
{
	SAFE_DELETE(pActorLookUp);
	SAFE_DELETE(pFactionMap);
	SAFE_DELETE(pFactionSystem);
	SAFE_DELETE(pGoalOpFactory);
	SAFE_DELETE(pStatsUpr);
	SAFE_DELETE(pTacticalPointSystem);
	SAFE_DELETE(pTargetTrackUpr);
	SAFE_DELETE(pObjectContainer);
	SAFE_DELETE(pFormationUpr);
}

IAIDebugRenderer* SAIEnvironment::GetDebugRenderer()
{
	return pDebugRenderer;
}

IAIDebugRenderer* SAIEnvironment::GetNetworkDebugRenderer()
{
	return pNetworkDebugRenderer;
}

void SAIEnvironment::SetDebugRenderer(IAIDebugRenderer* pAIDebugRenderer)
{
	pDebugRenderer = pAIDebugRenderer ? pAIDebugRenderer : &nullAIRenderer;
}

void SAIEnvironment::SetNetworkDebugRenderer(IAIDebugRenderer* pAINetworkDebugRenderer)
{
	pNetworkDebugRenderer = pAINetworkDebugRenderer ? pAINetworkDebugRenderer : &nullAIRenderer;
}

#ifdef DRXAISYS_DEBUG

CAIRecorder* SAIEnvironment::GetAIRecorder()
{
	return pRecorder;
}

void SAIEnvironment::SetAIRecorder(CAIRecorder* pAIRecorder)
{
	pRecorder = pAIRecorder;
}

#endif //DRXAISYS_DEBUG

SAIEnvironment gAIEnv;
