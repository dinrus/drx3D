// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Geo.h>

//////////////////////////////////////////////////////////////////////////
// IMeshObj:
//		Minimal base for static and animated mesh objects
//		Uses DrxNameId facilities, to support drxinterface_cast<>, to query for derived classes.
//		But not based on IDrxUnknown, as existing mesh classes are not compatible with IDrxUnknown creation and ref-counting.

struct IMeshObj
{
	virtual ~IMeshObj() {}

	//! Standard reference-counting methods
	virtual i32 AddRef() = 0;
	virtual i32 Release() = 0;
	virtual i32 GetRefCount() const = 0;

	//! Get the local bounding box
	virtual AABB GetAABB() const = 0;

	//! Get the object radius or radius^2
	virtual float GetRadiusSqr() const { return GetAABB().GetRadiusSqr(); }
	virtual float GetRadius() const    { return GetAABB().GetRadius(); }

	//! Get the mesh extent in the requested dimension
	virtual float GetExtent(EGeomForm eForm) = 0;

	//! Generate a random point in the requested dimension
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const = 0;

	//! Return the rendering material
	virtual struct IMaterial* GetMaterial() const = 0;

	//! Return the rendering geometry
	virtual struct IRenderMesh* GetRenderMesh() const = 0;

	//! Return the associated physical entity
	virtual struct IPhysicalEntity* GetPhysEntity() const = 0;

	//! Return the physical representation of the object.
	//! Arguments:
	//!     nType - one of PHYS_GEOM_TYPE_'s, or an explicit slot index
	virtual struct phys_geometry* GetPhysGeom(i32 nType = PHYS_GEOM_TYPE_DEFAULT) const = 0;

	//! Render the object
	virtual void Render(const struct SRendParams& rParams, const SRenderingPassInfo& passInfo) = 0;
};
