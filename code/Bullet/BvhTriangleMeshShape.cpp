//#define DISABLE_BVH

#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/OptimizedBvh.h>
#include <drx3D/Maths/Linear/Serializer.h>

///Bvh Concave triangle mesh is a static-triangle mesh shape with Bounding Volume Hierarchy optimization.
///Uses an interface to access the triangles to allow for sharing graphics/physics triangles.
BvhTriangleMeshShape::BvhTriangleMeshShape(StridingMeshInterface* meshInterface, bool useQuantizedAabbCompression, bool buildBvh)
	: TriangleMeshShape(meshInterface),
	  m_bvh(0),
	  m_triangleInfoMap(0),
	  m_useQuantizedAabbCompression(useQuantizedAabbCompression),
	  m_ownsBvh(false)
{
	m_shapeType = TRIANGLE_MESH_SHAPE_PROXYTYPE;
	//construct bvh from meshInterface
#ifndef DISABLE_BVH

	if (buildBvh)
	{
		buildOptimizedBvh();
	}

#endif  //DISABLE_BVH
}

BvhTriangleMeshShape::BvhTriangleMeshShape(StridingMeshInterface* meshInterface, bool useQuantizedAabbCompression, const Vec3& bvhAabbMin, const Vec3& bvhAabbMax, bool buildBvh)
	: TriangleMeshShape(meshInterface),
	  m_bvh(0),
	  m_triangleInfoMap(0),
	  m_useQuantizedAabbCompression(useQuantizedAabbCompression),
	  m_ownsBvh(false)
{
	m_shapeType = TRIANGLE_MESH_SHAPE_PROXYTYPE;
	//construct bvh from meshInterface
#ifndef DISABLE_BVH

	if (buildBvh)
	{
		uk mem = AlignedAlloc(sizeof(OptimizedBvh), 16);
		m_bvh = new (mem) OptimizedBvh();

		m_bvh->build(meshInterface, m_useQuantizedAabbCompression, bvhAabbMin, bvhAabbMax);
		m_ownsBvh = true;
	}

#endif  //DISABLE_BVH
}

void BvhTriangleMeshShape::partialRefitTree(const Vec3& aabbMin, const Vec3& aabbMax)
{
	m_bvh->refitPartial(m_meshInterface, aabbMin, aabbMax);

	m_localAabbMin.setMin(aabbMin);
	m_localAabbMax.setMax(aabbMax);
}

void BvhTriangleMeshShape::refitTree(const Vec3& aabbMin, const Vec3& aabbMax)
{
	m_bvh->refit(m_meshInterface, aabbMin, aabbMax);

	recalcLocalAabb();
}

BvhTriangleMeshShape::~BvhTriangleMeshShape()
{
	if (m_ownsBvh)
	{
		m_bvh->~OptimizedBvh();
		AlignedFree(m_bvh);
	}
}

void BvhTriangleMeshShape::performRaycast(TriangleCallback* callback, const Vec3& raySource, const Vec3& rayTarget)
{
	struct MyNodeOverlapCallback : public NodeOverlapCallback
	{
		StridingMeshInterface* m_meshInterface;
		TriangleCallback* m_callback;

		MyNodeOverlapCallback(TriangleCallback* callback, StridingMeshInterface* meshInterface)
			: m_meshInterface(meshInterface),
			  m_callback(callback)
		{
		}

		virtual void processNode(i32 nodeSubPart, i32 nodeTriangleIndex)
		{
			Vec3 m_triangle[3];
			u8k* vertexbase;
			i32 numverts;
			PHY_ScalarType type;
			i32 stride;
			u8k* indexbase;
			i32 indexstride;
			i32 numfaces;
			PHY_ScalarType indicestype;

			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase,
				numverts,
				type,
				stride,
				&indexbase,
				indexstride,
				numfaces,
				indicestype,
				nodeSubPart);

			u32* gfxbase = (u32*)(indexbase + nodeTriangleIndex * indexstride);

			const Vec3& meshScaling = m_meshInterface->getScaling();
			for (i32 j = 2; j >= 0; j--)
			{
				i32 graphicsindex;
                                switch (indicestype) {
                                        case PHY_INTEGER: graphicsindex = gfxbase[j]; break;
                                        case PHY_SHORT: graphicsindex = ((unsigned short*)gfxbase)[j]; break;
                                        case PHY_UCHAR: graphicsindex = ((u8*)gfxbase)[j]; break;
                                        default: Assert(0);
                                }

				if (type == PHY_FLOAT)
				{
					float* graphicsbase = (float*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
				}
				else
				{
					double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(Scalar(graphicsbase[0]) * meshScaling.getX(), Scalar(graphicsbase[1]) * meshScaling.getY(), Scalar(graphicsbase[2]) * meshScaling.getZ());
				}
			}

			/* Perform ray vs. triangle collision here */
			m_callback->processTriangle(m_triangle, nodeSubPart, nodeTriangleIndex);
			m_meshInterface->unLockReadOnlyVertexBase(nodeSubPart);
		}
	};

	MyNodeOverlapCallback myNodeCallback(callback, m_meshInterface);

	m_bvh->reportRayOverlappingNodex(&myNodeCallback, raySource, rayTarget);
}

