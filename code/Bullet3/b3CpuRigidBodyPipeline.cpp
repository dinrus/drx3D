#include <drx3D/Physics/Dynamics/b3CpuRigidBodyPipeline.h>

#include <drx3D/Physics/Dynamics/shared/b3IntegrateTransforms.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/BroadPhase/b3DynamicBvhBroadphase.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Config.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3CpuNarrowPhase.h>
#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Physics/Dynamics/shared/b3ContactConstraint4.h>
#include <drx3D/Physics/Dynamics/shared/b3Inertia.h>

struct b3CpuRigidBodyPipelineInternalData
{
	b3AlignedObjectArray<b3RigidBodyData> m_rigidBodies;
	b3AlignedObjectArray<b3Inertia> m_inertias;
	b3AlignedObjectArray<b3Aabb> m_aabbWorldSpace;

	b3DynamicBvhBroadphase* m_bp;
	b3CpuNarrowPhase* m_np;
	b3Config m_config;
};

b3CpuRigidBodyPipeline::b3CpuRigidBodyPipeline(class b3CpuNarrowPhase* narrowphase, struct b3DynamicBvhBroadphase* broadphaseDbvt, const b3Config& config)
{
	m_data = new b3CpuRigidBodyPipelineInternalData;
	m_data->m_np = narrowphase;
	m_data->m_bp = broadphaseDbvt;
	m_data->m_config = config;
}

b3CpuRigidBodyPipeline::~b3CpuRigidBodyPipeline()
{
	delete m_data;
}

void b3CpuRigidBodyPipeline::updateAabbWorldSpace()
{
	for (i32 i = 0; i < this->getNumBodies(); i++)
	{
		b3RigidBodyData* body = &m_data->m_rigidBodies[i];
		b3Float4 position = body->m_pos;
		b3Quat orientation = body->m_quat;

		i32 collidableIndex = body->m_collidableIdx;
		b3Collidable& collidable = m_data->m_np->getCollidableCpu(collidableIndex);
		i32 shapeIndex = collidable.m_shapeIndex;

		if (shapeIndex >= 0)
		{
			b3Aabb localAabb = m_data->m_np->getLocalSpaceAabb(shapeIndex);
			b3Aabb& worldAabb = m_data->m_aabbWorldSpace[i];
			float margin = 0.f;
			b3TransformAabb2(localAabb.m_minVec, localAabb.m_maxVec, margin, position, orientation, &worldAabb.m_minVec, &worldAabb.m_maxVec);
			m_data->m_bp->setAabb(i, worldAabb.m_minVec, worldAabb.m_maxVec, 0);
		}
	}
}

void b3CpuRigidBodyPipeline::computeOverlappingPairs()
{
	i32 numPairs = m_data->m_bp->getOverlappingPairCache()->getNumOverlappingPairs();
	m_data->m_bp->calculateOverlappingPairs();
	numPairs = m_data->m_bp->getOverlappingPairCache()->getNumOverlappingPairs();
	printf("numPairs=%d\n", numPairs);
}

void b3CpuRigidBodyPipeline::computeContactPoints()
{
	b3AlignedObjectArray<b3Int4>& pairs = m_data->m_bp->getOverlappingPairCache()->getOverlappingPairArray();

	m_data->m_np->computeContacts(pairs, m_data->m_aabbWorldSpace, m_data->m_rigidBodies);
}
void b3CpuRigidBodyPipeline::stepSimulation(float deltaTime)
{
	//update world space aabb's
	updateAabbWorldSpace();

	//compute overlapping pairs
	computeOverlappingPairs();

	//compute contacts
	computeContactPoints();

	//solve contacts

	//update transforms
	integrate(deltaTime);
}

static inline float b3CalcRelVel(const b3Vec3& l0, const b3Vec3& l1, const b3Vec3& a0, const b3Vec3& a1,
								 const b3Vec3& linVel0, const b3Vec3& angVel0, const b3Vec3& linVel1, const b3Vec3& angVel1)
{
	return b3Dot(l0, linVel0) + b3Dot(a0, angVel0) + b3Dot(l1, linVel1) + b3Dot(a1, angVel1);
}

static inline void b3SetLinearAndAngular(const b3Vec3& n, const b3Vec3& r0, const b3Vec3& r1,
										 b3Vec3& linear, b3Vec3& angular0, b3Vec3& angular1)
{
	linear = -n;
	angular0 = -b3Cross(r0, n);
	angular1 = b3Cross(r1, n);
}

