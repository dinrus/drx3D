#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>

#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodyHelpers.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <iostream>
#include <fstream>

ReducedDeformableBody::ReducedDeformableBody(SoftBodyWorldInfo* worldInfo, i32 node_count, const Vec3* x, const Scalar* m)
 : SoftBody(worldInfo, node_count, x, m), m_rigidOnly(false)
{
  // reduced deformable
  m_reducedModel = true;
  m_nReduced = 0;
  m_nFull = 0;
  m_nodeIndexOffset = 0;

  m_transform_lock = false;
  m_ksScale = 1.0;
  m_rhoScale = 1.0;

  // rigid motion
  m_linearVelocity.setZero();
	m_angularVelocity.setZero();
  m_internalDeltaLinearVelocity.setZero();
  m_internalDeltaAngularVelocity.setZero();
  m_angularVelocityFromReduced.setZero();
  m_internalDeltaAngularVelocityFromReduced.setZero();
	m_angularFactor.setVal(1, 1, 1);
	m_linearFactor.setVal(1, 1, 1);
  // m_invInertiaLocal.setVal(1, 1, 1);
  m_invInertiaLocal.setIdentity();
  m_mass = 0.0;
  m_inverseMass = 0.0;

  m_linearDamping = 0;
  m_angularDamping = 0;

  // Rayleigh damping
  m_dampingAlpha = 0;
  m_dampingBeta = 0;

  m_rigidTransform2World.setIdentity();
}

void ReducedDeformableBody::setReducedModes(i32 num_modes, i32 full_size)
{
  m_nReduced = num_modes;
  m_nFull = full_size;
  m_reducedDofs.resize(m_nReduced, 0);
  m_reducedDofsBuffer.resize(m_nReduced, 0);
  m_reducedVelocity.resize(m_nReduced, 0);
  m_reducedVelocityBuffer.resize(m_nReduced, 0);
  m_reducedForceElastic.resize(m_nReduced, 0);
  m_reducedForceDamping.resize(m_nReduced, 0);
  m_reducedForceExternal.resize(m_nReduced, 0);
  m_internalDeltaReducedVelocity.resize(m_nReduced, 0);
  m_nodalMass.resize(full_size, 0);
  m_localMomentArm.resize(m_nFull);
}

void ReducedDeformableBody::setMassProps(const tDenseArray& mass_array)
{
  Scalar total_mass = 0;
  Vec3 CoM(0, 0, 0);
	for (i32 i = 0; i < m_nFull; ++i)
	{
		m_nodalMass[i] = m_rhoScale * mass_array[i];
		m_nodes[i].m_im = mass_array[i] > 0 ? 1.0 / (m_rhoScale * mass_array[i]) : 0;
		total_mass += m_rhoScale * mass_array[i];

    CoM += m_nodalMass[i] * m_nodes[i].m_x;
	}
  // total rigid body mass
  m_mass = total_mass;
  m_inverseMass = total_mass > 0 ? 1.0 / total_mass : 0;
  // original CoM
  m_initialCoM = CoM / total_mass;
}

void ReducedDeformableBody::setInertiaProps()
{
  // make sure the initial CoM is at the origin (0,0,0)
  // for (i32 i = 0; i < m_nFull; ++i)
  // {
  //   m_nodes[i].m_x -= m_initialCoM;
  // }
  // m_initialCoM.setZero();
  m_rigidTransform2World.setOrigin(m_initialCoM);
  m_interpolationWorldTransform = m_rigidTransform2World;
  
  updateLocalInertiaTensorFromNodes();

  // update world inertia tensor
  Matrix3x3 rotation;
  rotation.setIdentity();
  updateInitialInertiaTensor(rotation);
  updateInertiaTensor();
  m_interpolateInvInertiaTensorWorld = m_invInertiaTensorWorld;
}

void ReducedDeformableBody::setRigidVelocity(const Vec3& v)
{
  m_linearVelocity = v;
}

