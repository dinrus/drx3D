// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/SkeletonAnim.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <float.h>

void CSkeletonAnim::SetDesiredMotionParam(EMotionParamID nParameterID, float fParameter, float deltaTime222)
{
	if (nParameterID >= eMotionParamID_COUNT)
		return; //not a valid parameter

	//we store the parameters in the run-time structure of the ParametricSampler
	for (i32 layer = 0; layer < ISkeletonAnim::LayerCount; ++layer)
	{
		i32k animCount = GetNumAnimsInFIFO(layer);
		for (i32 i = 0; i < animCount; i++)
		{
			CAnimation& anim = GetAnimFromFIFO(layer, i);
			if (anim.GetParametricSampler() == 0)
				continue;

			u32k blendingOut = (i < animCount - 1);

			//It's a Parametric Animation
			SParametricSampler& lmg = *anim.GetParametricSampler();
			for (u32 d = 0; d < lmg.m_numDimensions; d++)
			{
				if (lmg.m_MotionParameterID[d] == nParameterID)
				{
					u32k locked = (lmg.m_MotionParameterFlags[d] & CA_Dim_LockedParameter);
					u32k init   = lmg.m_MotionParameterFlags[d] & CA_Dim_Initialized;
					if (init == 0 || (locked == 0 && blendingOut == 0)) //if already initialized and locked or blending out, then we can't change the parameter any more
					{
						lmg.m_MotionParameter[d]                 = fParameter;
						lmg.m_MotionParameterForNextIteration[d] = fParameter;
						lmg.m_MotionParameterFlags[d]           |= CA_Dim_Initialized;
					}
					if (locked)
					{
						lmg.m_MotionParameterForNextIteration[d] = fParameter;
					}
				}
			}
		}
	}
}

bool CSkeletonAnim::GetDesiredMotionParam(EMotionParamID id, float& value) const
{
	for (i32 layer = 0; layer < ISkeletonAnim::LayerCount; ++layer)
	{
		uint samplerCount = GetNumAnimsInFIFO(layer);
		uint samplerMax = MAX_EXEC_QUEUE;
		if (samplerMax > samplerCount)
			samplerMax = samplerCount;
		if (samplerCount > samplerMax)
			samplerCount = samplerMax;

		uint samplerActiveCount = 0;
		for (uint i = 0; i < samplerCount; ++i)
		{
			const CAnimation& animation = GetAnimFromFIFO(layer, i);
			samplerActiveCount += animation.IsActivated() ? 1 : 0;
		}

		if (samplerActiveCount > samplerMax)
			samplerActiveCount = samplerMax;
		samplerMax = samplerActiveCount;

		for (i32 i = samplerMax - 1; i >= 0; --i)
		{
			const CAnimation& animation = GetAnimFromFIFO(layer, i);
			const SParametricSampler* pParametric = animation.GetParametricSampler();
			if (!pParametric)
				continue;

			for (i32 motionParam = 0; motionParam < MAX_LMG_DIMENSIONS; ++motionParam)
			{
				if (pParametric->m_MotionParameterID[motionParam] == id)
				{
					value = pParametric->m_MotionParameter[motionParam];
					return true;
				}
			}
		}
	}

	return false;
}
