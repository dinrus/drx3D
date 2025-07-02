
#include <drx3D/Physics/Collision/Shapes/StridingMeshInterface.h>
#include <drx3D/Maths/Linear/Serializer.h>

StridingMeshInterface::~StridingMeshInterface()
{
}

void StridingMeshInterface::InternalProcessAllTriangles(InternalTriangleIndexCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	(void)aabbMin;
	(void)aabbMax;
	i32 numtotalphysicsverts = 0;
	i32 part, graphicssubparts = getNumSubParts();
	u8k* vertexbase;
	u8k* indexbase;
	i32 indexstride;
	PHY_ScalarType type;
	PHY_ScalarType gfxindextype;
	i32 stride, numverts, numtriangles;
	i32 gfxindex;
	Vec3 triangle[3];

	Vec3 meshScaling = getScaling();

	///if the number of parts is big, the performance might drop due to the innerloop switch on indextype
	for (part = 0; part < graphicssubparts; part++)
	{
		getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride, &indexbase, indexstride, numtriangles, gfxindextype, part);
		numtotalphysicsverts += numtriangles * 3;  //upper bound

		///unlike that developers want to pass in double-precision meshes in single-precision drx3D build
		///so disable this feature by default
		///see patch http://code.google.com/p/bullet/issues/detail?id=213

		switch (type)
		{
			case PHY_FLOAT:
			{
				float* graphicsbase;

				switch (gfxindextype)
				{
					case PHY_INTEGER:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u32* tri_indices = (u32*)(indexbase + gfxindex * indexstride);
							graphicsbase = (float*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					case PHY_SHORT:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u16* tri_indices = (u16*)(indexbase + gfxindex * indexstride);
							graphicsbase = (float*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					case PHY_UCHAR:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u8* tri_indices = (u8*)(indexbase + gfxindex * indexstride);
							graphicsbase = (float*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (float*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal(graphicsbase[0] * meshScaling.getX(), graphicsbase[1] * meshScaling.getY(), graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					default:
						Assert((gfxindextype == PHY_INTEGER) || (gfxindextype == PHY_SHORT));
				}
				break;
			}

			case PHY_DOUBLE:
			{
				double* graphicsbase;

				switch (gfxindextype)
				{
					case PHY_INTEGER:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u32* tri_indices = (u32*)(indexbase + gfxindex * indexstride);
							graphicsbase = (double*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					case PHY_SHORT:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u16* tri_indices = (u16*)(indexbase + gfxindex * indexstride);
							graphicsbase = (double*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					case PHY_UCHAR:
					{
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u8* tri_indices = (u8*)(indexbase + gfxindex * indexstride);
							graphicsbase = (double*)(vertexbase + tri_indices[0] * stride);
							triangle[0].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((Scalar)graphicsbase[0] * meshScaling.getX(), (Scalar)graphicsbase[1] * meshScaling.getY(), (Scalar)graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					default:
						Assert((gfxindextype == PHY_INTEGER) || (gfxindextype == PHY_SHORT));
				}
				break;
			}
			default:
				Assert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
		}

		unLockReadOnlyVertexBase(part);
	}
}

void StridingMeshInterface::calculateAabbBruteForce(Vec3& aabbMin, Vec3& aabbMax)
{
	struct AabbCalculationCallback : public InternalTriangleIndexCallback
	{
		Vec3 m_aabbMin;
		Vec3 m_aabbMax;

		AabbCalculationCallback()
		{
			m_aabbMin.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
			m_aabbMax.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
		}

		virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
		{
			(void)partId;
			(void)triangleIndex;

			m_aabbMin.setMin(triangle[0]);
			m_aabbMax.setMax(triangle[0]);
			m_aabbMin.setMin(triangle[1]);
			m_aabbMax.setMax(triangle[1]);
			m_aabbMin.setMin(triangle[2]);
			m_aabbMax.setMax(triangle[2]);
		}
	};

	//first calculate the total aabb for all triangles
	AabbCalculationCallback aabbCallback;
	aabbMin.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
	aabbMax.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	InternalProcessAllTriangles(&aabbCallback, aabbMin, aabbMax);

	aabbMin = aabbCallback.m_aabbMin;
	aabbMax = aabbCallback.m_aabbMax;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk StridingMeshInterface::serialize(uk dataBuffer, Serializer* serializer) const
{
	StridingMeshInterfaceData* trimeshData = (StridingMeshInterfaceData*)dataBuffer;

	trimeshData->m_numMeshParts = getNumSubParts();

	//uk uniquePtr = 0;

	trimeshData->m_meshPartsPtr = 0;

	if (trimeshData->m_numMeshParts)
	{
		Chunk* chunk = serializer->allocate(sizeof(MeshPartData), trimeshData->m_numMeshParts);
		MeshPartData* memPtr = (MeshPartData*)chunk->m_oldPtr;
		trimeshData->m_meshPartsPtr = (MeshPartData*)serializer->getUniquePointer(memPtr);

		//	i32 numtotalphysicsverts = 0;
		i32 part, graphicssubparts = getNumSubParts();
		u8k* vertexbase;
		u8k* indexbase;
		i32 indexstride;
		PHY_ScalarType type;
		PHY_ScalarType gfxindextype;
		i32 stride, numverts, numtriangles;
		i32 gfxindex;
		//	Vec3 triangle[3];

		//	Vec3 meshScaling = getScaling();

		///if the number of parts is big, the performance might drop due to the innerloop switch on indextype
		for (part = 0; part < graphicssubparts; part++, memPtr++)
		{
			getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride, &indexbase, indexstride, numtriangles, gfxindextype, part);
			memPtr->m_numTriangles = numtriangles;  //indices = 3*numtriangles
			memPtr->m_numVertices = numverts;
			memPtr->m_indices16 = 0;
			memPtr->m_indices32 = 0;
			memPtr->m_3indices16 = 0;
			memPtr->m_3indices8 = 0;
			memPtr->m_vertices3f = 0;
			memPtr->m_vertices3d = 0;

			switch (gfxindextype)
			{
				case PHY_INTEGER:
				{
					i32 numindices = numtriangles * 3;

					if (numindices)
					{
						Chunk* chunk = serializer->allocate(sizeof(IntIndexData), numindices);
						IntIndexData* tmpIndices = (IntIndexData*)chunk->m_oldPtr;
						memPtr->m_indices32 = (IntIndexData*)serializer->getUniquePointer(tmpIndices);
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u32* tri_indices = (u32*)(indexbase + gfxindex * indexstride);
							tmpIndices[gfxindex * 3].m_value = tri_indices[0];
							tmpIndices[gfxindex * 3 + 1].m_value = tri_indices[1];
							tmpIndices[gfxindex * 3 + 2].m_value = tri_indices[2];
						}
						serializer->finalizeChunk(chunk, "IntIndexData", DRX3D_ARRAY_CODE, (uk )chunk->m_oldPtr);
					}
					break;
				}
				case PHY_SHORT:
				{
					if (numtriangles)
					{
						Chunk* chunk = serializer->allocate(sizeof(ShortIntIndexTripletData), numtriangles);
						ShortIntIndexTripletData* tmpIndices = (ShortIntIndexTripletData*)chunk->m_oldPtr;
						memPtr->m_3indices16 = (ShortIntIndexTripletData*)serializer->getUniquePointer(tmpIndices);
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u16* tri_indices = (u16*)(indexbase + gfxindex * indexstride);
							tmpIndices[gfxindex].m_values[0] = tri_indices[0];
							tmpIndices[gfxindex].m_values[1] = tri_indices[1];
							tmpIndices[gfxindex].m_values[2] = tri_indices[2];
							// Fill padding with zeros to appease msan.
							tmpIndices[gfxindex].m_pad[0] = 0;
							tmpIndices[gfxindex].m_pad[1] = 0;
						}
						serializer->finalizeChunk(chunk, "ShortIntIndexTripletData", DRX3D_ARRAY_CODE, (uk )chunk->m_oldPtr);
					}
					break;
				}
				case PHY_UCHAR:
				{
					if (numtriangles)
					{
						Chunk* chunk = serializer->allocate(sizeof(CharIndexTripletData), numtriangles);
						CharIndexTripletData* tmpIndices = (CharIndexTripletData*)chunk->m_oldPtr;
						memPtr->m_3indices8 = (CharIndexTripletData*)serializer->getUniquePointer(tmpIndices);
						for (gfxindex = 0; gfxindex < numtriangles; gfxindex++)
						{
							u8* tri_indices = (u8*)(indexbase + gfxindex * indexstride);
							tmpIndices[gfxindex].m_values[0] = tri_indices[0];
							tmpIndices[gfxindex].m_values[1] = tri_indices[1];
							tmpIndices[gfxindex].m_values[2] = tri_indices[2];
							// Fill padding with zeros to appease msan.
							tmpIndices[gfxindex].m_pad = 0;
						}
						serializer->finalizeChunk(chunk, "CharIndexTripletData", DRX3D_ARRAY_CODE, (uk )chunk->m_oldPtr);
					}
					break;
				}
				default:
				{
					Assert(0);
					//unknown index type
				}
			}

			switch (type)
			{
				case PHY_FLOAT:
				{
					float* graphicsbase;

					if (numverts)
					{
						Chunk* chunk = serializer->allocate(sizeof(Vec3FloatData), numverts);
						Vec3FloatData* tmpVertices = (Vec3FloatData*)chunk->m_oldPtr;
						memPtr->m_vertices3f = (Vec3FloatData*)serializer->getUniquePointer(tmpVertices);
						for (i32 i = 0; i < numverts; i++)
						{
							graphicsbase = (float*)(vertexbase + i * stride);
							tmpVertices[i].m_floats[0] = graphicsbase[0];
							tmpVertices[i].m_floats[1] = graphicsbase[1];
							tmpVertices[i].m_floats[2] = graphicsbase[2];
						}
						serializer->finalizeChunk(chunk, "Vec3FloatData", DRX3D_ARRAY_CODE, (uk )chunk->m_oldPtr);
					}
					break;
				}

				case PHY_DOUBLE:
				{
					if (numverts)
					{
						Chunk* chunk = serializer->allocate(sizeof(Vec3DoubleData), numverts);
						Vec3DoubleData* tmpVertices = (Vec3DoubleData*)chunk->m_oldPtr;
						memPtr->m_vertices3d = (Vec3DoubleData*)serializer->getUniquePointer(tmpVertices);
						for (i32 i = 0; i < numverts; i++)
						{
							double* graphicsbase = (double*)(vertexbase + i * stride);  //for now convert to float, might leave it at double
							tmpVertices[i].m_floats[0] = graphicsbase[0];
							tmpVertices[i].m_floats[1] = graphicsbase[1];
							tmpVertices[i].m_floats[2] = graphicsbase[2];
						}
						serializer->finalizeChunk(chunk, "Vec3DoubleData", DRX3D_ARRAY_CODE, (uk )chunk->m_oldPtr);
					}
					break;
				}

				default:
					Assert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
			}

			unLockReadOnlyVertexBase(part);
		}

		serializer->finalizeChunk(chunk, "MeshPartData", DRX3D_ARRAY_CODE, chunk->m_oldPtr);
	}

	// Fill padding with zeros to appease msan.
	memset(trimeshData->m_padding, 0, sizeof(trimeshData->m_padding));

	m_scaling.serializeFloat(trimeshData->m_scaling);
	return "StridingMeshInterfaceData";
}
