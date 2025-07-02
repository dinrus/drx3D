// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObject.h"
#include "AreaBox.h"

/*!
 *	CAreaSphere is a sphere volume in space where entities can be attached to.
 *
 */
class CAreaSphere : public CAreaObjectBase
{
public:
	DECLARE_DYNCREATE(CAreaSphere)

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool         CreateGameObject();
	virtual void InitVariables();
	void         Display(CObjectRenderHelper& objRenderHelper) override;
	bool         IsScalable() const override  { return false; }
	bool         IsRotatable() const override { return false; }
	void         GetLocalBounds(AABB& box);
	bool         HitTest(HitContext& hc);

	virtual void PostLoad(CObjectArchive& ar);

	void         Serialize(CObjectArchive& ar);
	XmlNodeRef   Export(const string& levelPath, XmlNodeRef& xmlNode);

	void         SetAreaId(i32 nAreaId);
	i32          GetAreaId();
	void         SetRadius(float fRadius);
	float        GetRadius();

	virtual void OnEntityAdded(IEntity const* const pIEntity);
	virtual void OnEntityRemoved(IEntity const* const pIEntity);

	virtual void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

private:

	void UpdateGameArea();
	void UpdateAttachedEntities();

protected:
	//! Dtor must be protected.
	CAreaSphere();

	void DeleteThis() { delete this; }

	void Reload(bool bReloadScript = false) override;
	void OnAreaChange(IVariable* pVar) override;
	void OnSizeChange(IVariable* pVar);

	CVariable<float> m_innerFadeDistance;
	CVariable<i32>   m_areaId;
	CVariable<float> m_edgeWidth;
	CVariable<float> m_radius;
	CVariable<i32>   mv_groupId;
	CVariable<i32>   mv_priority;
	CVariable<bool>  mv_filled;
};

/*!
 * Class Description of AreaBox.
 */
class CAreaSphereClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_VOLUME; }
	tukk    ClassName()         { return "AreaSphere"; }
	tukk    UIName()            { return "Sphere"; }
	tukk    Category()          { return "Area"; }
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CAreaSphere); }
};