void ReducedDeformableBody::setRigidAngularVelocity(const Vec3& omega)
{
  m_angularVelocity = omega;
}

void ReducedDeformableBody::setStiffnessScale(const Scalar ks)
{
  m_ksScale = ks;
}

void ReducedDeformableBody::setMassScale(const Scalar rho)
{
  m_rhoScale = rho;
}

void ReducedDeformableBody::setFixedNodes(i32k n_node)
{
  m_fixedNodes.push_back(n_node);
  m_nodes[n_node].m_im = 0;   // set inverse mass to be zero for the constraint solver.
}

void ReducedDeformableBody::setDamping(const Scalar alpha, const Scalar beta)
{
  m_dampingAlpha = alpha;
  m_dampingBeta = beta;
}

void ReducedDeformableBody::internalInitialization()
{
  // zeroing
  endOfTimeStepZeroing();
  // initialize rest position
  updateRestNodalPositions();
  // initialize local nodal moment arm form the CoM
  updateLocalMomentArm();
  // initialize projection matrix
  updateExternalForceProjectMatrix(false);
}

void ReducedDeformableBody::updateLocalMomentArm()
{
  TVStack delta_x;
  delta_x.resize(m_nFull);

  for (i32 i = 0; i < m_nFull; ++i)
  {
    for (i32 k = 0; k < 3; ++k)
    {
      // compute displacement
      delta_x[i][k] = 0;
      for (i32 j = 0; j < m_nReduced; ++j) 
      {
        delta_x[i][k] += m_modes[j][3 * i + k] * m_reducedDofs[j];
      }
    }
    // get new moment arm Sq + x0
    m_localMomentArm[i] = m_x0[i] - m_initialCoM + delta_x[i];
  }
}

void ReducedDeformableBody::updateExternalForceProjectMatrix(bool initialized)
{
  // if not initialized, need to compute both P_A and Cq
  // otherwise, only need to udpate Cq
  if (!initialized)
  {
    // resize
    m_projPA.resize(m_nReduced);
    m_projCq.resize(m_nReduced);

    m_STP.resize(m_nReduced);
    m_MrInvSTP.resize(m_nReduced);

    // P_A
    for (i32 r = 0; r < m_nReduced; ++r)
    {
      m_projPA[r].resize(3 * m_nFull, 0);
      for (i32 i = 0; i < m_nFull; ++i)
      {
        Matrix3x3 mass_scaled_i = Diagonal(1) - Diagonal(m_nodalMass[i] / m_mass);
        Vec3 s_ri(m_modes[r][3 * i], m_modes[r][3 * i + 1], m_modes[r][3 * i + 2]);
        Vec3 prod_i = mass_scaled_i * s_ri;

        for (i32 k = 0; k < 3; ++k)
          m_projPA[r][3 * i + k] = prod_i[k];

        // Scalar ratio = m_nodalMass[i] / m_mass;
        // m_projPA[r] += Vec3(- m_modes[r][3 * i] * ratio,
        //                          - m_modes[r][3 * i + 1] * ratio,
        //                          - m_modes[r][3 * i + 2] * ratio);
      }
    }
  }

  // C(q) is updated once per position update
  for (i32 r = 0; r < m_nReduced; ++r)
  {
  	m_projCq[r].resize(3 * m_nFull, 0);
    for (i32 i = 0; i < m_nFull; ++i)
    {
      Matrix3x3 r_star = Cross(m_localMomentArm[i]);
      Vec3 s_ri(m_modes[r][3 * i], m_modes[r][3 * i + 1], m_modes[r][3 * i + 2]);
      Vec3 prod_i = r_star * m_invInertiaTensorWorld * r_star * s_ri;

      for (i32 k = 0; k < 3; ++k)
        m_projCq[r][3 * i + k] = m_nodalMass[i] * prod_i[k];

      // Vec3 si(m_modes[r][3 * i], m_modes[r][3 * i + 1], m_modes[r][3 * i + 2]);
      // m_projCq[r] += m_nodalMass[i] * si.cross(m_localMomentArm[i]);
    }
  }
}

