// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SceneElementCommon.h"

struct phys_geometry;

class CSceneElementProxyGeom : public CSceneElementCommon
{
public:
	CSceneElementProxyGeom(CScene* pScene, i32 id);

	void SetPhysGeom(phys_geometry* pPhysGeom);

	phys_geometry* GetPhysGeom();

	void Serialize(Serialization::IArchive& ar);

	virtual ESceneElementType GetType() const override;
private:
	phys_geometry* m_pPhysGeom;
};

