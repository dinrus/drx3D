// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "ProximityTriggerObject.h"

REGISTER_CLASS_DESC(CProximityTriggerClassDesc);
IMPLEMENT_DYNCREATE(CProximityTrigger, CEntityObject)

CProximityTrigger::CProximityTrigger()
{
	m_entityClass = "ProximityTrigger";
}

void CProximityTrigger::Display(CObjectRenderHelper& objRenderHelper)
{
	DisplayContext& dc = objRenderHelper.GetDisplayContextRef();
	if (IEntity* pEntity = GetIEntity())
	{
		if (IScriptTable* pScriptTable = pEntity->GetScriptTable())
		{
			SmartScriptTable properties;
			pScriptTable->GetValue("Properties", properties);

			if (properties)
			{
				Vec3 dimensions;
				properties->GetValue("DimX", dimensions.x);
				properties->GetValue("DimY", dimensions.y);
				properties->GetValue("DimZ", dimensions.z);

				dc.DrawWireBox(pEntity->GetWorldPos() + dimensions * -0.5f, pEntity->GetWorldPos() + dimensions * 0.5f);
			}
		}
	}

	CEntityObject::Display(objRenderHelper);
}
