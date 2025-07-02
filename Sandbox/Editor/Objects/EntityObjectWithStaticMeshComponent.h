// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObjectWithComponent.h"

class CEntityObjectWithStaticMeshComponent : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CEntityObjectWithStaticMeshComponent)

	// CEntityObject
	virtual bool Init(CBaseObject* prev, const string& file) override;
	virtual bool CreateGameObject() override;
	virtual bool ConvertFromObject(CBaseObject* object) override;
	// ~CEntityObject

protected:
	string m_file;
};

/*!
* Class Description of Entity with a default component
*/
class CEntityWithStaticMeshComponentClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType() { return OBJTYPE_ENTITY; }
	tukk         ClassName() { return "EntityWithStaticMeshComponent"; }
	tukk         Category() { return "Static Mesh Entity"; }
	CRuntimeClass*      GetRuntimeClass() { return RUNTIME_CLASS(CEntityObjectWithStaticMeshComponent); }
	tukk         GetFileSpec() { return "*.cgf"; }
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
};

