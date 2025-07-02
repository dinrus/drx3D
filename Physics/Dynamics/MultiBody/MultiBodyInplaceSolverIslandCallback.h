#ifndef DRX3D_MULTIBODY_INPLACE_SOLVER_ISLAND_CALLBACK_H
#define DRX3D_MULTIBODY_INPLACE_SOLVER_ISLAND_CALLBACK_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>

SIMD_FORCE_INLINE i32 GetConstraintIslandId2(const TypedConstraint* lhs)
{
    i32 islandId;
    
    const CollisionObject2& rcolObj0 = lhs->getRigidBodyA();
    const CollisionObject2& rcolObj1 = lhs->getRigidBodyB();
    islandId = rcolObj0.getIslandTag() >= 0 ? rcolObj0.getIslandTag() : rcolObj1.getIslandTag();
    return islandId;
}
class SortConstraintOnIslandPredicate2
{
public:
    bool operator()(const TypedConstraint* lhs, const TypedConstraint* rhs) const
    {
        i32 rIslandId0, lIslandId0;
        rIslandId0 = GetConstraintIslandId2(rhs);
        lIslandId0 = GetConstraintIslandId2(lhs);
        return lIslandId0 < rIslandId0;
    }
};

SIMD_FORCE_INLINE i32 GetMultiBodyConstraintIslandId(const MultiBodyConstraint* lhs)
{
    i32 islandId;
    
    i32 islandTagA = lhs->getIslandIdA();
    i32 islandTagB = lhs->getIslandIdB();
    islandId = islandTagA >= 0 ? islandTagA : islandTagB;
    return islandId;
}

class SortMultiBodyConstraintOnIslandPredicate
{
public:
    bool operator()(const MultiBodyConstraint* lhs, const MultiBodyConstraint* rhs) const
    {
        i32 rIslandId0, lIslandId0;
        rIslandId0 = GetMultiBodyConstraintIslandId(rhs);
        lIslandId0 = GetMultiBodyConstraintIslandId(lhs);
        return lIslandId0 < rIslandId0;
    }
};

struct MultiBodyInplaceSolverIslandCallback : public SimulationIslandManager::IslandCallback
{

    ContactSolverInfo* m_solverInfo;
    MultiBodyConstraintSolver* m_solver;
    MultiBodyConstraint** m_multiBodySortedConstraints;
    i32 m_numMultiBodyConstraints;
    
    TypedConstraint** m_sortedConstraints;
    i32 m_numConstraints;
    IDebugDraw* m_debugDrawer;
    Dispatcher* m_dispatcher;
    
    AlignedObjectArray<CollisionObject2*> m_bodies;
	AlignedObjectArray<CollisionObject2*> m_softBodies;
    AlignedObjectArray<PersistentManifold*> m_manifolds;
    AlignedObjectArray<TypedConstraint*> m_constraints;
    AlignedObjectArray<MultiBodyConstraint*> m_multiBodyConstraints;
    
    AlignedObjectArray<SolverAnalyticsData> m_islandAnalyticsData;
    
    MultiBodyInplaceSolverIslandCallback(MultiBodyConstraintSolver* solver,
                                         Dispatcher* dispatcher)
    : m_solverInfo(nullptr),
    m_solver(solver),
    m_multiBodySortedConstraints(nullptr),
    m_numConstraints(0),
    m_debugDrawer(nullptr),
    m_dispatcher(dispatcher)
    {
    }
    
    MultiBodyInplaceSolverIslandCallback& operator=(const MultiBodyInplaceSolverIslandCallback& other)
    {
        Assert(0);
        (void)other;
        return *this;
    }
    
    SIMD_FORCE_INLINE virtual void setup(ContactSolverInfo* solverInfo, TypedConstraint** sortedConstraints, i32 numConstraints, MultiBodyConstraint** sortedMultiBodyConstraints, i32 numMultiBodyConstraints, IDebugDraw* debugDrawer)
    {
        m_islandAnalyticsData.clear();
        Assert(solverInfo);
        m_solverInfo = solverInfo;
        
        m_multiBodySortedConstraints = sortedMultiBodyConstraints;
        m_numMultiBodyConstraints = numMultiBodyConstraints;
        m_sortedConstraints = sortedConstraints;
        m_numConstraints = numConstraints;
        
        m_debugDrawer = debugDrawer;
        m_bodies.resize(0);
        m_manifolds.resize(0);
        m_constraints.resize(0);
        m_multiBodyConstraints.resize(0);
    }
    
    void setMultiBodyConstraintSolver(MultiBodyConstraintSolver* solver)
    {
        m_solver = solver;
    }
    
