#ifndef DeformableBodyInplaceSolverIslandCallback_h
#define DeformableBodyInplaceSolverIslandCallback_h

struct DeformableBodyInplaceSolverIslandCallback : public MultiBodyInplaceSolverIslandCallback
{
	DeformableMultiBodyConstraintSolver* m_deformableSolver;

	DeformableBodyInplaceSolverIslandCallback(DeformableMultiBodyConstraintSolver* solver,
											  Dispatcher* dispatcher)
		: MultiBodyInplaceSolverIslandCallback(solver, dispatcher), m_deformableSolver(solver)
	{
	}

	virtual void processConstraints(i32 islandId = -1)
	{
		CollisionObject2** bodies = m_bodies.size() ? &m_bodies[0] : 0;
		CollisionObject2** softBodies = m_softBodies.size() ? &m_softBodies[0] : 0;
		PersistentManifold** manifold = m_manifolds.size() ? &m_manifolds[0] : 0;
		TypedConstraint** constraints = m_constraints.size() ? &m_constraints[0] : 0;
		MultiBodyConstraint** multiBodyConstraints = m_multiBodyConstraints.size() ? &m_multiBodyConstraints[0] : 0;

		//printf("mb contacts = %d, mb constraints = %d\n", mbContacts, m_multiBodyConstraints.size());

		m_deformableSolver->solveDeformableBodyGroup(bodies, m_bodies.size(), softBodies, m_softBodies.size(), manifold, m_manifolds.size(), constraints, m_constraints.size(), multiBodyConstraints, m_multiBodyConstraints.size(), *m_solverInfo, m_debugDrawer, m_dispatcher);
		if (m_bodies.size() && (m_solverInfo->m_reportSolverAnalytics & 1))
		{
			m_deformableSolver->m_analyticsData.m_islandId = islandId;
			m_islandAnalyticsData.push_back(m_solver->m_analyticsData);
		}
		m_bodies.resize(0);
		m_softBodies.resize(0);
		m_manifolds.resize(0);
		m_constraints.resize(0);
		m_multiBodyConstraints.resize(0);
	}
};

#endif /* DeformableBodyInplaceSolverIslandCallback_h */
