#ifndef DRX3D_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H
#define DRX3D_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H

#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>

ATTRIBUTE_ALIGNED16(struct)
MaterialProperties
{
	///m_materialBase ==========> 2 Scalar values make up one material, friction then restitution
	i32 m_numMaterials;
	u8k* m_materialBase;
	i32 m_materialStride;
	PHY_ScalarType m_materialType;
	///m_numTriangles <=========== This exists in the IndexedMesh object for the same subpart, but since we're
	///                           padding the structure, it can be reproduced at no real cost
	///m_triangleMaterials =====> 1 integer value makes up one entry
	///                           eg: m_triangleMaterials[1] = 5; // This will set triangle 2 to use material 5
	i32 m_numTriangles;
	u8k* m_triangleMaterialsBase;
	i32 m_triangleMaterialStride;
	///m_triangleType <========== Automatically set in addMaterialProperties
	PHY_ScalarType m_triangleType;
};

typedef AlignedObjectArray<MaterialProperties> MaterialArray;

///Teh TriangleIndexVertexMaterialArray is built on TriangleIndexVertexArray
///The addition of a material array allows for the utilization of the partID and
///triangleIndex that are returned in the ContactAddedCallback.  As with
///TriangleIndexVertexArray, no duplicate is made of the material data, so it
///is the users responsibility to maintain the array during the lifetime of the
///TriangleIndexVertexMaterialArray.
ATTRIBUTE_ALIGNED16(class)
TriangleIndexVertexMaterialArray : public TriangleIndexVertexArray
{
protected:
	MaterialArray m_materials;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	TriangleIndexVertexMaterialArray()
	{
	}

	TriangleIndexVertexMaterialArray(i32 numTriangles, i32* triangleIndexBase, i32 triangleIndexStride,
									   i32 numVertices, Scalar* vertexBase, i32 vertexStride,
									   i32 numMaterials, u8* materialBase, i32 materialStride,
									   i32* triangleMaterialsBase, i32 materialIndexStride);

	virtual ~TriangleIndexVertexMaterialArray() {}

	void addMaterialProperties(const MaterialProperties& mat, PHY_ScalarType triangleType = PHY_INTEGER)
	{
		m_materials.push_back(mat);
		m_materials[m_materials.size() - 1].m_triangleType = triangleType;
	}

	virtual void getLockedMaterialBase(u8** materialBase, i32& numMaterials, PHY_ScalarType& materialType, i32& materialStride,
									   u8** triangleMaterialBase, i32& numTriangles, i32& triangleMaterialStride, PHY_ScalarType& triangleType, i32 subpart = 0);

	virtual void getLockedReadOnlyMaterialBase(u8k** materialBase, i32& numMaterials, PHY_ScalarType& materialType, i32& materialStride,
											   u8k** triangleMaterialBase, i32& numTriangles, i32& triangleMaterialStride, PHY_ScalarType& triangleType, i32 subpart = 0);
};

#endif  //DRX3D_MULTIMATERIAL_TRIANGLE_INDEX_VERTEX_ARRAY_H
