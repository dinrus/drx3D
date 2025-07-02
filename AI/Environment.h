// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AIEnvironment.h
//  Created:     29/02/2008 by Matthew
//  Описание: Simple global struct for accessing major module pointers
//  Notes:       auto_ptrs to handle init/delete would be great,
//               but exposing autoptrs to the rest of the code needs thought
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AIENVIRONMENT
#define __AIENVIRONMENT

#pragma once

#include <drx3D/AI/IAISystem.h>

#include <drx3D/AI/Configuration.h>
#include <drx3D/AI/AIConsoleVariables.h>
#include <drx3D/AI/AISignalCRCs.h>

//////////////////////////////////////////////////////////////////////////
// AI system environment.
// For internal use only!
// Contain pointers to commonly needed modules for simple, efficient lookup
// Need a clear point in execution when these will all be valid
//////////////////////////////////////////////////////////////////////////
class ActorLookUp;
struct IGoalOpFactory;
class CObjectContainer;
class CStatsUpr;
class CTacticalPointSystem;
class CTargetTrackUpr;
struct IAIDebugRenderer;
class CPipeUpr;
namespace MNM
{
class PathfinderNavigationSystemUser;
}
class CMNMPathfinder;
class CNavigation;
class CAIActionUpr;
class CSmartObjectUpr;
class CPerceptionUpr;
class CCommunicationUpr;
class CCoverSystem;
namespace BehaviorTree
{
class BehaviorTreeUpr;
class GraftUpr;
}
namespace Perception
{
	class CAuditionMap;
}
class CVisionMap;
class CFactionMap;
class CFactionSystem;
class CGroupUpr;
class CCollisionAvoidanceSystem;
class CAIObjectUpr;
class NavigationSystem;
namespace AIActionSequence {
class SequenceUpr;
}
class ClusterDetector;
class CFormationUpr;

#ifdef DRXAISYS_DEBUG
class CAIRecorder;
struct IAIBubblesSystem;
#endif //DRXAISYS_DEBUG

struct SAIEnvironment
{
	AIConsoleVars            CVars;
	AISIGNALS_CRC            SignalCRCs;

	SConfiguration           configuration;

	ActorLookUp*             pActorLookUp;
	IGoalOpFactory*          pGoalOpFactory;
	CObjectContainer*        pObjectContainer;

	CStatsUpr*                       pStatsUpr;
	CTacticalPointSystem*                pTacticalPointSystem;
	CTargetTrackUpr*                 pTargetTrackUpr;
	CAIObjectUpr*                    pAIObjectUpr;
	CPipeUpr*                        pPipeUpr;
	MNM::PathfinderNavigationSystemUser* pPathfinderNavigationSystemUser;
	CMNMPathfinder*                      pMNMPathfinder; // superseded by NavigationSystem - remove when all links are cut
	CNavigation*                         pNavigation;    // superseded by NavigationSystem - remove when all links are cut
	CAIActionUpr*                    pAIActionUpr;
	CSmartObjectUpr*                 pSmartObjectUpr;

	CCommunicationUpr*               pCommunicationUpr;
	CCoverSystem*                        pCoverSystem;
	NavigationSystem*                    pNavigationSystem;
	BehaviorTree::BehaviorTreeUpr*   pBehaviorTreeUpr;
	BehaviorTree::GraftUpr*          pGraftUpr;
	Perception::CAuditionMap*            pAuditionMap;
	CVisionMap*                          pVisionMap;
	CFactionMap*                         pFactionMap;
	CFactionSystem*                      pFactionSystem;
	CGroupUpr*                       pGroupUpr;
	CCollisionAvoidanceSystem*           pCollisionAvoidanceSystem;
	struct IMovementSystem*              pMovementSystem;
	AIActionSequence::SequenceUpr*   pSequenceUpr;
	ClusterDetector*                     pClusterDetector;
	CFormationUpr*                   pFormationUpr;

#ifdef DRXAISYS_DEBUG
	IAIBubblesSystem* pBubblesSystem;
#endif

	IAISystem::GlobalRayCaster*          pRayCaster;
	IAISystem::GlobalIntersectionTester* pIntersectionTester;

	//more cache friendly
	IPhysicalWorld* pWorld;//TODO use this more, or eliminate it.

	SAIEnvironment();
	~SAIEnvironment();

	IAIDebugRenderer* GetDebugRenderer();
	IAIDebugRenderer* GetNetworkDebugRenderer();
	void              SetDebugRenderer(IAIDebugRenderer* pAIDebugRenderer);
	void              SetNetworkDebugRenderer(IAIDebugRenderer* pAIDebugRenderer);

#ifdef DRXAISYS_DEBUG
	CAIRecorder* GetAIRecorder();
	void         SetAIRecorder(CAIRecorder* pAIRecorder);
#endif //DRXAISYS_DEBUG

	void ShutDown();

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
private:
	IAIDebugRenderer* pDebugRenderer;
	IAIDebugRenderer* pNetworkDebugRenderer;

#ifdef DRXAISYS_DEBUG
	CAIRecorder* pRecorder;
#endif //DRXAISYS_DEBUG
};

extern SAIEnvironment gAIEnv;

#endif // __AIENVIRONMENT
