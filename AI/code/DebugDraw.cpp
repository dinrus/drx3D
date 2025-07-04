// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   DebugDraw.cpp
   $Id$
   Описание: move all the debug drawing functionality from AISystem. Need to clean it up eventually and
   make nice debug output functions, instead of one huge.

   -------------------------------------------------------------------------
   История:
   - 29:11:2004   19:02 : Created by Kirill Bulatsev

 *********************************************************************/

#include <drx3D/AI/StdAfx.h>

#ifdef DRXAISYS_DEBUG

// AI includes
#include <drx3D/AI/CAISystem.h>
#include <drx3D/AI/DebugDrawContext.h>
#include <drx3D/AI/AILog.h>

#include <drx3D/AI/Puppet.h>
#include <drx3D/AI/AIVehicle.h>
#include <drx3D/AI/GoalPipe.h>
#include <drx3D/AI/GoalOp.h>
#include <drx3D/AI/AIPlayer.h>
#include <drx3D/AI/PipeUser.h>
#include <drx3D/AI/Leader.h>
#include <drx3D/AI/SmartObjects.h>
#include <drx3D/AI/PathFollower.h>
#include <drx3D/AI/Shape.h>
#include <drx3D/AI/StatsUpr.h>
#include <drx3D/AI/FireCommand.h>

// (MATT) TODO Get a lightweight DebugDraw interface rather than pulling in this header file {2008/12/04}
#include <drx3D/AI/TacticalPointSystem.h>
#include <drx3D/AI/TargetTrackUpr.h>
#include <drx3D/AI/CommunicationUpr.h>
#include <drx3D/AI/CoverSystem.h>
#include <drx3D/AI/NavigationSystem.h>
#include <drx3D/AI/CollisionAvoidanceSystem.h>
#include <drx3D/AI/GroupUpr.h>
#include <drx3D/AI/CentralInterestUpr.h>
#include <drx3D/AI/PersonalInterestUpr.h>
#include <drx3D/AI/BehaviorTreeUpr.h>
#include <drx3D/AI/FormationUpr.h>

#include <drx3D/AI/IAIBubblesSystem.h>
#include <drx3D/AI/IAISystemComponent.h>

#define whiteTrans ColorB(255, 255, 255, 179)
#define redTrans   ColorB(255, 0, 0, 179)

void CAISystem::DebugDrawRecorderRange() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	const float fRecorderDrawStart = m_recorderDebugContext.fStartPos;
	const float fRecorderDrawEnd = m_recorderDebugContext.fEndPos;
	const float fRecorderDrawCursor = m_recorderDebugContext.fCursorPos;

	const bool bTimelineValid = (fabsf(fRecorderDrawEnd - fRecorderDrawStart) > FLT_EPSILON);
	const bool bCursorValid = bTimelineValid ? (fRecorderDrawCursor >= fRecorderDrawStart && fRecorderDrawCursor <= fRecorderDrawEnd) : (fRecorderDrawCursor > FLT_EPSILON);

	const ColorB colGray(128, 128, 128, 255);

	CDebugDrawContext dc;

	// Debug all of the objects
	TDebugObjectsArray::const_iterator itObject = m_recorderDebugContext.DebugObjects.begin();
	TDebugObjectsArray::const_iterator itObjectEnd = m_recorderDebugContext.DebugObjects.end();
	for (; itObject != itObjectEnd; ++itObject)
	{
		const SAIRecorderObjectDebugContext& objectContext = *itObject;
		if (false == objectContext.bEnableDrawing)
			continue;

		CAIObject* pAIObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(objectContext.sName.c_str());
		if (!pAIObject)
			continue;

		IAIDebugRecord* pRecord = pAIObject->GetAIDebugRecord();
		if (!pRecord)
			continue;

		IAIDebugStream* pPosStream = pRecord->GetStream(IAIRecordable::E_AGENTPOS);
		IAIDebugStream* pDirStream = pRecord->GetStream(IAIRecordable::E_AGENTDIR);
		if (!pPosStream || !pDirStream)
			continue;

		const ColorB colDebug = objectContext.color;
		float fCurrPosTime = 0.0f;
		float fCurrDirTime = 0.0f;
		Vec3* pPos = NULL;
		Vec3* pDir = NULL;

		// Draw range
		if (bTimelineValid && fRecorderDrawStart < pPosStream->GetEndTime() && fRecorderDrawEnd > pPosStream->GetStartTime())
		{
			pPosStream->Seek(fRecorderDrawStart);
			pDirStream->Seek(fRecorderDrawStart);
			pPos = (Vec3*)(pPosStream->GetCurrent(fCurrPosTime));
			pDir = (Vec3*)(pDirStream->GetCurrent(fCurrDirTime));

			// Draw start cursor pos
			if (pPos)
			{
				const Vec3& vPos(*pPos);
				dc->DrawSphere(vPos, 0.25f, colGray);

				if (pDir)
				{
					const Vec3& vDir(*pDir);
					dc->DrawArrow(vPos, vDir, 0.25f, colGray);
				}

				dc->DrawCone(vPos + Vec3(0, 0, 5), Vec3(0, 0, -1), 0.5f, 4.0f, colGray);
				dc->Draw3dLabel(vPos + Vec3(0, 0, 0.8f), 1.0f, "%s START\n%.1fs", pAIObject->GetName(), fCurrPosTime);
			}

			i32 j = 0;
			while (fCurrPosTime <= fRecorderDrawEnd && pPosStream->GetCurrentIdx() < pPosStream->GetSize())
			{
				float fNextPosTime = 0.0f;
				float fNextDirTime = 0.0f;
				Vec3* pNextPos = (Vec3*)(pPosStream->GetNext(fNextPosTime));
				pDirStream->Seek(fNextPosTime);
				Vec3* pNextDir = (Vec3*)(pDirStream->GetNext(fNextDirTime));
				if (pPos && pNextPos)
				{
					const Vec3& vPos(*pPos);
					const Vec3& vNext(*pNextPos);
					const Vec3& vNextDir = (pNextDir ? *pNextDir : Vec3Constants<float>::fVec3_Zero);

					if ((j & 1) == 0)
					{
						dc->DrawLine(vPos, colDebug, vNext, colDebug);
						if (!vNextDir.IsZero())
							dc->DrawArrow(vNext, vNextDir, 0.25f, colDebug);
					}
					else
					{
						dc->DrawLine(vPos, colGray, vNext, colGray);
						if (!vNextDir.IsZero())
							dc->DrawArrow(vNext, vNextDir, 0.25f, colGray);
					}
				}

				fCurrPosTime = fNextPosTime;
				pPos = pNextPos;
				++j;
			}

			// Draw start end pos
			pPosStream->Seek(fRecorderDrawEnd);
			pDirStream->Seek(fRecorderDrawEnd);
			pPos = (Vec3*)(pPosStream->GetCurrent(fCurrPosTime));
			pDir = (Vec3*)(pDirStream->GetCurrent(fCurrDirTime));
			if (pPos)
			{
				const Vec3& vPos(*pPos);
				dc->DrawSphere(vPos, 0.25f, colGray);

				if (pDir)
				{
					const Vec3& vDir(*pDir);
					dc->DrawArrow(vPos, vDir, 0.25f, colGray);
				}

				dc->DrawCone(vPos + Vec3(0, 0, 5), Vec3(0, 0, -1), 0.5f, 4.0f, colGray);
				dc->Draw3dLabel(vPos + Vec3(0, 0, 0.8f), 1.0f, "%s END\n%.1fs", pAIObject->GetName(), fCurrPosTime);
			}
		}

		// Draw cursor current pos
		if (bCursorValid)
		{
			Vec3 vCurrPos;

			pPosStream->Seek(fRecorderDrawCursor);
			pDirStream->Seek(fRecorderDrawCursor);
			pPos = (Vec3*)(pPosStream->GetCurrent(fCurrPosTime));
			pDir = (Vec3*)(pDirStream->GetCurrent(fCurrDirTime));
			if (pPos)
			{
				const Vec3& vDir = (pDir ? *pDir : Vec3Constants<float>::fVec3_OneZ);

				// Create label text that depicts everything that happened at this moment from all streams
				string sCursorText;
				sCursorText.Format("%s CURRENT\n%.1fs", pAIObject->GetName(), fCurrPosTime);

				for (i32 i = IAIRecordable::E_NONE; i < IAIRecordable::E_COUNT; ++i)
				{
					IAIDebugStream* pEventStream = pRecord->GetStream((IAIRecordable::e_AIDbgEvent)i);
					if (!pEventStream)
						continue;

					string sShortLabel, sText;
					pEventStream->Seek(fRecorderDrawCursor);
					if (pEventStream->GetCurrentString(sShortLabel, fCurrPosTime) &&
					    fabsf(fCurrPosTime - fRecorderDrawCursor) <= 0.1f)
					{
						sText.Format("\n%s: %s", pEventStream->GetName(), sShortLabel.c_str());
						sCursorText += sText;
					}
				}

				vCurrPos = *pPos;
				dc->DrawSphere(vCurrPos, 0.25f, colDebug);
				dc->DrawCone(vCurrPos + Vec3(0, 0, 5), Vec3(0, 0, -1), 0.5f, 4.0f, colDebug);
				dc->Draw3dLabel(vCurrPos + Vec3(0, 0, 0.8f), 1.0f, "%s", sCursorText.c_str());
			}

			// Current attention target info
			IAIDebugStream* pAttTargetPosStream = pRecord->GetStream(IAIRecordable::E_ATTENTIONTARGETPOS);
			if (pAttTargetPosStream)
			{
				pAttTargetPosStream->Seek(fRecorderDrawCursor);
				Vec3* pAttTargetPos = (Vec3*)(pAttTargetPosStream->GetCurrent(fCurrPosTime));
				if (pAttTargetPos)
				{
					string sName = "Att Target: ";

					IAIDebugStream* pAttTargetStream = pRecord->GetStream(IAIRecordable::E_ATTENTIONTARGET);
					if (pAttTargetStream)
					{
						pAttTargetStream->Seek(fRecorderDrawCursor);
						sName += (tukk)(pAttTargetStream->GetCurrent(fCurrPosTime));
					}
					else
					{
						sName += "Unknown";
					}

					Vec3 vPos(*pAttTargetPos);
					ColorB colAttTargetDebug = colDebug;
					colAttTargetDebug.a /= 2;
					dc->DrawSphere(vPos, 0.25f, colAttTargetDebug);
					dc->DrawCone(vPos + Vec3(0, 0, 5), Vec3(0, 0, -1), 0.5f, 4.0f, colAttTargetDebug);
					dc->DrawLine(vCurrPos, colDebug, vPos, colAttTargetDebug);
					dc->Draw3dLabel(vPos + Vec3(0, 0, 0.8f), 1.0f, "%s", sName.c_str());
				}
			}
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawDamageControlGraph() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	Vec3 camPos = dc->GetCameraPos();

	static std::vector<CPuppet*> shooters;
	shooters.clear();

	AIActorSet::const_iterator it = m_enabledAIActorsSet.begin();
	AIActorSet::const_iterator itEnd = m_enabledAIActorsSet.end();
	for (; it != itEnd; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;

		CPuppet* pPuppet = pAIActor->CastToCPuppet();
		if (!pPuppet)
			continue;

		if (pPuppet->CastToCAIVehicle())
			continue;

		if (!pPuppet->m_targetDamageHealthThrHistory)
			continue;

		if (Distance::Point_PointSq(camPos, pPuppet->GetPos()) > sqr(150.f))
			continue;

		shooters.push_back(pPuppet);
	}

	if (shooters.empty())
		return;

	dc->Init2DMode();
	dc->SetAlphaBlended(true);
	dc->SetBackFaceCulling(false);

	const Vec3 u(1, 0, 0);
	const Vec3 v(0, -1, 0);
	const Vec3 w(0, 0, 1);

	static std::vector<Vec3> values;

	if (gAIEnv.CVars.DebugDrawDamageControl > 2)
	{
		// Combined graph
		static std::vector<CAIActor*> targets;
		targets.clear();
		for (u32 i = 0; i < shooters.size(); ++i)
		{
			CPuppet* pPuppet = shooters[i];
			if (pPuppet->GetAttentionTarget())
			{
				CAIActor* pTargetActor = 0;
				CAIObject* pTarget = (CAIObject*)pPuppet->GetAttentionTarget();
				assert(pTarget);
				if (pTarget->IsAgent())
					pTargetActor = pTarget->CastToCAIActor();
				else
				{
					CAIActor* pActor = CastToCAIActorSafe(pTarget->GetAssociation().GetAIObject());
					if (pActor && pActor->IsAgent())
						pTargetActor = pActor;
				}
				if (pTargetActor)
				{
					for (u32 j = 0; j < targets.size(); ++j)
					{
						if (pTargetActor == targets[j])
						{
							pTargetActor = 0;
							break;
						}
					}
					if (pTargetActor)
						targets.push_back(pTargetActor);
				}
			}
		}

		const Vec3 u2(1, 0, 0);
		const Vec3 v2(0, -1, 0);
		const Vec3 w2(0, 0, 1);

		const float sizex = 0.7f;
		const float sizey = 0.2f;

		Vec3 orig = Vec3(0.15f, 1 - 0.05f, 0);

		// dim BG
		ColorB bg1(0, 0, 0, 128);
		ColorB bg2(128, 128, 128, 64);
		dc->DrawTriangle(orig, bg1, orig + u2 * sizex, bg1, orig + u2 * sizex + v2 * sizey, bg2);
		dc->DrawTriangle(orig, bg1, orig + u2 * sizex + v2 * sizey, bg2, orig + v2 * sizey, bg2);

	#ifdef DRXAISYS_DEBUG
		CValueHistory<float>* history = 0;

		float timeLen = 1.0f;
		float maxVal = 1.0f;
		for (u32 i = 0; i < targets.size(); ++i)
		{
			if (!targets[i]->m_healthHistory) continue;

			float maxHealth = (float)targets[i]->GetProxy()->GetActorMaxHealth();
			float maxHealthArmor = (float)(targets[i]->GetProxy()->GetActorMaxHealth() + targets[i]->GetProxy()->GetActorMaxArmor());
			maxVal = max(maxVal, maxHealthArmor / maxHealth);

			timeLen = max(timeLen, (float)(targets[i]->m_healthHistory->GetMaxSampleCount() *
			                               targets[i]->m_healthHistory->GetSampleInterval()));
		}

		// Draw value lines
		dc->DrawLine(orig, ColorB(255, 255, 255, 128), orig + u2 * sizex, ColorB(255, 255, 255, 128));
		dc->DrawLine(orig + v2 * sizey * (0.5f / maxVal), ColorB(255, 255, 255, 64), orig + u2 * sizex + v2 * sizey * (0.5f / maxVal), ColorB(255, 255, 255, 64));
		dc->DrawLine(orig + v2 * sizey * (1.0f / maxVal), ColorB(255, 255, 255, 128), orig + u2 * sizex + v2 * sizey * (1.0f / maxVal), ColorB(255, 255, 255, 128));

		// Draw time lines
		const float tickInterval = 1.0f; //seconds
		u32 tickCount = (u32)floor(timeLen / tickInterval);
		for (u32 j = 0; j < tickCount; ++j)
		{
			float t = ((j * tickInterval) / timeLen) * sizex;
			dc->DrawLine(orig + t * u2, ColorB(255, 255, 255, 64), orig + t * u2 + v2 * sizey, ColorB(255, 255, 255, 64));
		}

		const float s = 1.0f / maxVal;

		// Draw targets
		for (u32 i = 0; i < targets.size(); ++i)
		{
			history = targets[i]->m_healthHistory;
			if (history)
			{
				u32 n = history->GetSampleCount();
				values.resize(n);
				float dt = history->GetSampleInterval();

				for (u32 j = 0; j < n; ++j)
				{
					float val = min(history->GetSample(j) * s, 1.0f) * sizey;
					float t = ((timeLen - j * dt) / timeLen) * sizex;
					values[j] = orig + t * u2 + v2 * val;
				}
				if (n > 0)
					dc->DrawPolyline(&values[0], n, false, ColorB(255, 255, 255, 160), 1.5f);
			}
		}

		// Draw shooters
		for (u32 i = 0; i < shooters.size(); ++i)
		{
			history = shooters[i]->m_targetDamageHealthThrHistory;
			if (history)
			{
				u32 n = history->GetSampleCount();
				values.resize(n);
				float dt = history->GetSampleInterval();
				for (u32 j = 0; j < n; ++j)
				{
					float val = min(history->GetSample(j) * s, 1.0f) * sizey;
					float t = ((timeLen - j * dt) / timeLen) * sizex;
					values[j] = orig + t * u2 + v2 * val;
				}
				if (n > 0)
					dc->DrawPolyline(&values[0], n, false, ColorB(240, 32, 16, 240));
			}
		}
	#endif // ifdef _DEBUG
	}
	else
	{
		// Separate graphs
		const float spacingy = min(0.9f / (shooters.size() + 1.f), 0.2f);
		const float sizex = 0.25f;
		const float sizey = spacingy * 0.95f;

		i32 sw = dc->GetWidth();
		i32 sh = dc->GetHeight();

		const ColorB white(0, 192, 255);

		for (u32 i = 0; i < shooters.size(); ++i)
		{
			CPuppet* pPuppet = shooters[i];
			Vec3 orig = Vec3(0.05f, 0.05f + (i + 1) * spacingy, 0);

			// dim BG
			ColorB bg1(0, 0, 0, 128);
			ColorB bg2(128, 128, 128, 64);
			dc->DrawTriangle(orig, bg1, orig + u * sizex, bg1, orig + u * sizex + v * sizey, bg2);
			dc->DrawTriangle(orig, bg1, orig + u * sizex + v * sizey, bg2, orig + v * sizey, bg2);

	#ifdef DRXAISYS_DEBUG
			// Draw time lines
			float timeLen = pPuppet->m_targetDamageHealthThrHistory->GetMaxSampleCount() *
			                pPuppet->m_targetDamageHealthThrHistory->GetSampleInterval();
			const float tickInterval = 1.0f; //seconds
			u32 tickCount = (u32)floor(timeLen / tickInterval);
			for (u32 j = 0; j < tickCount; ++j)
			{
				float t = ((j * tickInterval) / timeLen) * sizex;
				dc->DrawLine(orig + t * u, ColorB(255, 255, 255, 64), orig + t * u + v * sizey, ColorB(255, 255, 255, 64));
			}

			// Draw curve
			CValueHistory<float>* history = 0;

			float s = 1.0f;

			if (pPuppet->GetAttentionTarget())
			{
				CAIActor* pTargetActor = 0;
				CAIObject* pTarget = (CAIObject*)pPuppet->GetAttentionTarget();
				assert(pTarget);
				if (pTarget->IsAgent())
					pTargetActor = pTarget->CastToCAIActor();
				else
				{
					CAIActor* pActor = CastToCAIActorSafe(pTarget->GetAssociation().GetAIObject());
					if (pActor && pActor->IsAgent())
						pTargetActor = pActor;
				}

				if (pTargetActor)
				{
					history = pTargetActor->m_healthHistory;
					if (history)
					{
						float maxHealth = (float)pTargetActor->GetProxy()->GetActorMaxHealth();
						float maxHealthArmor = (float)(pTargetActor->GetProxy()->GetActorMaxHealth() + pTargetActor->GetProxy()->GetActorMaxArmor());
						float maxVal = maxHealthArmor / maxHealth;

						s = 1.0f / maxVal;

						// Draw value lines
						dc->DrawLine(orig, ColorB(255, 255, 255, 128), orig + u * sizex, ColorB(255, 255, 255, 128));
						dc->DrawLine(orig + v * sizey * 0.5f * s, ColorB(255, 255, 255, 64), orig + u * sizex + v * sizey * 0.5f * s, ColorB(255, 255, 255, 64));
						dc->DrawLine(orig + v * sizey * s, ColorB(255, 255, 255, 128), orig + u * sizex + v * sizey * s, ColorB(255, 255, 255, 128));

						u32 n = history->GetSampleCount();
						values.resize(n);
						float dt = history->GetSampleInterval();
						for (u32 j = 0; j < n; ++j)
						{
							float val = min(history->GetSample(j) * s, 1.0f) * sizey;
							float t = ((timeLen - j * dt) / timeLen) * sizex;
							values[j] = orig + t * u + v * val;
						}
						if (n > 0)
							dc->DrawPolyline(&values[0], n, false, ColorB(255, 255, 255, 160), 1.5f);
					}
				}
			}
			else
			{
				float maxVal = max(1.0f, pPuppet->m_targetDamageHealthThrHistory->GetMaxSampleValue());
				s = 1.0f / maxVal;
			}

			history = pPuppet->m_targetDamageHealthThrHistory;
			if (history)
			{
				u32 n = history->GetSampleCount();
				values.resize(n);
				float dt = history->GetSampleInterval();
				for (u32 j = 0; j < n; ++j)
				{
					float val = min(history->GetSample(j) * s, 1.0f) * sizey;
					float t = ((timeLen - j * dt) / timeLen) * sizex;
					values[j] = orig + t * u + v * val;
				}
				if (n > 0)
					dc->DrawPolyline(&values[0], n, false, ColorB(240, 32, 16, 240));
			}
	#endif

			tuk szZone = "";
			switch (pPuppet->m_targetZone)
			{
			case AIZONE_OUT:
				szZone = "Out";
				break;
			case AIZONE_WARN:
				szZone = "Warn";
				break;
			case AIZONE_COMBAT_NEAR:
				szZone = "Combat-Near";
				break;
			case AIZONE_COMBAT_FAR:
				szZone = "Combat-Far";
				break;
			case AIZONE_KILL:
				szZone = "Kill";
				break;
			case AIZONE_IGNORE:
				szZone = "Ignored";
				break;
			}

			// Shooter name
			dc->Draw2dLabel(orig.x * sw + 3, (orig.y - spacingy + sizey) * sh - 25, 1.2f, white, false, "%s", pPuppet->GetName());

			float aliveTime = pPuppet->GetTargetAliveTime();
			const float accuracy = pPuppet->GetParameters().m_fAccuracy;
			char szAlive[32] = "inf";
			if (accuracy > 0.001f)
				drx_sprintf(szAlive, "%.1fs", aliveTime / accuracy);

			IAIObject* pAttTarget = pPuppet->GetAttentionTarget();
			const float fFireReactionTime = (pAttTarget ? pPuppet->GetFiringReactionTime(pAttTarget->GetPos()) : 0.0f);
			const float fCurrentReactionTime = pPuppet->GetCurrentFiringReactionTime();
			const bool bFireReactionPassed = pPuppet->HasFiringReactionTimePassed();

			dc->Draw2dLabel(orig.x * sw + 3, (orig.y - spacingy + sizey) * sh - 10, 1.1f, (bFireReactionPassed ? whiteTrans : redTrans), false, "Thr: %.2f Alive: %s  React: %.1f / %.1f  Acc: %.3f  Zone: %s  %s",
			                pPuppet->m_targetDamageHealthThr, szAlive, max(fFireReactionTime - fCurrentReactionTime, 0.0f), fFireReactionTime,
			                accuracy, szZone, pPuppet->IsAllowedToHitTarget() ? "FIRE" : "");
		}
	}

}

void CAISystem::DrawDebugShape(const SDebugSphere& sphere)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->DrawSphere(sphere.pos, sphere.radius, sphere.color);
}