static inline void b3SolveContact(b3ContactConstraint4& cs,
								  const b3Vec3& posA, b3Vec3& linVelA, b3Vec3& angVelA, float invMassA, const b3Matrix3x3& invInertiaA,
								  const b3Vec3& posB, b3Vec3& linVelB, b3Vec3& angVelB, float invMassB, const b3Matrix3x3& invInertiaB,
								  float maxRambdaDt[4], float minRambdaDt[4])
{
	b3Vec3 dLinVelA;
	dLinVelA.setZero();
	b3Vec3 dAngVelA;
	dAngVelA.setZero();
	b3Vec3 dLinVelB;
	dLinVelB.setZero();
	b3Vec3 dAngVelB;
	dAngVelB.setZero();

	for (i32 ic = 0; ic < 4; ic++)
	{
		//	dont necessary because this makes change to 0
		if (cs.m_jacCoeffInv[ic] == 0.f) continue;

		{
			b3Vec3 angular0, angular1, linear;
			b3Vec3 r0 = cs.m_worldPos[ic] - (b3Vec3&)posA;
			b3Vec3 r1 = cs.m_worldPos[ic] - (b3Vec3&)posB;
			b3SetLinearAndAngular((const b3Vec3&)-cs.m_linear, (const b3Vec3&)r0, (const b3Vec3&)r1, linear, angular0, angular1);

			float rambdaDt = b3CalcRelVel((const b3Vec3&)cs.m_linear, (const b3Vec3&)-cs.m_linear, angular0, angular1,
										  linVelA, angVelA, linVelB, angVelB) +
							 cs.m_b[ic];
			rambdaDt *= cs.m_jacCoeffInv[ic];

			{
				float prevSum = cs.m_appliedRambdaDt[ic];
				float updated = prevSum;
				updated += rambdaDt;
				updated = d3Max(updated, minRambdaDt[ic]);
				updated = d3Min(updated, maxRambdaDt[ic]);
				rambdaDt = updated - prevSum;
				cs.m_appliedRambdaDt[ic] = updated;
			}

			b3Vec3 linImp0 = invMassA * linear * rambdaDt;
			b3Vec3 linImp1 = invMassB * (-linear) * rambdaDt;
			b3Vec3 angImp0 = (invInertiaA * angular0) * rambdaDt;
			b3Vec3 angImp1 = (invInertiaB * angular1) * rambdaDt;
#ifdef _WIN32
			drx3DAssert(_finite(linImp0.getX()));
			drx3DAssert(_finite(linImp1.getX()));
#endif
			{
				linVelA += linImp0;
				angVelA += angImp0;
				linVelB += linImp1;
				angVelB += angImp1;
			}
		}
	}
}

static inline void b3SolveFriction(b3ContactConstraint4& cs,
								   const b3Vec3& posA, b3Vec3& linVelA, b3Vec3& angVelA, float invMassA, const b3Matrix3x3& invInertiaA,
								   const b3Vec3& posB, b3Vec3& linVelB, b3Vec3& angVelB, float invMassB, const b3Matrix3x3& invInertiaB,
								   float maxRambdaDt[4], float minRambdaDt[4])
{
	if (cs.m_fJacCoeffInv[0] == 0 && cs.m_fJacCoeffInv[0] == 0) return;
	const b3Vec3& center = (const b3Vec3&)cs.m_center;

	b3Vec3 n = -(const b3Vec3&)cs.m_linear;

	b3Vec3 tangent[2];

	b3PlaneSpace1(n, tangent[0], tangent[1]);

	b3Vec3 angular0, angular1, linear;
	b3Vec3 r0 = center - posA;
	b3Vec3 r1 = center - posB;
	for (i32 i = 0; i < 2; i++)
	{
		b3SetLinearAndAngular(tangent[i], r0, r1, linear, angular0, angular1);
		float rambdaDt = b3CalcRelVel(linear, -linear, angular0, angular1,
									  linVelA, angVelA, linVelB, angVelB);
		rambdaDt *= cs.m_fJacCoeffInv[i];

		{
			float prevSum = cs.m_fAppliedRambdaDt[i];
			float updated = prevSum;
			updated += rambdaDt;
			updated = d3Max(updated, minRambdaDt[i]);
			updated = d3Min(updated, maxRambdaDt[i]);
			rambdaDt = updated - prevSum;
			cs.m_fAppliedRambdaDt[i] = updated;
		}

		b3Vec3 linImp0 = invMassA * linear * rambdaDt;
		b3Vec3 linImp1 = invMassB * (-linear) * rambdaDt;
		b3Vec3 angImp0 = (invInertiaA * angular0) * rambdaDt;
		b3Vec3 angImp1 = (invInertiaB * angular1) * rambdaDt;
#ifdef _WIN32
		drx3DAssert(_finite(linImp0.getX()));
		drx3DAssert(_finite(linImp1.getX()));
#endif
		linVelA += linImp0;
		angVelA += angImp0;
		linVelB += linImp1;
		angVelB += angImp1;
	}

	{  //	angular damping for point constraint
		b3Vec3 ab = (posB - posA).normalized();
		b3Vec3 ac = (center - posA).normalized();
		if (b3Dot(ab, ac) > 0.95f || (invMassA == 0.f || invMassB == 0.f))
		{
			float angNA = b3Dot(n, angVelA);
			float angNB = b3Dot(n, angVelB);

			angVelA -= (angNA * 0.1f) * n;
			angVelB -= (angNB * 0.1f) * n;
		}
	}
}

