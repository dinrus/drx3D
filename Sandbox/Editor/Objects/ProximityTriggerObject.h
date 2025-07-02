// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObject.h"

class CProximityTrigger : public CEntityObject
{
public:
	CProximityTrigger();
	DECLARE_DYNCREATE(CProximityTrigger)

	virtual void Display(CObjectRenderHelper& objRenderHelper) override;

protected:
	void  DeleteThis() { delete this; };
};

class CProximityTriggerClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()   { return OBJTYPE_ENTITY; };
	tukk         ClassName()       { return "Entity::ProximityTrigger"; };
	tukk         Category()        { return ""; };
	CRuntimeClass*      GetRuntimeClass() { return RUNTIME_CLASS(CProximityTrigger); };
	virtual tukk GetTextureIcon()  { return "%EDITOR%/ObjectIcons/proximitytrigger.bmp"; };
	virtual bool   IsCreatable() const override { return gEnv->pEntitySystem->GetClassRegistry()->FindClass("ProximityTrigger") != nullptr; }
};