void BvhTriangleMeshShape::performConvexcast(TriangleCallback* callback, const Vec3& raySource, const Vec3& rayTarget, const Vec3& aabbMin, const Vec3& aabbMax)
{
	struct MyNodeOverlapCallback : public NodeOverlapCallback
	{
		StridingMeshInterface* m_meshInterface;
		TriangleCallback* m_callback;

		MyNodeOverlapCallback(TriangleCallback* callback, StridingMeshInterface* meshInterface)
			: m_meshInterface(meshInterface),
			  m_callback(callback)
		{
		}

		virtual void processNode(i32 nodeSubPart, i32 nodeTriangleIndex)
		{
			Vec3 m_triangle[3];
			u8k* vertexbase;
			i32 numverts;
			PHY_ScalarType type;
			i32 stride;
			u8k* indexbase;
			i32 indexstride;
			i32 numfaces;
			PHY_ScalarType indicestype;

			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase,
				numverts,
				type,
				stride,
				&indexbase,
				indexstride,
				numfaces,
				indicestype,
				nodeSubPart);

			u32* gfxbase = (u32*)(indexbase + nodeTriangleIndex * indexstride);

			const Vec3& meshScaling = m_meshInterface->getScaling();
			for (i32 j = 2; j >= 0; j--)
			{
				i32 graphicsindex;
                                switch (indicestype) {
                                        case PHY_INTEGER: graphicsindex = gfxbase[j]; break;
                                        case PHY_SHORT: graphicsindex = ((unsigned short*)gfxbase)[j]; break;
                                        case PHY_UCHAR: graphicsindex = ((u8*)gfxbase)[j]; break;
                                        default: Assert(0);
                                }

				if (type == PHY_FLOAT)
				{
					float* graphicsbase = (float*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
				}
				else
				{
					double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(Scalar(graphicsbase[0]) * meshScaling.getX(), Scalar(graphicsbase[1]) * meshScaling.getY(), Scalar(graphicsbase[2]) * meshScaling.getZ());
				}
			}

			/* Perform ray vs. triangle collision here */
			m_callback->processTriangle(m_triangle, nodeSubPart, nodeTriangleIndex);
			m_meshInterface->unLockReadOnlyVertexBase(nodeSubPart);
		}
	};

	MyNodeOverlapCallback myNodeCallback(callback, m_meshInterface);

	m_bvh->reportBoxCastOverlappingNodex(&myNodeCallback, raySource, rayTarget, aabbMin, aabbMax);
}

//perform bvh tree traversal and report overlapping triangles to 'callback'
void BvhTriangleMeshShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
#ifdef DISABLE_BVH
	//brute force traverse all triangles
	TriangleMeshShape::processAllTriangles(callback, aabbMin, aabbMax);
