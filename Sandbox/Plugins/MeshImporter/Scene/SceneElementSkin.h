// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SceneElementCommon.h"

class CSceneElementSkin : public CSceneElementCommon
{
public:
	CSceneElementSkin(CScene* pScene, i32 id);
	virtual ~CSceneElementSkin() {}

	virtual ESceneElementType GetType() const override;
};
