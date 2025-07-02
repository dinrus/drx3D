// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __visareashapeobject_h__
#define __visareashapeobject_h__
#pragma once

#include "ShapeObject.h"

//////////////////////////////////////////////////////////////////////////
class C3DEngineAreaObjectBase : public CShapeObject
{
public:
	C3DEngineAreaObjectBase();
	virtual void       Done();
	virtual XmlNodeRef Export(const string& levelPath, XmlNodeRef& xmlNode);
	virtual bool       CreateGameObject();
	virtual void       Validate()
	{
		CBaseObject::Validate();
	}

protected:
	struct IVisArea* m_area;
	struct IRamArea* m_RamArea;
};

/** Represent Visibility Area, visibility areas can be connected with portals.
 */
class CVisAreaShapeObject : public C3DEngineAreaObjectBase
{
	DECLARE_DYNCREATE(CVisAreaShapeObject)
public:
	CVisAreaShapeObject();

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	virtual void InitVariables() override;
	virtual void SetWorldPos(const Vec3& pos, i32 flags = 0) override;
	virtual void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;
	//////////////////////////////////////////////////////////////////////////

	virtual void OnShapeChange(IVariable* pVariable) override;

protected:
	virtual void UpdateGameArea();

	// Visibility area in 3d engine.
	CVariable<Vec3>  mv_vAmbientColor;
	CVariable<bool>  mv_bAffectedBySun;
	CVariable<bool>  mv_bIgnoreSkyColor;
	CVariable<float> mv_fViewDistRatio;
	CVariable<bool>  mv_bSkyOnly;
	CVariable<bool>  mv_bOceanIsVisible;
	CVariable<bool>  mv_bIgnoreGI;
	CVariable<bool>  mv_bIgnoreOutdoorAO;
};

/** Represent Portal Area.
    Portal connect visibility areas, visibility between vis areas are only done with portals.
 */
class CPortalShapeObject : public CVisAreaShapeObject
{
	DECLARE_DYNCREATE(CPortalShapeObject)
public:
	CPortalShapeObject();
	void InitVariables();

protected:
	virtual i32  GetMaxPoints() const { return 4; };
	virtual void UpdateGameArea();

	CVariable<bool>  mv_bUseDeepness;
	CVariable<bool>  mv_bDoubleSide;
	CVariable<bool>  mv_bPortalBlending;
	CVariable<float> mv_fPortalBlendValue;
};

/** Represent Occluder Area.
   Areas that occlude objects behind it.
 */
class COccluderPlaneObject : public C3DEngineAreaObjectBase
{
	DECLARE_DYNCREATE(COccluderPlaneObject)
public:
	COccluderPlaneObject();

	void InitVariables();

protected:
	virtual i32  GetMinPoints() const { return 2; };
	virtual i32  GetMaxPoints() const { return 2; };
	virtual void UpdateGameArea();

	CVariable<bool>  mv_bUseInIndoors;
	CVariable<bool>  mv_bDoubleSide;
	CVariable<float> mv_fViewDistRatio;
};

/** Represent Occluder Plane.
   Areas that occlude objects behind it.
 */
class COccluderAreaObject : public C3DEngineAreaObjectBase
{
	DECLARE_DYNCREATE(COccluderAreaObject)
public:
	COccluderAreaObject();

	void InitVariables();

protected:
	virtual i32  GetMinPoints() const { return 4; };
	virtual i32  GetMaxPoints() const { return 4; };
	virtual void UpdateGameArea();

	CVariable<bool>  mv_bUseInIndoors;
	CVariable<float> mv_fViewDistRatio;
};

/*!
 * Class Description of CVisAreaShapeObject.
 */
class CVisAreaShapeObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_VOLUME; };
	tukk    ClassName()         { return "VisArea"; };
	tukk    Category()          { return "Area"; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CVisAreaShapeObject); };
};

/*!
 * Class Description of CPortalShapeObject.
 */
class CPortalShapeObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_VOLUME; };
	tukk    ClassName()         { return "Portal"; };
	tukk    Category()          { return "Area"; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CPortalShapeObject); };
};

/*!
 * Class Description of COccluderPlaneObject.
 */
class COccluderPlaneObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_VOLUME; };
	tukk    ClassName()         { return "OccluderPlane"; };
	tukk    Category()          { return "Area"; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(COccluderPlaneObject); };
};

/*!
 * Class Description of COccluderAreaObject.
 */
class COccluderAreaObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_VOLUME; };
	tukk    ClassName()         { return "OccluderArea"; };
	tukk    UIName()            { return "Occluder"; };
	tukk    Category()          { return "Area"; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(COccluderAreaObject); };
};

#endif // __visareashapeobject_h__

