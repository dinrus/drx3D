// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/FlyHelpers_TacticalPointLanguageExtender.h>
#include <drx3D/AI/FlyHelpers_Path.h>

#include <drx3D/AI/PipeUser.h>

namespace FlyHelpers
{

//////////////////////////////////////////////////////////////////////////
void CTacticalPointLanguageExtender::Initialize()
{
	if (gEnv->pAISystem->GetTacticalPointSystem())
	{
		RegisterWithTacticalPointSystem();
		RegisterQueries();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTacticalPointLanguageExtender::Deinitialize()
{
	if (gEnv->pAISystem->GetTacticalPointSystem())
	{
		UnregisterFromTacticalPointSystem();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTacticalPointLanguageExtender::GeneratePoints(TGenerateParameters& parameters, SGenerateDetails& details, TObjectType obj, const Vec3& objectPos, TObjectType auxObject, const Vec3& auxObjectPos) const
{
	const static u32 s_pointsInDesignerPathCrc = CCrc32::Compute("pointsInDesignerPath");
	u32k queryNameCrc = CCrc32::Compute(parameters.query);

	if (queryNameCrc == s_pointsInDesignerPathCrc)
	{
		IF_UNLIKELY (details.fDensity <= 1.0f)
		{
			return false;
		}

		IEntity* pEntity = obj->GetEntity();
		DRX_ASSERT(pEntity);

		IAIObject* pAiObject = pEntity->GetAI();
		IF_UNLIKELY (!pAiObject)
		{
			return false;
		}

		CPipeUser* pPipeUser = pAiObject->CastToCPipeUser();
		IF_UNLIKELY (!pPipeUser)
		{
			return false;
		}

		tukk pathName = pPipeUser->GetPathToFollow();
		DRX_ASSERT(pathName);

		SShape path;
		const bool getPathSuccess = gAIEnv.pNavigation->GetDesignerPath(pathName, path);
		IF_UNLIKELY (!getPathSuccess)
		{
			return false;
		}

		const bool isValidPath = (!path.shape.empty());
		IF_UNLIKELY (!isValidPath)
		{
			return false;
		}

		// TODO: Remove unnecessary creation of this path?
		FlyHelpers::Path flyPath;
		for (size_t i = 0; i < path.shape.size(); ++i)
		{
			const Vec3& point = path.shape[i];
			flyPath.AddPoint(point);
		}

		// TODO: Make this an option, or get this property by other means.
		const bool shouldConsiderAsLoopingPath = true;
		if (shouldConsiderAsLoopingPath)
		{
			flyPath.MakeLooping();
		}

		const size_t segmentCount = flyPath.GetSegmentCount();
		for (size_t i = 0; i < flyPath.GetSegmentCount(); ++i)
		{
			const Lineseg segment = flyPath.GetSegment(i);
			const float length = flyPath.GetSegmentLength(i);

			DRX_ASSERT(0 < length);
			const float advance = details.fDensity / length;

			float sampled = 0;
			while (sampled < 1.0f)
			{
				parameters.result->AddPoint(segment.GetPoint(sampled));
				sampled += advance;
			}
		}

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTacticalPointLanguageExtender::RegisterWithTacticalPointSystem()
{
	ITacticalPointSystem* pTacticalPointSystem = gEnv->pAISystem->GetTacticalPointSystem();
	DRX_ASSERT(pTacticalPointSystem);

	const bool successfullyAddedLanguageExtender = pTacticalPointSystem->AddLanguageExtender(this);
	DRX_ASSERT_MESSAGE(successfullyAddedLanguageExtender, "Failed to add tactical point language extender.");
}

//////////////////////////////////////////////////////////////////////////
void CTacticalPointLanguageExtender::RegisterQueries()
{
	ITacticalPointSystem* pTacticalPointSystem = gEnv->pAISystem->GetTacticalPointSystem();
	DRX_ASSERT(pTacticalPointSystem);

	pTacticalPointSystem->ExtendQueryLanguage("pointsInDesignerPath", eTPQT_GENERATOR, eTPQC_IGNORE);
}

//////////////////////////////////////////////////////////////////////////
void CTacticalPointLanguageExtender::UnregisterFromTacticalPointSystem()
{
	ITacticalPointSystem* pTacticalPointSystem = gEnv->pAISystem->GetTacticalPointSystem();
	DRX_ASSERT(pTacticalPointSystem);

	const bool successfullyRemovedLanguageExtender = pTacticalPointSystem->RemoveLanguageExtender(this);
	DRX_ASSERT_MESSAGE(successfullyRemovedLanguageExtender, "Failed to remove tactical point language extender.");
}

}
