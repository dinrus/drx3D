// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "EntityObject.h"

//////////////////////////////////////////////////////////////////////////
// WindArea entity.
//////////////////////////////////////////////////////////////////////////
class CWindAreaEntity : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CWindAreaEntity)

	//////////////////////////////////////////////////////////////////////////
	CWindAreaEntity(){}
	virtual void Display(CObjectRenderHelper& objRenderHelper);

private:
};

/*!
 * Class Description of Entity
 */
class CWindAreaEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_ENTITY; };
	tukk         ClassName()                         { return "Entity::WindArea"; };
	tukk         Category()                          { return ""; };
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CWindAreaEntity); };
	tukk         GetFileSpec()                       { return "*.cgf;*.chr;*.cga;*.cdf"; };
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
};

// Constraint entity.
//////////////////////////////////////////////////////////////////////////
class CConstraintEntity : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CConstraintEntity)

	//////////////////////////////////////////////////////////////////////////
	CConstraintEntity() : m_body0(NULL), m_body1(NULL) {}

	virtual void Display(CObjectRenderHelper& objRenderHelper);

private:
	// the orientation of the constraint in world, body0 and body1 space
	quaternionf m_qframe, m_qloc0, m_qloc1;

	// the position of the constraint in world space
	Vec3 m_posLoc;

	// pointers to the two joined bodies
	IPhysicalEntity* m_body0, * m_body1;
};

/*!
 * Class Description of Entity
 */
class CConstraintEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_ENTITY; };
	// this was done because the string was prefixed in the object manager and would otherwise return by default CEntityObject
	tukk         ClassName()                         { return "Entity::Constraint"; };
	tukk         Category()                          { return ""; };
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CConstraintEntity); };
	tukk         GetFileSpec()                       { return "*.cgf;*.chr;*.cga;*.cdf"; };
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
};

#if defined(USE_GEOM_CACHES)
// GeomCache entity.
class SANDBOX_API CGeomCacheEntity : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CGeomCacheEntity)

	CGeomCacheEntity() {}

	virtual bool HitTestEntity(HitContext& hc, bool& bHavePhysics) override
	{
		IGeomCacheRenderNode* pGeomCacheRenderNode = m_pEntity->GetGeomCacheRenderNode(0);
		if (pGeomCacheRenderNode)
		{
			SRayHitInfo hitInfo;
			ZeroStruct(hitInfo);
			hitInfo.inReferencePoint = hc.raySrc;
			hitInfo.inRay = Ray(hitInfo.inReferencePoint, hc.rayDir.GetNormalized());
			hitInfo.bInFirstHit = false;
			hitInfo.bUseCache = false;

			if (pGeomCacheRenderNode->RayIntersection(hitInfo))
			{
				hc.object = this;
				hc.dist = hitInfo.fDistance;
				return true;
			}
		}

		return false;
	}
};
#endif

#if defined(USE_GEOM_CACHES)
/*!
 * Class Description of Entity
 */
class CGeomCacheEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_GEOMCACHE; };
	tukk         ClassName()                         { return "Entity::GeomCache"; };
	tukk         Category()                          { return ""; };
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CGeomCacheEntity); };
	tukk         GetFileSpec()                       { return "*.cax"; };
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
	virtual bool        RenderTextureOnTop() const override { return true; }
};
#endif

//////////////////////////////////////////////////////////////////////////
// JointGen entity
//////////////////////////////////////////////////////////////////////////
class CJointGenEntity : public CEntityObject
{
public:
	DECLARE_DYNCREATE(CJointGenEntity)
	CJointGenEntity() {}
	virtual void Display(CObjectRenderHelper& objRenderHelper);
private:
	string                m_fname;
	_smart_ptr<IStatObj>  m_pObj;
	_smart_ptr<IGeometry> m_pSph;
};

class CJointGenEntityClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()                     { return OBJTYPE_ENTITY; };
	tukk         ClassName()                         { return "Entity::JointGen"; };
	tukk         Category()                          { return ""; };
	CRuntimeClass*      GetRuntimeClass()                   { return RUNTIME_CLASS(CJointGenEntity); };
	tukk         GetFileSpec()                       { return "*.cgf"; };
	virtual tukk GetDataFilesFilterString() override { return GetFileSpec(); }
};