void ReducedDeformableBody::endOfTimeStepZeroing()
{
  for (i32 i = 0; i < m_nReduced; ++i)
  {
    m_reducedForceElastic[i] = 0;
    m_reducedForceDamping[i] = 0;
    m_reducedForceExternal[i] = 0;
    m_internalDeltaReducedVelocity[i] = 0;
    m_reducedDofsBuffer[i] = m_reducedDofs[i];
    m_reducedVelocityBuffer[i] = m_reducedVelocity[i];
  }
  // std::cout << "zeroed!\n";
}

void ReducedDeformableBody::applyInternalVelocityChanges()
{
  m_linearVelocity += m_internalDeltaLinearVelocity;
  m_angularVelocity += m_internalDeltaAngularVelocity;
  m_internalDeltaLinearVelocity.setZero();
  m_internalDeltaAngularVelocity.setZero();
  for (i32 r = 0; r < m_nReduced; ++r)
  {
    m_reducedVelocity[r] += m_internalDeltaReducedVelocity[r];
    m_internalDeltaReducedVelocity[r] = 0;
  }
}

void ReducedDeformableBody::predictIntegratedTransform2(Scalar dt, Transform2& predictedTransform2)
{
	Transform2Util::integrateTransform(m_rigidTransform2World, m_linearVelocity, m_angularVelocity, dt, predictedTransform2);
}

void ReducedDeformableBody::updateReducedDofs(Scalar solverdt)
{
  for (i32 r = 0; r < m_nReduced; ++r)
  { 
    m_reducedDofs[r] = m_reducedDofsBuffer[r] + solverdt * m_reducedVelocity[r];
  }
}

void ReducedDeformableBody::mapToFullPosition(const Transform2& ref_trans)
{
  Vec3 origin = ref_trans.getOrigin();
  Matrix3x3 rotation = ref_trans.getBasis();
  

  for (i32 i = 0; i < m_nFull; ++i)
  {
    m_nodes[i].m_x = rotation * m_localMomentArm[i] + origin;
    m_nodes[i].m_q = m_nodes[i].m_x;
  }
}

void ReducedDeformableBody::updateReducedVelocity(Scalar solverdt)
{
  // update reduced velocity
  for (i32 r = 0; r < m_nReduced; ++r)
  {
    // the reduced mass is always identity!
    Scalar delta_v = 0;
    delta_v = solverdt * (m_reducedForceElastic[r] + m_reducedForceDamping[r]);
    // delta_v = solverdt * (m_reducedForceElastic[r] + m_reducedForceDamping[r] + m_reducedForceExternal[r]);
    m_reducedVelocity[r] = m_reducedVelocityBuffer[r] + delta_v;
  }
}

