// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/LimbIk.h>

#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/PoseModifierHelper.h>

/*
   CLimbIk
 */

DRXREGISTER_CLASS(CLimbIk)

//

CLimbIk::CLimbIk()
{
	m_pSetups = m_setupsBuffer[0];
	m_setupCount = 0;

	m_pSetupsExecute = m_setupsBuffer[1];
	m_setupCountExecute = 0;
}

//

void CLimbIk::AddSetup(LimbIKDefinitionHandle setup, const Vec3& targetPositionLocal)
{
	if (m_setupCount >= sizeof(m_setupsBuffer[0]) / sizeof(m_setupsBuffer[0][0]))
		return;

	for (u32 i = 0; i < m_setupCount; ++i)
	{
		if (m_pSetups[i].setup != setup)
			continue;

		m_pSetups[i].targetPositionLocal = targetPositionLocal;
		return;
	}

	m_pSetups[m_setupCount].setup = setup;
	m_pSetups[m_setupCount].targetPositionLocal = targetPositionLocal;
	m_setupCount++;
}

// IAnimationPoseModifier

bool CLimbIk::Prepare(const SAnimationPoseModifierParams& params)
{
	std::swap(m_pSetups, m_pSetupsExecute);
	m_setupCountExecute = m_setupCount;
	m_setupCount = 0;
	return true;
}

bool CLimbIk::Execute(const SAnimationPoseModifierParams& params)
{
	DEFINE_PROFILER_FUNCTION();

	Skeleton::CPoseData* pPoseData = Skeleton::CPoseData::GetPoseData(params.pPoseData);
	if (!pPoseData)
		return false;

	const CDefaultSkeleton& defaultSkeleton = PoseModifierHelper::GetDefaultSkeleton(params);
	for (u32 i = 0; i < m_setupCountExecute; ++i)
	{
		PoseModifierHelper::IK_Solver(
		  defaultSkeleton, m_pSetupsExecute[i].setup, m_pSetupsExecute[i].targetPositionLocal,
		  *pPoseData);
	}
	return true;
}

void CLimbIk::Synchronize()
{
}