struct b3SolveTask  // : public ThreadPool::Task
{
	b3SolveTask(b3AlignedObjectArray<b3RigidBodyData>& bodies,
				b3AlignedObjectArray<b3Inertia>& shapes,
				b3AlignedObjectArray<b3ContactConstraint4>& constraints,
				i32 start, i32 nConstraints,
				i32 maxNumBatches,
				b3AlignedObjectArray<i32>* wgUsedBodies, i32 curWgidx)
		: m_bodies(bodies), m_shapes(shapes), m_constraints(constraints), m_wgUsedBodies(wgUsedBodies), m_curWgidx(curWgidx), m_start(start), m_nConstraints(nConstraints), m_solveFriction(true), m_maxNumBatches(maxNumBatches)
	{
	}

	u16 getType() { return 0; }

	void run(i32 tIdx)
	{
		b3AlignedObjectArray<i32> usedBodies;
		//printf("run..............\n");

		for (i32 bb = 0; bb < m_maxNumBatches; bb++)
		{
			usedBodies.resize(0);
			for (i32 ic = m_nConstraints - 1; ic >= 0; ic--)
			//for(i32 ic=0; ic<m_nConstraints; ic++)
			{
				i32 i = m_start + ic;
				if (m_constraints[i].m_batchIdx != bb)
					continue;

				float frictionCoeff = b3GetFrictionCoeff(&m_constraints[i]);
				i32 aIdx = (i32)m_constraints[i].m_bodyA;
				i32 bIdx = (i32)m_constraints[i].m_bodyB;
				//i32 localBatch = m_constraints[i].m_batchIdx;
				b3RigidBodyData& bodyA = m_bodies[aIdx];
				b3RigidBodyData& bodyB = m_bodies[bIdx];

#if 0
				if ((bodyA.m_invMass) && (bodyB.m_invMass))
				{
				//	printf("aIdx=%d, bIdx=%d\n", aIdx,bIdx);
				}
				if (bIdx==10)
				{
					//printf("ic(b)=%d, localBatch=%d\n",ic,localBatch);
				}
#endif
				if (aIdx == 10)
				{
					//printf("ic(a)=%d, localBatch=%d\n",ic,localBatch);
				}
				if (usedBodies.size() < (aIdx + 1))
				{
					usedBodies.resize(aIdx + 1, 0);
				}

				if (usedBodies.size() < (bIdx + 1))
				{
					usedBodies.resize(bIdx + 1, 0);
				}

				if (bodyA.m_invMass)
				{
					drx3DAssert(usedBodies[aIdx] == 0);
					usedBodies[aIdx]++;
				}

				if (bodyB.m_invMass)
				{
					drx3DAssert(usedBodies[bIdx] == 0);
					usedBodies[bIdx]++;
				}

				if (!m_solveFriction)
				{
					float maxRambdaDt[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
					float minRambdaDt[4] = {0.f, 0.f, 0.f, 0.f};

					b3SolveContact(m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass, (const b3Matrix3x3&)m_shapes[aIdx].m_invInertiaWorld,
								   (b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass, (const b3Matrix3x3&)m_shapes[bIdx].m_invInertiaWorld,
								   maxRambdaDt, minRambdaDt);
				}
				else
				{
					float maxRambdaDt[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
					float minRambdaDt[4] = {0.f, 0.f, 0.f, 0.f};

					float sum = 0;
					for (i32 j = 0; j < 4; j++)
					{
						sum += m_constraints[i].m_appliedRambdaDt[j];
					}
					frictionCoeff = 0.7f;
					for (i32 j = 0; j < 4; j++)
					{
						maxRambdaDt[j] = frictionCoeff * sum;
						minRambdaDt[j] = -maxRambdaDt[j];
					}

					b3SolveFriction(m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass, (const b3Matrix3x3&)m_shapes[aIdx].m_invInertiaWorld,
									(b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass, (const b3Matrix3x3&)m_shapes[bIdx].m_invInertiaWorld,
									maxRambdaDt, minRambdaDt);
				}
			}

			if (m_wgUsedBodies)
			{
				if (m_wgUsedBodies[m_curWgidx].size() < usedBodies.size())
				{
					m_wgUsedBodies[m_curWgidx].resize(usedBodies.size());
				}
				for (i32 i = 0; i < usedBodies.size(); i++)
				{
					if (usedBodies[i])
					{
						//printf("cell %d uses body %d\n", m_curWgidx,i);
						m_wgUsedBodies[m_curWgidx][i] = 1;
					}
				}
			}
		}
	}

	b3AlignedObjectArray<b3RigidBodyData>& m_bodies;
	b3AlignedObjectArray<b3Inertia>& m_shapes;
	b3AlignedObjectArray<b3ContactConstraint4>& m_constraints;
	b3AlignedObjectArray<i32>* m_wgUsedBodies;
	i32 m_curWgidx;
	i32 m_start;
	i32 m_nConstraints;
	bool m_solveFriction;
	i32 m_maxNumBatches;
};

void b3CpuRigidBodyPipeline::solveContactConstraints()
{
	i32 m_nIterations = 4;

	b3AlignedObjectArray<b3ContactConstraint4> contactConstraints;
	//	const b3AlignedObjectArray<b3Contact4Data>& contacts = m_data->m_np->getContacts();
	i32 n = contactConstraints.size();
	//convert contacts...

	i32 maxNumBatches = 250;

	for (i32 iter = 0; iter < m_nIterations; iter++)
	{
		b3SolveTask task(m_data->m_rigidBodies, m_data->m_inertias, contactConstraints, 0, n, maxNumBatches, 0, 0);
		task.m_solveFriction = false;
		task.run(0);
	}

	for (i32 iter = 0; iter < m_nIterations; iter++)
	{
		b3SolveTask task(m_data->m_rigidBodies, m_data->m_inertias, contactConstraints, 0, n, maxNumBatches, 0, 0);
		task.m_solveFriction = true;
		task.run(0);
	}
}

void b3CpuRigidBodyPipeline::integrate(float deltaTime)
{
	float angDamping = 0.f;
	b3Vec3 gravityAcceleration = b3MakeVector3(0, -9, 0);

	//integrate transforms (external forces/gravity should be moved into constraint solver)
	for (i32 i = 0; i < m_data->m_rigidBodies.size(); i++)
	{
		b3IntegrateTransform(&m_data->m_rigidBodies[i], deltaTime, angDamping, gravityAcceleration);
	}
}

i32 b3CpuRigidBodyPipeline::registerPhysicsInstance(float mass, const float* position, const float* orientation, i32 collidableIndex, i32 userData)
{
	b3RigidBodyData body;
	i32 bodyIndex = m_data->m_rigidBodies.size();
	body.m_invMass = mass ? 1.f / mass : 0.f;
	body.m_angVel.setVal(0, 0, 0);
	body.m_collidableIdx = collidableIndex;
	body.m_frictionCoeff = 0.3f;
	body.m_linVel.setVal(0, 0, 0);
	body.m_pos.setVal(position[0], position[1], position[2]);
	body.m_quat.setVal(orientation[0], orientation[1], orientation[2], orientation[3]);
	body.m_restituitionCoeff = 0.f;

	m_data->m_rigidBodies.push_back(body);

	if (collidableIndex >= 0)
	{
		b3Aabb& worldAabb = m_data->m_aabbWorldSpace.expand();

		b3Aabb localAabb = m_data->m_np->getLocalSpaceAabb(collidableIndex);
		b3Vec3 localAabbMin = b3MakeVector3(localAabb.m_min[0], localAabb.m_min[1], localAabb.m_min[2]);
		b3Vec3 localAabbMax = b3MakeVector3(localAabb.m_max[0], localAabb.m_max[1], localAabb.m_max[2]);

		b3Scalar margin = 0.01f;
		b3Transform t;
		t.setIdentity();
		t.setOrigin(b3MakeVector3(position[0], position[1], position[2]));
		t.setRotation(b3Quat(orientation[0], orientation[1], orientation[2], orientation[3]));
		b3TransformAabb(localAabbMin, localAabbMax, margin, t, worldAabb.m_minVec, worldAabb.m_maxVec);

		m_data->m_bp->createProxy(worldAabb.m_minVec, worldAabb.m_maxVec, bodyIndex, 0, 1, 1);
		//		b3Vec3 aabbMin,aabbMax;
		//	m_data->m_bp->getAabb(bodyIndex,aabbMin,aabbMax);
	}
	else
	{
		drx3DError("registerPhysicsInstance using invalid collidableIndex\n");
	}

	return bodyIndex;
}

const struct b3RigidBodyData* b3CpuRigidBodyPipeline::getBodyBuffer() const
{
	return m_data->m_rigidBodies.size() ? &m_data->m_rigidBodies[0] : 0;
}

i32 b3CpuRigidBodyPipeline::getNumBodies() const
{
	return m_data->m_rigidBodies.size();
}
