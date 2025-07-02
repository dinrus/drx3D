
#include <drx3D/Physics/Collision/NarrowPhase/b3StridingMeshInterface.h>

b3StridingMeshInterface::~b3StridingMeshInterface()
{
}

void b3StridingMeshInterface::InternalProcessAllTriangles(b3InternalTriangleIndexCallback* callback, const b3Vec3& aabbMin, const b3Vec3& aabbMax) const
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
	b3Vec3 triangle[3];

	b3Vec3 meshScaling = getScaling();

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
						drx3DAssert((gfxindextype == PHY_INTEGER) || (gfxindextype == PHY_SHORT));
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
							triangle[0].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
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
							triangle[0].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
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
							triangle[0].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[1] * stride);
							triangle[1].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							graphicsbase = (double*)(vertexbase + tri_indices[2] * stride);
							triangle[2].setVal((b3Scalar)graphicsbase[0] * meshScaling.getX(), (b3Scalar)graphicsbase[1] * meshScaling.getY(), (b3Scalar)graphicsbase[2] * meshScaling.getZ());
							callback->internalProcessTriangleIndex(triangle, part, gfxindex);
						}
						break;
					}
					default:
						drx3DAssert((gfxindextype == PHY_INTEGER) || (gfxindextype == PHY_SHORT));
				}
				break;
			}
			default:
				drx3DAssert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
		}

		unLockReadOnlyVertexBase(part);
	}
}

void b3StridingMeshInterface::calculateAabbBruteForce(b3Vec3& aabbMin, b3Vec3& aabbMax)
{
	struct AabbCalculationCallback : public b3InternalTriangleIndexCallback
	{
		b3Vec3 m_aabbMin;
		b3Vec3 m_aabbMax;

		AabbCalculationCallback()
		{
			m_aabbMin.setVal(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));
			m_aabbMax.setVal(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
		}

		virtual void internalProcessTriangleIndex(b3Vec3* triangle, i32 partId, i32 triangleIndex)
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
	aabbMin.setVal(b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT), b3Scalar(-D3_LARGE_FLOAT));
	aabbMax.setVal(b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT), b3Scalar(D3_LARGE_FLOAT));
	InternalProcessAllTriangles(&aabbCallback, aabbMin, aabbMax);

	aabbMin = aabbCallback.m_aabbMin;
	aabbMax = aabbCallback.m_aabbMax;
}
