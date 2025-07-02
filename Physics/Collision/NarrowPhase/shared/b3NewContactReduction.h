
#ifndef D3_NEW_CONTACT_REDUCTION_H
#define D3_NEW_CONTACT_REDUCTION_H

#include <drx3D/Common/shared/b3Float4.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>

#define GET_NPOINTS(x) (x).m_worldNormalOnB.w

i32 b3ExtractManifoldSequentialGlobal(__global const b3Float4* p, i32 nPoints, b3Float4ConstArg nearNormal, b3Int4* contactIdx)
{
	if (nPoints == 0)
		return 0;

	if (nPoints <= 4)
		return nPoints;

	if (nPoints > 64)
		nPoints = 64;

	b3Float4 center = b3MakeFloat4(0, 0, 0, 0);
	{
		for (i32 i = 0; i < nPoints; i++)
			center += p[i];
		center /= (float)nPoints;
	}

	//	sample 4 directions

	b3Float4 aVector = p[0] - center;
	b3Float4 u = b3Cross(nearNormal, aVector);
	b3Float4 v = b3Cross(nearNormal, u);
	u = b3Normalized(u);
	v = b3Normalized(v);

	//keep point with deepest penetration
	float minW = FLT_MAX;

	i32 minIndex = -1;

	b3Float4 maxDots;
	maxDots.x = FLT_MIN;
	maxDots.y = FLT_MIN;
	maxDots.z = FLT_MIN;
	maxDots.w = FLT_MIN;

	//	idx, distance
	for (i32 ie = 0; ie < nPoints; ie++)
	{
		if (p[ie].w < minW)
		{
			minW = p[ie].w;
			minIndex = ie;
		}
		float f;
		b3Float4 r = p[ie] - center;
		f = b3Dot(u, r);
		if (f < maxDots.x)
		{
			maxDots.x = f;
			contactIdx[0].x = ie;
		}

		f = b3Dot(-u, r);
		if (f < maxDots.y)
		{
			maxDots.y = f;
			contactIdx[0].y = ie;
		}

		f = b3Dot(v, r);
		if (f < maxDots.z)
		{
			maxDots.z = f;
			contactIdx[0].z = ie;
		}

		f = b3Dot(-v, r);
		if (f < maxDots.w)
		{
			maxDots.w = f;
			contactIdx[0].w = ie;
		}
	}

	if (contactIdx[0].x != minIndex && contactIdx[0].y != minIndex && contactIdx[0].z != minIndex && contactIdx[0].w != minIndex)
	{
		//replace the first contact with minimum (todo: replace contact with least penetration)
		contactIdx[0].x = minIndex;
	}

	return 4;
}

__kernel void b3NewContactReductionKernel(__global b3Int4* pairs,
										  __global const b3RigidBodyData_t* rigidBodies,
										  __global const b3Float4* separatingNormals,
										  __global i32k* hasSeparatingAxis,
										  __global struct b3Contact4Data* globalContactsOut,
										  __global b3Int4* clippingFaces,
										  __global b3Float4* worldVertsB2,
										   __global i32* nGlobalContactsOut,
										  i32 vertexFaceCapacity,
										  i32 contactCapacity,
										  i32 numPairs,
										  i32 pairIndex)
{
	//    i32 i = get_global_id(0);
	//i32 pairIndex = i;
	i32 i = pairIndex;

	b3Int4 contactIdx;
	contactIdx = b3MakeInt4(0, 1, 2, 3);

	if (i < numPairs)
	{
		if (hasSeparatingAxis[i])
		{
			i32 nPoints = clippingFaces[pairIndex].w;

			if (nPoints > 0)
			{
				__global b3Float4* pointsIn = &worldVertsB2[pairIndex * vertexFaceCapacity];
				b3Float4 normal = -separatingNormals[i];

				i32 nReducedContacts = b3ExtractManifoldSequentialGlobal(pointsIn, nPoints, normal, &contactIdx);

				i32 dstIdx;
				dstIdx = b3AtomicInc(nGlobalContactsOut);

				//#if 0
				drx3DAssert(dstIdx < contactCapacity);
				if (dstIdx < contactCapacity)
				{
					__global struct b3Contact4Data* c = &globalContactsOut[dstIdx];
					c->m_worldNormalOnB = -normal;
					c->m_restituitionCoeffCmp = (0.f * 0xffff);
					c->m_frictionCoeffCmp = (0.7f * 0xffff);
					c->m_batchIdx = pairIndex;
					i32 bodyA = pairs[pairIndex].x;
					i32 bodyB = pairs[pairIndex].y;

					pairs[pairIndex].w = dstIdx;

					c->m_bodyAPtrAndSignBit = rigidBodies[bodyA].m_invMass == 0 ? -bodyA : bodyA;
					c->m_bodyBPtrAndSignBit = rigidBodies[bodyB].m_invMass == 0 ? -bodyB : bodyB;
					c->m_childIndexA = -1;
					c->m_childIndexB = -1;

					switch (nReducedContacts)
					{
						case 4:
							c->m_worldPosB[3] = pointsIn[contactIdx.w];
						case 3:
							c->m_worldPosB[2] = pointsIn[contactIdx.z];
						case 2:
							c->m_worldPosB[1] = pointsIn[contactIdx.y];
						case 1:
							c->m_worldPosB[0] = pointsIn[contactIdx.x];
						default:
						{
						}
					};

					GET_NPOINTS(*c) = nReducedContacts;
				}

				//#endif

			}  //		if (numContactsOut>0)
		}      //		if (hasSeparatingAxis[i])
	}          //	if (i<numPairs)
}
#endif
