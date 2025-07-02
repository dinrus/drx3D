#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexMaterialArray.h>

TriangleIndexVertexMaterialArray::TriangleIndexVertexMaterialArray(i32 numTriangles, i32* triangleIndexBase, i32 triangleIndexStride,
																	   i32 numVertices, Scalar* vertexBase, i32 vertexStride,
																	   i32 numMaterials, u8* materialBase, i32 materialStride,
																	   i32* triangleMaterialsBase, i32 materialIndexStride) : TriangleIndexVertexArray(numTriangles, triangleIndexBase, triangleIndexStride, numVertices, vertexBase, vertexStride)
{
	MaterialProperties mat;

	mat.m_numMaterials = numMaterials;
	mat.m_materialBase = materialBase;
	mat.m_materialStride = materialStride;
#ifdef DRX3D_USE_DOUBLE_PRECISION
	mat.m_materialType = PHY_DOUBLE;
#else
	mat.m_materialType = PHY_FLOAT;
#endif

	mat.m_numTriangles = numTriangles;
	mat.m_triangleMaterialsBase = (u8*)triangleMaterialsBase;
	mat.m_triangleMaterialStride = materialIndexStride;
	mat.m_triangleType = PHY_INTEGER;

	addMaterialProperties(mat);
}

void TriangleIndexVertexMaterialArray::getLockedMaterialBase(u8** materialBase, i32& numMaterials, PHY_ScalarType& materialType, i32& materialStride,
															   u8** triangleMaterialBase, i32& numTriangles, i32& triangleMaterialStride, PHY_ScalarType& triangleType, i32 subpart)
{
	Assert(subpart < getNumSubParts());

	MaterialProperties& mats = m_materials[subpart];

	numMaterials = mats.m_numMaterials;
	(*materialBase) = (u8*)mats.m_materialBase;
#ifdef DRX3D_USE_DOUBLE_PRECISION
	materialType = PHY_DOUBLE;
#else
	materialType = PHY_FLOAT;
#endif
	materialStride = mats.m_materialStride;

	numTriangles = mats.m_numTriangles;
	(*triangleMaterialBase) = (u8*)mats.m_triangleMaterialsBase;
	triangleMaterialStride = mats.m_triangleMaterialStride;
	triangleType = mats.m_triangleType;
}

void TriangleIndexVertexMaterialArray::getLockedReadOnlyMaterialBase(u8k** materialBase, i32& numMaterials, PHY_ScalarType& materialType, i32& materialStride,
																	   u8k** triangleMaterialBase, i32& numTriangles, i32& triangleMaterialStride, PHY_ScalarType& triangleType, i32 subpart)
{
	MaterialProperties& mats = m_materials[subpart];

	numMaterials = mats.m_numMaterials;
	(*materialBase) = (u8k*)mats.m_materialBase;
#ifdef DRX3D_USE_DOUBLE_PRECISION
	materialType = PHY_DOUBLE;
#else
	materialType = PHY_FLOAT;
#endif
	materialStride = mats.m_materialStride;

	numTriangles = mats.m_numTriangles;
	(*triangleMaterialBase) = (u8k*)mats.m_triangleMaterialsBase;
	triangleMaterialStride = mats.m_triangleMaterialStride;
	triangleType = mats.m_triangleType;
}
