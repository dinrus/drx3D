// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/NavigationIdTypes.h>

namespace MNM
{

//! Defines area type annotation of triangles in NavMesh
//! Can be used for having different costs of traversing triangles and possibly more
//! Area type have default area flags (see SAreaFlag), that are assigned to triangles when the NavMesh is generated
struct SAreaType
{
	SAreaType() {}
	SAreaType(u32 id, tukk szName, u32 flags)
		: id(id)
		, name(szName)
		, defaultFlags(flags)
	{}

	u32 id;
	AreaAnnotation::value_type defaultFlags;
	string name;
};

//! Defines area flags annotation of triangles in NavMesh
//! Can be used for filtering accessible triangles in navigation queries (path finding, raycast, triangles queries, etc.)
struct SAreaFlag
{
	SAreaFlag(u32 id, tukk szName)
		: name(szName)
		, id(id)
		, value(BIT(id))
		, color(Col_Red, 0.5f)
	{}

	u32 id;
	AreaAnnotation::value_type value;
	ColorB color;
	string name;
};

//! Container for storing all registered area types and flags used for annotating triangles in NavMesh
struct IAnnotationsLibrary
{
	virtual NavigationAreaTypeID GetAreaTypeID(tukk szName) const = 0;
	virtual tukk          GetAreaTypeName(const NavigationAreaTypeID areaTypeID) const = 0;
	virtual const SAreaType*     GetAreaType(const NavigationAreaTypeID areaTypeID) const = 0;
	virtual size_t               GetAreaTypeCount() const = 0;
	virtual NavigationAreaTypeID GetAreaTypeID(const size_t index) const = 0;
	virtual const SAreaType*     GetAreaType(const size_t index) const = 0;
	virtual const SAreaType&     GetDefaultAreaType() const = 0;

	virtual NavigationAreaFlagID GetAreaFlagID(tukk szName) const = 0;
	virtual tukk          GetAreaFlagName(const NavigationAreaFlagID areaFlagID) const = 0;
	virtual const SAreaFlag*     GetAreaFlag(const NavigationAreaFlagID areaFlagID) const = 0;
	virtual size_t               GetAreaFlagCount() const = 0;
	virtual NavigationAreaFlagID GetAreaFlagID(const size_t index) const = 0;
	virtual const SAreaFlag*     GetAreaFlag(const size_t index) const = 0;

	virtual void                 GetAreaColor(const AreaAnnotation annotation, ColorB& color) const = 0;
};

} //endns MNM