void CAISystem::DrawDebugShape(const SDebugBox& box)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->DrawOBB(box.obb, box.pos, true, box.color, eBBD_Faceted);
}

void CAISystem::DrawDebugShape(const SDebugLine& line)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->DrawLine(line.start, line.color, line.end, line.color, line.thickness);
}

void CAISystem::DrawDebugShape(const SDebugCylinder& cylinder)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->DrawCylinder(cylinder.pos, cylinder.dir, cylinder.radius, cylinder.height, cylinder.color);
}

void CAISystem::DrawDebugShape(const SDebugCone& cone)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->DrawCone(cone.pos, cone.dir, cone.radius, cone.height, cone.color);
}

template<typename ShapeContainer>
void CAISystem::DrawDebugShapes(ShapeContainer& shapes, float dt)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	for (u32 i = 0; i < shapes.size(); )
	{
		typename ShapeContainer::value_type& shape = shapes[i];
		// NOTE Mai 29, 2007: <pvl> draw it at least once
		DrawDebugShape(shape);

		shape.time -= dt;
		if (shape.time < 0)
		{
			shapes[i] = shapes.back();
			shapes.pop_back();
		}
		else
		{
			++i;
		}
	}
}

void CAISystem::DebugDrawFakeTracers() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->SetAlphaBlended(true);

	for (size_t i = 0; i < m_DEBUG_fakeTracers.size(); ++i)
	{
		Vec3 p0 = m_DEBUG_fakeTracers[i].p0;
		Vec3 p1 = m_DEBUG_fakeTracers[i].p1;
		Vec3 dir = p1 - p0;
		float u = 1 - m_DEBUG_fakeTracers[i].t / m_DEBUG_fakeTracers[i].tmax;
		p0 += dir * u * 0.5f;
		p1 = p0 + dir * 0.5f;

		float a = (m_DEBUG_fakeTracers[i].a * m_DEBUG_fakeTracers[i].t / m_DEBUG_fakeTracers[i].tmax) * 0.75f + 0.25f;
		Vec3 mid((p0 + p1) / 2);
		dc->DrawLine(p0, ColorB(128, 128, 128, 0), mid, ColorB(255, 255, 255, (u8)(255 * a)), 6.0f);
		dc->DrawLine(p1, ColorB(128, 128, 128, 0), mid, ColorB(255, 255, 255, (u8)(255 * a)), 6.0f);
	}
}

void CAISystem::DebugDrawFakeHitEffects() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->SetAlphaBlended(true);

	for (size_t i = 0; i < m_DEBUG_fakeHitEffect.size(); ++i)
	{
		float a = m_DEBUG_fakeHitEffect[i].t / m_DEBUG_fakeHitEffect[i].tmax;
		Vec3 pos = m_DEBUG_fakeHitEffect[i].p + m_DEBUG_fakeHitEffect[i].n * (1 - sqr(a));
		float r = m_DEBUG_fakeHitEffect[i].r * (0.5f + (1 - a) * 0.5f);
		dc->DrawSphere(pos, r, ColorB(m_DEBUG_fakeHitEffect[i].c.r, m_DEBUG_fakeHitEffect[i].c.g, m_DEBUG_fakeHitEffect[i].c.b, (u8)(m_DEBUG_fakeHitEffect[i].c.a * a)));
	}
}

void CAISystem::DebugDrawFakeDamageInd() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	i32 mode = gAIEnv.CVars.DrawFakeDamageInd;

	CAIObject* pPlayer = GetPlayer();
	if (pPlayer)
	{
		// full screen quad
		CDebugDrawContext dc;
		dc->Init2DMode();
		dc->SetAlphaBlended(true);
		dc->SetBackFaceCulling(false);

		// Fullscreen flash for damage indication.
		if (m_DEBUG_screenFlash > 0.0f)
		{
			float f = m_DEBUG_screenFlash * 2.0f;
			float a = clamp_tpl(f, 0.0f, 1.0f);
			ColorB color(239, 50, 25, (u8)(a * 255));
			dc->DrawTriangle(Vec3(0, 0, 0), color, Vec3(1, 0, 0), color, Vec3(1, 1, 0), color);
			dc->DrawTriangle(Vec3(0, 0, 0), color, Vec3(1, 1, 0), color, Vec3(0, 1, 0), color);
		}

		// Damage indicator triangles
		Matrix33 basis;
		basis.SetRotationVDir(pPlayer->GetViewDir());
		Vec3 u = basis.GetColumn0();
		Vec3 v = basis.GetColumn1();
		Vec3 w = basis.GetColumn2();

		float rw = (float)dc->GetWidth();
		float rh = (float)dc->GetHeight();
		float as = rh / rw;

		const float FOV = dc->GetCameraFOV() * 0.95f;

		for (u32 i = 0; i < m_DEBUG_fakeDamageInd.size(); ++i)
		{
			Vec3 dir = m_DEBUG_fakeDamageInd[i].p - pPlayer->GetPos();
			dir.NormalizeSafe();
			float x = u.Dot(dir);
			float y = v.Dot(dir);
			float d = sqrtf(sqr(x) + sqr(y));
			if (d < 0.00001f)
				continue;
			x /= d;
			y /= d;
			float nx = y;
			float ny = -x;

			const float r0 = 0.15f;
			const float r1 = 0.25f;
			const float wi = 0.04f;

			float a = 1 - sqr(1 - m_DEBUG_fakeDamageInd[i].t / m_DEBUG_fakeDamageInd[i].tmax);

			bool targetVis = false;

			if (mode == 2)
			{
				Vec3 dirLocal(u.Dot(dir), v.Dot(dir), w.Dot(dir));
				if (dirLocal.y > 0.1f)
				{
					dirLocal.x /= dirLocal.y;
					dirLocal.z /= dirLocal.y;

					float tz = tanf(FOV / 2);
					float tx = rw / rh * tz;

					if (fabsf(dirLocal.x) < tx && fabsf(dirLocal.z) < tz)
						targetVis = true;
				}
			}

			ColorB color(239, 50, 25, (u8)(a * 255));

			if (!targetVis)
			{
				dc->DrawTriangle(Vec3(0.5f + (x * r0) * as, 0.5f - (y * r0), 0), color,
				                 Vec3(0.5f + (x * r1 + nx * wi) * as, 0.5f - (y * r1 + ny * wi), 0), color,
				                 Vec3(0.5f + (x * r1 - nx * wi) * as, 0.5f - (y * r1 - ny * wi), 0), color);
			}
			else
			{
				// Draw silhouette when on FOV.
				static std::vector<Vec3> pos2d;
				static std::vector<Vec3> pos2dSil;
				static std::vector<ColorB> colorSil;
				pos2d.resize(m_DEBUG_fakeDamageInd[i].verts.size());

				for (u32 j = 0, j_aux = 0; j < m_DEBUG_fakeDamageInd[i].verts.size(); ++j)
				{
					const Vec3& v2 = m_DEBUG_fakeDamageInd[i].verts[j];
					Vec3& o = pos2d[j_aux];

					if (dc->ProjectToScreen(v2.x, v2.y, v2.z, &o.x, &o.y, &o.z))
					{
						o.x /= 100.0f;
						o.y /= 100.0f;
						++j_aux;
					}
				}

				pos2dSil.clear();
				ConvexHull2D(pos2dSil, pos2d);

				if (pos2dSil.size() > 2)
				{
					colorSil.resize(pos2dSil.size());
					float miny = FLT_MAX, maxy = -FLT_MAX;
					for (u32 j = 0; j < pos2dSil.size(); ++j)
					{
						miny = min(miny, pos2dSil[j].y);
						maxy = max(maxy, pos2dSil[j].y);
					}
					float range = maxy - miny;
					if (range > 0.0001f)
						range = 1.0f / range;
					for (u32 j = 0; j < pos2dSil.size(); ++j)
					{
						float aa = clamp_tpl((1 - (pos2dSil[j].y - miny) * range) * 2.0f, 0.0f, 1.0f);
						colorSil[j] = color;
						colorSil[j].a *= (u8)(aa * a);
					}
					dc->DrawPolyline(&pos2dSil[0], pos2dSil.size(), true, &colorSil[0], 2.0f);
				}
			}
		}

		// Draw ambient fire indicators
		if (gAIEnv.CVars.DebugDrawDamageControl > 1)
		{
			const Vec3& playerPos = pPlayer->GetPos();
			AIObjectOwners::const_iterator aio = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR);
			for (; aio != gAIEnv.pAIObjectUpr->m_Objects.end(); ++aio)
			{
				if (aio->first != AIOBJECT_ACTOR) break;
				CPuppet* pPuppet = aio->second.GetAIObject()->CastToCPuppet();
				if (!pPuppet) continue;
				if (!pPuppet->IsEnabled()) continue;
				if (Distance::Point_PointSq(playerPos, pPuppet->GetPos()) > sqr(150.0f)) continue;

				Vec3 dir = pPuppet->GetPos() - playerPos;
				float x = u.Dot(dir);
				float y = v.Dot(dir);
				float d = sqrtf(sqr(x) + sqr(y));
				if (d < 0.00001f)
					continue;
				x /= d;
				y /= d;
				float nx = y;
				float ny = -x;

				const float r0 = 0.25f;
				const float r1 = 0.28f;
				const float w2 = 0.01f;

				ColorB color(255, 255, 255, pPuppet->IsAllowedToHitTarget() ? 255 : 64);
				dc->DrawTriangle(Vec3(0.5f + (x * r0) * as, 0.5f - (y * r0), 0), color,
				                 Vec3(0.5f + (x * r1 + nx * w2) * as, 0.5f - (y * r1 + ny * w2), 0), color,
				                 Vec3(0.5f + (x * r1 - nx * w2) * as, 0.5f - (y * r1 - ny * w2), 0), color);
			}
		}
	}
}

// Draw rings around player to assist in gauging target distance
void CAISystem::DebugDrawPlayerRanges() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->SetAlphaBlended(true);

	CAIObject* player = GetPlayer();
	if (player && !player->GetPos().IsZero())
		dc->DrawCircles(player->GetPos(), 5.f, 20.f, 4, Vec3(1, 0, 0), Vec3(1, 1, 0));
}

// Draw Perception Indicators
void CAISystem::DebugDrawPerceptionIndicators()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	static CTimeValue lastTime(-1.0f);
	if (lastTime.GetSeconds() < 0.0f)
		lastTime = GetFrameStartTime();
	CTimeValue time = GetFrameStartTime();
	float dt = (time - lastTime).GetSeconds();
	lastTime = time;

	CDebugDrawContext dc;
	dc->SetAlphaBlended(true);

	for (std::list<SPerceptionDebugLine>::iterator lineIt = m_lstDebugPerceptionLines.begin(); lineIt != m_lstDebugPerceptionLines.end(); )
	{
		SPerceptionDebugLine& line = (*lineIt);
		line.time -= dt;
		if (line.time < 0)
			lineIt = m_lstDebugPerceptionLines.erase(lineIt);
		else
		{
			dc->DrawLine(line.start, line.color, line.end, line.color, line.thickness);
			++lineIt;
		}
	}
}

// Draw Perception Modifiers
void CAISystem::DebugDrawPerceptionModifiers()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;
	dc->SetAlphaBlended(true);

	ColorB color(20, 255, 255);
	PerceptionModifierShapeMap::iterator pmsi = m_mapPerceptionModifiers.begin(), pmsiEnd = m_mapPerceptionModifiers.end();
	for (; pmsi != pmsiEnd; ++pmsi)
	{
		SPerceptionModifierShape& shape = pmsi->second;
		if (shape.shape.empty()) continue;

		ListPositions::iterator first = shape.shape.begin();
		ListPositions::iterator second = shape.shape.begin();
		++second;
		for (; first != shape.shape.end(); ++first, ++second)
		{
			Vec3 firstTop(first->x, first->y, shape.aabb.max.z);
			Vec3 firstBottom(first->x, first->y, shape.aabb.min.z);

			if (second == shape.shape.end())
			{
				// Handle the last side of shape based on whether shape is open or closed
				if (!shape.closed)
				{
					dc->DrawLine(firstBottom, color, firstTop, color, 1.f);
					continue;
				}
				else
					second = shape.shape.begin();
			}

			Vec3 secondTop(second->x, second->y, shape.aabb.max.z);
			Vec3 secondBottom(second->x, second->y, shape.aabb.min.z);

			dc->DrawLine(firstBottom, color, secondBottom, color, 1.f);
			dc->DrawLine(firstBottom, color, firstTop, color, 1.f);
			dc->DrawLine(firstTop, color, secondTop, color, 1.f);
		}
	}
}

void CAISystem::DebugDrawTargetTracks() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	gAIEnv.pTargetTrackUpr->DebugDraw();
}

void CAISystem::DebugDrawDebugAgent()
{
	if (IEntity* entity = gEnv->pEntitySystem->GetEntity(m_agentDebugTarget))
	{
		if (IAIObject* ai = entity->GetAI())
		{
			// Show a visual cue to make it clear which entity we're debugging picked
			AddDebugSphere(ai->GetPos() + Vec3(0.0f, 0.0f, 0.5f), 0.1f, 0, 255, 0, 0.0f);
		}
	}
}

void CAISystem::DebugDrawNavigation() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	m_pNavigation->DebugDraw();
}

void CAISystem::DebugDrawLightUpr()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	m_lightUpr.DebugDraw();
}

void CAISystem::DebugDrawP0AndP1() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;

	IEntity* ent0 = gEnv->pEntitySystem->FindEntityByName("p0");
	IEntity* ent1 = gEnv->pEntitySystem->FindEntityByName("p1");
	if (ent0 && ent1)
	{
		Vec3 p0 = ent0->GetWorldPos();
		Vec3 p1 = ent1->GetWorldPos();

		dc->DrawSphere(p0, 0.7f, ColorB(255, 255, 255, 128));
		dc->DrawLine(p0, ColorB(255, 255, 255), p1, ColorB(255, 255, 255));

		Vec3 hit;
		float hitDist = 0;
		if (IntersectSweptSphere(&hit, hitDist, Lineseg(p0, p1), 0.65f, AICE_STATIC))
		{
			Vec3 dir = p1 - p0;
			dir.Normalize();
			dc->DrawSphere(p0 + dir * hitDist, 0.7f, ColorB(255, 0, 0));
		}
	}
}

void CAISystem::DebugDrawPuppetPaths()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIObject* pAI = gAIEnv.pAIObjectUpr->GetAIObjectByName("DebugRequestPathInDirection");
	if (!pAI)
		return;

	u16 nType = pAI->GetType();
	if ((nType != AIOBJECT_ACTOR) && (nType != AIOBJECT_VEHICLE))
		return;

	CPipeUser* pPipeUser = pAI->CastToCPipeUser();
	if (!pPipeUser)
		return;

	static CTimeValue lastTime = GetAISystem()->GetFrameStartTime();
	CTimeValue thisTime = GetAISystem()->GetFrameStartTime();
	const float regenTime = 1.0f;

	if ((thisTime - lastTime).GetSeconds() > regenTime)
	{
		lastTime = thisTime;
		pPipeUser->m_Path.Clear("DebugRequestPathInDirection");
		Vec3 vStartPos = pPipeUser->GetPhysicsPos();
		const float maxDist = 15.0f;
	}

	DebugDrawPathSingle(pPipeUser);
}

void CAISystem::DebugDrawCheckCapsules() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	IEntity* ent = gEnv->pEntitySystem->FindEntityByName("CheckCapsule");
	if (ent)
	{
		// match the params in CheckWalkability
		const float radius = WalkabilityRadius;
		const Vec3 dir(0, 0, 0.9f);

		const Vec3& pos = ent->GetPos();

		bool result = OverlapCapsule(Lineseg(pos, pos + dir), radius, AICE_ALL);
		ColorB color;
		if (result)
			color.set(255, 0, 0, 255);
		else
			color.set(0, 255, 0, 255);
		CDebugDrawContext dc;
		dc->DrawSphere(pos, radius, color);
		dc->DrawSphere(pos + dir, radius, color);
	}
}

void CAISystem::DebugDrawCheckRay() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	IEntity* entFrom = gEnv->pEntitySystem->FindEntityByName("CheckRayFrom");
	IEntity* entTo = gEnv->pEntitySystem->FindEntityByName("CheckRayTo");
	if (entFrom && entTo)
	{
		const Vec3& posFrom = entFrom->GetPos();
		const Vec3& posTo = entTo->GetPos();

		bool result = OverlapSegment(Lineseg(posFrom, posTo), AICE_ALL);
		ColorB color;
		if (result)
			color.set(255, 0, 0, 255);
		else
			color.set(0, 255, 0, 255);

		CDebugDrawContext dc;
		dc->DrawLine(posFrom, color, posTo, color);
	}
}

void CAISystem::DebugDrawCheckFloorPos() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	IEntity* ent = gEnv->pEntitySystem->FindEntityByName("CheckFloorPos");
	if (ent)
	{
		const Vec3& pos = ent->GetPos();
		const float up = 0.5f;
		const float down = 5.0f;
		const float radius = 0.1f;
		Vec3 floorPos;
		bool result = GetFloorPos(floorPos, pos, up, down, radius, AICE_ALL);
		ColorB color;
		if (result)
			color.set(0, 255, 0, 255);
		else
			color.set(255, 0, 0, 255);

		CDebugDrawContext dc;
		dc->DrawSphere(floorPos, radius, color);
	}
}

void CAISystem::DebugDrawCheckGravity() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	IEntity* ent = gEnv->pEntitySystem->FindEntityByName("CheckGravity");
	if (ent)
	{
		Vec3 pos = ent->GetPos();
		Vec3 gravity;
		pe_params_buoyancy junk;
		gEnv->pPhysicalWorld->CheckAreas(pos, gravity, &junk);
		ColorB color(255, 255, 255);
		const float lenScale = 0.3f;
		CDebugDrawContext dc;
		dc->DrawLine(pos, color, pos + lenScale * gravity, color);
	}
}

// Draw debug shapes
// These shapes come from outside the AI system (such as some debug code in script bind in DinrusAction) but related to AI.
void CAISystem::DebugDrawDebugShapes()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	const float dt = GetFrameDeltaTime();
	DrawDebugShapes(m_vecDebugLines, dt);
	DrawDebugShapes(m_vecDebugBoxes, dt);
	DrawDebugShapes(m_vecDebugSpheres, dt);
	DrawDebugShapes(m_vecDebugCylinders, dt);
	DrawDebugShapes(m_vecDebugCones, dt);
}

void CAISystem::DebugDrawGroupTactic()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	for (AIGroupMap::iterator it = m_mapAIGroups.begin(); it != m_mapAIGroups.end(); ++it)
		it->second->DebugDraw();
}

void CAISystem::DebugDrawDamageParts() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIPlayer* pPlayer = CastToCAIPlayerSafe(GetPlayer());
	CDebugDrawContext dc;
	if (pPlayer && pPlayer->GetDamageParts())
	{
		DamagePartVector* parts = pPlayer->GetDamageParts();
		for (DamagePartVector::iterator it = parts->begin(); it != parts->end(); ++it)
		{
			SAIDamagePart& part = *it;
			dc->Draw3dLabel(part.pos, 1, "^ DMG:%.2f\n  VOL:%.1f", part.damageMult, part.volume);
		}
	}
}

// Draw the approximate stance size for the player.
void CAISystem::DebugDrawStanceSize() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIObject* playerObject = GetPlayer();
	if (playerObject)
	{
		if (CAIPlayer* player = playerObject->CastToCAIPlayer())
		{
			const SAIBodyInfo& bodyInfo = player->GetBodyInfo();
			Vec3 pos = player->GetPhysicsPos();
			AABB aabb(bodyInfo.stanceSize);

			aabb.Move(pos);
			CDebugDrawContext dc;
			dc->DrawAABB(aabb, true, ColorB(255, 255, 255, 128), eBBD_Faceted);
		}
	}
}

void CAISystem::DebugDrawForceAGSignal() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	ColorB colorRed(255, 0, 0);
	tukk szInput = gAIEnv.CVars.ForceAGSignal;

	CDebugDrawContext dc;
	dc->Draw2dLabel(10.f, dc->GetHeight() - 90.f, 2.0f, colorRed, false, "Forced AG Signal Input: %s", szInput);
}

void CAISystem::DebugDrawForceAGAction() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	ColorB colorRed(255, 0, 0);
	tukk szInput = gAIEnv.CVars.ForceAGAction;

	CDebugDrawContext dc;
	dc->Draw2dLabel(10.f, dc->GetHeight() - 60.f, 2.0f, colorRed, false, "Forced AG Action Input: %s", szInput);
}

void CAISystem::DebugDrawForceStance() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	ColorB colorRed(255, 0, 0);
	tukk szStance = GetStanceName(gAIEnv.CVars.ForceStance);

	CDebugDrawContext dc;
	dc->Draw2dLabel(10.f, dc->GetHeight() - 30.f, 2.0f, colorRed, false, "Forced Stance: %s", szStance);
}

void CAISystem::DebugDrawForcePosture() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	ColorB colorRed(255, 0, 0);
	tukk szPosture = gAIEnv.CVars.ForcePosture;

	CDebugDrawContext dc;
	dc->Draw2dLabel(10.f, dc->GetHeight() - 30.f, 2.0f, colorRed, false, "Forced Posture: %s", szPosture);
}