void ReducedDeformableBody::mapToFullVelocity(const Transform2& ref_trans)
{
  // compute the reduced contribution to the angular velocity
  // Vec3 sum_linear(0, 0, 0);
  // Vec3 sum_angular(0, 0, 0);
  // m_linearVelocityFromReduced.setZero();
  // m_angularVelocityFromReduced.setZero();
  // for (i32 i = 0; i < m_nFull; ++i)
  // {
  //   Vec3 r_com = ref_trans.getBasis() * m_localMomentArm[i];
  //   Matrix3x3 r_star = Cross(r_com);

  //   Vec3 v_from_reduced(0, 0, 0);
  //   for (i32 k = 0; k < 3; ++k)
  //   {
  //     for (i32 r = 0; r < m_nReduced; ++r)
  //     {
  //       v_from_reduced[k] += m_modes[r][3 * i + k] * m_reducedVelocity[r];
  //     }
  //   }

  //   Vec3 delta_linear = m_nodalMass[i] * v_from_reduced;
  //   Vec3 delta_angular = m_nodalMass[i] * (r_star * ref_trans.getBasis() * v_from_reduced);
  //   sum_linear += delta_linear;
  //   sum_angular += delta_angular;
  //   // std::cout << "delta_linear: " << delta_linear[0] << "\t" << delta_linear[1] << "\t" << delta_linear[2] << "\n";
  //   // std::cout << "delta_angular: " << delta_angular[0] << "\t" << delta_angular[1] << "\t" << delta_angular[2] << "\n";
  //   // std::cout << "sum_linear: " << sum_linear[0] << "\t" << sum_linear[1] << "\t" << sum_linear[2] << "\n";
  //   // std::cout << "sum_angular: " << sum_angular[0] << "\t" << sum_angular[1] << "\t" << sum_angular[2] << "\n";
  // }
  // m_linearVelocityFromReduced = 1.0 / m_mass * (ref_trans.getBasis() * sum_linear);
  // m_angularVelocityFromReduced = m_interpolateInvInertiaTensorWorld * sum_angular;

  // m_linearVelocity -= m_linearVelocityFromReduced;
  // m_angularVelocity -= m_angularVelocityFromReduced;

  for (i32 i = 0; i < m_nFull; ++i)
  {
    m_nodes[i].m_v = computeNodeFullVelocity(ref_trans, i);
  }
}

const Vec3 ReducedDeformableBody::computeTotalAngularMomentum() const
{
  Vec3 L_rigid = m_invInertiaTensorWorld.inverse() * m_angularVelocity;
  Vec3 L_reduced(0, 0, 0);
  Matrix3x3 omega_prime_star = Cross(m_angularVelocityFromReduced);

  for (i32 i = 0; i < m_nFull; ++i)
  {
    Vec3 r_com = m_rigidTransform2World.getBasis() * m_localMomentArm[i];
    Matrix3x3 r_star = Cross(r_com);

    Vec3 v_from_reduced(0, 0, 0);
    for (i32 k = 0; k < 3; ++k)
    {
      for (i32 r = 0; r < m_nReduced; ++r)
      {
        v_from_reduced[k] += m_modes[r][3 * i + k] * m_reducedVelocity[r];
      }
    }

    L_reduced += m_nodalMass[i] * (r_star * (m_rigidTransform2World.getBasis() * v_from_reduced - omega_prime_star * r_com));
    // L_reduced += m_nodalMass[i] * (r_star * (m_rigidTransform2World.getBasis() * v_from_reduced));
  }
  return L_rigid + L_reduced;
}

const Vec3 ReducedDeformableBody::computeNodeFullVelocity(const Transform2& ref_trans, i32 n_node) const
{
  Vec3 v_from_reduced(0, 0, 0);
  Vec3 r_com = ref_trans.getBasis() * m_localMomentArm[n_node];
  // compute velocity contributed by the reduced velocity
  for (i32 k = 0; k < 3; ++k)
  {
    for (i32 r = 0; r < m_nReduced; ++r)
    {
      v_from_reduced[k] += m_modes[r][3 * n_node + k] * m_reducedVelocity[r];
    }
  }
  // get new velocity
  Vec3 vel = m_angularVelocity.cross(r_com) + 
                  ref_trans.getBasis() * v_from_reduced +
                  m_linearVelocity;
  return vel;
}

const Vec3 ReducedDeformableBody::internalComputeNodeDeltaVelocity(const Transform2& ref_trans, i32 n_node) const
{
  Vec3 deltaV_from_reduced(0, 0, 0);
  Vec3 r_com = ref_trans.getBasis() * m_localMomentArm[n_node];

  // compute velocity contributed by the reduced velocity
  for (i32 k = 0; k < 3; ++k)
  {
    for (i32 r = 0; r < m_nReduced; ++r)
    {
      deltaV_from_reduced[k] += m_modes[r][3 * n_node + k] * m_internalDeltaReducedVelocity[r];
    }
  }

  // get delta velocity
  Vec3 deltaV = m_internalDeltaAngularVelocity.cross(r_com) + 
                     ref_trans.getBasis() * deltaV_from_reduced +
                     m_internalDeltaLinearVelocity;
  return deltaV;
}

