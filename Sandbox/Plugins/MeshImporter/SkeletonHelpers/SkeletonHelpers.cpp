// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SkeletonHelpers.h"

#include <QViewport.h>

#include <DrxAnimation/IDrxAnimation.h>

i32k kInvalidJointId = -1;

static inline float GetJointSize()
{
	return gEnv->pConsole->GetCVar("mi_jointSize")->GetFVal();
}

static void ComputeConnectionOBB(OBB& obb, Vec3& center, const Vec3& from, const Vec3& to)
{
	const Vec3 axis = to - from;
	const float len2 = axis.GetLengthSquared();
	// const float invLen = isqrt_fast_tpl(len2); This is actually not precise enough.
	const float invLen = isqrt_tpl(len2);
	const float len = invLen * len2;

	obb.h = 0.5f * GetJointSize() * Vec3(0.0f, 1.0f, 1.0f) + 0.5f * len * Vec3(1.0f, 0.0f, 0.0f);
	obb.m33 = Matrix33(Quat::CreateRotationV0V1(Vec3(1.0f, 0.0f, 0.0f), invLen * axis));

	center = 0.5f * axis + from;
}

static void DrawJointConnectionOBBs(const IDefaultSkeleton& skel, const ISkeletonPose& pose)
{
	OBB obb;
	obb.c = Vec3(ZERO);
	const ColorB black = ColorB(0, 0, 0);
	IRenderAuxGeom* pAuxGeom = gEnv->pRenderer->GetIRenderAuxGeom();
	for (i32 jid = 0, N = (i32)skel.GetJointCount(); jid < N; ++jid)
	{
		i32k pid = skel.GetJointParentIDByID(jid);
		if (pid == kInvalidJointId)
		{
			continue;
		}

		Vec3 center;
		ComputeConnectionOBB(obb, center, pose.GetAbsJointByID(jid).t, pose.GetAbsJointByID(pid).t);

		pAuxGeom->DrawOBB(obb, Matrix34::CreateTranslationMat(center), false, black, EBoundingBoxDrawStyle::eBBD_Faceted);
	}
}

static bool RaycastJoints(const Ray& ray, const Vec3& eye, const IDefaultSkeleton& skel, const ISkeletonPose& pose, i32& outHitJoint, float& outHitDist)
{
	OBB obb;
	obb.c = Vec3(ZERO);
	obb.h = 0.5f * GetJointSize() * Vec3(1.0f, 1.0f, 1.0f);
	float hitDist;
	i32 hitJoint = kInvalidJointId;
	Vec3 where;
	for (i32 jid = 0, N = (i32)skel.GetJointCount(); jid < N; ++jid)
	{
		const QuatT& jointFrame = pose.GetAbsJointByID(jid);
		obb.m33 = Matrix33(jointFrame.q);
		if (Intersect::Ray_OBB(ray, jointFrame.t, obb, where)) // Consider more precise collision detection.
		{
			const float dist = (where - eye).GetLengthSquared();
			if (hitJoint == kInvalidJointId || dist < hitDist)
			{
				hitJoint = jid;
				hitDist = dist;
			}
		}
	}

	outHitJoint = hitJoint;
	outHitDist = hitDist;

	return hitJoint != kInvalidJointId;
}

static bool RaycastConnections(const Ray& ray, const Vec3& eye, const IDefaultSkeleton& skel, const ISkeletonPose& pose, i32& outHitJoint, float& outDist)
{	
	OBB obb;
	obb.c = Vec3(ZERO);
	const ColorB black = ColorB(0, 0, 0);
	IRenderAuxGeom* pAuxGeom = gEnv->pRenderer->GetIRenderAuxGeom();
	float hitDist;
	i32 hitJoint = kInvalidJointId;
	for (i32 jid = 0, N = (i32)skel.GetJointCount(); jid < N; ++jid)
	{
		i32k pid = skel.GetJointParentIDByID(jid);
		if (pid == kInvalidJointId)
		{
			continue;
		}

		Vec3 center;
		ComputeConnectionOBB(obb, center, pose.GetAbsJointByID(jid).t, pose.GetAbsJointByID(pid).t);

		Vec3 where;
		if (Intersect::Ray_OBB(ray, center, obb, where))
		{
			float dist = (eye - where).GetLengthSquared();
			if (hitJoint == kInvalidJointId || dist < hitDist)
			{
				hitJoint = pid;
				hitDist = dist;
			}
		}
	}

	outHitJoint = hitJoint;
	outDist = hitDist;

	return hitJoint != kInvalidJointId;
}