// Player actions
void CAISystem::DebugDrawPlayerActions() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIPlayer* pPlayer = CastToCAIPlayerSafe(GetPlayer());
	if (pPlayer)
		pPlayer->DebugDraw();
}

void CAISystem::DebugDrawCrowdControl()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;

	Vec3 camPos = dc->GetCameraPos();

	VectorSet<const CAIActor*> avoidedActors;

	AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR);
	for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_ACTOR)
			break;
		CAIObject* obj = ai->second.GetAIObject();
		CPuppet* pPuppet = obj->CastToCPuppet();
		if (!pPuppet)
			continue;
		if (!pPuppet->IsEnabled())
			continue;
		if (!pPuppet->m_steeringEnabled)
			continue;
		if (Distance::Point_PointSq(camPos, pPuppet->GetPos()) > sqr(150.0f))
			continue;

		Vec3 center = pPuppet->GetPhysicsPos() + Vec3(0, 0, 0.75f);
		const float rad = pPuppet->GetMovementAbility().pathRadius;

		/* Draw circle sector */
		pPuppet->m_steeringOccupancy.DebugDraw(center, ColorB(0, 196, 255, 240));

		for (u32 i = 0, ni = pPuppet->m_steeringObjects.size(); i < ni; ++i)
		{
			const CAIActor* pActor = pPuppet->m_steeringObjects[i]->CastToCAIActor();
			avoidedActors.insert(pActor);
		}
	}

	for (u32 i = 0, ni = avoidedActors.size(); i < ni; ++i)
	{
		const CAIActor* pActor = avoidedActors[i];
		if (pActor->GetType() == AIOBJECT_VEHICLE)
		{
			IEntity* pActorEnt = pActor->GetEntity();
			AABB localBounds;
			pActorEnt->GetLocalBounds(localBounds);
			const Matrix34& tm = pActorEnt->GetWorldTM();

			SAIRect3 r;
			GetFloorRectangleFromOrientedBox(tm, localBounds, r);
			// Extend the box based on velocity.
			Vec3 vel = pActor->GetVelocity();
			float speedu = r.axisu.Dot(vel) * 0.25f;
			float speedv = r.axisv.Dot(vel) * 0.25f;
			if (speedu > 0)
				r.max.x += speedu;
			else
				r.min.x += speedu;
			if (speedv > 0)
				r.max.y += speedv;
			else
				r.min.y += speedv;

			Vec3 rect[4];
			rect[0] = r.center + r.axisu * r.min.x + r.axisv * r.min.y;
			rect[1] = r.center + r.axisu * r.max.x + r.axisv * r.min.y;
			rect[2] = r.center + r.axisu * r.max.x + r.axisv * r.max.y;
			rect[3] = r.center + r.axisu * r.min.x + r.axisv * r.max.y;

			/* ??? */
			dc->DrawPolyline(rect, 4, true, ColorB(0, 196, 255, 128));
		}
		else
		{
			Vec3 pos = avoidedActors[i]->GetPhysicsPos() + Vec3(0, 0, 0.5f);
			const float rad = avoidedActors[i]->GetMovementAbility().pathRadius;
			/* Draw additional cyan circle around puppets */
			dc->DrawCircleOutline(pos, rad, ColorB(0, 196, 255, 128));
		}
	}
}

static tukk GetMovemengtUrgencyLabel(i32 idx)
{
	switch (idx > 0 ? idx : -idx)
	{
	case 0:
		return "Zero";
		break;
	case 1:
		return "Slow";
		break;
	case 2:
		return "Walk";
		break;
	case 3:
		return "Run";
		break;
	default:
		break;
	}
	return "Sprint";
}

void CAISystem::DebugDrawAdaptiveUrgency() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;

	Vec3 camPos = dc->GetCameraPos();

	i32 mode = gAIEnv.CVars.DebugDrawAdaptiveUrgency;

	AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR);
	for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_ACTOR)
			break;
		CAIObject* obj = ai->second.GetAIObject();
		CPuppet* pPuppet = obj->CastToCPuppet();
		if (!pPuppet)
			continue;
		if (!pPuppet->IsEnabled())
			continue;
		if (Distance::Point_PointSq(camPos, pPuppet->GetPos()) > sqr(150.0f))
			continue;

		if (pPuppet->m_adaptiveUrgencyScaleDownPathLen <= 0.0001f)
			continue;

		i32 minIdx = MovementUrgencyToIndex(pPuppet->m_adaptiveUrgencyMin);
		i32 maxIdx = MovementUrgencyToIndex(pPuppet->m_adaptiveUrgencyMax);
		i32 curIdx = MovementUrgencyToIndex(pPuppet->GetState().fMovementUrgency);

		const SAIBodyInfo& bi = pPuppet->GetBodyInfo();

		AABB stanceSize(bi.stanceSize);
		stanceSize.Move(pPuppet->GetPhysicsPos());

		Vec3 top = stanceSize.GetCenter() + Vec3(0, 0, stanceSize.GetSize().z / 2 + 0.3f);

		if (mode > 1)
			dc->DrawAABB(stanceSize, false, ColorB(240, 220, 0, 128), eBBD_Faceted);

		if (curIdx < maxIdx)
		{
			dc->DrawAABB(stanceSize, true, ColorB(240, 220, 0, 128), eBBD_Faceted);
			dc->Draw3dLabel(top, 1.1f, "%s -> %s\n%d", GetMovemengtUrgencyLabel(maxIdx), GetMovemengtUrgencyLabel(curIdx), maxIdx - curIdx);
		}
	}
}

static void DrawRadarCircle(const Vec3& pos, float radius, const ColorB& color, const Matrix34& world, const Matrix34& screen)
{
	Vec3 last;

	float r = world.TransformVector(Vec3(radius, 0, 0)).GetLength();
	u32 n = (u32)((r * gf_PI2) / 5.0f);
	if (n < 5) n = 5;
	if (n > 50) n = 50;

	CDebugDrawContext dc;
	for (u32 i = 0; i < n; i++)
	{
		float a = (float)i / (float)(n - 1) * gf_PI2;
		Vec3 p = world.TransformPoint(pos + Vec3(cosf(a) * radius, sinf(a) * radius, 0));
		if (i > 0)
			dc->DrawLine(screen.TransformPoint(last), color, screen.TransformPoint(p), color);
		last = p;
	}
}

void CAISystem::DrawRadarPath(CPipeUser* pPipeUser, const Matrix34& world, const Matrix34& screen)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	tukk pName = gAIEnv.CVars.DrawPath;
	if (!pName)
		return;
	CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pName);
	if (pTargetObject)
	{
		CPipeUser* pTargetPipeUser = pTargetObject->CastToCPipeUser();
		if (!pTargetPipeUser)
			return;
		DebugDrawPathSingle(pTargetPipeUser);
		return;
	}
	if (strcmp(pName, "all"))
		return;

	// draw the first part of the path in a different colour
	if (!pPipeUser->m_OrigPath.GetPath().empty())
	{
		TPathPoints::const_iterator li, linext;
		li = pPipeUser->m_OrigPath.GetPath().begin();
		linext = li;
		++linext;
		// (MATT) BUG! Surely. m_Path or m_OrigPath, which is it? Appears elsewhere too. {2008/05/29}
		Vec3 endPt = pPipeUser->m_Path.GetNextPathPos();
		CDebugDrawContext dc;
		while (linext != pPipeUser->m_OrigPath.GetPath().end())
		{
			Vec3 p0 = world.TransformPoint(li->vPos);
			Vec3 p1 = world.TransformPoint(linext->vPos);
			dc->DrawLine(screen.TransformPoint(p0), ColorB(255, 0, 255), screen.TransformPoint(p1), ColorB(255, 0, 255));
			li = linext++;
		}
	}
}

void CAISystem::DebugDrawRadar()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	i32 size = gAIEnv.CVars.DrawRadar;
	if (size == 0)
		return;

	CDebugDrawContext dc;
	dc->Init2DMode();
	dc->SetAlphaBlended(true);
	dc->SetBackFaceCulling(false);

	i32 w = dc->GetWidth();
	i32 h = dc->GetHeight();

	i32 centerx = w / 2;
	i32 centery = h / 2;

	i32 radarDist = gAIEnv.CVars.DrawRadarDist;
	float worldSize = radarDist * 2.0f;

	Matrix34 worldToScreen;
	worldToScreen.SetIdentity();
	const CCamera& cam = GetISystem()->GetViewCamera();
	Vec3 camPos = cam.GetPosition();
	Vec3 camForward = cam.GetMatrix().TransformVector(FORWARD_DIRECTION);
	float rot(atan2f(camForward.x, camForward.y));
	// inv camera
	//	worldToScreen.SetRotationZ(-rot);
	worldToScreen.AddTranslation(-camPos);
	worldToScreen = Matrix34::CreateRotationZ(rot) * worldToScreen;
	// world to screenspace conversion
	float s = (float)size / worldSize;
	worldToScreen = Matrix34::CreateScale(Vec3(s, s, 0)) * worldToScreen;
	// offset the position to upper right corner
	worldToScreen.AddTranslation(Vec3(static_cast<float>(centerx), static_cast<float>(centery), 0.f));
	// Inverse Y
	worldToScreen = Matrix34::CreateScale(Vec3(1, -1, 0)) * worldToScreen;
	worldToScreen.AddTranslation(Vec3(0, static_cast<float>(h), 0));

	Matrix34 screenToNorm;
	screenToNorm.SetIdentity();
	// normalize to 0..1
	screenToNorm = Matrix34::CreateScale(Vec3(1.0f / (float)w, 1.0f / (float)h, 1)) * screenToNorm;

	// Draw 15m circle.
	DrawRadarCircle(camPos, 0.5f, ColorB(255, 255, 255, 128), worldToScreen, screenToNorm);
	for (i32 i = 0; i < (radarDist / 5); i++)
	{
		ColorB color(120, 120, 120, 64);
		if (i & 1) color.a = 128;
		DrawRadarCircle(camPos, (1 + i) * 5.0f, color, worldToScreen, screenToNorm);
	}

	const ColorB white(255, 255, 255);
	const ColorB green(137, 226, 9);
	const ColorB orange(255, 162, 0);
	const ColorB red(239, 50, 25);
	const ColorB black(0, 0, 0, 179);

	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itend = enabledAIActorsSet.end(); it != itend; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;

		CPuppet* pPuppet = pAIActor->CastToCPuppet();

		if (Distance::Point_PointSq(pAIActor->GetPos(), camPos) > sqr(radarDist * 1.25f))
			continue;

		float rad(pAIActor->GetParameters().m_fPassRadius);

		Vec3 pos, forw, right;
		pos = worldToScreen.TransformPoint(pAIActor->GetPos());
		forw = worldToScreen.TransformVector(pAIActor->GetViewDir() * rad);
		//			forw.NormalizeSafe();
		right.Set(-forw.y, forw.x, 0);

		ColorB color(255, 0, 0, 128);

		i32 alertness = pAIActor->GetProxy()->GetAlertnessState();
		if (alertness == 0)
			color = green;
		else if (alertness == 1)
			color = orange;
		else if (alertness == 2)
			color = red;
		color.a = 255;
		if (!pAIActor->IsEnabled())
			color.a = 64;

		const float arrowSize = 1.5f;

		dc->DrawTriangle(screenToNorm.TransformVector(pos - forw * 0.71f * arrowSize + right * 0.71f * arrowSize), color,
		                 screenToNorm.TransformVector(pos - forw * 0.71f * arrowSize - right * 0.71f * arrowSize), color,
		                 screenToNorm.TransformVector(pos + forw * arrowSize), color);
		DrawRadarCircle(pAIActor->GetPos(), rad, ColorB(255, 255, 255, 64), worldToScreen, screenToNorm);

		if (pPuppet)
			DrawRadarPath(pPuppet, worldToScreen, screenToNorm);

		if (pAIActor->GetState().fire)
		{
			ColorB fireColor(255, 26, 0);
			if (pPuppet && !pPuppet->IsAllowedToHitTarget())
				fireColor.set(255, 255, 255, 128);
			Vec3 tgt = worldToScreen.TransformPoint(pAIActor->GetState().vShootTargetPos);
			dc->DrawLine(screenToNorm.TransformVector(pos), fireColor, screenToNorm.TransformVector(tgt), fireColor);
		}

		float accuracy = 0.0f;
		if (pPuppet && pPuppet->GetFireTargetObject())
			accuracy = pPuppet->GetAccuracy(pPuppet->GetFireTargetObject());

		char szMsg[256];

		if (pPuppet && !pPuppet->IsAllowedToHitTarget())
			drx_sprintf(szMsg, "%s\nAcc:%.3f\nAMBIENT", pAIActor->GetName(), accuracy);
		else
			drx_sprintf(szMsg, "%s\nAcc:%.3f\n", pAIActor->GetName(), accuracy);

		dc->Draw2dLabel(pos.x + 1, pos.y - 1, 1.2f, black, true, "%s", szMsg);
		dc->Draw2dLabel(pos.x, pos.y, 1.2f, white, true, "%s", szMsg);
	}
}

void CAISystem::DebugDrawDistanceLUT()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (gAIEnv.CVars.DrawDistanceLUT < 1)
		return;
}

struct SSlopeTriangle
{
	SSlopeTriangle(const Triangle& tri, float slope) : triangle(tri), slope(slope) {}
	SSlopeTriangle() : triangle(Vec3(ZERO), Vec3(ZERO), Vec3(ZERO)), slope(0.0f) {}
	Triangle triangle;
	float    slope;
};

//====================================================================
// DebugDrawSteepSlopes
// Caches triangles first time this is enabled - then on subsequent
// calls just draws. Gets reset when the debug draw value changes
//====================================================================
void CAISystem::DebugDrawSteepSlopes()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	static std::vector<SSlopeTriangle> triangles;
	/// Current centre for the region we draw
	static Vec3 currentPos(0.0f, 0.0f, 0.0f);

	// Size of the area to calculate/use
	const float drawBoxWidth = 200.0f;
	// update when current position is greater than this (horizontally)
	// distance from lastPos
	const float drawUpdateDist = drawBoxWidth * 0.25f;
	const float zOffset = 0.05f;

	CDebugDrawContext dc;

	Vec3 playerPos = dc->GetCameraPos();
	playerPos.z = 0.0f;

	if ((playerPos - currentPos).GetLength() > drawUpdateDist)
	{
		currentPos = playerPos;
		triangles.resize(0); // force a refresh
	}
	if (gAIEnv.CVars.DebugDraw != 85)
	{
		triangles.resize(0);
		return;
	}

	if (gEnv->IsEditor())
	{
		triangles.resize(0);
	}

	float criticalSlopeUp = gAIEnv.CVars.SteepSlopeUpValue;
	float criticalSlopeAcross = gAIEnv.CVars.SteepSlopeAcrossValue;

	if (triangles.empty())
	{
		I3DEngine* pEngine = gEnv->p3DEngine;

		i32 terrainArraySize = pEngine->GetTerrainSize();
		float dx = pEngine->GetHeightMapUnitSize();

		float minX = currentPos.x - 0.5f * drawBoxWidth;
		float minY = currentPos.y - 0.5f * drawBoxWidth;
		float maxX = currentPos.x + 0.5f * drawBoxWidth;
		float maxY = currentPos.y + 0.5f * drawBoxWidth;
		i32 minIx = (i32)(minX / dx);
		i32 minIy = (i32)(minY / dx);
		i32 maxIx = (i32)(maxX / dx);
		i32 maxIy = (i32)(maxY / dx);
		Limit(minIx, 1, terrainArraySize - 1);
		Limit(minIy, 1, terrainArraySize - 1);
		Limit(maxIx, 1, terrainArraySize - 1);
		Limit(maxIy, 1, terrainArraySize - 1);

		// indices start at +1 so we can use the previous indices
		for (i32 ix = minIx; ix < maxIx; ++ix)
		{
			for (i32 iy = minIy; iy < maxIy; ++iy)
			{
				Vec3 v11(dx * ix, dx * iy, 0.0f);
				Vec3 v01(dx * (ix - 1), dx * iy, 0.0f);
				Vec3 v10(dx * ix, dx * (iy - 1), 0.0f);
				Vec3 v00(dx * (ix - 1), dx * (iy - 1), 0.0f);

				v11.z = zOffset + dc->GetDebugDrawZ(v11, true);
				v01.z = zOffset + dc->GetDebugDrawZ(v01, true);
				v10.z = zOffset + dc->GetDebugDrawZ(v10, true);
				v00.z = zOffset + dc->GetDebugDrawZ(v00, true);

				Vec3 n1 = ((v10 - v00) % (v01 - v00)).GetNormalized();
				float slope1 = fabs(sqrtf(1.0f - n1.z * n1.z) / n1.z);
				if (slope1 > criticalSlopeUp)
					triangles.push_back(SSlopeTriangle(Triangle(v00, v10, v01), slope1));
				else if (slope1 > criticalSlopeAcross)
					triangles.push_back(SSlopeTriangle(Triangle(v00, v10, v01), -slope1));

				Vec3 n2 = ((v11 - v10) % (v01 - v10)).GetNormalized();
				float slope2 = fabs(sqrtf(1.0f - n2.z * n2.z) / n2.z);
				if (slope2 > criticalSlopeUp)
					triangles.push_back(SSlopeTriangle(Triangle(v10, v11, v01), slope2));
				else if (slope2 > criticalSlopeAcross)
					triangles.push_back(SSlopeTriangle(Triangle(v10, v11, v01), -slope2));
			}
		}
	}

	u32 numTris = triangles.size();
	for (u32 i = 0; i < numTris; ++i)
	{
		const SSlopeTriangle& tri = triangles[i];
		ColorF color;
		// convert slope into 0-1 (the z-value of a unit vector having that slope)
		float slope = tri.slope > 0.0f ? tri.slope : -tri.slope;
		float a = sqrtf(slope * slope / (1.0f + slope * slope));
		if (tri.slope > 0.0f)
			color.set(1.0f - 0.5f * a, 0.0f, 0.0f, 1.0f);
		else
			color.set(0.0f, 1.0f - 0.5f * a, 1.0f, 1.0f);
		dc->DrawTriangle(tri.triangle.v0, color, tri.triangle.v1, color, tri.triangle.v2, color);
	}
}

//===================================================================
// DebugDrawVegetationCollision
//===================================================================
void CAISystem::DebugDrawVegetationCollision()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	float range = static_cast<float>(gAIEnv.CVars.DebugDrawVegetationCollisionDist);
	if (range < 0.1f)
		return;

	I3DEngine* pEngine = gEnv->p3DEngine;

	CDebugDrawContext dc;

	Vec3 playerPos = dc->GetCameraPos();
	playerPos.z = pEngine->GetTerrainElevation(playerPos.x, playerPos.y);

	AABB aabb(AABB::RESET);
	aabb.Add(playerPos + Vec3(range, range, range));
	aabb.Add(playerPos - Vec3(range, range, range));

	ColorB triCol(128, 255, 255);
	const float zOffset = 0.05f;

	IPhysicalEntity** pObstacles;
	i32 count = gAIEnv.pWorld->GetEntitiesInBox(aabb.min, aabb.max, pObstacles, ent_static | ent_ignore_noncolliding);
	for (i32 i = 0; i < count; ++i)
	{
		IPhysicalEntity* pPhysics = pObstacles[i];

		pe_status_pos status;
		status.ipart = 0;
		pPhysics->GetStatus(&status);

		Vec3 obstPos = status.pos;
		Matrix33 obstMat = Matrix33(status.q);
		obstMat *= status.scale;

		IGeometry* geom = status.pGeom;
		i32 type = geom->GetType();
		if (type == GEOM_TRIMESH)
		{
			const primitives::primitive* prim = geom->GetData();
			const mesh_data* mesh = static_cast<const mesh_data*>(prim);

			i32 numVerts = mesh->nVertices;
			i32 numTris = mesh->nTris;

			static std::vector<Vec3> vertices;
			vertices.resize(numVerts);

			for (i32 j = 0; j < numVerts; j++)
				vertices[j] = obstPos + obstMat * mesh->pVertices[j];

			for (i32 j = 0; j < numTris; ++j)
			{
				i32 vidx0 = mesh->pIndices[j * 3 + 0];
				i32 vidx1 = mesh->pIndices[j * 3 + 1];
				i32 vidx2 = mesh->pIndices[j * 3 + 2];

				Vec3 pt0 = vertices[vidx0];
				Vec3 pt1 = vertices[vidx1];
				Vec3 pt2 = vertices[vidx2];

				float z0 = pEngine->GetTerrainElevation(pt0.x, pt0.y);
				float z1 = pEngine->GetTerrainElevation(pt1.x, pt1.y);
				float z2 = pEngine->GetTerrainElevation(pt2.x, pt2.y);

				const float criticalAlt = 1.8f;
				if (pt0.z < z0 && pt1.z < z1 && pt2.z < z2)
					continue;
				if (pt0.z > z0 + criticalAlt && pt1.z > z1 + criticalAlt && pt2.z > z2 + criticalAlt)
					continue;
				pt0.z = z0 + zOffset;
				pt1.z = z1 + zOffset;
				pt2.z = z2 + zOffset;

				dc->DrawTriangle(pt0, triCol, pt1, triCol, pt2, triCol);
			}
		}
	}
}

//====================================================================
// DebugDrawDynamicHideObjects
//====================================================================
void CAISystem::DebugDrawDynamicHideObjects()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	float range = static_cast<float>(gAIEnv.CVars.DebugDrawDynamicHideObjectsRange);
	if (range < 0.1f)
		return;

	Vec3 cameraPos = GetISystem()->GetViewCamera().GetPosition();
	Vec3 cameraDir = GetISystem()->GetViewCamera().GetViewdir();

	Vec3 pos = cameraPos + cameraDir * (range / 2);
	Vec3 size(range / 2, range / 2, range / 2);

	SEntityProximityQuery query;
	query.box.min = pos - size;
	query.box.max = pos + size;
	query.nEntityFlags = (u32)ENTITY_FLAG_AI_HIDEABLE; // Filter by entity flag.

	CDebugDrawContext dc;
	gEnv->pEntitySystem->QueryProximity(query);
	for (i32 i = 0; i < query.nCount; ++i)
	{
		IEntity* pEntity = query.pEntities[i];
		if (!pEntity) continue;

		AABB bbox;
		pEntity->GetLocalBounds(bbox);
		dc->DrawAABB(bbox, pEntity->GetWorldTM(), true, ColorB(255, 0, 0, 128), eBBD_Faceted);
	}

	m_dynHideObjectUpr.DebugDraw();
}