void ReducedDeformableBody::proceedToTransform2(Scalar dt, bool end_of_time_step)
{
  Transform2Util::integrateTransform(m_rigidTransform2World, m_linearVelocity, m_angularVelocity, dt, m_interpolationWorldTransform);
  updateInertiaTensor();
  // m_interpolateInvInertiaTensorWorld = m_interpolationWorldTransform.getBasis().scaled(m_invInertiaLocal) * m_interpolationWorldTransform.getBasis().transpose();
  m_rigidTransform2World = m_interpolationWorldTransform;
  m_invInertiaTensorWorld = m_interpolateInvInertiaTensorWorld;
}

void ReducedDeformableBody::transformTo(const Transform2& trs)
{
	Transform2 current_transform = getRigidTransform();
	Transform2 new_transform(trs.getBasis() * current_transform.getBasis().transpose(),
                            trs.getOrigin() - current_transform.getOrigin());
  transform(new_transform);
}

void ReducedDeformableBody::transform(const Transform2& trs)
{
  m_transform_lock = true;

  // transform mesh
  {
    const Scalar margin = getCollisionShape()->getMargin();
    ATTRIBUTE_ALIGNED16(DbvtVolume)
    vol;

    Vec3 CoM = m_rigidTransform2World.getOrigin();
    Vec3 translation = trs.getOrigin();
    Matrix3x3 rotation = trs.getBasis();

    for (i32 i = 0; i < m_nodes.size(); ++i)
    {
      Node& n = m_nodes[i];
      n.m_x = rotation * (n.m_x - CoM) + CoM + translation;
      n.m_q = rotation * (n.m_q - CoM) + CoM + translation;
      n.m_n = rotation * n.m_n;
      vol = DbvtVolume::FromCR(n.m_x, margin);

      m_ndbvt.update(n.m_leaf, vol);
    }
    updateNormals();
    updateBounds();
    updateConstants();
  }

  // update modes
  updateModesByRotation(trs.getBasis());

  // update inertia tensor
  updateInitialInertiaTensor(trs.getBasis());
  updateInertiaTensor();
  m_interpolateInvInertiaTensorWorld = m_invInertiaTensorWorld;
  
  // update rigid frame (No need to update the rotation. Nodes have already been updated.)
  m_rigidTransform2World.setOrigin(m_initialCoM + trs.getOrigin());
  m_interpolationWorldTransform = m_rigidTransform2World;
  m_initialCoM = m_rigidTransform2World.getOrigin();

  internalInitialization();
}

void ReducedDeformableBody::scale(const Vec3& scl)
{
  // Scaling the mesh after transform is applied is not allowed
  Assert(!m_transform_lock);

  // scale the mesh
  {
    const Scalar margin = getCollisionShape()->getMargin();
    ATTRIBUTE_ALIGNED16(DbvtVolume)
    vol;

    Vec3 CoM = m_rigidTransform2World.getOrigin();

    for (i32 i = 0; i < m_nodes.size(); ++i)
    {
      Node& n = m_nodes[i];
      n.m_x = (n.m_x - CoM) * scl + CoM;
      n.m_q = (n.m_q - CoM) * scl + CoM;
      vol = DbvtVolume::FromCR(n.m_x, margin);
      m_ndbvt.update(n.m_leaf, vol);
    }
    updateNormals();
    updateBounds();
    updateConstants();
    initializeDmInverse();
  }

  // update inertia tensor
  updateLocalInertiaTensorFromNodes();

  Matrix3x3 id;
  id.setIdentity();
  updateInitialInertiaTensor(id);   // there is no rotation, but the local inertia tensor has changed
  updateInertiaTensor();
  m_interpolateInvInertiaTensorWorld = m_invInertiaTensorWorld;

  internalInitialization();
}

