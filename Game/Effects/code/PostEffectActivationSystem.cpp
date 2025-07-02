// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Simple data driven system to activate post effects.

// Includes
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PostEffectActivationSystem.h>
#include <drx3D/Act/IItemSystem.h>

//--------------------------------------------------------------------------------------------------
// Name: Initialise
// Desc: Initialises post effect activation system from data
//       Uses the xml node name for the post effect, and activeValue and nonActiveValue attributes
//--------------------------------------------------------------------------------------------------
void CPostEffectActivationSystem::Initialise(const IItemParamsNode* postEffectListXmlNode)
{
	if(postEffectListXmlNode)
	{
		const IItemParamsNode* postEffectXmlNode = NULL;
		SPostEffectParam* param = NULL;
		i32 childCount = postEffectListXmlNode->GetChildCount();
		i32 postEffectCount = childCount;

		const IItemParamsNode* vecsXmlNode = postEffectListXmlNode->GetChild("vecs");

		if(vecsXmlNode)
		{
			postEffectCount--;
		}

		m_postEffectParam.resize(postEffectCount);

		i32 paramIndex=0;
		for(i32 c=0; c<childCount; c++)
		{
			postEffectXmlNode = postEffectListXmlNode->GetChild(c);
			if(postEffectXmlNode && postEffectXmlNode != vecsXmlNode)
			{
				param = &m_postEffectParam[paramIndex];
				drx_strcpy(param->name, postEffectXmlNode->GetName());
				postEffectXmlNode->GetAttribute("activeValue",param->activeValue);
				postEffectXmlNode->GetAttribute("nonActiveValue",param->nonActiveValue);

				i32 forceValue = 0;
				postEffectXmlNode->GetAttribute("forceValue",forceValue);
				param->forceValue = (forceValue) ? true : false;
				paramIndex++;
			}
		}

		if(vecsXmlNode)
		{
			SPostEffectParamVec* paramVec = NULL;
			i32k vecCount = vecsXmlNode->GetChildCount();
			m_postEffectParamVec.resize(vecCount);
			for(i32 i=0; i<vecCount; i++)
			{
				postEffectXmlNode = vecsXmlNode->GetChild(i);

				paramVec = &m_postEffectParamVec[i];
				drx_strcpy(paramVec->name, postEffectXmlNode->GetName());

				Vec3 vecValue(0.0f,0.0f,0.0f);
				float wValue = 0.0f;

				postEffectXmlNode->GetAttribute("activeVec3",vecValue);
				postEffectXmlNode->GetAttribute("activeW",wValue);
				paramVec->activeValue = Vec4(vecValue,wValue);

				postEffectXmlNode->GetAttribute("nonActiveVec3",vecValue);
				postEffectXmlNode->GetAttribute("nonActiveW",wValue);
				paramVec->nonActiveValue = Vec4(vecValue,wValue);

				i32 forceValue = 0;
				postEffectXmlNode->GetAttribute("forceValue",forceValue);
				paramVec->forceValue = (forceValue) ? true : false;
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Release
// Desc: Releases data used for post effect activation system
//--------------------------------------------------------------------------------------------------
void CPostEffectActivationSystem::Release()
{
	m_postEffectParam.Free();
	m_postEffectParamVec.Free();
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetPostEffectsActive
// Desc: Sets active status of post effects
//--------------------------------------------------------------------------------------------------
void CPostEffectActivationSystem::SetPostEffectsActive(bool isActive)
{
	SPostEffectParam* param = NULL;
	float value = 0.0f;
	for(size_t i=0; i<m_postEffectParam.Size(); i++)
	{
		param = &m_postEffectParam[i];
		value = (isActive) ? param->activeValue : param->nonActiveValue;
		gEnv->p3DEngine->SetPostEffectParam( param->name, value, param->forceValue );
	}

	SPostEffectParamVec* paramVec = NULL;
	Vec4* pVec = NULL;
	for(size_t i=0; i<m_postEffectParamVec.Size(); i++)
	{
		paramVec = &m_postEffectParamVec[i];
		pVec = (isActive) ? &paramVec->activeValue : &paramVec->nonActiveValue;
		gEnv->p3DEngine->SetPostEffectParamVec4( paramVec->name, *pVec, paramVec->forceValue );
	}
}//-------------------------------------------------------------------------------------------------