#else

	//first get all the nodes

	struct MyNodeOverlapCallback : public NodeOverlapCallback
	{
		StridingMeshInterface* m_meshInterface;
		TriangleCallback* m_callback;
		Vec3 m_triangle[3];
		i32 m_numOverlap;

		MyNodeOverlapCallback(TriangleCallback* callback, StridingMeshInterface* meshInterface)
			: m_meshInterface(meshInterface),
			  m_callback(callback),
			  m_numOverlap(0)
		{
		}

		virtual void processNode(i32 nodeSubPart, i32 nodeTriangleIndex)
		{
			m_numOverlap++;
			u8k* vertexbase;
			i32 numverts;
			PHY_ScalarType type;
			i32 stride;
			u8k* indexbase;
			i32 indexstride;
			i32 numfaces;
			PHY_ScalarType indicestype;

			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase,
				numverts,
				type,
				stride,
				&indexbase,
				indexstride,
				numfaces,
				indicestype,
				nodeSubPart);

			u32* gfxbase = (u32*)(indexbase + nodeTriangleIndex * indexstride);
			Assert(indicestype == PHY_INTEGER || indicestype == PHY_SHORT || indicestype == PHY_UCHAR);

			const Vec3& meshScaling = m_meshInterface->getScaling();
			for (i32 j = 2; j >= 0; j--)
			{
				i32 graphicsindex = indicestype == PHY_SHORT ? ((unsigned short*)gfxbase)[j] : indicestype == PHY_INTEGER ? gfxbase[j] : ((u8*)gfxbase)[j];

#ifdef DEBUG_TRIANGLE_MESH
				printf("%d ,", graphicsindex);
#endif  //DEBUG_TRIANGLE_MESH
				if (type == PHY_FLOAT)
				{
					float* graphicsbase = (float*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(
						graphicsbase[0] * meshScaling.getX(),
						graphicsbase[1] * meshScaling.getY(),
						graphicsbase[2] * meshScaling.getZ());
				}
				else
				{
					double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);

					m_triangle[j] = Vec3(
						Scalar(graphicsbase[0]) * meshScaling.getX(),
						Scalar(graphicsbase[1]) * meshScaling.getY(),
						Scalar(graphicsbase[2]) * meshScaling.getZ());
				}
#ifdef DEBUG_TRIANGLE_MESH
				printf("triangle vertices:%f,%f,%f\n", triangle[j].x(), triangle[j].y(), triangle[j].z());
#endif  //DEBUG_TRIANGLE_MESH
			}

			m_callback->processTriangle(m_triangle, nodeSubPart, nodeTriangleIndex);
			m_meshInterface->unLockReadOnlyVertexBase(nodeSubPart);
		}
	};

	MyNodeOverlapCallback myNodeCallback(callback, m_meshInterface);

	m_bvh->reportAabbOverlappingNodex(&myNodeCallback, aabbMin, aabbMax);

#endif  //DISABLE_BVH
}

void BvhTriangleMeshShape::setLocalScaling(const Vec3& scaling)
{
	if ((getLocalScaling() - scaling).length2() > SIMD_EPSILON)
	{
		TriangleMeshShape::setLocalScaling(scaling);
		buildOptimizedBvh();
	}
}

void BvhTriangleMeshShape::buildOptimizedBvh()
{
	if (m_ownsBvh)
	{
		m_bvh->~OptimizedBvh();
		AlignedFree(m_bvh);
	}
	///m_localAabbMin/m_localAabbMax is already re-calculated in TriangleMeshShape. We could just scale aabb, but this needs some more work
	uk mem = AlignedAlloc(sizeof(OptimizedBvh), 16);
	m_bvh = new (mem) OptimizedBvh();
	//rebuild the bvh...
	m_bvh->build(m_meshInterface, m_useQuantizedAabbCompression, m_localAabbMin, m_localAabbMax);
	m_ownsBvh = true;
}

