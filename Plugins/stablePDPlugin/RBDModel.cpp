#include "RBDModel.h"
#include "RBDUtil.h"
#include "KinTree.h"
#include <drx3D/Maths/Linear/Quickprof.h>
cRBDModel::cRBDModel()
{
}

cRBDModel::~cRBDModel()
{
}

void cRBDModel::Init(const Eigen::MatrixXd& joint_mat, const Eigen::MatrixXd& body_defs, const tVector& gravity)
{
	assert(joint_mat.rows() == body_defs.rows());
	mGravity = gravity;
	mJointMat = joint_mat;
	mBodyDefs = body_defs;

	i32 num_dofs = GetNumDof();
	i32 num_joints = GetNumJoints();
	i32k svs = gSpVecSize;

	mPose = Eigen::VectorXd::Zero(num_dofs);
	mVel = Eigen::VectorXd::Zero(num_dofs);

	tMatrix trans_mat;
	InitJointSubspaceArr();
	mChildParentMatArr = Eigen::MatrixXd::Zero(num_joints * trans_mat.rows(), trans_mat.cols());
	mSpWorldJointTransArr = Eigen::MatrixXd::Zero(num_joints * gSVTransRows, gSVTransCols);
	mMassMat = Eigen::MatrixXd::Zero(num_dofs, num_dofs);
	mBiasForce = Eigen::VectorXd::Zero(num_dofs);
	mInertiaBuffer = Eigen::MatrixXd::Zero(num_joints * svs, svs);
}

void cRBDModel::Update(const Eigen::VectorXd& pose, const Eigen::VectorXd& vel)
{
	SetPose(pose);
	SetVel(vel);

	{
		DRX3D_PROFILE("rbdModel::UpdateJointSubspaceArr");
		UpdateJointSubspaceArr();
	}
	{
		DRX3D_PROFILE("rbdModel::UpdateChildParentMatArr");
		UpdateChildParentMatArr();
	}
	{
		DRX3D_PROFILE("rbdModel::UpdateSpWorldTrans");
		UpdateSpWorldTrans();
	}
	{
		DRX3D_PROFILE("UpdateMassMat");
		UpdateMassMat();
	}
	{
		DRX3D_PROFILE("UpdateBiasForce");
		UpdateBiasForce();
	}
}

i32 cRBDModel::GetNumDof() const
{
	return cKinTree::GetNumDof(mJointMat);
}

i32 cRBDModel::GetNumJoints() const
{
	return cKinTree::GetNumJoints(mJointMat);
}

void cRBDModel::SetGravity(const tVector& gravity)
{
	mGravity = gravity;
}



const tVector& cRBDModel::GetGravity() const
{
	return mGravity;
}

const Eigen::MatrixXd& cRBDModel::GetJointMat() const
{
	return mJointMat;
}

const Eigen::MatrixXd& cRBDModel::GetBodyDefs() const
{
	return mBodyDefs;
}

const Eigen::VectorXd& cRBDModel::GetPose() const
{
	return mPose;
}

const Eigen::VectorXd& cRBDModel::GetVel() const
{
	return mVel;
}

i32 cRBDModel::GetParent(i32 j) const
{
	return cKinTree::GetParent(mJointMat, j);
}

const Eigen::MatrixXd& cRBDModel::GetMassMat() const
{
	return mMassMat;
}

const Eigen::VectorXd& cRBDModel::GetBiasForce() const
{
	return mBiasForce;
}

Eigen::MatrixXd& cRBDModel::GetInertiaBuffer()
{
	return mInertiaBuffer;
}

tMatrix cRBDModel::GetChildParentMat(i32 j) const
{
	assert(j >= 0 && j < GetNumJoints());
	tMatrix trans;
	i32 r = static_cast<i32>(trans.rows());
	i32 c = static_cast<i32>(trans.cols());
	trans = mChildParentMatArr.block(j * r, 0, r, c);
	return trans;
}

tMatrix cRBDModel::GetParentChildMat(i32 j) const
{
	tMatrix child_parent_trans = GetChildParentMat(j);
	tMatrix parent_child_trans = cMathUtil::InvRigidMat(child_parent_trans);
	return parent_child_trans;
}

cSpAlg::tSpTrans cRBDModel::GetSpChildParentTrans(i32 j) const
{
	tMatrix mat = GetChildParentMat(j);
	return cSpAlg::MatToTrans(mat);
}

