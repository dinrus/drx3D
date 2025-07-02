#ifndef D3_CONTACT_SOLVER_INFO
#define D3_CONTACT_SOLVER_INFO

#include <drx3D/Common/b3Scalar.h>

enum b3SolverMode
{
	D3_SOLVER_RANDMIZE_ORDER = 1,
	D3_SOLVER_FRICTION_SEPARATE = 2,
	D3_SOLVER_USE_WARMSTARTING = 4,
	D3_SOLVER_USE_2_FRICTION_DIRECTIONS = 16,
	D3_SOLVER_ENABLE_FRICTION_DIRECTION_CACHING = 32,
	D3_SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION = 64,
	D3_SOLVER_CACHE_FRIENDLY = 128,
	D3_SOLVER_SIMD = 256,
	D3_SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS = 512,
	D3_SOLVER_ALLOW_ZERO_LENGTH_FRICTION_DIRECTIONS = 1024
};

struct b3ContactSolverInfoData
{
	b3Scalar m_tau;
	b3Scalar m_damping;  //global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
	b3Scalar m_friction;
	b3Scalar m_timeStep;
	b3Scalar m_restitution;
	i32 m_numIterations;
	b3Scalar m_maxErrorReduction;
	b3Scalar m_sor;
	b3Scalar m_erp;        //used as Baumgarte factor
	b3Scalar m_erp2;       //used in Split Impulse
	b3Scalar m_globalCfm;  //constraint force mixing
	i32 m_splitImpulse;
	b3Scalar m_splitImpulsePenetrationThreshold;
	b3Scalar m_splitImpulseTurnErp;
	b3Scalar m_linearSlop;
	b3Scalar m_warmstartingFactor;

	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;
	b3Scalar m_maxGyroscopicForce;
	b3Scalar m_singleAxisRollingFrictionThreshold;
};

struct b3ContactSolverInfo : public b3ContactSolverInfoData
{
	inline b3ContactSolverInfo()
	{
		m_tau = b3Scalar(0.6);
		m_damping = b3Scalar(1.0);
		m_friction = b3Scalar(0.3);
		m_timeStep = b3Scalar(1.f / 60.f);
		m_restitution = b3Scalar(0.);
		m_maxErrorReduction = b3Scalar(20.);
		m_numIterations = 10;
		m_erp = b3Scalar(0.2);
		m_erp2 = b3Scalar(0.8);
		m_globalCfm = b3Scalar(0.);
		m_sor = b3Scalar(1.);
		m_splitImpulse = true;
		m_splitImpulsePenetrationThreshold = -.04f;
		m_splitImpulseTurnErp = 0.1f;
		m_linearSlop = b3Scalar(0.0);
		m_warmstartingFactor = b3Scalar(0.85);
		//m_solverMode =  D3_SOLVER_USE_WARMSTARTING |  D3_SOLVER_SIMD | D3_SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION|D3_SOLVER_USE_2_FRICTION_DIRECTIONS|D3_SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;// | D3_SOLVER_RANDMIZE_ORDER;
		m_solverMode = D3_SOLVER_USE_WARMSTARTING | D3_SOLVER_SIMD;  // | D3_SOLVER_RANDMIZE_ORDER;
		m_restingContactRestitutionThreshold = 2;                    //unused as of 2.81
		m_minimumSolverBatchSize = 128;                              //try to combine islands until the amount of constraints reaches this limit
		m_maxGyroscopicForce = 100.f;                                ///only used to clamp forces for bodies that have their D3_ENABLE_GYROPSCOPIC_FORCE flag set (using b3RigidBody::setFlag)
		m_singleAxisRollingFrictionThreshold = 1e30f;                ///if the velocity is above this threshold, it will use a single constraint row (axis), otherwise 3 rows.
	}
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3ContactSolverInfoDoubleData
{
	double m_tau;
	double m_damping;  //global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
	double m_friction;
	double m_timeStep;
	double m_restitution;
	double m_maxErrorReduction;
	double m_sor;
	double m_erp;        //used as Baumgarte factor
	double m_erp2;       //used in Split Impulse
	double m_globalCfm;  //constraint force mixing
	double m_splitImpulsePenetrationThreshold;
	double m_splitImpulseTurnErp;
	double m_linearSlop;
	double m_warmstartingFactor;
	double m_maxGyroscopicForce;
	double m_singleAxisRollingFrictionThreshold;

	i32 m_numIterations;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;
	i32 m_splitImpulse;
	char m_padding[4];
};
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct b3ContactSolverInfoFloatData
{
	float m_tau;
	float m_damping;  //global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
	float m_friction;
	float m_timeStep;

	float m_restitution;
	float m_maxErrorReduction;
	float m_sor;
	float m_erp;  //used as Baumgarte factor

	float m_erp2;       //used in Split Impulse
	float m_globalCfm;  //constraint force mixing
	float m_splitImpulsePenetrationThreshold;
	float m_splitImpulseTurnErp;

	float m_linearSlop;
	float m_warmstartingFactor;
	float m_maxGyroscopicForce;
	float m_singleAxisRollingFrictionThreshold;

	i32 m_numIterations;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;

	i32 m_splitImpulse;
	char m_padding[4];
};

#endif  //D3_CONTACT_SOLVER_INFO
