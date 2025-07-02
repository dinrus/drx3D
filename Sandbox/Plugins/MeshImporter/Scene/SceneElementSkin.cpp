// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SceneElementSkin.h"
#include "SceneElementTypes.h"


CSceneElementSkin::CSceneElementSkin(CScene* pScene, i32 id)
	: CSceneElementCommon(pScene, id)
{
}

ESceneElementType CSceneElementSkin::GetType() const
{
	return ESceneElementType::Skin;
}

