// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __aiwave_h__
#define __aiwave_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "EntityObject.h"

class CAIWaveObject : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CAIWaveObject)

	virtual void InitVariables() {}

	void         SetName(const string& newName);

protected:
	//! Ctor must be protected.
	CAIWaveObject();

	void DeleteThis() { delete this; };
};

class CAIWaveObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_ENTITY; };
	tukk    ClassName()         { return "Entity::AIWave"; };
	tukk    Category()          { return ""; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CAIWaveObject); };
};

#endif // __aiwave_h__

