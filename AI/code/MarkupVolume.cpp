// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MarkupVolume.h>

namespace MNM
{

void SMarkupVolume::Swap(SMarkupVolume& other)
{
	BoundingVolume::Swap(other);

	std::swap(areaAnnotation, other.areaAnnotation);
	std::swap(bStoreTriangles, other.bStoreTriangles);
	std::swap(bExpandByAgentRadius, other.bExpandByAgentRadius);
}

}