void ReducedDeformableBody::setTotalMass(Scalar mass, bool fromfaces)
{
  // Changing the total mass after transform is applied is not allowed
  Assert(!m_transform_lock);

  Scalar scale_ratio = mass / m_mass;

  // update nodal mass
  for (i32 i = 0; i < m_nFull; ++i)
  {
    m_nodalMass[i] *= scale_ratio;
  }
  m_mass = mass;
  m_inverseMass = mass > 0 ? 1.0 / mass : 0;

  // update inertia tensors
  updateLocalInertiaTensorFromNodes();

  Matrix3x3 id;
  id.setIdentity();
  updateInitialInertiaTensor(id);   // there is no rotation, but the local inertia tensor has changed
  updateInertiaTensor();
  m_interpolateInvInertiaTensorWorld = m_invInertiaTensorWorld;

  internalInitialization();
}

void ReducedDeformableBody::updateRestNodalPositions()
{
  // update reset nodal position
  m_x0.resize(m_nFull);
  for (i32 i = 0; i < m_nFull; ++i)
  {
    m_x0[i] = m_nodes[i].m_x;
  }
}

// reference notes:
// https://ocw.mit.edu/courses/aeronautics-and-astronautics/16-07-dynamics-fall-2009/lecture-notes/MIT16_07F09_Lec26.pdf
void ReducedDeformableBody::updateLocalInertiaTensorFromNodes()
{
  Matrix3x3 inertia_tensor;
  inertia_tensor.setZero();

  for (i32 p = 0; p < m_nFull; ++p)
  {
    Matrix3x3 particle_inertia;
    particle_inertia.setZero();

    Vec3 r = m_nodes[p].m_x - m_initialCoM;

    particle_inertia[0][0] = m_nodalMass[p] * (r[1] * r[1] + r[2] * r[2]);
    particle_inertia[1][1] = m_nodalMass[p] * (r[0] * r[0] + r[2] * r[2]);
    particle_inertia[2][2] = m_nodalMass[p] * (r[0] * r[0] + r[1] * r[1]);

    particle_inertia[0][1] = - m_nodalMass[p] * (r[0] * r[1]);
    particle_inertia[0][2] = - m_nodalMass[p] * (r[0] * r[2]);
    particle_inertia[1][2] = - m_nodalMass[p] * (r[1] * r[2]);

    particle_inertia[1][0] = particle_inertia[0][1];
    particle_inertia[2][0] = particle_inertia[0][2];
    particle_inertia[2][1] = particle_inertia[1][2];

    inertia_tensor += particle_inertia;
  }
  m_invInertiaLocal = inertia_tensor.inverse();
}

void ReducedDeformableBody::updateInitialInertiaTensor(const Matrix3x3& rotation)
{
  // m_invInertiaTensorWorldInitial = rotation.scaled(m_invInertiaLocal) * rotation.transpose();
  m_invInertiaTensorWorldInitial = rotation * m_invInertiaLocal * rotation.transpose();
}

void ReducedDeformableBody::updateModesByRotation(const Matrix3x3& rotation)
{
  for (i32 r = 0; r < m_nReduced; ++r)
  {
    for (i32 i = 0; i < m_nFull; ++i)
    {
      Vec3 nodal_disp(m_modes[r][3 * i], m_modes[r][3 * i + 1], m_modes[r][3 * i + 2]);
      nodal_disp = rotation * nodal_disp;

      for (i32 k = 0; k < 3; ++k)
      {
        m_modes[r][3 * i + k] = nodal_disp[k];
      }
    }
  }
}