void BvhTriangleMeshShape::setOptimizedBvh(OptimizedBvh* bvh, const Vec3& scaling)
{
	Assert(!m_bvh);
	Assert(!m_ownsBvh);

	m_bvh = bvh;
	m_ownsBvh = false;
	// update the scaling without rebuilding the bvh
	if ((getLocalScaling() - scaling).length2() > SIMD_EPSILON)
	{
		TriangleMeshShape::setLocalScaling(scaling);
	}
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk BvhTriangleMeshShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	TriangleMeshShapeData* trimeshData = (TriangleMeshShapeData*)dataBuffer;

	CollisionShape::serialize(&trimeshData->m_collisionShapeData, serializer);

	m_meshInterface->serialize(&trimeshData->m_meshInterface, serializer);

	trimeshData->m_collisionMargin = float(m_collisionMargin);

	if (m_bvh && !(serializer->getSerializationFlags() & DRX3D_SERIALIZE_NO_BVH))
	{
		uk chunk = serializer->findPointer(m_bvh);
		if (chunk)
		{
#ifdef DRX3D_USE_DOUBLE_PRECISION
			trimeshData->m_quantizedDoubleBvh = (QuantizedBvhData*)chunk;
			trimeshData->m_quantizedFloatBvh = 0;
#else
			trimeshData->m_quantizedFloatBvh = (QuantizedBvhData*)chunk;
			trimeshData->m_quantizedDoubleBvh = 0;
#endif  //DRX3D_USE_DOUBLE_PRECISION
		}
		else
		{
#ifdef DRX3D_USE_DOUBLE_PRECISION
			trimeshData->m_quantizedDoubleBvh = (QuantizedBvhData*)serializer->getUniquePointer(m_bvh);
			trimeshData->m_quantizedFloatBvh = 0;
#else
			trimeshData->m_quantizedFloatBvh = (QuantizedBvhData*)serializer->getUniquePointer(m_bvh);
			trimeshData->m_quantizedDoubleBvh = 0;
#endif  //DRX3D_USE_DOUBLE_PRECISION

			i32 sz = m_bvh->calculateSerializeBufferSizeNew();
			Chunk* chunk = serializer->allocate(sz, 1);
			tukk structType = m_bvh->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_QUANTIZED_BVH_CODE, m_bvh);
		}
	}
	else
	{
		trimeshData->m_quantizedFloatBvh = 0;
		trimeshData->m_quantizedDoubleBvh = 0;
	}

	if (m_triangleInfoMap && !(serializer->getSerializationFlags() & DRX3D_SERIALIZE_NO_TRIANGLEINFOMAP))
	{
		uk chunk = serializer->findPointer(m_triangleInfoMap);
		if (chunk)
		{
			trimeshData->m_triangleInfoMap = (TriangleInfoMapData*)chunk;
		}
		else
		{
			trimeshData->m_triangleInfoMap = (TriangleInfoMapData*)serializer->getUniquePointer(m_triangleInfoMap);
			i32 sz = m_triangleInfoMap->calculateSerializeBufferSize();
			Chunk* chunk = serializer->allocate(sz, 1);
			tukk structType = m_triangleInfoMap->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_TRIANLGE_INFO_MAP, m_triangleInfoMap);
		}
	}
	else
	{
		trimeshData->m_triangleInfoMap = 0;
	}

	// Fill padding with zeros to appease msan.
	memset(trimeshData->m_pad3, 0, sizeof(trimeshData->m_pad3));

	return "TriangleMeshShapeData";
}

void BvhTriangleMeshShape::serializeSingleBvh(Serializer* serializer) const
{
	if (m_bvh)
	{
		i32 len = m_bvh->calculateSerializeBufferSizeNew();  //make sure not to use calculateSerializeBufferSize because it is used for in-place
		Chunk* chunk = serializer->allocate(len, 1);
		tukk structType = m_bvh->serialize(chunk->m_oldPtr, serializer);
		serializer->finalizeChunk(chunk, structType, DRX3D_QUANTIZED_BVH_CODE, (uk )m_bvh);
	}
}

void BvhTriangleMeshShape::serializeSingleTriangleInfoMap(Serializer* serializer) const
{
	if (m_triangleInfoMap)
	{
		i32 len = m_triangleInfoMap->calculateSerializeBufferSize();
		Chunk* chunk = serializer->allocate(len, 1);
		tukk structType = m_triangleInfoMap->serialize(chunk->m_oldPtr, serializer);
		serializer->finalizeChunk(chunk, structType, DRX3D_TRIANLGE_INFO_MAP, (uk )m_triangleInfoMap);
	}
}
