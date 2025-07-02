// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SceneElementCommon.h"

struct SPhysProxies;

class CSceneElementPhysProxies : public CSceneElementCommon
{
public:
	CSceneElementPhysProxies(CScene* pScene, i32 id);

	void SetPhysProxies(SPhysProxies* pPhysProxies);

	SPhysProxies* GetPhysProxies();

	void Serialize(Serialization::IArchive& ar);

	virtual ESceneElementType GetType() const override;
private:
	SPhysProxies* m_pPhysProxies;
};