void ReducedDeformableBody::updateInertiaTensor()
{
	m_invInertiaTensorWorld = m_rigidTransform2World.getBasis() * m_invInertiaTensorWorldInitial * m_rigidTransform2World.getBasis().transpose();
}

void ReducedDeformableBody::applyDamping(Scalar timeStep)
{
  m_linearVelocity *= Scalar(1) - m_linearDamping;
  m_angularDamping *= Scalar(1) - m_angularDamping;
}

void ReducedDeformableBody::applyCentralImpulse(const Vec3& impulse)
{
  m_linearVelocity += impulse * m_linearFactor * m_inverseMass;
  #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
  clampVelocity(m_linearVelocity);
  #endif
}

void ReducedDeformableBody::applyTorqueImpulse(const Vec3& torque)
{
  m_angularVelocity += m_interpolateInvInertiaTensorWorld * torque * m_angularFactor;
  #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
  clampVelocity(m_angularVelocity);
  #endif
}

void ReducedDeformableBody::internalApplyRigidImpulse(const Vec3& impulse, const Vec3& rel_pos)
{
  if (m_inverseMass == Scalar(0.))
  {
    std::cout << "something went wrong...probably didn't initialize?\n";
    Assert(false);
  }
  // delta linear velocity
  m_internalDeltaLinearVelocity += impulse * m_linearFactor * m_inverseMass;
  // delta angular velocity
  Vec3 torque = rel_pos.cross(impulse * m_linearFactor);
  m_internalDeltaAngularVelocity += m_interpolateInvInertiaTensorWorld * torque * m_angularFactor;
}

Vec3 ReducedDeformableBody::getRelativePos(i32 n_node)
{
  Matrix3x3 rotation = m_interpolationWorldTransform.getBasis();
  Vec3 ri = rotation * m_localMomentArm[n_node];
  return ri;
}

Matrix3x3 ReducedDeformableBody::getImpulseFactor(i32 n_node)
{
  // relative position
  Matrix3x3 rotation = m_interpolationWorldTransform.getBasis();
  Vec3 ri = rotation * m_localMomentArm[n_node];
  Matrix3x3 ri_skew = Cross(ri);

  // calculate impulse factor
  // rigid part
  Scalar inv_mass = m_nodalMass[n_node] > Scalar(0) ? Scalar(1) / m_mass : Scalar(0);
  Matrix3x3 K1 = Diagonal(inv_mass);
  K1 -= ri_skew * m_interpolateInvInertiaTensorWorld * ri_skew;

  // reduced deformable part
  Matrix3x3 SA;
  SA.setZero();
  for (i32 i = 0; i < 3; ++i)
  {
    for (i32 j = 0; j < 3; ++j)
    {
      for (i32 r = 0; r < m_nReduced; ++r)
      {
        SA[i][j] += m_modes[r][3 * n_node + i] * (m_projPA[r][3 * n_node + j] + m_projCq[r][3 * n_node + j]);
      }
    }
  }
  Matrix3x3 RSARinv = rotation * SA * rotation.transpose();


  TVStack omega_helper; // Sum_i m_i r*_i R S_i
  omega_helper.resize(m_nReduced);
  for (i32 r = 0; r < m_nReduced; ++r)
  {
    omega_helper[r].setZero();
    for (i32 i = 0; i < m_nFull; ++i)
    {
      Matrix3x3 mi_rstar_i = rotation * Cross(m_localMomentArm[i]) * m_nodalMass[i];
      Vec3 s_ri(m_modes[r][3 * i], m_modes[r][3 * i + 1], m_modes[r][3 * i + 2]);
      omega_helper[r] += mi_rstar_i * rotation * s_ri;
    }
  }

  Matrix3x3 sum_multiply_A;
  sum_multiply_A.setZero();
  for (i32 i = 0; i < 3; ++i)
  {
    for (i32 j = 0; j < 3; ++j)
    {
      for (i32 r = 0; r < m_nReduced; ++r)
      {
        sum_multiply_A[i][j] += omega_helper[r][i] * (m_projPA[r][3 * n_node + j] + m_projCq[r][3 * n_node + j]);
      }
    }
  }

  Matrix3x3 K2 = RSARinv + ri_skew * m_interpolateInvInertiaTensorWorld * sum_multiply_A * rotation.transpose();

  return m_rigidOnly ? K1 : K1 + K2;
}

