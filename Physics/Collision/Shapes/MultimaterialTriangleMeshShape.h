#ifndef DRX3D_BVH_TRIANGLE_MATERIAL_MESH_SHAPE_H
#define DRX3D_BVH_TRIANGLE_MATERIAL_MESH_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/Material.h>

///The BvhTriangleMaterialMeshShape extends the BvhTriangleMeshShape. Its main contribution is the interface into a material array, which allows per-triangle friction and restitution.
ATTRIBUTE_ALIGNED16(class)
MultimaterialTriangleMeshShape : public BvhTriangleMeshShape
{
	AlignedObjectArray<Material *> m_materialList;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultimaterialTriangleMeshShape(StridingMeshInterface * meshInterface, bool useQuantizedAabbCompression, bool buildBvh = true) : BvhTriangleMeshShape(meshInterface, useQuantizedAabbCompression, buildBvh)
	{
		m_shapeType = MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE;

		u8k *vertexbase;
		i32 numverts;
		PHY_ScalarType type;
		i32 stride;
		u8k *indexbase;
		i32 indexstride;
		i32 numfaces;
		PHY_ScalarType indicestype;

		//m_materialLookup = (i32**)(AlignedAlloc(sizeof(i32*) * meshInterface->getNumSubParts(), 16));

		for (i32 i = 0; i < meshInterface->getNumSubParts(); i++)
		{
			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase,
				numverts,
				type,
				stride,
				&indexbase,
				indexstride,
				numfaces,
				indicestype,
				i);
			//m_materialLookup[i] = (i32*)(AlignedAlloc(sizeof(i32) * numfaces, 16));
		}
	}

	///optionally pass in a larger bvh aabb, used for quantization. This allows for deformations within this aabb
	MultimaterialTriangleMeshShape(StridingMeshInterface * meshInterface, bool useQuantizedAabbCompression, const Vec3 &bvhAabbMin, const Vec3 &bvhAabbMax, bool buildBvh = true) : BvhTriangleMeshShape(meshInterface, useQuantizedAabbCompression, bvhAabbMin, bvhAabbMax, buildBvh)
	{
		m_shapeType = MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE;

		u8k *vertexbase;
		i32 numverts;
		PHY_ScalarType type;
		i32 stride;
		u8k *indexbase;
		i32 indexstride;
		i32 numfaces;
		PHY_ScalarType indicestype;

		//m_materialLookup = (i32**)(AlignedAlloc(sizeof(i32*) * meshInterface->getNumSubParts(), 16));

		for (i32 i = 0; i < meshInterface->getNumSubParts(); i++)
		{
			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase,
				numverts,
				type,
				stride,
				&indexbase,
				indexstride,
				numfaces,
				indicestype,
				i);
			//m_materialLookup[i] = (i32*)(AlignedAlloc(sizeof(i32) * numfaces * 2, 16));
		}
	}

	virtual ~MultimaterialTriangleMeshShape()
	{
		/*
        for(i32 i = 0; i < m_meshInterface->getNumSubParts(); i++)
        {
            AlignedFree(m_materialValues[i]);
            m_materialLookup[i] = NULL;
        }
        AlignedFree(m_materialValues);
        m_materialLookup = NULL;
*/
	}
	//debugging
	virtual tukk getName() const { return "MULTIMATERIALTRIANGLEMESH"; }

	///Obtains the material for a specific triangle
	const Material *getMaterialProperties(i32 partID, i32 triIndex);
};

#endif  //DRX3D_BVH_TRIANGLE_MATERIAL_MESH_SHAPE_H