//====================================================================
// CheckDistance
//====================================================================
static inline bool CheckDistance(const Vec3 pos1, const std::vector<Vec3>* focusPositions, float radius)
{
	if (!focusPositions)
		return true;
	const std::vector<Vec3>& positions = *focusPositions;

	for (u32 i = 0; i < positions.size(); ++i)
	{
		Vec3 delta = pos1 - positions[i];
		delta.z = 0.0f;
		if (delta.GetLengthSquared() < radius * radius)
			return true;
	}
	return false;
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawPath()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	tukk pName = gAIEnv.CVars.DrawPath;
	if (!pName)
		return;
	CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pName);
	if (pTargetObject)
	{
		CPipeUser* pTargetPipeUser = pTargetObject->CastToCPipeUser();
		if (!pTargetPipeUser)
			return;
		DebugDrawPathSingle(pTargetPipeUser);
		return;
	}
	if (strcmp(pName, "all"))
		return;
	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;

		CPipeUser* pPipeUser = pAIActor->CastToCPipeUser();
		if (!pPipeUser)
			continue;

		DebugDrawPathSingle(pPipeUser);
	}
}

//===================================================================
// DebugDrawPathAdjustments
//===================================================================
void CAISystem::DebugDrawPathAdjustments() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	tukk pName = gAIEnv.CVars.DrawPathAdjustment;
	if (!pName)
		return;
	CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pName);
	if (pTargetObject)
	{
		if (CPuppet* pTargetPuppet = pTargetObject->CastToCPuppet())
			pTargetPuppet->GetPathAdjustmentObstacles(false).DebugDraw();
		if (CAIVehicle* pTargetVehicle = pTargetObject->CastToCAIVehicle())
			pTargetVehicle->GetPathAdjustmentObstacles(false).DebugDraw();
		return;
	}
	if (strcmp(pName, "all"))
		return;
	// find if there are any puppets
	i32 cnt = 0;
	AIObjectOwners::const_iterator ai;
	if ((ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR)) != gAIEnv.pAIObjectUpr->m_Objects.end())
	{
		for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
		{
			if (ai->first != AIOBJECT_ACTOR)
				break;
			cnt++;
			CPuppet* pPuppet = (CPuppet*)ai->second.GetAIObject();
			pPuppet->GetPathAdjustmentObstacles(false).DebugDraw();
		}
	}
	if ((ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_VEHICLE)) != gAIEnv.pAIObjectUpr->m_Objects.end())
	{
		for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
		{
			if (ai->first != AIOBJECT_VEHICLE)
				break;
			cnt++;
			CAIVehicle* pVehicle = (CAIVehicle*)ai->second.GetAIObject();
			pVehicle->GetPathAdjustmentObstacles(false).DebugDraw();
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawPathSingle(const CPipeUser* pPipeUser) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (!pPipeUser->IsEnabled())
		return;

	// debug path draws all the path
	// m_Path gets nodes popped off the front...
	pPipeUser->m_Path.Draw();
	if (pPipeUser->m_pPathFollower)
		pPipeUser->m_pPathFollower->Draw();

	// draw the first part of the path in a different colour
	if (!pPipeUser->m_OrigPath.GetPath().empty())
	{
		TPathPoints::const_iterator li, linext;
		li = pPipeUser->m_OrigPath.GetPath().begin();
		linext = li;
		++linext;
		Vec3 endPt = pPipeUser->m_Path.GetNextPathPos();
		CDebugDrawContext dc;
		while (linext != pPipeUser->m_OrigPath.GetPath().end())
		{
			Vec3 p0 = li->vPos;
			Vec3 p1 = linext->vPos;
			p0.z = dc->GetDebugDrawZ(p0, li->navType == IAISystem::NAV_TRIANGULAR);
			p1.z = dc->GetDebugDrawZ(p1, li->navType == IAISystem::NAV_TRIANGULAR);
			dc->DrawLine(p0, ColorB(255, 0, 255), p1, ColorB(255, 0, 255));
			endPt.z = li->vPos.z;
			if (endPt.IsEquivalent(li->vPos, 0.1f))
				break;
			li = linext++;
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawAgents() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (gAIEnv.CVars.AgentStatsDist <= 1.0f)
		return;

	bool filterName = strcmp("", gAIEnv.CVars.FilterAgentName) != 0;

	CDebugDrawContext dc;
	const float drawDistSq = sqr(gAIEnv.CVars.AgentStatsDist);

	DrxFixedArray<GroupID, 128> enabledGroups;

	stack_string groups = gAIEnv.CVars.DrawAgentStatsGroupFilter;

	if (!groups.empty())
	{
		i32 start = 0;
		stack_string groupIDFilter = groups.Tokenize(", ", start);

		while (!groupIDFilter.empty())
		{
			tuk end;
			GroupID groupID = strtol(groupIDFilter.c_str(), &end, 0);

			stl::push_back_unique(enabledGroups, groupID);
			groupIDFilter = groups.Tokenize(":", start);
		}
	}

	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;

		float distSq = (dc->GetCameraPos() - pAIActor->GetPos()).GetLengthSquared();
		if (distSq > drawDistSq)
			continue;

		if ((!enabledGroups.size() || (std::find(enabledGroups.begin(), enabledGroups.end(), pAIActor->GetGroupId()) != enabledGroups.end())) &&
		    !filterName || !strcmp(pAIActor->GetName(), gAIEnv.CVars.FilterAgentName))
			DebugDrawAgent(pAIActor);
	}

	// output all vehicles
	const AIObjectOwners& objects = gAIEnv.pAIObjectUpr->m_Objects;
	AIObjectOwners::const_iterator ai = objects.find(AIOBJECT_VEHICLE);
	for (; ai != objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_VEHICLE)
			break;

		float distSq = (dc->GetCameraPos() - (ai->second.GetAIObject())->GetPos()).GetLengthSquared();
		if (distSq > drawDistSq)
			continue;

		CAIObject* aiObject = ai->second.GetAIObject();

		if (aiObject &&
		    ((!enabledGroups.size() || (std::find(enabledGroups.begin(), enabledGroups.end(), aiObject->GetGroupId()) != enabledGroups.end())) &&
		     !filterName || !strcmp(aiObject->GetName(), gAIEnv.CVars.FilterAgentName)))
			DebugDrawAgent(aiObject);
	}

	// TODO(marcio): Sort by Z, and maybe reduce opacity to far away puppets!
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawAgent(CAIObject* pAgentObj) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (!pAgentObj || !pAgentObj->IsEnabled())
		return;

	CAIActor* pAgent = pAgentObj->CastToCAIActor();
	if (!pAgent)
		return;

	#ifdef DRXAISYS_DEBUG
	if (gAIEnv.CVars.DebugDrawDamageControl > 0)
		pAgent->UpdateHealthHistory();
	#endif

	CPuppet* pPuppet = pAgent->CastToCPuppet();

	if (pPuppet)
		if (!stricmp(gAIEnv.CVars.DrawPerceptionHandlerModifiers, pAgent->GetName()))
			pPuppet->DebugDrawPerceptionHandlerModifiers();

	PREFAST_SUPPRESS_WARNING(6237);
	if (gEnv->IsEditor() && (pAgentObj->GetType() == AIOBJECT_PLAYER))
		return;

	SOBJECTSTATE& state = pAgent->m_State;
	IAIActorProxy* pProxy = pAgent->GetProxy();
	IAIObject* pAttTarget = pAgent->GetAttentionTarget();

	CPipeUser* pPipeUser = pAgent->CastToCPipeUser();
	if (pPipeUser)
	{
		if (gAIEnv.CVars.DebugDrawCover)
		{
			gAIEnv.pCoverSystem->DebugDrawCoverUser(pPipeUser->GetEntityID());
		}
	}

	CAIVehicle* pVehicle = pAgent->CastToCAIVehicle();
	if (pVehicle && !pVehicle->IsDriverInside())
		return;

	enum EEnabledStateFlags
	{
		Name                  = 1 << 0,
		GroupID               = 1 << 1,
		Distances             = 1 << 2,
		Cover                 = 1 << 3,
		BehaviorSelectionNode = 1 << 4,
		Behavior              = 1 << 5,
		Target                = 1 << 6,
		GoalPipe              = 1 << 7,
		GoalOp                = 1 << 8,
		Stance                = 1 << 9,
		Firemode              = 1 << 10,
		TerritoryWave         = 1 << 11,
		Pathfinding           = 1 << 12,
		LightLevel            = 1 << 13,
		DirectionArrows       = 1 << 14,
		Position              = 1 << 15,
		PersonalLog           = 1 << 16,
		Alertness             = 1 << 17,
	};

	struct
	{
		char   ch;
		u32 flag;
	} flagMap[] =
	{
		{ 'N', Name                  },
		{ 'k', GroupID               },
		{ 'd', Distances             },
		{ 'c', Cover                 },
		{ 'B', BehaviorSelectionNode },
		{ 'b', Behavior              },
		{ 't', Target                },
		{ 'G', GoalPipe              },
		{ 'g', GoalOp                },
		{ 'S', Stance                },
		{ 'f', Firemode              },
		{ 'w', TerritoryWave         },
		{ 'p', Pathfinding           },
		{ 'l', LightLevel            },
		{ 'D', DirectionArrows       },
		{ 'P', Position              },
		{ 'L', PersonalLog           },
		{ 'a', Alertness             },
	};
	u32k flagCount = DRX_ARRAY_COUNT(flagMap);

	tukk enabledFlags = gAIEnv.CVars.DrawAgentStats;
	u32 enabledStats = 0;
	for (u32 i = 0; i < flagCount; ++i)
	{
		if (strchr(enabledFlags, flagMap[i].ch))
			enabledStats |= flagMap[i].flag;
	}

	// count enabled flags to estimate the offset
	u32 enabledBits = enabledStats;
	u32 enabledCount = 0;

	for (; enabledBits; ++enabledCount)
		enabledBits &= enabledBits - 1;

	const Vec3& pos = pAgent->GetPos();
	const Vec3 agentHead = pos;
	const Vec3 attTargetPos = pAttTarget ? pAttTarget->GetPos() : ZERO;
	const Vec3 vPhysicsPos = pAgent->GetPhysicsPos();

	CDebugDrawContext dc;

	float x, y, z;
	if (!dc->ProjectToScreen(agentHead.x, agentHead.y, agentHead.z, &x, &y, &z))
		return;

	if ((z < 0.0f) || (z > 1.0f))
		return;

	x *= (float)dc->GetWidth() * 0.01f;
	y *= (float)dc->GetHeight() * 0.01f;

	const bool bCameraNear = (dc->GetCameraPos() - pos).GetLengthSquared() <
	                         sqr(gAIEnv.CVars.DebugDrawArrowLabelsVisibilityDistance);

	// Define some colors used in the whole code.
	const ColorB white(255, 255, 255, 255);
	const ColorB grey(112, 112, 112, 255);
	const ColorB green(34, 200, 34, 255);
	const ColorB orange(218, 165, 96, 255);
	const ColorB red(139, 0, 0, 255);
	const ColorB yellow(245, 222, 32, 255);
	const ColorB blue(70, 120, 180, 255);
	// ---

	const float fontSize = 1.25f;
	const float fontHeight = 10.0f * fontSize;

	if (pProxy && pProxy->GetLinkedVehicleEntityId())
		y -= 85.0f;

	y -= 20.0f;
	x -= 50.0f;

	const bool bCoverOK = gAIEnv.pCoverSystem->GetSurfaceCount() > 0;

	{
		float offset = pVehicle ? 100.0f : 130.0f;
		offset *= (enabledCount / (float)flagCount);
		y -= offset;
	}

	if (bCoverOK)
	{
		float offset = pVehicle ? 100.0f : 130.0f;
		offset *= (enabledCount / (float)flagCount);
		y -= offset;
	}
	else
	{
		const float fCurrTimeInSeconds = GetAISystem()->GetFrameStartTime().GetSeconds();
		const bool bFlashRed = ((i32)fCurrTimeInSeconds % 1 == 0);
		dc->Draw2dLabel(x, y, fontSize * 1.5f, bFlashRed ? red : white, false, "REGENERATE COVER SURFACES");
		y += fontHeight * 1.5f;
	}

	if (enabledStats & Name)
	{
		stack_string sNameAndPos = pAgent->GetName();
		if (enabledStats & Position)
		{
			sNameAndPos.Format("%s (%.1f %.1f %.f)", sNameAndPos.c_str(), vPhysicsPos.x, vPhysicsPos.y, vPhysicsPos.z);
		}
		dc->Draw2dLabel(x, y, fontSize * 1.25f, pVehicle ? green : white, false, "%s", sNameAndPos.c_str());
		y += fontHeight * 1.25f;
	}

	if (enabledStats & Distances)
	{
		const IAIObject* pPlayer = GetPlayer();
		const float distToPlayer = (pPlayer) ? (agentHead - pPlayer->GetPos()).len() : 0.0f;
		const float distToTarget = (pAttTarget) ? (agentHead - attTargetPos).len() : 0.0f;
		const float distToPathEnd = (state.fDistanceToPathEnd > 0.0001f) ? state.fDistanceToPathEnd : 0.0f;

		dc->Draw2dLabel(x, y, fontSize, white, false, "Plyr: %.2f Trgt: %.2f Path: %.2f",
		                distToPlayer, distToTarget, distToPathEnd);
		y += fontHeight;
	}

	if (pPipeUser && (enabledStats & GroupID))
		dc->Draw2dLabel(x, y, fontSize, Col_SlateBlue, false, "%d", pPipeUser->GetGroupId());

	if (pPipeUser && !pVehicle && (enabledStats & Cover))
	{
		float cx = x;
		float cy = y;
		if ((enabledStats & GroupID))
		{
			cx += 5 * 7.75f;
			cy += 1.5f;
		}

		dc->Draw2dLabel(cx, cy, fontSize * 0.85f, pPipeUser->IsMovingToCover() ? green : grey, false, "%s", "MC");
		dc->Draw2dLabel(cx + 14.0f, cy, fontSize * 0.85f, pPipeUser->IsMovingInCover() ? green : grey, false, "%s", "M");
		dc->Draw2dLabel(cx + 20.0f, cy, fontSize * 0.85f, pPipeUser->IsInCover() ? green : grey, false, "%s", "IC");
		dc->Draw2dLabel(cx + 34.0f, cy, fontSize * 0.85f, pPipeUser->IsCoverCompromised() ? red : grey, false, "%s", "CC");

		if (pPuppet)
			dc->Draw2dLabel(cx + 48.0f, cy, fontSize * 0.85f, pPuppet->IsAlarmed() ? red : grey, false, "%s", "AL");
	}

	if (pPipeUser && (enabledStats & GroupID))
		y += fontHeight;
	else if (pPipeUser && (enabledStats & Cover))
		y += fontHeight * 0.85f;

	y += fontHeight * 0.5f; // extra spacing

	static string text;
	text.clear();

	// Behavior
	if (enabledStats & Behavior)
	{
		text = pProxy->GetCurrentBehaviorName();
		const bool isTextEmpty = text.empty();
		dc->Draw2dLabel(x, y, fontSize * 0.85f, isTextEmpty ? red : white, false, "%s", isTextEmpty ? "No Behavior" : text.c_str());
		y += fontHeight * 0.85f;
	}

	// Personal log
	#ifdef AI_COMPILE_WITH_PERSONAL_LOG
	if (enabledStats & PersonalLog)
	{
		const PersonalLog::Messages& messages = pAgent->GetPersonalLog().GetMessages();
		if (!messages.empty())
			text = messages.back();
		else
			text = "";
		dc->Draw2dLabel(x, y, fontSize * 0.85f, white, false, "%s", text.c_str());
		y += fontHeight * 0.85f;
	}
	#endif

	// Alertness
	if (enabledStats & Alertness)
	{
		if (pAgent->GetProxy())
		{
			i32k alertness = std::min(2, std::max(0, pAgent->GetProxy()->GetAlertnessState()));
			tukk alertnessNames[3] = { "Green", "Orange", "Red" };
			dc->Draw2dLabel(x, y, fontSize * 0.85f, white, false, "%d (%s)", alertness, alertnessNames[alertness]);
			y += fontHeight * 0.85f;
		}
	}

	const SAIBodyInfo& bodyInfo = pPipeUser ? pPipeUser->GetBodyInfo() : SAIBodyInfo();

	// Target
	if (enabledStats & Target)
	{
		if (pAttTarget)
		{
			text = ">> ";
			text += pAttTarget->GetName();

			switch (pAgent->GetAttentionTargetType())
			{
			case AITARGET_VISUAL:
				text += "  <VIS";
				break;
			case AITARGET_MEMORY:
				text += "  <MEM";
				break;
			case AITARGET_SOUND:
				text += "  <SND";
				break;
			default:
				text += "  <-  ";
				break;
			}

			switch (pAgent->GetAttentionTargetThreat())
			{
			case AITHREAT_AGGRESSIVE:
				text += " AGG>";
				break;
			case AITHREAT_THREATENING:
				text += " THR>";
				break;
			case AITHREAT_SUSPECT:
				text += " SUS>";
				break;
			case AITHREAT_INTERESTING:
				text += " INT>";
				break;
			default:
				text += " -  >";
				break;
			}

			// attentionTarget
			if (gAIEnv.CVars.DrawAttentionTargetsPosition)
				dc->DrawSphere(attTargetPos, 0.1f, Col_Red);
		}
		else
			text = ">> No Target";

		dc->Draw2dLabel(x, y, fontSize, white, false, "%s", text.c_str());
		y += fontHeight;
	}

	// GoalPipe
	if (pPipeUser)
	{
		if (CGoalPipe* pPipe = pPipeUser->GetCurrentGoalPipe())
		{
			if (enabledStats & GoalPipe)
			{
				// cppcheck-suppress constStatement
				i32 lineCount = 1;
				const string& pipeDebugName = pPipe->GetDebugName();
				text = pipeDebugName.empty() ? pPipe->GetNameAsString() : pipeDebugName;

				CGoalPipe* pSubPipe = pPipe;

				while (pSubPipe = pSubPipe->GetSubpipe())
				{
					const string& subPipeDebugName = pSubPipe->GetDebugName();
					const string& subpipeName = subPipeDebugName.empty() ? pSubPipe->GetNameAsString() : subPipeDebugName;

					text += ", ";

					if ((text.length() + subpipeName.length()) < 112)
						text += subpipeName;
					else
					{
						dc->Draw2dLabel(x, y, fontSize, white, false, "%s", text.c_str());
						y += 10.f * fontSize;

						if (lineCount++ < 3)
						{
							text = "  ";
							text += subpipeName;
						}
						else
						{
							text = "  ...";
							break;
						}
					}
				}

				const ColorB color = pPipeUser->IsPaused() ? ColorB(Col_Grey) : white;

				dc->Draw2dLabel(x, y, fontSize, color, false, "%s", text.c_str());
				y += fontHeight;
			}

			// Goal Op
			if (enabledStats & GoalOp)
			{
				const ColorB color = pPipeUser->IsPaused() ? ColorB(Col_Grey) : white;

				if (pPipeUser->m_lastExecutedGoalop != eGO_LAST)
					dc->Draw2dLabel(x, y, fontSize, color, false, "%s", pPipe->GetGoalOpName(pPipeUser->m_lastExecutedGoalop));
				y += fontHeight;
			}
		}
	}

	// Stance
	if (!pVehicle && (enabledStats & Stance))
	{
		// Desired stance in the ai object state
		text = GetStanceName(state.bodystate);

		// Actual stance on the game side
		text += " (";
		text += GetStanceName(bodyInfo.stance);
		text += ")";

		if (state.lean < -0.01f)
			text += "  LeanLeft";
		else if (state.lean > 0.01f)
			text += "  LeanRight";
		if (state.peekOver > 0.01f)
			text += "  PeekOver";

		if (state.allowStrafing)
			text += "  Strafe";

		float urgency = fabs_tpl(state.fMovementUrgency);
		bool signal = state.fMovementUrgency >= 0.0f;

		if (urgency > 0.001f)
		{
			if (urgency < AISPEED_WALK)
				text += signal ? " Slow" : " -Slow";
			else if (urgency < AISPEED_RUN)
				text += signal ? " Walk" : " -Walk";
			else if (urgency < AISPEED_SPRINT)
				text += signal ? " Run" : " -Run";
			else
				text += signal ? " Sprint" : " -Sprint";
		}

		dc->Draw2dLabel(x, y, fontSize, orange, false, "%s", text.c_str());
		y += fontHeight;
	}

	// FireMode
	if (pPipeUser && (enabledStats & Firemode))
	{
		// cppcheck-suppress constStatement
		EAimState aimState = pPipeUser->GetAimState();
		EFireMode fireMode = pPipeUser->GetFireMode();

		const ColorB* firemodeColor = &green;

		if (fireMode != FIREMODE_OFF)
		{
			text = "";
			if (aimState == AI_AIM_WAITING)
			{
				text = "\\";
				firemodeColor = &yellow;
			}
			else if (aimState == AI_AIM_READY)
			{
				text = "--";
			}
			else if (aimState == AI_AIM_FORCED)
			{
				text = "==";
			}
			else if (aimState == AI_AIM_OBSTRUCTED)
			{
				firemodeColor = &red;
				text = "||";
			}

			if ((fireMode == FIREMODE_MELEE) || (fireMode == FIREMODE_MELEE_FORCED))
			{
				text = "Melee";
			}
			else if (fireMode == FIREMODE_SECONDARY)
			{
				if (state.fireSecondary)
					text += " )) ** ";
				else
					text += " )) ";
			}
			else if (fireMode != FIREMODE_AIM)
			{
				if (state.fire)
					text += " >> ** ";
				else
					text += " >> ";
			}
		}
		else
		{
			firemodeColor = &grey;
			text = "off";
		}

		dc->Draw2dLabel(x, y, fontSize, *firemodeColor, false, "%s", text.c_str());

		if (pPipeUser->m_State.aimObstructed)
		{
			dc->Draw2dLabel(x + 36.0f, y, fontSize, red, false, "%s", "||");
		}

		if (fireMode != FIREMODE_OFF)
		{
			bool isAmbient = pPuppet && !pPuppet->IsAllowedToHitTarget();

			text = "";
			const ColorB* color = &white;

			if (isAmbient)
			{
				const float centerY = y + 0.5f * (fontHeight - (fontHeight * 0.85f));
				dc->Draw2dLabel(x + 55.0f, centerY, fontSize * 0.85f, yellow, false, "%s", "A");
				color = &yellow;
			}

			switch (fireMode)
			{
			case FIREMODE_BURST:
				text = "(Burst)";
				break;
			case FIREMODE_CONTINUOUS:
				text = "(Continuous)";
				break;
			case FIREMODE_FORCED:
				text = "(Forced)";
				break;
			case FIREMODE_AIM:
				text = "(Aim)";
				break;
			case FIREMODE_SECONDARY:
				text = "(Secondary)";
				break;
			case FIREMODE_SECONDARY_SMOKE:
				text = "(Secondary Smoke)";
				break;
			case FIREMODE_KILL:
				text = "(Kill)";
				break;
			case FIREMODE_BURST_WHILE_MOVING:
				text = "(Burst While Moving)";
				break;
			case FIREMODE_PANIC_SPREAD:
				text = "(Panic Spread)";
				break;
			case FIREMODE_BURST_DRAWFIRE:
				text = "(Draw Fire)";
				break;
			case FIREMODE_MELEE:
				text = "(Melee)";
				break;
			case FIREMODE_MELEE_FORCED:
				text = "(Melee Forced)";
				break;
			case FIREMODE_BURST_SNIPE:
				text = "(Burst Snipe)";
				break;
			case FIREMODE_AIM_SWEEP:
				text = "(Aim Sweep)";
				break;
			case FIREMODE_BURST_ONCE:
				text = "(Burst Once)";
				break;
			case FIREMODE_VEHICLE:
				text = "(Vehicle)";
				break;
			default:
				text = "(Invalid Fire Mode)";
				break;
			}

			SAIWeaponInfo weaponInfo;
			pAgent->GetProxy()->QueryWeaponInfo(weaponInfo);
			if (weaponInfo.isReloading)
			{
				text += " [RELOADING]";
				color = &blue;
			}
			else if (weaponInfo.outOfAmmo)
			{
				text += " [OUT OF AMMO]";
				color = &red;
			}

			if (!text.empty())
			{
				const float centerY = y + 0.5f * (fontHeight - (fontHeight * 0.85f));
				dc->Draw2dLabel(x + 68.0f, centerY, fontSize * 0.85f, *color, false, "%s", text.c_str());
			}
		}

		y += fontHeight;
	}

	// Territory and Wave
	if (enabledStats & TerritoryWave)
	{
		text.Format("Territory: %s", pAgent->GetTerritoryShapeName());
		dc->Draw2dLabel(x, y, fontSize, green, false, "%s", text.c_str());
		y += fontHeight;

		text.Format("Wave: %s", pAgent->GetWaveName());
		dc->Draw2dLabel(x, y, fontSize, green, false, "%s", text.c_str());
		y += fontHeight;
	}

	// Light level (perception)
	if (enabledStats & LightLevel)
	{
		const ColorB* color = &white;
		const bool bIsAffectedByLight = pAgent->IsAffectedByLight();
		if (bIsAffectedByLight)
		{
			text = "Light Level: ";
			const EAILightLevel lightLevel = pAgent->GetLightLevel();
			switch (lightLevel)
			{
			case AILL_NONE:
				text += "None";
				color = &grey;
				break;
			case AILL_LIGHT:
				text += "Light";
				color = &green;
				break;
			case AILL_MEDIUM:
				text += "Medium";
				color = &yellow;
				break;
			case AILL_DARK:
				text += "Dark";
				color = &red;
				break;
			case AILL_SUPERDARK:
				text += "SuperDark";
				color = &orange;
				break;
			default:
				DRX_ASSERT_MESSAGE(false, "CAISystem::DebugDrawAgent Unhandled light level");
				break;
			}
		}
		else
		{
			text = "Not Affected by Light";
			color = &white;
		}

		dc->Draw2dLabel(x, y, fontSize, *color, false, "%s", text.c_str());
		y += fontHeight;
	}

	if (pPipeUser)
	{
		// Debug Draw goal ops.
		if (gAIEnv.CVars.DrawGoals)
			pPipeUser->DebugDrawGoals();

		if (gAIEnv.CVars.DebugDrawCover)
		{
			if (pPipeUser->m_CurrentHideObject.IsValid())
				pPipeUser->m_CurrentHideObject.DebugDraw();
		}
	}

	// Desired view direction and movement direction are drawn in yellow, actual look and move directions are drawn in blue.
	// The lookat direction is marked with sphere, and the movement direction is parked with cone.

	const ColorB desiredColor(255, 204, 13);
	const ColorB actualColor(26, 51, 204, 128);

	const float rad = pAgent->m_Parameters.m_fPassRadius;

	const Vec3 physPos = vPhysicsPos + Vec3(0, 0, 0.25f);

	// Pathfinding radius circle
	dc->DrawCircleOutline(physPos, rad, actualColor);

	if (enabledStats & DirectionArrows)
	{
		// The desired movement direction.
		if (state.vMoveDir.GetLengthSquared() > 0.0f)
		{
			const Vec3& dir = state.vMoveDir;
			const Vec3 target = physPos + dir * (rad + 0.7f);
			dc->DrawLine(physPos + dir * rad, desiredColor, target, desiredColor);
			dc->DrawCone(target, dir, 0.05f, 0.2f, desiredColor);
			if (bCameraNear)
			{
				dc->Draw3dLabelEx(target, 1.f, desiredColor, false, true, false, false, "state.vMoveDir");
			}
		}
		// The desired lookat direction.
		if (!state.vLookTargetPos.IsZero())
		{
			Vec3 dir = state.vLookTargetPos - pos;
			DRX_ASSERT(dir.IsValid());
			dir.NormalizeSafe();
			const Vec3 target = agentHead + dir * (rad + 0.7f);
			dc->DrawLine(pos, desiredColor, target, desiredColor);
			dc->DrawSphere(target, 0.07f, desiredColor);
			if (bCameraNear)
			{
				dc->Draw3dLabelEx(target, 1.f, desiredColor, false, true, false, false, "state.vLookTargetPos");
			}
		}

		// The actual movement direction
		if (!pAgent->GetMoveDir().IsZero())
		{
			const Vec3& dir = pAgent->GetMoveDir();
			const Vec3 target = physPos + dir * (rad + 1.0f);
			dc->DrawLine(physPos + dir * rad, actualColor, target, actualColor);
			dc->DrawCone(target, dir, 0.05f, 0.2f, actualColor);
			if (bCameraNear)
			{
				dc->Draw3dLabelEx(target, 1.f, actualColor, false, true, false, false, "m_vMoveDir");
			}
		}

		// The actual entity direction
		if (!pAgent->GetEntityDir().IsZero())
		{
			const Vec3& dir = pAgent->GetEntityDir();
			const Vec3 target = physPos + dir * rad;
			dc->DrawCone(target, dir, 0.07f, 0.3f, actualColor);
			if (bCameraNear)
			{
				dc->Draw3dLabelEx(target, 1.f, actualColor, false, true, false, false, "m_vEntityDir");
			}
		}

		// The actual lookat direction
		if (!pAgent->GetViewDir().IsZero())
		{
			const Vec3& dir = pAgent->GetViewDir();
			const Vec3 target = pos + dir * (rad + 1.0f);
			dc->DrawLine(pos, actualColor, target, actualColor);
			dc->DrawSphere(target, 0.07f, actualColor);
			if (bCameraNear)
			{
				dc->Draw3dLabelEx(target, 1.f, actualColor, false, true, false, false, "m_vView");
			}
		}

		// The aim & fire direction
		{
			const ColorB fireColor(255, 26, 0, 179);
			const Vec3& firePos = pAgent->GetFirePos();

			// Aim dir
			if (!state.vAimTargetPos.IsZero())
			{
				Vec3 dir = state.vAimTargetPos - firePos;
				dir.NormalizeSafe();
				const Vec3 target = firePos + dir * (rad + 0.5f);
				dc->DrawLine(firePos, desiredColor, target, desiredColor);
				dc->DrawCylinder(target, dir, 0.035f, 0.07f, desiredColor);
				dc->Draw3dLabelEx(target, 1.f, desiredColor, false, true, false, false, "state.vAimTargetPos");
			}

			// Weapon dir
			{
				const Vec3& dir = bodyInfo.vFireDir;
				const Vec3 target = firePos + dir * (rad + 0.8f);
				dc->DrawLine(firePos, fireColor, target, fireColor);
				dc->DrawCylinder(target, dir, 0.035f, 0.07f, fireColor);
				if (bCameraNear)
				{
					dc->Draw3dLabelEx(target, 1.f, fireColor, false, true, false, false, "bodyInfo.vFireDir");
				}
			}
		}
	}

	const float drawFOV = gAIEnv.CVars.DrawAgentFOV;
	if (drawFOV > 0)
	{
		// Draw the view cone
		const float fovPrimary = pAgent->m_Parameters.m_PerceptionParams.FOVPrimary;
		const float fovSeconday = pAgent->m_Parameters.m_PerceptionParams.FOVSecondary;
		const float sightRange = pAgent->m_Parameters.m_PerceptionParams.sightRange;
		const Vec3& viewDir = pAgent->GetViewDir();
		dc->DrawWireFOVCone(pos, viewDir, sightRange * drawFOV * 0.99f, DEG2RAD(fovPrimary), white);

		dc->DrawWireFOVCone(pos, viewDir, sightRange * drawFOV, DEG2RAD(fovSeconday), grey);

		const Vec3 midPoint = pos + viewDir * sightRange * drawFOV / 2;
		dc->DrawLine(pos, white, midPoint, white);
		dc->Draw3dLabel(midPoint, 1.5f, "FOV %.1f/%.1f", fovPrimary, fovSeconday);
	}

	// my Ref point - yellow
	stack_string rfname = gAIEnv.CVars.DrawRefPoints;
	if (rfname != "")
	{
		// cppcheck-suppress constStatement
		i32 group = atoi(rfname);
		if (rfname == "all" || rfname == pAgent->GetName() || ((rfname == "0" || group > 0) && group == pAgent->GetGroupId()))
		{
			if (pPipeUser && pPipeUser->GetRefPoint())
			{
				Vec3 rppos = pPipeUser->GetRefPoint()->GetPos();
				dc->DrawLine(pPipeUser->GetPos(), red, rppos, yellow);
				dc->DrawSphere(rppos, .5, yellow);
				rppos.z += 1.5f;
				dc->Draw3dLabel(rppos, fontSize, "%s", pPipeUser->GetRefPoint()->GetName());
			}
		}
	}

	if (pPuppet && (gAIEnv.CVars.DebugTargetSilhouette > 0))
	{
		CPuppet::STargetSilhouette& targetSilhouette = pPuppet->m_targetSilhouette;
		if (targetSilhouette.valid && !targetSilhouette.points.empty())
		{
			const Vec3& u = targetSilhouette.baseMtx.GetColumn0();
			const Vec3& v = targetSilhouette.baseMtx.GetColumn2();

			// Draw silhouette
			const size_t n = targetSilhouette.points.size();
			for (size_t i = 0; i < n; ++i)
			{
				size_t j = (i + 1) % n;
				const Vec3 pi = targetSilhouette.center + u * targetSilhouette.points[i].x + v * targetSilhouette.points[i].y;
				const Vec3 pj = targetSilhouette.center + u * targetSilhouette.points[j].x + v * targetSilhouette.points[j].y;
				dc->DrawLine(pi, white, pj, white);
			}

			if (!pPuppet->m_targetLastMissPoint.IsZero())
			{
				dc->Draw3dLabel(pPuppet->m_targetLastMissPoint, 1.0f, "Last Miss");
				dc->DrawSphere(pPuppet->m_targetLastMissPoint, 0.1f, orange);
			}

			// The attentiontarget on the silhouette plane.
			if (!pPuppet->m_targetPosOnSilhouettePlane.IsZero())
			{
				const Vec3 dir = (pPuppet->m_targetPosOnSilhouettePlane - pPuppet->GetFirePos()).GetNormalizedSafe();
				dc->DrawCone(pPuppet->m_targetPosOnSilhouettePlane, dir, 0.15f, 0.2f, ColorB(255, 0, 0, pPuppet->m_targetDistanceToSilhouette < 3.0f ? 255 : 64));
				dc->DrawLine(pPuppet->GetFirePos(), red, pPuppet->m_targetPosOnSilhouettePlane, red);
			}

			{
				const Vec3& u2 = targetSilhouette.baseMtx.GetColumn0();
				const Vec3& v2 = targetSilhouette.baseMtx.GetColumn2();

				Vec3 targetBiasDirectionProj = targetSilhouette.ProjectVectorOnSilhouette(pPuppet->m_targetBiasDirection);
				targetBiasDirectionProj.NormalizeSafe();
				const Vec3 pos2 = targetSilhouette.center + u2 * targetBiasDirectionProj.x + v2 * targetBiasDirectionProj.y;

				dc->DrawLine(targetSilhouette.center, ColorB(255, 0, 0, 0), pos2, red);

				dc->DrawLine(targetSilhouette.center, ColorB(0, 0, 255, 0), targetSilhouette.center + pPuppet->m_targetBiasDirection, blue);

				dc->DrawLine(pos2, ColorB(255, 255, 255, 128), targetSilhouette.center + pPuppet->m_targetBiasDirection, ColorB(255, 255, 255, 128));
			}

			tuk szZone = "";
			switch (pPuppet->m_targetZone)
			{
			case AIZONE_OUT:
				szZone = "Out";
				break;
			case AIZONE_WARN:
				szZone = "Warn";
				break;
			case AIZONE_COMBAT_NEAR:
				szZone = "Combat-Near";
				break;
			case AIZONE_COMBAT_FAR:
				szZone = "Combat-Far";
				break;
			case AIZONE_KILL:
				szZone = "Kill";
				break;
			case AIZONE_IGNORE:
				szZone = "Ignored";
				break;
			}

			dc->Draw3dLabel(pPuppet->GetPos() - Vec3(0, 0, 1.5f), 1.2f, "Focus:%d\nZone:%s", (i32)(pPuppet->m_targetFocus * 100.0f), szZone);

		}
	}

	// Display the readibilities.
	if (gAIEnv.CVars.DrawReadibilities)
		pAgent->GetProxy()->DebugDraw(2);

	if (gAIEnv.CVars.DrawProbableTarget > 0)
	{
		if (pPipeUser)
		{
			if (pPipeUser->GetTimeSinceLastLiveTarget() >= 0.0f)
			{
				const Vec3& pos2 = pPipeUser->GetPos();
				const Vec3& livePos = pPipeUser->GetLastLiveTargetPosition();
				Vec3 livePosConstrained = livePos;

				const ColorB color1(171, 60, 184);
				const ColorB color1Trans(171, 60, 184, 128);
				const ColorB color2(110, 60, 184);
				const ColorB color2Trans(110, 60, 184, 128);

				if (pPipeUser->GetTerritoryShape() && pPipeUser->GetTerritoryShape()->ConstrainPointInsideShape(livePosConstrained, true))
				{
					const Vec3 mid = livePosConstrained * 0.7f + pos2 * 0.3f;
					dc->DrawLine(mid, color1, livePosConstrained, color1Trans);
					dc->DrawLine(livePosConstrained, color2Trans, livePos, color2Trans);
					dc->DrawSphere(livePosConstrained, 0.25f, color1Trans);
					dc->DrawSphere(livePos, 0.5f, color2Trans);
				}
				else
				{
					const Vec3 mid = livePos * 0.7f + pos2 * 0.3f;
					dc->DrawLine(mid, color1, livePos, color1Trans);
					dc->DrawSphere(livePos, 0.5f, color1Trans);
				}
			}
		}
	}

	// Display damage parts.
	if (gAIEnv.CVars.DebugDrawDamageParts > 0)
	{
		if (pAgent->GetDamageParts())
		{
			const DamagePartVector* parts = pAgent->GetDamageParts();
			for (DamagePartVector::const_iterator it = parts->begin(); it != parts->end(); ++it)
			{
				const SAIDamagePart& part = *it;
				dc->Draw3dLabel(part.pos, 1, "^ DMG:%.2f\n  VOL:%.1f", part.damageMult, part.volume);
			}
		}
	}

	// Draw the approximate stance size.
	if (gAIEnv.CVars.DebugDrawStanceSize > 0)
	{
		if (pAgent->GetProxy())
		{
			Vec3 pos2 = vPhysicsPos;
			AABB aabb(bodyInfo.stanceSize);
			aabb.Move(pos2);
			dc->DrawAABB(aabb, true, ColorB(255, 255, 255, 128), eBBD_Faceted);
		}
	}

	// Draw active exact positioning request
	{
		if (pPipeUser)
		{
			// Draw pending actor target request. These should not be hanging around, so draw one of it is requested.
			if (const SAIActorTargetRequest* pReq = pPipeUser->GetActiveActorTargetRequest())
			{
				const Vec3 pos2 = pReq->approachLocation;
				const Vec3 dir = pReq->approachDirection;
				dc->DrawArrow(pos2 - dir * 0.5f, dir, 0.2f, ColorB(0, 255, 0, 196));
				dc->DrawLine(pos2, ColorB(0, 255, 0), pos2 - Vec3(0, 0, 0.5f), green);
				dc->DrawLine(pReq->approachLocation, ColorB(255, 255, 255, 128), pReq->animLocation, ColorB(255, 255, 255, 128));
				if (!pReq->animation.empty())
					dc->Draw3dLabel(pos2 + Vec3(0, 0, 0.5f), 1.5f, "%s", pReq->animation.c_str());
				else if (!pReq->vehicleName.empty())
					dc->Draw3dLabel(pos2 + Vec3(0, 0, 0.5f), 1.5f, "%s Seat:%d", pReq->vehicleName.c_str(), pReq->vehicleSeat);
			}

			// Actor target phase.
			const SAIActorTargetRequest& actorTargetReq = state.actorTargetReq;
			if (actorTargetReq.id != 0)
			{
				const Vec3 pos2 = actorTargetReq.approachLocation;

				dc->DrawRangeCircle(pos2 + Vec3(0, 0, 0.3f), actorTargetReq.startArcAngle, actorTargetReq.startArcAngle, ColorB(255, 255, 255, 128), white, true);

				tukk szPhase = "";
				switch (state.curActorTargetPhase)
				{
				case eATP_None:
					szPhase = "None";
					break;
				case eATP_Waiting:
					szPhase = "Waiting";
					break;
				case eATP_Starting:
					szPhase = "Starting";
					break;
				case eATP_Started:
					szPhase = "Started";
					break;
				case eATP_Playing:
					szPhase = "Playing";
					break;
				case eATP_StartedAndFinished:
					szPhase = "StartedAndFinished";
					break;
				case eATP_Finished:
					szPhase = "Finished";
					break;
				case eATP_Error:
					szPhase = "Error";
					break;
				}

				tukk szType = "<INVALID!>";

				if (pPipeUser->GetActiveActorTargetRequest())
					szType = "ACTOR_TGT";

				dc->Draw3dLabel(pos2 + Vec3(0, 0, 1.0f), 1, "%s\nPhase:%s ID:%d", szType, szPhase, actorTargetReq.id);
			}
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawPendingEvents(CPuppet* pTargetPuppet, i32 xPos, i32 yPos) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	typedef std::map<float, string> t_PendingMap;
	t_PendingMap eventsMap;

	PotentialTargetMap targetMap;
	pTargetPuppet->GetPotentialTargets(targetMap);

	PotentialTargetMap::const_iterator ei = targetMap.begin();
	for (; ei != targetMap.end(); ++ei)
	{
		const SAIPotentialTarget& ed = ei->second;
		CAIObject* pNextTarget = 0;
		if ((ed.type == AITARGET_VISUAL && ed.threat == AITHREAT_AGGRESSIVE) || ed.refDummyRepresentation.IsNil())
			pNextTarget = ei->first.GetAIObject();
		else
			pNextTarget = ed.refDummyRepresentation.GetAIObject();

		string curName("NULL");
		if (pNextTarget)
			curName = pNextTarget->GetName();

		float timeout = max(0.0f, ed.GetTimeout(pTargetPuppet->GetParameters().m_PerceptionParams));

		tukk szTargetType = "";
		tukk szTargetThreat = "";

		switch (ed.type)
		{
		case AITARGET_VISUAL:
			szTargetType = "VIS";
			break;
		case AITARGET_MEMORY:
			szTargetType = "MEM";
			break;
		case AITARGET_SOUND:
			szTargetType = "SND";
			break;
		default:
			szTargetType = "-  ";
			break;
		}

		switch (ed.threat)
		{
		case AITHREAT_AGGRESSIVE:
			szTargetThreat = "AGG";
			break;
		case AITHREAT_THREATENING:
			szTargetThreat = "THR";
			break;
		case AITHREAT_INTERESTING:
			szTargetThreat = "INT";
			break;
		default:
			szTargetThreat = "-  ";
			break;
		}

		char buffString[256];

		drx_sprintf(buffString, "%.1fs  <%s %s>  %s", timeout, szTargetType, szTargetThreat, curName.c_str());

		//		eventsMap[ed.fPriority] = buffString;
		eventsMap.insert(std::make_pair(ed.priority, buffString));
	}
	i32 column(40);
	i32 row(5);
	char buff[256];
	CDebugDrawContext dc;
	for (t_PendingMap::reverse_iterator itr = eventsMap.rbegin(); itr != eventsMap.rend(); ++itr)
	{
		drx_sprintf(buff, "%.3f %s", itr->first, itr->second.c_str());

		ColorB color;
		if (itr == eventsMap.rbegin())
		{
			// Highlight the current target
			color[0] = 255;
			color[1] = 255;
			color[2] = 255;
			color[3] = 255;
		}
		else
		{
			color[0] = 0;
			color[1] = 192;
			color[2] = 255;
			color[3] = 255;
		}

		dc->Draw2dLabel(column, row, buff, color);
		++row;
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawStatsTarget(tukk pName)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (!pName || !strcmp(pName, ""))
		return;

	CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pName);
	if (!pTargetObject)
		return;

	CDebugDrawContext dc;

	if (!pTargetObject->IsEnabled())
	{
		dc->TextToScreen(0, 60, "%s  #####>>>-----    IS DISABLED -----<<<#####", pTargetObject->GetName());
		return;
	}

	CAIActor* pTargetAIActor = pTargetObject->CastToCAIActor();
	if (!pTargetAIActor)
		return;

	IAIActorProxy* pTargetProxy = pTargetAIActor->GetProxy();
	SOBJECTSTATE& targetState = pTargetAIActor->m_State;

	CPipeUser* pTargetPipeUser = pTargetObject->CastToCPipeUser();
	CPuppet* pTargetPuppet = pTargetObject->CastToCPuppet();

	DebugDrawAgent(pTargetObject);

	if (targetState.fire)
		dc->TextToScreen(0, 60, ">>>FIRING<<<");
	else if (pTargetPipeUser && pTargetPipeUser->AllowedToFire())
		dc->TextToScreen(0, 60, "---FIRING---");
	else
		dc->TextToScreen(0, 60, "no fire");

	// Output stance
	stack_string stanceStr;
	const SAIBodyInfo& bodyInfo = pTargetAIActor->GetBodyInfo();
	stanceStr = GetStanceName(bodyInfo.stance);

	dc->TextToScreen(0, 48, "%s", pTargetAIActor->GetName());
	if (pTargetPuppet && !pTargetPuppet->m_bCanReceiveSignals)
		dc->TextToScreen(0, 50, ">> Cannot Receive Signals! <<");
	dc->TextToScreen(0, 52, "Stance: %s", stanceStr.c_str());

	// This logic is borrowed from AIproxy in order to display the same urgency as sent further down the pipeline from AIproxy.
	float urgency = targetState.fMovementUrgency;
	const Vec3& vMoveDir = targetState.vMoveDir;
	bool wantsToMove = !vMoveDir.IsZero();
	if (pTargetProxy && pTargetProxy->IsAnimationBlockingMovement())
		wantsToMove = false;
	if (!wantsToMove)
		urgency = (targetState.curActorTargetPhase == eATP_Waiting) ? fabs(targetState.fMovementUrgency) : 0.0f;

	dc->TextToScreen(0, 56, "DesiredSpd (urgency): %5.3f (%5.3f) Dir: (%5.3f, %5.3f, %5.3f)",
	                 targetState.fDesiredSpeed, urgency, vMoveDir.x, vMoveDir.y, vMoveDir.z);

	dc->TextToScreen(0, 58, "Turn speed: %-5.3f rad/s  Slope:%-5.3f", pTargetAIActor->m_bodyTurningSpeed, bodyInfo.slopeAngle);

	if (pTargetPipeUser)
	{
		if (CGoalPipe* pPipe = pTargetPipeUser->GetCurrentGoalPipe())
		{
			dc->TextToScreen(0, 62, "Goalpipe: %s", pTargetPipeUser->GetCurrentGoalPipe()->GetName());

			tukk szGoalopName = "--";
			if (pTargetPipeUser->m_lastExecutedGoalop != eGO_LAST)
				szGoalopName = pPipe->GetGoalOpName(pTargetPipeUser->m_lastExecutedGoalop);
			dc->TextToScreen(0, 64, "Current goal: %s", szGoalopName);

			i32 i = 0;
			CGoalPipe* pSubPipe = pPipe;
			stack_string subPipeName;
			while (pSubPipe->IsInSubpipe())
			{
				pSubPipe = pSubPipe->GetSubpipe();
				subPipeName.Format("+%s", pSubPipe->GetName());
				dc->TextToScreen(0.f, 66.f + 2.f * i, "%s", subPipeName.c_str());
				i++;
			}
		}

		if (pTargetPuppet)
		{
			DebugDrawPendingEvents(pTargetPuppet, 50, 20);
		}
	}

	if (!targetState.vLookTargetPos.IsZero())
		dc->TextToScreen(0, 70, "LOOK");
	if (!targetState.vAimTargetPos.IsZero())
		dc->TextToScreen(10, 70, "AIMLOOK");

	PotentialTargetMap targetMap;
	const IAIObject* pAttentionTarget = pTargetPuppet ? pTargetPuppet->GetAttentionTarget() : NULL;
	tukk attTargetName = pAttentionTarget ? pAttentionTarget->GetName() : "<no target>";
	if (pTargetPuppet)
	{
		//PotentialTargetMap targetMap;
		pTargetPuppet->GetPotentialTargets(targetMap);

		float maxExposure = 0.0f;
		float maxThreat = 0.0f;
		PotentialTargetMap::const_iterator ei = targetMap.begin();
		for (; ei != targetMap.end(); ++ei)
		{
			const SAIPotentialTarget& ed = ei->second;
			maxExposure = max(maxExposure, ed.exposure);
			maxThreat = max(maxThreat, ed.threatTime);
		}

		dc->TextToScreen(0, 74, "Attention target: %s", attTargetName);
		dc->TextToScreen(0, 76, "(maxThreat:%.3f  maxExposure:%.3f) alert: %d  %s",
		                 maxThreat,
		                 maxExposure,
		                 pTargetPuppet->GetProxy()->GetAlertnessState(),
		                 pTargetPuppet->IsAlarmed() ? "Alarmed" : "");
	}
	else if (pTargetPipeUser)
	{
		dc->TextToScreen(0, 74, "Attention target: %s", attTargetName);
		dc->TextToScreen(0, 76, "alert: %d",
		                 pTargetPipeUser->GetProxy()->GetAlertnessState());
	}

	if (!targetState.vSignals.empty())
	{
		i32 i = 0;

		dc->TextToScreen(0, 78, "Pending signals:");
		DynArray<AISIGNAL>::iterator sig, iend = targetState.vSignals.end();
		for (sig = targetState.vSignals.begin(); sig != iend; ++sig, i++)
		{
			dc->TextToScreen(0.f, 80.f + 2.f * i, "%s", (*sig).strText);
		}
	}

	dc->TextToScreen(50, 62, "GR.MEMBERS:%" PRISIZE_T " GROUPID:%d", m_mapGroups.count(static_cast<short>(pTargetAIActor->GetGroupId())), pTargetAIActor->GetGroupId());

	if (pTargetProxy)
		pTargetProxy->DebugDraw(1);

	// Defining some useful colors
	const ColorB desiredColor(255, 204, 13, 77);
	const ColorB actualColor(26, 51, 204, 128);
	const ColorB fireColor(255, 26, 0);
	const ColorB yellow(255, 255, 26);
	const ColorB white(255, 255, 255);
	const ColorB blueMarine(0, 255, 179);
	// ---

	const Vec3& pos = pTargetObject->GetPos();

	if (pTargetPuppet)
	{
		// Draw fire command handler stuff.
		if (pTargetPuppet->m_pFireCmdHandler)
			pTargetPuppet->m_pFireCmdHandler->DebugDraw();
		if (pTargetPuppet->m_pFireCmdGrenade)
			pTargetPuppet->m_pFireCmdGrenade->DebugDraw();

		// draw hide point
		if (pTargetPipeUser->m_CurrentHideObject.IsValid())
		{
			dc->DrawCone(pTargetPipeUser->m_CurrentHideObject.GetObjectPos() + Vec3(0, 0, 5), Vec3(0, 0, -1), .2f, 5.f,
			             pTargetPipeUser->AllowedToFire() ? fireColor : yellow);
		}
	}

	// draw predicted info
	for (i32 iPred = 0; iPred < targetState.predictedCharacterStates.nStates; ++iPred)
	{
		const SAIPredictedCharacterState& predState = targetState.predictedCharacterStates.states[iPred];
		const ColorB& posColor = white;
		const ColorB& velColor = blueMarine;
		const Vec3& predPos = predState.position;
		dc->DrawSphere(predPos, 0.1f, posColor);

		const Vec3 velOffset(0.0f, 0.0f, 0.1f);
		dc->DrawSphere(predPos + velOffset, 0.1f, velColor);
		dc->DrawLine(predPos + velOffset, posColor, predPos + velOffset + predState.velocity * 0.2f, posColor);
		static string speedTxt;
		speedTxt.Format("%5.2f", predState.velocity.GetLength());
		dc->Draw3dLabel(predPos + 4 * velOffset, 1.0f, "%s", speedTxt.c_str());
	}

	// Track and draw the trajectory of the agent.
	Vec3 physicsPos = pTargetObject->GetPhysicsPos();
	if (gAIEnv.CVars.DrawTrajectory)
	{
		i32 type = gAIEnv.CVars.DrawTrajectory;
		static i32 lastReason = -1;
		bool updated = false;

		i32 reason = 0;

	#ifdef _DEBUG
		if ((type == 1) && pTargetPipeUser)
			reason = pTargetPipeUser->m_DEBUGmovementReason;
		else
	#endif
		if (type == 2)
			reason = bodyInfo.stance;

		if (!m_lastStatsTargetTrajectoryPoint.IsZero())
		{
			Vec3 delta = m_lastStatsTargetTrajectoryPoint - physicsPos;
			if (delta.len2() > sqr(0.1f))
			{
				Vec3 prevDelta(delta);
				if (!m_lstStatsTargetTrajectory.empty())
					prevDelta = m_lstStatsTargetTrajectory.back().end - m_lstStatsTargetTrajectory.back().start;

				float c = delta.GetNormalizedSafe().Dot(prevDelta.GetNormalizedSafe());

				if (lastReason != reason || c < cosf(DEG2RAD(15.0f)) || Distance::Point_Point(m_lastStatsTargetTrajectoryPoint, physicsPos) > 2.0f)
				{
					ColorB color;

	#ifdef _DEBUG
					if ((type == 1) && pTargetPipeUser)
					{
						// Color the path based on movement reason.
						switch (pTargetPipeUser->m_DEBUGmovementReason)
						{
						case CPipeUser::AIMORE_UNKNOWN:
							color.set(171, 168, 166, 255);
							break;
						case CPipeUser::AIMORE_TRACE:
							color.set(95, 182, 223, 255);
							break;
						case CPipeUser::AIMORE_MOVE:
							color.set(49, 110, 138, 255);
							break;
						case CPipeUser::AIMORE_MANEUVER:
							color.set(85, 191, 48, 255);
							break;
						case CPipeUser::AIMORE_SMARTOBJECT:
							color.set(240, 169, 16, 255);
							break;
						}
					}
					else
	#endif
					if (type == 2)
					{
						// Color the path based on stance.
						switch (bodyInfo.stance)
						{
						case STANCE_NULL:
							color.set(171, 168, 166, 255);
							break;
						case STANCE_PRONE:
							color.set(85, 191, 48, 255);
							break;
						case STANCE_STAND:
							color.set(95, 182, 223, 255);
							break;
						case STANCE_STEALTH:
							color.set(49, 110, 138, 255);
							break;
						case STANCE_LOW_COVER:
							color.set(32, 98, 108, 255);
							break;
						case STANCE_HIGH_COVER:
							color.set(32, 98, 108, 255);
							break;
						case STANCE_ALERTED:
							color.set(192, 110, 138, 255);
							break;
						}
					}
					else if (type == 3)
					{
						float sp = targetState.fDesiredSpeed;
						float ur = targetState.fMovementUrgency;
						if (sp <= 0.0f)
							color.set(0, 0, 255, 255);
						else if (ur <= 1.0f)
							color.set(0, u8(sp * 255), 0, 255);
						else
							color.set(u8((sp - 1.0f) * 255), 0, 0, 255);
					}
					m_lstStatsTargetTrajectory.push_back(SDebugLine(m_lastStatsTargetTrajectoryPoint, physicsPos, color, 0, 1.0f));
					m_lastStatsTargetTrajectoryPoint = physicsPos;

					lastReason = reason;
				}
			}

			if (!updated && !m_lstStatsTargetTrajectory.empty())
				m_lstStatsTargetTrajectory.back().end = physicsPos;
		}
		else
		{
			m_lastStatsTargetTrajectoryPoint = physicsPos;
		}

		for (std::list<SDebugLine>::iterator lineIt = m_lstStatsTargetTrajectory.begin(); lineIt != m_lstStatsTargetTrajectory.end(); ++lineIt)
		{
			SDebugLine& line = (*lineIt);
			dc->DrawLine(line.start, line.color, line.end, line.color);
		}

		if (m_lstStatsTargetTrajectory.size() > 600)
			m_lstStatsTargetTrajectory.pop_front();
	}

	const ColorB targetNoneColor(0, 0, 0, 128);
	const ColorB targetInterestingColor(128, 255, 0, 196);
	const ColorB targetThreateningColor(255, 210, 0, 196);
	const ColorB targetAggressiveColor(255, 64, 0, 196);

	CDebugDrawContext dc2;
	dc2->SetAlphaBlended(true);
	dc2->SetDepthWrite(false);

	const CCamera& cam = GetISystem()->GetViewCamera();
	Vec3 axisx = cam.GetMatrix().TransformVector(Vec3(1, 0, 0));
	Vec3 axisy = cam.GetMatrix().TransformVector(Vec3(0, 0, 1));

	if (pTargetPipeUser)
	{
		// Draw perception events
		for (PotentialTargetMap::iterator ei2 = targetMap.begin(), end = targetMap.end(); ei2 != end; ++ei2)
		{
			SAIPotentialTarget& ed = ei2->second;
			CAIObject* pOwner = ei2->first.GetAIObject();

			ColorB color;
			switch (ed.threat)
			{
			case AITHREAT_INTERESTING:
				color = targetInterestingColor;
				break;
			case AITHREAT_THREATENING:
				color = targetThreateningColor;
				break;
			case AITHREAT_AGGRESSIVE:
				color = targetAggressiveColor;
				break;
			default:
				color = targetNoneColor;
			}

			Vec3 pos2(0, 0, 0);
			tukk szType = "NONE";
			CAIObject* const pDummyRep = ed.refDummyRepresentation.GetAIObject();
			switch (ed.type)
			{
			case AITARGET_VISUAL:
				szType = "VISUAL";
				{
					ColorB vcolor(255, 255, 255);
					if (ed.threat == AITHREAT_AGGRESSIVE)
					{
						pos2 = pOwner->GetPos();
					}
					else
					{
						pos2 = pDummyRep->GetPos();
						vcolor.a = 64;
					}
					dc2->DrawLine(pTargetPipeUser->GetPos(), vcolor, pos2, vcolor);
				}
				break;
			case AITARGET_BEACON:
				szType = "BEACON";
				{
					ColorB vcol(255, 255, 255);
					pos2 = pOwner->GetPos();
					dc2->DrawLine(pTargetPipeUser->GetPos(), vcol, pos2, vcol);
				}
				break;
			case AITARGET_MEMORY:
				szType = "MEMORY";
				pos2 = pDummyRep->GetPos();
				{
					ColorB vcolor(0, 0, 0, 128);
					dc2->DrawLine(pTargetPipeUser->GetPos(), vcolor, pos2, vcolor);
				}
				break;
			case AITARGET_SOUND:
				szType = "SOUND";
				pos2 = pDummyRep->GetPos();
				break;
			default:
				if (ed.visualThreatLevel > ed.soundThreatLevel)
					pos2 = ed.visualPos;
				else
					pos2 = ed.soundPos;
			}
			;

			dc2->DrawSphere(pos2, 0.25f, color);

			tukk szVisType = "";
			switch (ed.visualType)
			{
			case SAIPotentialTarget::VIS_VISIBLE:
				szVisType = "vis";
				break;
			case SAIPotentialTarget::VIS_MEMORY:
				szVisType = "mem";
				break;
			}
			;

			tukk szOwner = pOwner->GetName();

			dc2->Draw3dLabel(pos2, 1.0f, "@%s\n%s\nt=%.1fs/%.1fs\nexp=%.3f\nsound=%.3f\nsight=%.3f\n%s",
			                 szOwner, szType, ed.threatTimeout, ed.threatTime, ed.exposure, ed.soundThreatLevel, ed.visualThreatLevel, szVisType);
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawType() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	short type = static_cast<short>(gAIEnv.CVars.DrawType);

	AIObjectOwners::const_iterator ai;
	if ((ai = gAIEnv.pAIObjectUpr->m_Objects.find(type)) != gAIEnv.pAIObjectUpr->m_Objects.end())
		for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
		{
			if (ai->first != type)
				break;
			DebugDrawTypeSingle(ai->second.GetAIObject());
		}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawTypeSingle(CAIObject* pAIObj) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	Vec3 pos = pAIObj->GetPhysicsPos();
	ColorB color(230, 230, 26);
	CDebugDrawContext dc;
	dc->DrawLine(pos, color, pos + pAIObj->GetMoveDir() * 2.0f, color);
	if (pAIObj->IsEnabled())
		color.set(0, 0, 179, 255);
	else
		color.set(255, 77, 51, 255);
	dc->DrawSphere(pos, .3f, color);
	dc->Draw3dLabel(pos, 1, "%s", pAIObj->GetName());
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawTargetsList() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	float drawDist2 = static_cast<float>(gAIEnv.CVars.DrawTargets);
	drawDist2 *= drawDist2;
	i32 column(1), row(1);
	string eventDescr;
	string atTarget;
	AIObjectOwners::const_iterator ai;
	CDebugDrawContext dc;
	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor) continue;

		eventDescr = "--";
		atTarget = "--";
		float dist2 = (dc->GetCameraPos() - pAIActor->GetPos()).GetLengthSquared();
		if (dist2 > drawDist2)
			continue;

		CPuppet* pTargetPuppet = pAIActor->CastToCPuppet();
		if (pTargetPuppet)
		{
			DebugDrawTargetUnit(pTargetPuppet);
			float fMaxPriority = -1.f;
			const SAIPotentialTarget* maxEvent = 0;
			CAIObject* pNextTarget = 0;

			PotentialTargetMap targetMap;
			pTargetPuppet->GetPotentialTargets(targetMap);

			PotentialTargetMap::iterator ei = targetMap.begin(), eiend = targetMap.end();
			for (; ei != eiend; ++ei)
			{
				const SAIPotentialTarget& ed = ei->second;
				// target selection based on priority
				if (ed.priority > fMaxPriority)
				{
					fMaxPriority = ed.priority;
					maxEvent = &ed;

					if ((ed.type == AITARGET_VISUAL && ed.threat == AITHREAT_AGGRESSIVE) || ed.refDummyRepresentation.IsNil())
						pNextTarget = ei->first.GetAIObject();
					else
						pNextTarget = ed.refDummyRepresentation.GetAIObject();
				}
			}

			if (fMaxPriority >= 0.0f && maxEvent)
			{
				tukk szName = "NULL";
				if (pNextTarget)
					szName = pNextTarget->GetName();

				tukk szTargetType = "";
				tukk szTargetThreat = "";

				switch (maxEvent->type)
				{
				case AITARGET_VISUAL:
					szTargetType = "VIS";
					break;
				case AITARGET_MEMORY:
					szTargetType = "MEM";
					break;
				case AITARGET_SOUND:
					szTargetType = "SND";
					break;
				default:
					szTargetType = "-  ";
					break;
				}

				switch (maxEvent->threat)
				{
				case AITHREAT_AGGRESSIVE:
					szTargetThreat = "AGG";
					break;
				case AITHREAT_THREATENING:
					szTargetThreat = "THR";
					break;
				case AITHREAT_INTERESTING:
					szTargetThreat = "INT";
					break;
				default:
					szTargetThreat = "-  ";
					break;
				}

				float timeout = maxEvent->GetTimeout(pTargetPuppet->GetParameters().m_PerceptionParams);

				char bfr[256];
				drx_sprintf(bfr, "%.3f %.1f  <%s %s>  %s", fMaxPriority, timeout, szTargetType, szTargetThreat, szName);
				eventDescr = bfr;
			}
		}

		if (pAIActor->GetAttentionTarget())
		{
			atTarget = pAIActor->GetAttentionTarget()->GetName();
		}
		ColorB color(0, 255, 255);
		ColorB colorGray(128, 102, 102);

		dc->Draw2dLabel(column, row, pAIActor->GetName(), colorGray);
		dc->Draw2dLabel(column + 12, row, eventDescr.c_str(), color);
		dc->Draw2dLabel(column + 36, row, atTarget.c_str(), color);
		++row;
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawStatsList() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	float drawDist2 = static_cast<float>(gAIEnv.CVars.DrawStats);
	drawDist2 *= drawDist2;
	i32 column(1), row(1);
	// avoid memory allocations
	static string sGoalPipeName;
	static string atTargetName;
	AIObjectOwners::const_iterator ai;
	CDebugDrawContext dc;
	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor) continue;

		float dist2 = (dc->GetCameraPos() - pAIActor->GetPos()).GetLengthSquared();
		if (dist2 > drawDist2)
			continue;

		IAIObject* pAttTarget = pAIActor->GetAttentionTarget();
		atTargetName = pAttTarget ? pAttTarget->GetName() : "--";

		CPipeUser* pPipeUser = pAIActor->CastToCPipeUser();
		CGoalPipe* pPipe = pPipeUser ? pPipeUser->GetCurrentGoalPipe() : 0;
		sGoalPipeName = pPipe ? pPipe->GetNameAsString() : "--";

		ColorB color(0, 255, 255);
		dc->Draw2dLabel(column, row, pAIActor->GetName(), color);
		dc->Draw2dLabel(column + 12, row, atTargetName.c_str(), color);
		dc->Draw2dLabel(column + 36, row, sGoalPipeName.c_str(), color);

		tukk szGoalopName = "--";
		if (pPipeUser && pPipeUser->m_lastExecutedGoalop != eGO_LAST && pPipe)
			szGoalopName = pPipe->GetGoalOpName(pPipeUser->m_lastExecutedGoalop);
		dc->Draw2dLabel(column + 52, row, szGoalopName, color);
		++row;
	}
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawEnabledActors()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	const float xPos = 28.0f;
	const float yPos = 25.0f;
	const float yStep = 2.0f;

	float yOffset = 0.0f;

	CDebugDrawContext dc;
	dc->TextToScreen(xPos, yPos, "---- Enabled AI Actors  ----");
	yOffset += yStep * 2;

	i32 nEnabled = m_enabledAIActorsSet.size();

	AIActorSet::const_iterator it = m_enabledAIActorsSet.begin();
	AIActorSet::const_iterator itEnd = m_enabledAIActorsSet.end();

	for (; it != itEnd; ++it, yOffset += yStep)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;
		tukk szName = pAIActor->GetName();
		tukk szTerritory = pAIActor->GetTerritoryShapeName();
		tukk szWave = pAIActor->GetWaveName();
		if (CPuppet* pPuppet = pAIActor->CastToCPuppet())
		{
			dc->TextToScreen(xPos, yPos + yOffset, "%-20s (%d) - %s - %s", szName, pPuppet->GetAlertness(), szTerritory, szWave);
		}
		else
		{
			dc->TextToScreen(xPos, yPos + yOffset, "%-20s - %s - %s", szName, szTerritory, szWave);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawEnabledPlayers() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	static float xPos = 28.0f;
	static float yPos = 25.0f;
	static float yStep = 2.0;

	float yOffset = 0.0f;

	CDebugDrawContext dc;

	dc->TextToScreen(xPos, yPos, "---- Enabled AI Players ----");
	yOffset += yStep * 2;

	//AIObjects::const_iterator it=gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_PLAYER);
	AIObjectOwners::const_iterator it = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_PLAYER);
	for (; it != gAIEnv.pAIObjectUpr->m_Objects.end() && it->first == AIOBJECT_PLAYER; ++it)
	{
		CAIPlayer* pPlayer = CastToCAIPlayerSafe(it->second.GetAIObject());
		if (pPlayer)
		{
			IAIActorProxy* pProxy = pPlayer->GetProxy();

			tukk sName = pPlayer->GetName();
			const float fHealth = pProxy ? pProxy->GetActorHealth() : 0.0f;
			i32k iArmor = pProxy ? pProxy->GetActorArmor() : 0;
			dc->TextToScreen(xPos, yPos + yOffset, "%-20s - %8.2fH %dA", sName, fHealth, iArmor);
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawUpdate() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	EDrawUpdateMode mode = DRAWUPDATE_NONE;
	switch (gAIEnv.CVars.DebugDrawUpdate)
	{
	case 1:
		mode = DRAWUPDATE_NORMAL;
		break;
	case 2:
		mode = DRAWUPDATE_WARNINGS_ONLY;
		break;
	}
	;
	if (mode == DRAWUPDATE_NONE)
		return;

	i32 totalCount = 0;
	i32 activeCount[AIOBJECT_VEHICLE + 1] = { 0 };

	i32 row = 1;
	AIObjectOwners::const_iterator ai;

	u16 aTypes[] = { AIOBJECT_ACTOR, AIOBJECT_VEHICLE };
	for (i32 i = 0; i < (sizeof aTypes) / (sizeof aTypes[0]); ++i)
	{
		u16 nType = aTypes[i];

		for (ai = gAIEnv.pAIObjectUpr->m_Objects.find(nType); (ai != gAIEnv.pAIObjectUpr->m_Objects.end()) && (ai->first == nType); ++ai)
		{
			CAIObject* pAIObject = ai->second.GetAIObject();
			if (!pAIObject)
				break;

			CAIActor* pAIActor = pAIObject->CastToCAIActor();
			if (pAIActor->IsActive())
			{
				++activeCount[nType];
				if (DebugDrawUpdateUnit(pAIActor, row, mode))
					++row;
			}
			++totalCount;
		}
	}

	char buffString[128];
	drx_sprintf(buffString, "AI UPDATES - Actors: %3d  Vehicles: %3d  Total: %3d/%d  EnabledAIActorsSet: %" PRISIZE_T,
	            activeCount[AIOBJECT_ACTOR],
	            activeCount[AIOBJECT_VEHICLE],
	            (activeCount[AIOBJECT_ACTOR] + activeCount[AIOBJECT_VEHICLE]),
	            totalCount, m_enabledAIActorsSet.size());
	CDebugDrawContext dc;
	dc->Draw2dLabel(1, 0, buffString, ColorB(0, 192, 255));
}

//
//-----------------------------------------------------------------------------------------------------------
bool CAISystem::DebugDrawUpdateUnit(CAIActor* pTargetAIActor, i32 row, EDrawUpdateMode mode) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	IAIActorProxy* pAIProxy = pTargetAIActor->GetProxy();

	bool bShouldUpdate = pAIProxy->IfShouldUpdate();

	if (mode == DRAWUPDATE_WARNINGS_ONLY && bShouldUpdate)
		return false;

	ColorB colorOk(255, 255, 255);
	ColorB colorWarning(255, 192, 0);
	ColorB& color = bShouldUpdate ? colorOk : colorWarning;

	CDebugDrawContext dc;

	dc->Draw2dLabel(1, row, pTargetAIActor->GetName(), color);

	char buffString[32];
	float dist = Distance::Point_Point(pTargetAIActor->GetPos(), dc->GetCameraPos());
	drx_sprintf(buffString, "%6.1fm", dist);
	dc->Draw2dLabel(20, row, buffString, color);

	if (pAIProxy->IsUpdateAlways())
		dc->Draw2dLabel(27, row, "Always", color);

	return true;
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawLocate() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	tukk pString = gAIEnv.CVars.DrawLocate;

	if (pString[0] == 0 || !strcmp(pString, "none"))
		return;

	if (!strcmp(pString, "squad"))
	{
		const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
		for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
		{
			CAIActor* pAIActor = it->GetAIObject();
			if (!pAIActor) continue;

			if (pAIActor->GetGroupId() == 0)
				DebugDrawLocateUnit(pAIActor);
		}
	}
	else if (!strcmp(pString, "enemy"))
	{
		const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
		for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
		{
			CAIActor* pAIActor = it->GetAIObject();
			if (!pAIActor) continue;

			if (pAIActor->GetGroupId() != 0)
				DebugDrawLocateUnit(pAIActor);
		}
	}
	else if (!strcmp(pString, "all"))
	{
		const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
		for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
		{
			CAIActor* pAIActor = it->GetAIObject();
			if (!pAIActor) continue;

			DebugDrawLocateUnit(pAIActor);
		}
	}
	else if (strlen(pString) > 1)
	{
		CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pString);
		if (pTargetObject)
		{
			DebugDrawLocateUnit(pTargetObject);
		}
		else
		{
			i32 groupId = -1;
			if (sscanf(pString, "%d", &groupId) == 1)
			{
				const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
				for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itEnd = enabledAIActorsSet.end(); it != itEnd; ++it)
				{
					CAIActor* pAIActor = it->GetAIObject();
					if (!pAIActor) continue;

					if (pAIActor->GetGroupId() == groupId)
						DebugDrawLocateUnit(pAIActor);
				}
			}
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawTargetUnit(CAIObject* pAIObj) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIActor* pAIActor = pAIObj->CastToCAIActor();
	if (!pAIActor)
		return;
	CAIObject* pTarget = static_cast<CAIObject*>(pAIActor->GetAttentionTarget());
	if (!pTarget)
		return;
	Vec3 posSelf = pAIObj->GetPos();
	Vec3 posTarget = pTarget->GetPos();
	Vec3 verOffset(0, 0, 2);
	//Vec3	middlePoint( (posSelf+posTarget)*.5f + Vec3(0,0,3) );
	Vec3 middlePoint(posSelf + (posTarget - posSelf) * .73f + Vec3(0, 0, 3));

	ColorB colorSelf(250, 250, 25);
	ColorB colorTarget(250, 50, 50);
	ColorB colorMiddle(250, 100, 50);

	CDebugDrawContext dc;
	dc->DrawLine(posSelf, colorSelf, posSelf + verOffset, colorSelf);
	dc->DrawLine(posSelf + verOffset, colorSelf, middlePoint, colorMiddle);
	dc->DrawLine(middlePoint, colorMiddle, posTarget, colorTarget);
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawSelectedHideSpots() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	for (CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin(), itend = enabledAIActorsSet.end(); it != itend; ++it)
	{
		CAIActor* pAIActor = it->GetAIObject();
		if (!pAIActor)
			continue;

		float drawDist2 = sqr(static_cast<float>(gAIEnv.CVars.DrawHideSpots));
		CDebugDrawContext dc;

		if ((dc->GetCameraPos() - pAIActor->GetPhysicsPos()).GetLengthSquared() <= drawDist2)
		{
			DebugDrawMyHideSpot(pAIActor);
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawMyHideSpot(CAIObject* pAIObj) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CPipeUser* pPipeUser = pAIObj->CastToCPipeUser();
	if (!pPipeUser)
		return;

	if (!pPipeUser->m_CurrentHideObject.IsValid())
		return;

	CDebugDrawContext dc;
	dc->DrawCone(pPipeUser->m_CurrentHideObject.GetLastHidePos() + Vec3(0, 0, 5), Vec3(0, 0, -1), .2f, 5.f, ColorB(255, 26, 26));

	Vec3 posSelf = pAIObj->GetPhysicsPos();
	Vec3 posTarget = pPipeUser->m_CurrentHideObject.GetLastHidePos() + Vec3(0, 0, 5); // ObjectPos()+Vec3(0,0,5);
	Vec3 verOffset(0, 0, 2);
	Vec3 middlePoint((posSelf + posTarget) * .5f + Vec3(0, 0, 3));

	ColorB colorSelf(250, 250, 25);
	ColorB colorTarget(250, 50, 50);
	ColorB colorMiddle(250, 100, 50);

	pPipeUser->m_CurrentHideObject.DebugDraw();

	dc->DrawLine(posSelf, colorSelf, posSelf + verOffset, colorSelf);
	dc->DrawLine(posSelf + verOffset, colorSelf, middlePoint, colorMiddle);
	dc->DrawLine(middlePoint, colorMiddle, posTarget + verOffset, colorMiddle);
	dc->DrawLine(posTarget + verOffset, colorMiddle, posTarget, colorTarget);
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawLocateUnit(CAIObject* pAIObj) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIActor* pAIActor = CastToCAIActorSafe(pAIObj);
	if (!pAIObj || !pAIObj->IsEnabled())
		return;

	CPipeUser* pPipeUser = pAIObj->CastToCPipeUser();

	Vec3 posSelf = pAIObj->GetPos();
	Vec3 posMarker = posSelf + Vec3(0, 0, 3);
	ColorB colorSelf(20, 20, 250);
	ColorB colorMarker;

	CDebugDrawContext dc;

	if (pAIActor->GetAttentionTarget())
	{
		colorMarker.set(250, 27, 27, 255);
		dc->Draw3dLabel(posMarker - Vec3(0.f, 0.f, .21f), .8f, "%s\n\r%s", pAIObj->GetName(), pAIActor->GetAttentionTarget()->GetName());
	}
	else
	{
		colorMarker.set(250, 250, 30, 255);
		dc->Draw3dLabel(posMarker - Vec3(0.f, 0.f, .21f), .8f, "%s", pAIObj->GetName());
	}

	dc->Init3DMode();
	dc->SetDepthTest(false);
	dc->DrawSphere(posMarker, .2f, colorMarker);
	dc->DrawLine(posMarker, colorMarker, posSelf, colorSelf);

	if (pPipeUser && pPipeUser->AllowedToFire())
		dc->DrawCone(posSelf + Vec3(0.f, 0.f, .7f), Vec3(0, 0, -1), .15f, .5f, ColorB(255, 20, 20));
	if (pAIActor->m_State.fire)
		dc->DrawCone(posSelf + Vec3(0.f, 0.f, .7f), pAIObj->GetMoveDir(), .25f, .25f, ColorB(255, 20, 250));
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawGroups()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (!gAIEnv.CVars.DebugDrawGroups)
		return;

	bool drawWorld = gAIEnv.CVars.DebugDrawGroups > 2;

	{
		GroupID lastGroupID = -1;
		size_t groupCount = 0;

		AIObjects::const_iterator it = m_mapGroups.begin();
		AIObjects::const_iterator end = m_mapGroups.end();

		ColorB groupColors[] = {
			Col_SlateBlue,
			Col_Turquoise,
			Col_Tan,
			Col_YellowGreen,
			Col_SteelBlue,
			Col_DarkOrchid,
			Col_MediumSpringGreen,
			Col_VioletRed,
			Col_Sienna,
			Col_CornflowerBlue,
			Col_Pink,
			Col_Thistle,
			Col_BlueViolet,
			Col_Plum,
		};
		size_t colorCount = DRX_ARRAY_COUNT(groupColors);

		const float columnWidth = 160.0f;
		const float startY = 10.0f;
		float x = 1024.0f - 10.0f - columnWidth;
		float y = startY;

		float w = columnWidth;

		for (; it != end; ++it)
		{
			GroupID groupID = it->first;
			if (lastGroupID != groupID)
			{
				ColorB color = groupColors[groupCount % colorCount];

				DebugDrawOneGroup(x, y, w, 1.075f, static_cast<short>(groupID), color, color, drawWorld);
				y += 6.5f;

				if (y >= 635.0f)
				{
					x -= w + 2.5f;
					y = startY;
					w = columnWidth;
				}

				lastGroupID = groupID;
				++groupCount;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawOneGroup(float x, float& y, float& w, float fontSize, short groupID, const ColorB& textColor,
                                  const ColorB& worldColor, bool drawWorld)
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	AIObjects::const_iterator end = m_mapGroups.end();
	AIObjects::const_iterator it = m_mapGroups.find(groupID);
	if (it == end)
		return;

	bool drawInactive = gAIEnv.CVars.DebugDrawGroups == 2;

	CDebugDrawContext dc;

	const float LineHeight = 11.25f * fontSize;

	if (!drawWorld)
	{
		dc->Draw2dLabel(x, y, fontSize * 1.15f, textColor, false, "Group %d", groupID);
		y += LineHeight * 1.15f;
	}

	// Define some colors used later on in the code
	const ColorF white(Col_White);

	Vec3 groupCenter(ZERO);
	Vec3 leaderCenter(ZERO);
	bool hasLeader = false;
	size_t memberCount = 0;
	size_t activeMemberCount = 0;

	for (; it != end && it->first == groupID; ++it)
	{
		CAIObject* current(it->second.GetAIObject());
		if (!current)
			continue;

		bool isLeader = GetLeader(groupID) == current;

		if (isLeader)
		{
			hasLeader = true;
			leaderCenter = current->GetPos();
		}

		const float sizeMult = isLeader ? 1.15f : 1.0f;

		ColorB color = textColor;

		if (!current->IsEnabled())
		{
			if (!drawInactive)
				continue;
			else
				color = ColorF(Col_DarkGray, 0.5f);
		}
		else if (IAIActorProxy* proxy = current->GetProxy())
		{
			i32 alertness = proxy->GetAlertnessState();
			if ((alertness >= 0) && (alertness < 3))
			{
				ColorB alertnessColor[3] = {
					Col_Green,
					Col_Orange,
					Col_Red
				};

				color = alertnessColor[alertness];
			}

			++activeMemberCount;
			groupCenter += current->GetPos();
		}

		if (!drawWorld)
		{
			dc->Draw2dLabel(x + 2.5f, y, fontSize * sizeMult, color, false, "%s", current->GetName());
			y += LineHeight * sizeMult;
		}

		float textWidth = (fontSize * sizeMult * 6.5f) * strlen(current->GetName());
		if (w < textWidth)
			w = textWidth;

		++memberCount;
	}

	if (drawWorld)
	{
		if (hasLeader)
			groupCenter = leaderCenter;
		else if (activeMemberCount > 1)
			groupCenter /= static_cast<f32>(activeMemberCount);

		if (memberCount > 0)
		{
			Vec3 groupLocation = groupCenter + Vec3(0.0f, 0.0f, 2.5f);
			dc->DrawSphere(groupLocation, 0.5f, ColorF(worldColor.pack_abgr8888()) * 0.75f);

			float sx, sy, sz;
			if (dc->ProjectToScreen(groupLocation.x, groupLocation.y, groupLocation.z + 1.0f, &sx, &sy, &sz))
			{
				if ((sz >= 0.0f) && (sz <= 1.0f))
				{
					sx *= dc->GetWidth() * 0.01f;
					sy *= dc->GetHeight() * 0.01f;

					dc->Draw2dLabel(sx, sy, fontSize, white, true, "Group %d", groupID);
				}
			}

			if (memberCount > 1)
			{
				it = m_mapGroups.find(groupID);

				for (; it != end && it->first == groupID; ++it)
				{
					CAIObject* current(it->second.GetAIObject());
					if (GetLeader(groupID) == current)
						continue;

					if (!current->IsEnabled())
						continue;

					dc->DrawLine(groupLocation, worldColor, current->GetPos() + Vec3(0.0f, 0.0f, 0.35f), worldColor, 3.5f);
					dc->DrawSphere(current->GetPos() + Vec3(0.0f, 0.0f, 0.35f), 0.125f, worldColor);
				}
			}
		}

		if (CAIObject* beacon = (CAIObject*)GetAISystem()->GetBeacon(groupID))
		{
			Vec3 beaconLocation = beacon->GetPos() + Vec3(0.0f, 0.0f, 0.25f);
			dc->DrawCone(beaconLocation, Vec3(0.0f, 0.0f, -1.0f), 0.25f, 0.55f, worldColor);

			float sx, sy, sz;
			if (dc->ProjectToScreen(beaconLocation.x, beaconLocation.y, beaconLocation.z + 0.5f, &sx, &sy, &sz))
			{
				if ((sz >= 0.0f) && (sz <= 1.0f))
				{
					sx *= dc->GetWidth() * 0.01f;
					sy *= dc->GetHeight() * 0.01f;

					dc->Draw2dLabel(sx, sy, fontSize, white, true, "Group %d Beacon", groupID);

					sy += LineHeight;
					dc->Draw2dLabel(sx, sy, fontSize, white, true, "%s", beacon->GetName());
				}
			}
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawShooting() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	tukk pName(gAIEnv.CVars.DrawShooting);
	if (!pName)
		return;

	CAIObject* pTargetObject = gAIEnv.pAIObjectUpr->GetAIObjectByName(pName);
	if (!pTargetObject)
		return;

	CPuppet* pTargetPuppet = pTargetObject->CastToCPuppet();
	if (!pTargetPuppet)
		return;

	i32 column(1), row(1);
	char buffString[128];

	ColorB colorOn(26, 255, 26);
	ColorB colorOff(255, 26, 26);

	SAIWeaponInfo weaponInfo;
	pTargetObject->GetProxy()->QueryWeaponInfo(weaponInfo);
	CDebugDrawContext dc;

	if (weaponInfo.outOfAmmo)
	{
		dc->Draw2dLabel(column, row, ">>> OUT OF AMMO <<<", colorOn);
		++row;
	}

	switch (pTargetPuppet->GetAimState())
	{
	case AI_AIM_NONE:       // No aiming requested
		drx_sprintf(buffString, "aimState >>> none");
		break;
	case AI_AIM_WAITING:      // Aiming requested, but not yet ready.
		drx_sprintf(buffString, "aimState >>> waiting");
		break;
	case AI_AIM_OBSTRUCTED:   // Aiming obstructed.
		drx_sprintf(buffString, "aimState >>> OBSTRUCTED");
		break;
	case AI_AIM_READY:
		drx_sprintf(buffString, "aimState >>> ready");
		break;
	case AI_AIM_FORCED:
		drx_sprintf(buffString, "aimState >>> forced");
		break;
	default:
		drx_sprintf(buffString, "aimState >>> undefined");
		break;
	}

	dc->Draw2dLabel(column, row, buffString, colorOn);

	++row;

	drx_sprintf(buffString, "current accuracy: %.3f", pTargetPuppet->GetAccuracy((CAIObject*)pTargetPuppet->GetAttentionTarget()));
	dc->Draw2dLabel(column, row, buffString, colorOn);

	++row;

	if (pTargetPuppet->AllowedToFire())
		dc->Draw2dLabel(column, row, "fireCommand ON", colorOn);
	else
	{
		dc->Draw2dLabel(column, row, "fireCommand OFF", colorOff);
		return;
	}

	++row;

	if (pTargetPuppet->m_State.fire)
		dc->Draw2dLabel(column, row, "trigger DOWN", colorOn);
	else
		dc->Draw2dLabel(column, row, "trigger OFF", colorOff);

	++row;

	if (pTargetPuppet->m_pFireCmdHandler)
	{
		drx_sprintf(buffString, "CAN'T HANDLE CURRENT WEAPON -- %s", pTargetPuppet->m_pFireCmdHandler->GetName());
		dc->Draw2dLabel(column, row, buffString, colorOff);
	}
	else
	{
		dc->Draw2dLabel(column, row, "ZERO WEAPON !!!", colorOff);
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawAreas() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	i32k ALERT_STANDBY_IN_RANGE = 340;
	i32k ALERT_STANDBY_SPOT = 341;
	i32k COMBAT_TERRITORY = 342;

	AIObjectOwners::const_iterator end = gAIEnv.pAIObjectUpr->m_Objects.end();

	CDebugDrawContext dc;

	Vec3 camPos = dc->GetCameraPos();
	float statsDist = gAIEnv.CVars.AgentStatsDist;

	// Draw standby areas
	for (ShapeMap::const_iterator it = m_mapGenericShapes.begin(); it != m_mapGenericShapes.end(); ++it)
	{
		const SShape& shape = it->second;

		if (Distance::Point_AABBSq(camPos, shape.aabb) > sqr(100.0f))
			continue;

		i32 a = 255;
		if (!shape.enabled)
			a = 120;

		std::vector<Vec3> tempVector(shape.shape.begin(), shape.shape.end());
		if (!tempVector.empty())
		{
			switch (shape.type)
			{
#pragma warning(push)
#pragma warning(disable: 4244)
			case ALERT_STANDBY_IN_RANGE:
				dc->DrawRangePolygon(&tempVector[0], tempVector.size(), 0.75f, ColorB(255, 128, 0, a / 2), ColorB(255, 128, 0, a), true);
				break;
			case COMBAT_TERRITORY:
				dc->DrawRangePolygon(&tempVector[0], tempVector.size(), 0.75f, ColorB(40, 85, 180, a / 2), ColorB(40, 85, 180, a), true);
				break;
#pragma warning(pop)
			}
		}
	}

	// Draw attack directions.
	for (AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(ALERT_STANDBY_SPOT); ai != end; ++ai)
	{
		if (ai->first != ALERT_STANDBY_SPOT) break;
		CAIObject* obj = ai->second.GetAIObject();

		const float rad = obj->GetRadius();

		dc->DrawRangeCircle(obj->GetPos() + Vec3(0, 0, 0.5f), rad, 0.25f, ColorB(255, 255, 255, 120), ColorB(255, 255, 255), true);

		Vec3 dir(obj->GetMoveDir());
		dir.z = 0;
		dir.NormalizeSafe();
		dc->DrawRangeArc(obj->GetPos() + Vec3(0, 0, 0.5f), dir, DEG2RAD(120.0f), rad - 0.7f, rad / 2, ColorB(255, 0, 0, 64), ColorB(255, 255, 255), false);
	}

	AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR);
	for (; ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_ACTOR)
			break;

		CAIObject* obj = ai->second.GetAIObject();
		if (!obj->IsEnabled())
			continue;

		if (Distance::Point_PointSq(obj->GetPos(), camPos) > sqr(statsDist))
			continue;

		CPipeUser* pPipeUser = obj->CastToCPipeUser();
		if (!pPipeUser)
			continue;

		SShape* territoryShape = pPipeUser->GetTerritoryShape();
		if (territoryShape)
		{
			float dist = 0;
			Vec3 nearestPt;
			ListPositions::const_iterator it = territoryShape->NearestPointOnPath(pPipeUser->GetPos(), false, dist, nearestPt);
			Vec3 p0 = pPipeUser->GetPos() - Vec3(0, 0, 0.5f);
			dc->Draw3dLabel(p0 * 0.7f + nearestPt * 0.3f, 1, "Terr");
			dc->DrawLine(p0, ColorB(30, 208, 224), nearestPt, ColorB(30, 208, 224));
		}

		SShape* refShape = pPipeUser->GetRefShape();
		if (refShape)
		{
			float dist = 0;
			Vec3 nearestPt;
			ListPositions::const_iterator it = refShape->NearestPointOnPath(pPipeUser->GetPos(), false, dist, nearestPt);
			Vec3 p0 = pPipeUser->GetPos() - Vec3(0, 0, 1.0f);
			dc->Draw3dLabel(p0 * 0.7f + nearestPt * 0.3f, 1, "Ref");
			dc->DrawLine(p0, ColorB(135, 224, 30), nearestPt, ColorB(135, 224, 30));
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawExpensiveAccessoryQuota() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CDebugDrawContext dc;

	Vec3 cameraPos = dc->GetCameraPos();

	const float analyzeDist = 350;

	for (AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR); ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_ACTOR)
			break;
		CAIObject* obj = ai->second.GetAIObject();
		if (!obj->IsEnabled())
			continue;
		CPuppet* pPuppet = obj->CastToCPuppet();
		if (!pPuppet)
			continue;
		if (Distance::Point_PointSq(obj->GetPos(), cameraPos) > sqr(analyzeDist))
			continue;

		if (pPuppet->IsAllowedToUseExpensiveAccessory())
		{
			const SAIBodyInfo& bodyInfo = pPuppet->GetBodyInfo();
			Vec3 pos = pPuppet->GetPhysicsPos();
			AABB aabb(bodyInfo.stanceSize);
			aabb.Move(pos);
			dc->DrawAABB(aabb, true, ColorB(0, 196, 255, 64), eBBD_Faceted);
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawAmbientFire() const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	CAIPlayer* pPlayer = CastToCAIPlayerSafe(GetPlayer());
	if (!pPlayer)
		return;

	const float analyzeDist = 350;

	CDebugDrawContext dc;

	for (AIObjectOwners::const_iterator ai = gAIEnv.pAIObjectUpr->m_Objects.find(AIOBJECT_ACTOR); ai != gAIEnv.pAIObjectUpr->m_Objects.end(); ++ai)
	{
		if (ai->first != AIOBJECT_ACTOR)
			break;
		CAIObject* obj = ai->second.GetAIObject();
		if (!obj->IsEnabled())
			continue;
		CPuppet* pPuppet = obj->CastToCPuppet();
		if (!pPuppet)
			continue;
		if (Distance::Point_PointSq(obj->GetPos(), pPlayer->GetPos()) > sqr(analyzeDist))
			continue;

		if (pPuppet->IsAllowedToHitTarget())
		{
			//			dc->DrawCone(obj->GetPhysicsPos(), Vec3(0,0,1), 0.3f, 1.0f, ColorB(255,255,255));
			const SAIBodyInfo& bodyInfo = pPuppet->GetBodyInfo();
			Vec3 pos = pPuppet->GetPhysicsPos();
			AABB aabb(bodyInfo.stanceSize);

			aabb.Move(pos);
			dc->DrawAABB(aabb, true, ColorB(255, 0, 0, 64), eBBD_Faceted);
		}
	}

}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::AddDebugBox(const Vec3& pos, const OBB& obb, u8 r, u8 g, u8 b, float time)
{
	if (gAIEnv.CVars.DebugDraw > 0)
		m_vecDebugBoxes.push_back(SDebugBox(pos, obb, ColorB(r, g, b), time));
}

void CAISystem::AddDebugCylinder(const Vec3& pos, const Vec3& dir, float radius, float length, const ColorB& color, float time)
{
	if (gAIEnv.CVars.DebugDraw > 0)
		m_vecDebugCylinders.push_back(SDebugCylinder(pos, dir, radius, length, color, time));
}

void CAISystem::AddDebugCone(const Vec3& pos, const Vec3& dir, float radius, float length, const ColorB& color, float time)
{
	if (gAIEnv.CVars.DebugDraw > 0)
		m_vecDebugCones.push_back(SDebugCone(pos, dir, radius, length, color, time));
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::AddPerceptionDebugLine(tukk tag, const Vec3& start, const Vec3& end, u8 r, u8 g, u8 b, float time, float thickness)
{
	if (gAIEnv.CVars.DebugDraw > 0)
	{
		if (tag)
		{
			std::list<SPerceptionDebugLine>::iterator it = m_lstDebugPerceptionLines.begin();
			for (; it != m_lstDebugPerceptionLines.end(); ++it)
			{
				if (strlen(it->name) > 0 && _stricmp(it->name, tag) == 0)
				{
					*it = SPerceptionDebugLine(tag, start, end, ColorB(r, g, b), time, thickness);
					return;
				}
			}
		}

		m_lstDebugPerceptionLines.push_back(SPerceptionDebugLine(tag ? tag : "", start, end, ColorB(r, g, b), time, thickness));
	}
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawInterestSystem(i32 iLevel) const
{
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	const CCentralInterestUpr* pInterestUpr = CCentralInterestUpr::GetInstance();

	CDebugDrawContext dc;
	ColorB cGreen = ColorB(0, 255, 0);
	ColorB cYellow = ColorB(255, 255, 0);
	ColorB cRed = ColorB(255, 0, 0);
	ColorB cTransparentRed = ColorB(200, 0, 0, 100);

	if (iLevel > 0)
	{
		CCentralInterestUpr::TVecPIMs::const_iterator it = pInterestUpr->GetPIMs()->begin();
		CCentralInterestUpr::TVecPIMs::const_iterator itEnd = pInterestUpr->GetPIMs()->end();
		for (; it != itEnd; ++it)
		{
			const CPersonalInterestUpr* pPersonal = &(*it);
			if (!pPersonal->IsReset() && pPersonal->IsInterested())
			{
				dc->DrawLine(pPersonal->GetAssigned()->GetPos(), cYellow, pPersonal->GetInterestingPos(), cYellow);
			}
		}
	}

	CCentralInterestUpr::TVecInteresting::const_iterator it = pInterestUpr->GetInterestingEntities()->begin();
	CCentralInterestUpr::TVecInteresting::const_iterator itEnd = pInterestUpr->GetInterestingEntities()->end();
	for (; it != itEnd; ++it)
	{
		const SEntityInterest* pInteresting = &(*it);
		if (pInteresting->IsValid())
		{
			if (IEntity* pEntity = (const_cast<SEntityInterest*>(pInteresting))->GetEntity())
			{
				AABB box;
				pEntity->GetLocalBounds(box);

				dc->DrawWireSphere(pEntity->GetWorldTM() * pInteresting->m_vOffset, box.GetRadius() * 0.5f, cRed);

				if (iLevel > 1)
				{
					dc->DrawCircleOutline(pEntity->GetPos(), pInteresting->m_fRadius,
					                      (pInteresting->m_sActionName.empty()) ? cGreen : cYellow);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DEBUG_AddFakeDamageIndicator(CAIActor* pShooter, float t)
{
	m_DEBUG_fakeDamageInd.push_back(SDebugFakeDamageInd(pShooter->GetPos(), t));

	IPhysicalEntity* phys = 0;

	// If the AI is tied to a vehicle, use the vehicles physics to draw the silhouette.
	const SAIBodyInfo& bi = pShooter->GetBodyInfo();
	if (bi.GetLinkedVehicleEntity() && bi.GetLinkedVehicleEntity()->GetAI() && bi.GetLinkedVehicleEntity()->GetAI()->GetProxy())
		phys = bi.GetLinkedVehicleEntity()->GetAI()->GetProxy()->GetPhysics(true);

	if (!phys)
		phys = pShooter->GetProxy()->GetPhysics(true);

	if (!phys)
		return;

	SDebugFakeDamageInd& ind = m_DEBUG_fakeDamageInd.back();

	pe_status_nparts statusNParts;
	i32 nParts = phys->GetStatus(&statusNParts);

	pe_status_pos statusPos;
	phys->GetStatus(&statusPos);

	pe_params_part paramsPart;
	for (statusPos.ipart = 0, paramsPart.ipart = 0; statusPos.ipart < nParts; ++statusPos.ipart, ++paramsPart.ipart)
	{
		phys->GetParams(&paramsPart);
		phys->GetStatus(&statusPos);

		primitives::box box;
		statusPos.pGeomProxy->GetBBox(&box);

		box.center *= statusPos.scale;
		box.size *= statusPos.scale;

		box.size.x += 0.05f;
		box.size.y += 0.05f;
		box.size.z += 0.05f;

		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn0() * box.size.x));
		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn0() * -box.size.x));
		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn1() * box.size.y));
		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn1() * -box.size.y));
		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn2() * box.size.z));
		ind.verts.push_back(statusPos.pos + statusPos.q * (box.center + box.Basis.GetColumn2() * -box.size.z));
	}
}

void CAISystem::DebugDrawSelectedTargets()
{
	CDebugDrawContext dc;

	const CAISystem::AIActorSet& enabledAIActorsSet = GetAISystem()->GetEnabledAIActorSet();
	CAISystem::AIActorSet::const_iterator it = enabledAIActorsSet.begin();
	CAISystem::AIActorSet::const_iterator itEnd = enabledAIActorsSet.end();
	for (; it != itEnd; ++it)
	{
		if (CAIActor* aiActor = it->GetAIObject())
		{
			if (IAIObject* attentionTarget = aiActor->GetAttentionTarget())
			{
				dc->DrawLine(aiActor->GetPos(), ColorB(255, 255, 0, 200), aiActor->GetPos() + Vec3(0.0f, 0.0f, 1.0f), ColorB(255, 200, 0, 200), 5.0f);
				dc->DrawLine(aiActor->GetPos() + Vec3(0.0f, 0.0f, 1.0f), ColorB(255, 255, 0, 200), attentionTarget->GetPos(), ColorB(255, 0, 0, 200), 5.0f);
				dc->DrawSphere(attentionTarget->GetPos(), 0.25f, ColorB(255, 0, 0, 200));
			}
		}
	}
}

#endif //DRXAISYS_DEBUG

//-----------------------------------------------------------------------------------------------------------
// NOTE: The following are virtual! They cannot be stripped out just yet.
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDraw()
{
#ifdef DRXAISYS_DEBUG
	DRX_PROFILE_FUNCTION(PROFILE_AI);

	if (!m_bInitialized) return;

	gAIEnv.pStatsUpr->Render();
	gAIEnv.pStatsUpr->Reset(eStatReset_Frame);

	i32 debugDrawValue = gAIEnv.CVars.DebugDraw;

	// As a special case, we want to draw the InterestSystem alone sometimes
	if (gAIEnv.CVars.DebugInterest > 0)
		DebugDrawInterestSystem(gAIEnv.CVars.DebugInterest);

	if (gAIEnv.CVars.DrawModularBehaviorTreeStatistics > 0)
		gAIEnv.pBehaviorTreeUpr->DrawMemoryInformation();

	if (debugDrawValue < 0)
		return;   // if -ve ONLY display warnings

	// Set render state to standard - return it to this if you change it
	CDebugDrawContext dc;
	dc->Init3DMode();
	dc->SetAlphaBlended(true);

	for (auto& systemComponent : m_setSystemComponents)
	{
		systemComponent->DebugDraw(dc.operator->());
	}

	if (gAIEnv.CVars.DebugDrawCollisionAvoidance > 0)
		gAIEnv.pCollisionAvoidanceSystem->DebugDraw();

	if (gAIEnv.CVars.DebugDrawCommunication > 0)
		gAIEnv.pCommunicationUpr->DebugDraw();

	if (gAIEnv.pVisionMap && gAIEnv.CVars.DebugDrawVisionMap != 0)
		gAIEnv.pVisionMap->DebugDraw();

	if (gAIEnv.pCoverSystem)
		gAIEnv.pCoverSystem->DebugDraw();

	if (gAIEnv.pNavigationSystem && gAIEnv.CVars.DebugDrawNavigation > 0)
		gAIEnv.pNavigationSystem->DebugDraw();

	if (gAIEnv.CVars.DebugGlobalPerceptionScale > 0)
		m_globalPerceptionScale.DebugDraw();

	#ifdef DRXAISYS_DEBUG
	{
		DRX_PROFILE_REGION(PROFILE_AI, "AIBubblesSystem");
		gAIEnv.pBubblesSystem->Update();
	}
	#endif

	if (!IsEnabled())
	{
		dc->Draw2dLabel(100, 200, 1.5f, ColorB(255, 255, 255), false, "AI System is disabled; ai_DebugDraw is on");
		return;
	}

	if (gAIEnv.CVars.DebugDrawEnabledActors == 1)
		DebugDrawEnabledActors();   // Called only in this line => Maybe we should remove it from the interface?
	else if (gAIEnv.CVars.DebugDrawEnabledPlayers == 1)
		DebugDrawEnabledPlayers();

	DebugDrawUpdate();        // Called only in this line => Maybe we should remove it from the interface?

	if (gAIEnv.CVars.DrawFakeTracers > 0)
		DebugDrawFakeTracers();

	if (gAIEnv.CVars.DrawFakeHitEffects > 0)
		DebugDrawFakeHitEffects();

	if (gAIEnv.CVars.DrawFakeDamageInd > 0)
		DebugDrawFakeDamageInd();

	if (gAIEnv.CVars.DrawPlayerRanges > 0)
		DebugDrawPlayerRanges();

	if (gAIEnv.CVars.DrawPerceptionIndicators > 0 || gAIEnv.CVars.DrawPerceptionDebugging > 0)
		DebugDrawPerceptionIndicators();

	if (gAIEnv.CVars.DrawPerceptionModifiers > 0)
		DebugDrawPerceptionModifiers();

	DebugDrawTargetTracks();
	DebugDrawDebugAgent();

	DebugDrawDebugShapes();

	if (debugDrawValue == 0) return;

	AILogDisplaySavedMsgs();

	DebugDrawRecorderRange();

	DebugDrawNavigation();

	if (gAIEnv.CVars.DebugDrawDamageControl > 1)
		DebugDrawDamageControlGraph();

	if (gAIEnv.CVars.DrawAreas > 0)
		DebugDrawAreas();

	DebugDrawSteepSlopes();
	DebugDrawVegetationCollision();

	if (m_bUpdateSmartObjects && gAIEnv.CVars.DrawSmartObjects)
		m_pSmartObjectUpr->DebugDraw();

	DebugDrawPath();
	DebugDrawPathAdjustments();

	DebugDrawStatsTarget(gAIEnv.CVars.StatsTarget);

	if (gAIEnv.CVars.DrawFormations)
		gAIEnv.pFormationUpr->DebugDraw();

	DebugDrawType();
	DebugDrawLocate();

	if (gAIEnv.CVars.DrawTargets)
		DebugDrawTargetsList();

	if (gAIEnv.CVars.DrawStats)
		DebugDrawStatsList();

	DebugDrawGroups();

	if (gAIEnv.CVars.DrawHideSpots)
		DebugDrawSelectedHideSpots();

	DebugDrawDynamicHideObjects();
	DebugDrawAgents();
	DebugDrawShooting();

	gAIEnv.pTacticalPointSystem->DebugDraw();

	DebugDrawLightUpr();
	//DebugDrawP0AndP1();
	DebugDrawPuppetPaths();
	DebugDrawCheckCapsules();
	DebugDrawCheckRay();
	m_pSmartObjectUpr->DebugDrawValidateSmartObjectArea();
	DebugDrawCheckFloorPos();
	DebugDrawCheckGravity();
	//DebugDrawPaths();

	if (gAIEnv.CVars.DrawGroupTactic > 0)
		DebugDrawGroupTactic();

	DebugDrawRadar();

	if (gAIEnv.CVars.DebugDrawAmbientFire > 0)
		DebugDrawAmbientFire();

	if (gAIEnv.CVars.DebugDrawExpensiveAccessoryQuota > 0)
		DebugDrawExpensiveAccessoryQuota();

	if (gAIEnv.CVars.DebugDrawDamageParts > 0)
		DebugDrawDamageParts();

	if (gAIEnv.CVars.DebugDrawStanceSize > 0)
		DebugDrawStanceSize();

	if (strcmp(gAIEnv.CVars.ForcePosture, "0"))
	{
		DebugDrawForcePosture();
	}
	else
	{
		if (strcmp(gAIEnv.CVars.ForceAGAction, "0"))
			DebugDrawForceAGAction();

		if (strcmp(gAIEnv.CVars.ForceAGSignal, "0"))
			DebugDrawForceAGSignal();

		if (gAIEnv.CVars.ForceStance != -1)
			DebugDrawForceStance();
	}

	if (gAIEnv.CVars.DebugDrawAdaptiveUrgency > 0)
		DebugDrawAdaptiveUrgency();

	if (gAIEnv.CVars.DebugDrawPlayerActions > 0)
		DebugDrawPlayerActions();

	if (gAIEnv.CVars.DebugDrawCrowdControl > 0)
		DebugDrawCrowdControl();

	if (gAIEnv.CVars.DebugDrawBannedNavsos > 0)
		m_pSmartObjectUpr->DebugDrawBannedNavsos();

	if (gAIEnv.CVars.DrawSelectedTargets > 0)
		DebugDrawSelectedTargets();

	// Aux render flags restored by helper
#endif //DRXAISYS_DEBUG
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::DebugDrawFakeTracer(const Vec3& pos, const Vec3& dir)
{
#ifdef DRXAISYS_DEBUG
	CAIObject* pPlayer = GetPlayer();
	if (!pPlayer) return;

	Vec3 dirNorm = dir.GetNormalizedSafe();
	const Vec3& playerPos = pPlayer->GetPos();
	Vec3 dirShooterToPlayer = playerPos - pos;

	float projDist = dirNorm.Dot(dirShooterToPlayer);

	float distShooterToPlayer = dirShooterToPlayer.NormalizeSafe();

	if (projDist < 0.0f)
		return;

	Vec3 nearestPt = pos + dirNorm * distShooterToPlayer;

	float tracerDist = Distance::Point_Point(nearestPt, playerPos);

	const float maxTracerDist = 20.0f;
	if (tracerDist > maxTracerDist)
		return;

	float a = 1 - tracerDist / maxTracerDist;

	ray_hit hit;
	float maxd = distShooterToPlayer * 2.0f;
	if (gEnv->pPhysicalWorld->RayWorldIntersection(
	      pos, dirNorm * maxd, COVER_OBJECT_TYPES,
	      AI_VISION_RAY_CAST_FLAG_BLOCKED_BY_SOLID_COVER,
	      &hit, 1))
	{
		maxd = hit.dist;
		// fake hit fx
		Vec3 p = hit.pt + hit.n * 0.1f;
		ColorF col(drx_random(0.4f, 0.7f), drx_random(0.4f, 0.7f), drx_random(0.4f, 0.7f), 0.9f);
		m_DEBUG_fakeHitEffect.push_back(SDebugFakeHitEffect(p, hit.n, drx_random(0.45f, 0.9f), drx_random(0.5f, 0.8f), col));
	}

	const float maxTracerLen = 50.0f;
	float d0 = distShooterToPlayer - drx_random(0.75f, 1.0f) * maxTracerLen / 2;
	float d1 = d0 + drx_random(0.5f, 1.0f) * maxTracerLen;

	Limit(d0, 0.0f, maxd);
	Limit(d1, 0.0f, maxd);

	m_DEBUG_fakeTracers.push_back(SDebugFakeTracer(pos + dirNorm * d0, pos + dirNorm * d1, a, drx_random(0.15f, 0.3f)));
#endif //DRXAISYS_DEBUG
}

//-----------------------------------------------------------------------------------------------------------
void CAISystem::AddDebugLine(const Vec3& start, const Vec3& end, u8 r, u8 g, u8 b, float time)
{
#ifdef DRXAISYS_DEBUG
	if (gAIEnv.CVars.DebugDraw > 0)
		m_vecDebugLines.push_back(SDebugLine(start, end, ColorB(r, g, b), time, 1.0f));
#endif //DRXAISYS_DEBUG
}

//-----------------------------------------------------------------------------------------------------------
#ifdef DRXAISYS_DEBUG
void CAISystem::AddDebugLine(const Vec3& start, const Vec3& end, const ColorB& color, float time, float thickness)
{
	if (gAIEnv.CVars.DebugDraw > 0)
		m_vecDebugLines.push_back(SDebugLine(start, end, color, time, thickness));
}
#endif //DRXAISYS_DEBUG

//-----------------------------------------------------------------------------------------------------------
void CAISystem::AddDebugSphere(const Vec3& pos, float radius, u8 r, u8 g, u8 b, float time)
{
#ifdef DRXAISYS_DEBUG
	if (gAIEnv.CVars.DebugDraw >= 0)
		m_vecDebugSpheres.push_back(SDebugSphere(pos, radius, ColorB(r, g, b), time));
#endif //DRXAISYS_DEBUG
}