void ReducedDeformableBody::internalApplyFullSpaceImpulse(const Vec3& impulse, const Vec3& rel_pos, i32 n_node, Scalar dt)
{
  if (!m_rigidOnly)
  {
    // apply impulse force
    applyFullSpaceNodalForce(impulse / dt, n_node);

    // update delta damping force
    tDenseArray reduced_vel_tmp;
    reduced_vel_tmp.resize(m_nReduced);
    for (i32 r = 0; r < m_nReduced; ++r)
    {
      reduced_vel_tmp[r] = m_reducedVelocity[r] + m_internalDeltaReducedVelocity[r];
    }
    applyReducedDampingForce(reduced_vel_tmp);
    // applyReducedDampingForce(m_internalDeltaReducedVelocity);

    // delta reduced velocity
    for (i32 r = 0; r < m_nReduced; ++r)
    {
      // The reduced mass is always identity!
      m_internalDeltaReducedVelocity[r] += dt * (m_reducedForceDamping[r] + m_reducedForceExternal[r]);
    }
  }

  internalApplyRigidImpulse(impulse, rel_pos);
}

void ReducedDeformableBody::applyFullSpaceNodalForce(const Vec3& f_ext, i32 n_node)
{
  // f_local = R^-1 * f_ext //TODO: interpoalted transfrom
  // Vec3 f_local = m_rigidTransform2World.getBasis().transpose() * f_ext;
  Vec3 f_local = m_interpolationWorldTransform.getBasis().transpose() * f_ext;

  // f_ext_r = [S^T * P]_{n_node} * f_local
  tDenseArray f_ext_r;
  f_ext_r.resize(m_nReduced, 0);
  for (i32 r = 0; r < m_nReduced; ++r)
  {
    m_reducedForceExternal[r] = 0;
    for (i32 k = 0; k < 3; ++k)
    {
      f_ext_r[r] += (m_projPA[r][3 * n_node + k] + m_projCq[r][3 * n_node + k]) * f_local[k];
    }

    m_reducedForceExternal[r] += f_ext_r[r];
  }
}

void ReducedDeformableBody::applyRigidGravity(const Vec3& gravity, Scalar dt)
{
  // update rigid frame velocity
  m_linearVelocity += dt * gravity;
}

void ReducedDeformableBody::applyReducedElasticForce(const tDenseArray& reduce_dofs)
{
  for (i32 r = 0; r < m_nReduced; ++r) 
  {
    m_reducedForceElastic[r] = - m_ksScale * m_Kr[r] * reduce_dofs[r];
  }
}

void ReducedDeformableBody::applyReducedDampingForce(const tDenseArray& reduce_vel)
{
  for (i32 r = 0; r < m_nReduced; ++r) 
  {
    m_reducedForceDamping[r] = - m_dampingBeta * m_ksScale * m_Kr[r] * reduce_vel[r];
  }
}

Scalar ReducedDeformableBody::getTotalMass() const
{
  return m_mass;
}

Transform2& ReducedDeformableBody::getRigidTransform()
{
  return m_rigidTransform2World;
}

const Vec3& ReducedDeformableBody::getLinearVelocity() const
{
  return m_linearVelocity;
}

const Vec3& ReducedDeformableBody::getAngularVelocity() const
{
  return m_angularVelocity;
}

void ReducedDeformableBody::disableReducedModes(const bool rigid_only)
{
  m_rigidOnly = rigid_only;
}

bool ReducedDeformableBody::isReducedModesOFF() const
{
  return m_rigidOnly;
}