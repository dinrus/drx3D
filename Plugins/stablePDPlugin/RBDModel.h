#pragma once

#include "SpAlg.h"
#include "MathUtil.h"

// this class is mostly to help with efficiency by precomputing some useful
// quantities for RBD calculations
class cRBDModel
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	cRBDModel();
	virtual ~cRBDModel();

	virtual void Init(const Eigen::MatrixXd& joint_mat, const Eigen::MatrixXd& body_defs, const tVector& gravity);
	virtual void Update(const Eigen::VectorXd& pose, const Eigen::VectorXd& vel);

	virtual i32 GetNumDof() const;
	virtual i32 GetNumJoints() const;

	virtual const tVector& GetGravity() const;
	virtual void SetGravity(const tVector& gravity);

	virtual const Eigen::MatrixXd& GetJointMat() const;
	virtual const Eigen::MatrixXd& GetBodyDefs() const;
	virtual const Eigen::VectorXd& GetPose() const;
	virtual const Eigen::VectorXd& GetVel() const;
	virtual i32 GetParent(i32 j) const;

	virtual const Eigen::MatrixXd& GetMassMat() const;
	virtual const Eigen::VectorXd& GetBiasForce() const;
	virtual Eigen::MatrixXd& GetInertiaBuffer();

	virtual tMatrix GetChildParentMat(i32 j) const;
	virtual tMatrix GetParentChildMat(i32 j) const;
	virtual cSpAlg::tSpTrans GetSpChildParentTrans(i32 j) const;
	virtual cSpAlg::tSpTrans GetSpParentChildTrans(i32 j) const;

	virtual tMatrix GetWorldJointMat(i32 j) const;
	virtual tMatrix GetJointWorldMat(i32 j) const;
	virtual cSpAlg::tSpTrans GetSpWorldJointTrans(i32 j) const;
	virtual cSpAlg::tSpTrans GetSpJointWorldTrans(i32 j) const;

	virtual const Eigen::Block<const Eigen::MatrixXd> GetJointSubspace(i32 j) const;
	virtual tVector CalcJointWorldPos(i32 j) const;

protected:
	tVector mGravity;
	Eigen::MatrixXd mJointMat;
	Eigen::MatrixXd mBodyDefs;
	Eigen::VectorXd mPose;
	Eigen::VectorXd mVel;

	Eigen::MatrixXd mJointSubspaceArr;
	Eigen::MatrixXd mChildParentMatArr;
	Eigen::MatrixXd mSpWorldJointTransArr;
	Eigen::MatrixXd mMassMat;
	Eigen::VectorXd mBiasForce;
	Eigen::MatrixXd mInertiaBuffer;

	virtual void SetPose(const Eigen::VectorXd& pose);
	virtual void SetVel(const Eigen::VectorXd& vel);

	virtual void InitJointSubspaceArr();

	virtual void UpdateJointSubspaceArr();
	virtual void UpdateChildParentMatArr();
	virtual void UpdateSpWorldTrans();
	virtual void UpdateMassMat();
	virtual void UpdateBiasForce();
};