cSpAlg::tSpTrans cRBDModel::GetSpParentChildTrans(i32 j) const
{
	tMatrix mat = GetParentChildMat(j);
	return cSpAlg::MatToTrans(mat);
}

tMatrix cRBDModel::GetWorldJointMat(i32 j) const
{
	cSpAlg::tSpTrans trans = GetSpWorldJointTrans(j);
	return cSpAlg::TransToMat(trans);
}

tMatrix cRBDModel::GetJointWorldMat(i32 j) const
{
	cSpAlg::tSpTrans trans = GetSpJointWorldTrans(j);
	return cSpAlg::TransToMat(trans);
}

cSpAlg::tSpTrans cRBDModel::GetSpWorldJointTrans(i32 j) const
{
	assert(j >= 0 && j < GetNumJoints());
	cSpAlg::tSpTrans trans = cSpAlg::GetTrans(mSpWorldJointTransArr, j);
	return trans;
}

cSpAlg::tSpTrans cRBDModel::GetSpJointWorldTrans(i32 j) const
{
	cSpAlg::tSpTrans world_joint_trans = GetSpWorldJointTrans(j);
	return cSpAlg::InvTrans(world_joint_trans);
}

const Eigen::Block<const Eigen::MatrixXd> cRBDModel::GetJointSubspace(i32 j) const
{
	assert(j >= 0 && j < GetNumJoints());
	i32 offset = cKinTree::GetParamOffset(mJointMat, j);
	i32 dim = cKinTree::GetParamSize(mJointMat, j);
	i32 r = static_cast<i32>(mJointSubspaceArr.rows());
	return mJointSubspaceArr.block(0, offset, r, dim);
}

tVector cRBDModel::CalcJointWorldPos(i32 j) const
{
	cSpAlg::tSpTrans world_joint_trans = GetSpWorldJointTrans(j);
	tVector r = cSpAlg::GetRad(world_joint_trans);
	return r;
}

void cRBDModel::SetPose(const Eigen::VectorXd& pose)
{
	mPose = pose;
}

void cRBDModel::SetVel(const Eigen::VectorXd& vel)
{
	mVel = vel;
}

void cRBDModel::InitJointSubspaceArr()
{
	i32 num_dofs = GetNumDof();
	i32 num_joints = GetNumJoints();
	mJointSubspaceArr = Eigen::MatrixXd(gSpVecSize, num_dofs);
	for (i32 j = 0; j < num_joints; ++j)
	{
		i32 offset = cKinTree::GetParamOffset(mJointMat, j);
		i32 dim = cKinTree::GetParamSize(mJointMat, j);
		i32 r = static_cast<i32>(mJointSubspaceArr.rows());
		mJointSubspaceArr.block(0, offset, r, dim) = cRBDUtil::BuildJointSubspace(mJointMat, mPose, j);
	}
}

void cRBDModel::UpdateJointSubspaceArr()
{
	i32 num_joints = GetNumJoints();
	for (i32 j = 0; j < num_joints; ++j)
	{
		bool const_subspace = cRBDUtil::IsConstJointSubspace(mJointMat, j);
		if (!const_subspace)
		{
			i32 offset = cKinTree::GetParamOffset(mJointMat, j);
			i32 dim = cKinTree::GetParamSize(mJointMat, j);
			i32 r = static_cast<i32>(mJointSubspaceArr.rows());
			mJointSubspaceArr.block(0, offset, r, dim) = cRBDUtil::BuildJointSubspace(mJointMat, mPose, j);
		}
	}
}

void cRBDModel::UpdateChildParentMatArr()
{
	i32 num_joints = GetNumJoints();
	for (i32 j = 0; j < num_joints; ++j)
	{
		tMatrix child_parent_trans = cKinTree::ChildParentTrans(mJointMat, mPose, j);
		i32 r = static_cast<i32>(child_parent_trans.rows());
		i32 c = static_cast<i32>(child_parent_trans.cols());
		mChildParentMatArr.block(j * r, 0, r, c) = child_parent_trans;
	}
}

void cRBDModel::UpdateSpWorldTrans()
{
	cRBDUtil::CalcWorldJointTransforms(*this, mSpWorldJointTransArr);
}

void cRBDModel::UpdateMassMat()
{
	cRBDUtil::BuildMassMat(*this, mInertiaBuffer, mMassMat);
}

void cRBDModel::UpdateBiasForce()
{
	cRBDUtil::BuildBiasForce(*this, mBiasForce);
}