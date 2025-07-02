
#include <drx3D/Physics/Collision/Shapes/MultimaterialTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexMaterialArray.h>

///Obtains the material for a specific triangle
const Material *MultimaterialTriangleMeshShape::getMaterialProperties(i32 partID, i32 triIndex)
{
	u8k *materialBase = 0;
	i32 numMaterials;
	PHY_ScalarType materialType;
	i32 materialStride;
	u8k *triangleMaterialBase = 0;
	i32 numTriangles;
	i32 triangleMaterialStride;
	PHY_ScalarType triangleType;

	((TriangleIndexVertexMaterialArray *)m_meshInterface)->getLockedReadOnlyMaterialBase(&materialBase, numMaterials, materialType, materialStride, &triangleMaterialBase, numTriangles, triangleMaterialStride, triangleType, partID);

	// return the pointer to the place with the friction for the triangle
	// TODO: This depends on whether it's a moving mesh or not
	// BUG IN GIMPACT
	//return (Scalar*)(&materialBase[triangleMaterialBase[(triIndex-1) * triangleMaterialStride] * materialStride]);
	i32 *matInd = (i32 *)(&(triangleMaterialBase[(triIndex * triangleMaterialStride)]));
	Material *matVal = (Material *)(&(materialBase[*matInd * materialStride]));
	return (matVal);
}
