// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Objects/EntityObject.h"

class CEntityObjectWithParticleComponent : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CEntityObjectWithParticleComponent)

	// CEntityObject
	virtual bool Init(CBaseObject* prev, const string& file) override;
	virtual bool CreateGameObject() override;
	// ~CEntityObject

protected:
	string m_file;
};

/*!
* Class Description of Entity with a default component
*/
class CEntityObjectWithParticleComponentClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType() { return OBJTYPE_ENTITY; }
	tukk         ClassName() { return "EntityWithParticleComponent"; }
	tukk         Category() { return "Particle Emitter"; }
	CRuntimeClass*      GetRuntimeClass() { return RUNTIME_CLASS(CEntityObjectWithParticleComponent); }
	tukk         GetFileSpec() { return "*.pfx;*.pfx2"; }
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
	// Disallow creation from the Create Object panel, only use the Asset Browser
	virtual bool        IsCreatable() const { return false; }
};

