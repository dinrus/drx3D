// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

#include "Objects/BaseObject.h"
#include <DrxMovie/IMovieSystem.h>

class CSequenceObject : public CBaseObject, public IAnimSequenceOwner
{
public:
	DECLARE_DYNCREATE(CSequenceObject)

	// Overides from CBaseObject.
	bool           Init(CBaseObject* prev, const string& file) override;
	void           Display(CObjectRenderHelper& objRenderHelper) override;

	void           GetBoundBox(AABB& box) override;
	void           GetLocalBounds(AABB& box) override;
	void           SetName(const string& name) override;
	bool           CreateGameObject() override;
	void           Done() override;
	void           Serialize(CObjectArchive& ar) override;
	virtual void   PostLoad(CObjectArchive& ar) override;

	IAnimSequence* GetSequence() { return m_pSequence; }
	void           OnModified();

protected:
	//! Dtor must be protected.
	CSequenceObject();

	void DeleteThis() { delete this; };

	// Local callbacks.

private:
	_smart_ptr<struct IAnimSequence> m_pSequence;
	u32                           m_sequenceId;
};

/*!
 * Class Description of CSequenceObject.
 */
class CSequenceObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_OTHER; };
	tukk    ClassName()         { return "SequenceObject"; };
	tukk    Category()          { return ""; }; // Empty category name to prevent showing object in creation tab
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CSequenceObject); };
	tukk    GetFileSpec()       { return ""; };
	tukk    GetTextureIcon()    { return "%EDITOR%/ObjectIcons/sequence.bmp"; };
};