static i32 PickJoint(const IDefaultSkeleton& skel, const ISkeletonPose& pose, const Ray& ray, const Vec3& eye)
{
	i32 c0, c1;
	float d0, d1;
	bool bHitJoint = RaycastJoints(ray, eye, skel, pose, c0, d0);
	bool bHitConn = RaycastConnections(ray, eye, skel, pose, c1, d1);

	if (bHitJoint && bHitConn)
	{
		// When joints and connections are close together, joints take precedence.
		return d0 < 1.2f * d1 ? c0 : c1;
	}
	else if (bHitJoint)
	{
		return c0;
	}
	else if (bHitConn)
	{
		return c1;
	}
	else
	{
		return kInvalidJointId;
	}
}

i32 PickJoint(const ICharacterInstance& charIns, QViewport& viewPort, float x, float y)
{
	if (!charIns.GetISkeletonPose())
	{
		return kInvalidJointId;
	}

	const Vec3 eye_ws = viewPort.Camera()->GetPosition();

	Ray ray_ws;
	if (!viewPort.ScreenToWorldRay(&ray_ws, x, y))
	{
		return kInvalidJointId;
	}

	return PickJoint(charIns.GetIDefaultSkeleton(), *charIns.GetISkeletonPose(), ray_ws, eye_ws);
}

static void DrawJoints(const IDefaultSkeleton& skel, const ISkeletonPose& pose)
{
	OBB obb;
	obb.c = Vec3(ZERO);
	obb.h = 0.5f * GetJointSize() * Vec3(1.0f, 1.0f, 1.0f);
	const ColorB black = ColorB(0, 0, 0);
	IRenderAuxGeom* pAuxGeom = gEnv->pRenderer->GetIRenderAuxGeom();
	for (i32 jid = 0, N = skel.GetJointCount(); jid < N; ++jid)
	{
		const QuatT& jointFrame = pose.GetAbsJointByID(jid);
		obb.m33 = Matrix33(jointFrame.q);
		pAuxGeom->DrawOBB(obb, Matrix34::CreateTranslationMat(jointFrame.t), false, black, EBoundingBoxDrawStyle::eBBD_Faceted);
	}
}

void DrawJoints(const ICharacterInstance& charIns)
{
	if (charIns.GetISkeletonPose())
	{
		DrawJoints(charIns.GetIDefaultSkeleton(), *charIns.GetISkeletonPose());
	}
}

void DrawHighlightedJoints(const IDefaultSkeleton& skel, const ISkeletonPose& pose, const std::vector<float>& heat, const Vec3& eye)
{
	u32k jointCount = skel.GetJointCount();
	typedef std::pair<i32, float> JRef; // Stores joint's id together with distance.
	std::vector<JRef> highlighted;
	highlighted.reserve(jointCount / 2);

	for (i32 jid = 0; jid < jointCount; ++jid)
	{
		if (heat[jid] > 0.0f)
		{
			const float dist = (pose.GetAbsJointByID(jid).t - eye).GetLengthSquared();
			highlighted.emplace_back(jid, dist);
		}
	}

	std::sort(highlighted.begin(), highlighted.end(), [](const JRef& lhp, const JRef& rhp)
	{
		return lhp.second < rhp.second; // Sort highlighted joints by distance.
	});

	OBB obb;
	obb.c = Vec3(ZERO);
	obb.h = 0.5f * GetJointSize() * Vec3(1.0f, 1.0f, 1.0f);
	const ColorB black = ColorB(0, 0, 0);
	IRenderAuxGeom* pAuxGeom = gEnv->pRenderer->GetIRenderAuxGeom();
	pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags | e_DepthTestOff | e_AlphaBlended);

	const ColorB tp = ColorB(0, 0, 0, 0);
	const ColorB yellow = ColorB(255, 255, 0, 127);
	ColorB col;

	for (const JRef& jref : highlighted)
	{
		const QuatT& jointFrame = pose.GetAbsJointByID(jref.first);
		obb.m33 = Matrix33(jointFrame.q);
		col.lerpFloat(tp, yellow, heat[jref.first]);
		pAuxGeom->DrawOBB(obb, Matrix34::CreateTranslationMat(jointFrame.t), true, col, EBoundingBoxDrawStyle::eBBD_Faceted);
	}
}

void DrawHighlightedJoints(const ICharacterInstance& charIns, std::vector<float>& heat, const Vec3& eye)
{
	if (charIns.GetISkeletonPose())
	{
		DrawHighlightedJoints(charIns.GetIDefaultSkeleton(), *charIns.GetISkeletonPose(), heat, eye);
	}
}

