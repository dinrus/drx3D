// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObject.h"

class CSimpleEntity : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CSimpleEntity)

	//////////////////////////////////////////////////////////////////////////
	bool    Init(CBaseObject* prev, const string& file);
	bool    ConvertFromObject(CBaseObject* object);

	void    Validate();
	bool    IsSimilarObject(CBaseObject* pObject);

	string GetGeometryFile() const;
	void    SetGeometryFile(const string& filename);

private:
	void OnFileChange(string filename);
};

/*!
 * Class Description of Entity
 */
class CSimpleEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_ENTITY; };
	tukk         ClassName()                         { return "SimpleEntity"; };
	tukk         Category()                          { return ""; };
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CSimpleEntity); };
	tukk         GetFileSpec()                       { return "*.cgf;*.chr;*.cga;*.cdf"; };
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
};

