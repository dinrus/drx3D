// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TagPoint_h__
#define __TagPoint_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "EntityObject.h"
#include <EditorFramework/Preferences.h>

// Preferences
struct STagPointPreferences : public SPreferencePage
{
	STagPointPreferences()
		: SPreferencePage("Tag Point", "Viewport/Gizmo")
		, tagpointScaleMulti(0.5f)
	{
	}

	virtual bool Serialize(yasli::Archive& ar) override
	{
		ar.openBlock("tagPoint", "Tag Point");
		ar(tagpointScaleMulti, "tagpointScaleMulti", "Tagpoint Scale Multiplier");

		return true;
	}

	float tagpointScaleMulti;
};

/*!
 *	CTagPoint is an object that represent named 3d position in world.
 *
 */

class CTagPoint : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CTagPoint)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init(CBaseObject* prev, const string& file);
	void InitVariables();
	void Display(CObjectRenderHelper& objRenderHelper) override;
	bool IsScalable() const override { return false; }

	//! Called when object is being created.
	i32  MouseCreateCallback(IDisplayViewport* view, EMouseEvent event, CPoint& point, i32 flags);
	bool HitTest(HitContext& hc);

	void GetLocalBounds(AABB& box);
	void GetBoundBox(AABB& box);
	//////////////////////////////////////////////////////////////////////////

protected:
	CTagPoint();

	float GetRadius();

	void  DeleteThis() { delete this; };
};

/*
   NavigationSeedPoint is a special tag point used to generate the reachable
   part of a navigation mesh starting from his position
 */
class CNavigationSeedPoint : public CTagPoint
{
public:
	DECLARE_DYNCREATE(CNavigationSeedPoint)
	//////////////////////////////////////////////////////////////////////////
	// Overides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	virtual void Display(CObjectRenderHelper& objRenderHelper);
	virtual i32  MouseCreateCallback(IDisplayViewport* view, EMouseEvent event, CPoint& point, i32 flags);
	virtual void Done();
	virtual void SetModified(bool boModifiedTransformOnly, bool bNotifyObjectManager);

	//////////////////////////////////////////////////////////////////////////
protected:
	CNavigationSeedPoint();
};

/*!
 * Class Description of TagPoint.
 */
class CTagPointClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()   { return OBJTYPE_TAGPOINT; };
	tukk         ClassName()       { return "StdTagPoint"; };
	tukk         Category()        { return ""; };
	CRuntimeClass*      GetRuntimeClass() { return RUNTIME_CLASS(CTagPoint); };
	virtual tukk GetTextureIcon()  { return "%EDITOR%/ObjectIcons/TagPoint.bmp"; };
	virtual bool   IsCreatable() const override { return gEnv->pEntitySystem->GetClassRegistry()->FindClass("TagPoint") != nullptr; }
};

class CNavigationSeedPointClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()   { return OBJTYPE_TAGPOINT; };
	tukk    ClassName()       { return "NavigationSeedPoint"; };
	tukk    Category()        { return "AI"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CNavigationSeedPoint); };
	virtual bool   IsCreatable() const override { return gEnv->pEntitySystem->GetClassRegistry()->FindClass("NavigationSeedPoint") != nullptr; }
};

#endif // __TagPoint_h__