    virtual void processIsland(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifolds, i32 numManifolds, i32 islandId)
    {
        if (islandId < 0)
        {
            ///we don't split islands, so all constraints/contact manifolds/bodies are passed into the solver regardless the island id
            m_solver->solveMultiBodyGroup(bodies, numBodies, manifolds, numManifolds, m_sortedConstraints, m_numConstraints, &m_multiBodySortedConstraints[0], m_numConstraints, *m_solverInfo, m_debugDrawer, m_dispatcher);
            if (m_solverInfo->m_reportSolverAnalytics&1)
            {
                m_solver->m_analyticsData.m_islandId = islandId;
                m_islandAnalyticsData.push_back(m_solver->m_analyticsData);
            }
        }
        else
        {
            //also add all non-contact constraints/joints for this island
            TypedConstraint** startConstraint = 0;
            MultiBodyConstraint** startMultiBodyConstraint = 0;
            
            i32 numCurConstraints = 0;
            i32 numCurMultiBodyConstraints = 0;
            
            i32 i;
            
            //find the first constraint for this island
            
            for (i = 0; i < m_numConstraints; i++)
            {
                if (GetConstraintIslandId2(m_sortedConstraints[i]) == islandId)
                {
                    startConstraint = &m_sortedConstraints[i];
                    break;
                }
            }
            //count the number of constraints in this island
            for (; i < m_numConstraints; i++)
            {
                if (GetConstraintIslandId2(m_sortedConstraints[i]) == islandId)
                {
                    numCurConstraints++;
                }
            }
            
            for (i = 0; i < m_numMultiBodyConstraints; i++)
            {
                if (GetMultiBodyConstraintIslandId(m_multiBodySortedConstraints[i]) == islandId)
                {
                    startMultiBodyConstraint = &m_multiBodySortedConstraints[i];
                    break;
                }
            }
            //count the number of multi body constraints in this island
            for (; i < m_numMultiBodyConstraints; i++)
            {
                if (GetMultiBodyConstraintIslandId(m_multiBodySortedConstraints[i]) == islandId)
                {
                    numCurMultiBodyConstraints++;
                }
            }
            
            //if (m_solverInfo->m_minimumSolverBatchSize<=1)
            //{
            //    m_solver->solveGroup( bodies,numBodies,manifolds, numManifolds,startConstraint,numCurConstraints,*m_solverInfo,m_debugDrawer,m_dispatcher);
            //} else
            {
                for (i = 0; i < numBodies; i++)
				{
					bool isSoftBodyType = (bodies[i]->getInternalType() & CollisionObject2::CO_SOFT_BODY);
					if (!isSoftBodyType)
					{
						m_bodies.push_back(bodies[i]);
					}
					else
					{
						m_softBodies.push_back(bodies[i]);
					}
				}
                for (i = 0; i < numManifolds; i++)
                    m_manifolds.push_back(manifolds[i]);
                for (i = 0; i < numCurConstraints; i++)
                    m_constraints.push_back(startConstraint[i]);
                
                for (i = 0; i < numCurMultiBodyConstraints; i++)
                    m_multiBodyConstraints.push_back(startMultiBodyConstraint[i]);
                
                if ((m_multiBodyConstraints.size() + m_constraints.size() + m_manifolds.size()) > m_solverInfo->m_minimumSolverBatchSize)
                {
                    processConstraints(islandId);
                }
                else
                {
                    //printf("deferred\n");
                }
            }
        }
    }
    
    virtual void processConstraints(i32 islandId=-1)
    {
        CollisionObject2** bodies = m_bodies.size() ? &m_bodies[0] : 0;
        PersistentManifold** manifold = m_manifolds.size() ? &m_manifolds[0] : 0;
        TypedConstraint** constraints = m_constraints.size() ? &m_constraints[0] : 0;
        MultiBodyConstraint** multiBodyConstraints = m_multiBodyConstraints.size() ? &m_multiBodyConstraints[0] : 0;
        
        //printf("mb contacts = %d, mb constraints = %d\n", mbContacts, m_multiBodyConstraints.size());
        
        m_solver->solveMultiBodyGroup(bodies, m_bodies.size(), manifold, m_manifolds.size(), constraints, m_constraints.size(), multiBodyConstraints, m_multiBodyConstraints.size(), *m_solverInfo, m_debugDrawer, m_dispatcher);
        if (m_bodies.size() && (m_solverInfo->m_reportSolverAnalytics&1))
        {
            m_solver->m_analyticsData.m_islandId = islandId;
            m_islandAnalyticsData.push_back(m_solver->m_analyticsData);
        }
        m_bodies.resize(0);
		m_softBodies.resize(0);
        m_manifolds.resize(0);
        m_constraints.resize(0);
        m_multiBodyConstraints.resize(0);
    }
};


#endif /*DRX3D_MULTIBODY_INPLACE_SOLVER_ISLAND_CALLBACK_H */
