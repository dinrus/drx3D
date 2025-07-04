
#ifndef D3_CONTACT_SPHERE_SPHERE_H
#define D3_CONTACT_SPHERE_SPHERE_H

void computeContactSphereConvex(i32 pairIndex,
								i32 bodyIndexA, i32 bodyIndexB,
								i32 collidableIndexA, i32 collidableIndexB,
								const b3RigidBodyData* rigidBodies,
								const b3Collidable* collidables,
								const b3ConvexPolyhedronData* convexShapes,
								const b3Vec3* convexVertices,
								i32k* convexIndices,
								const b3GpuFace* faces,
								b3Contact4* globalContactsOut,
								i32& nGlobalContactsOut,
								i32 maxContactCapacity)
{
	float radius = collidables[collidableIndexA].m_radius;
	float4 spherePos1 = rigidBodies[bodyIndexA].m_pos;
	b3Quat sphereOrn = rigidBodies[bodyIndexA].m_quat;

	float4 pos = rigidBodies[bodyIndexB].m_pos;

	b3Quat quat = rigidBodies[bodyIndexB].m_quat;

	b3Transform tr;
	tr.setIdentity();
	tr.setOrigin(pos);
	tr.setRotation(quat);
	b3Transform trInv = tr.inverse();

	float4 spherePos = trInv(spherePos1);

	i32 collidableIndex = rigidBodies[bodyIndexB].m_collidableIdx;
	i32 shapeIndex = collidables[collidableIndex].m_shapeIndex;
	i32 numFaces = convexShapes[shapeIndex].m_numFaces;
	float4 closestPnt = b3MakeVector3(0, 0, 0, 0);
	float4 hitNormalWorld = b3MakeVector3(0, 0, 0, 0);
	float minDist = -1000000.f;  // TODO: What is the largest/smallest float?
	bool bCollide = true;
	i32 region = -1;
	float4 localHitNormal;
	for (i32 f = 0; f < numFaces; f++)
	{
		b3GpuFace face = faces[convexShapes[shapeIndex].m_faceOffset + f];
		float4 planeEqn;
		float4 localPlaneNormal = b3MakeVector3(face.m_plane.x, face.m_plane.y, face.m_plane.z, 0.f);
		float4 n1 = localPlaneNormal;  //quatRotate(quat,localPlaneNormal);
		planeEqn = n1;
		planeEqn[3] = face.m_plane.w;

		float4 pntReturn;
		float dist = signedDistanceFromPointToPlane(spherePos, planeEqn, &pntReturn);

		if (dist > radius)
		{
			bCollide = false;
			break;
		}

		if (dist > 0)
		{
			//might hit an edge or vertex
			b3Vec3 out;

			bool isInPoly = IsPointInPolygon(spherePos,
											 &face,
											 &convexVertices[convexShapes[shapeIndex].m_vertexOffset],
											 convexIndices,
											 &out);
			if (isInPoly)
			{
				if (dist > minDist)
				{
					minDist = dist;
					closestPnt = pntReturn;
					localHitNormal = planeEqn;
					region = 1;
				}
			}
			else
			{
				b3Vec3 tmp = spherePos - out;
				b3Scalar l2 = tmp.length2();
				if (l2 < radius * radius)
				{
					dist = b3Sqrt(l2);
					if (dist > minDist)
					{
						minDist = dist;
						closestPnt = out;
						localHitNormal = tmp / dist;
						region = 2;
					}
				}
				else
				{
					bCollide = false;
					break;
				}
			}
		}
		else
		{
			if (dist > minDist)
			{
				minDist = dist;
				closestPnt = pntReturn;
				localHitNormal = planeEqn;
				region = 3;
			}
		}
	}
	static i32 numChecks = 0;
	numChecks++;

	if (bCollide && minDist > -10000)
	{
		float4 normalOnSurfaceB1 = tr.getBasis() * localHitNormal;  //-hitNormalWorld;
		float4 pOnB1 = tr(closestPnt);
		//printf("dist ,%f,",minDist);
		float actualDepth = minDist - radius;
		if (actualDepth < 0)
		{
			//printf("actualDepth = ,%f,", actualDepth);
			//printf("normalOnSurfaceB1 = ,%f,%f,%f,", normalOnSurfaceB1.x,normalOnSurfaceB1.y,normalOnSurfaceB1.z);
			//printf("region=,%d,\n", region);
			pOnB1[3] = actualDepth;

			i32 dstIdx;
			//    dstIdx = nGlobalContactsOut++;//AppendInc( nGlobalContactsOut, dstIdx );

			if (nGlobalContactsOut < maxContactCapacity)
			{
				dstIdx = nGlobalContactsOut;
				nGlobalContactsOut++;

				b3Contact4* c = &globalContactsOut[dstIdx];
				c->m_worldNormalOnB = normalOnSurfaceB1;
				c->setFrictionCoeff(0.7);
				c->setRestituitionCoeff(0.f);

				c->m_batchIdx = pairIndex;
				c->m_bodyAPtrAndSignBit = rigidBodies[bodyIndexA].m_invMass == 0 ? -bodyIndexA : bodyIndexA;
				c->m_bodyBPtrAndSignBit = rigidBodies[bodyIndexB].m_invMass == 0 ? -bodyIndexB : bodyIndexB;
				c->m_worldPosB[0] = pOnB1;
				i32 numPoints = 1;
				c->m_worldNormalOnB.w = (b3Scalar)numPoints;
			}  //if (dstIdx < numPairs)
		}
	}  //if (hasCollision)
}
#endif  //D3_CONTACT_SPHERE_SPHERE_H
