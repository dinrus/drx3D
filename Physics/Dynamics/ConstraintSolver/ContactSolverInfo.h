#ifndef DRX3D_CONTACT_SOLVER_INFO
#define DRX3D_CONTACT_SOLVER_INFO

#include <drx3D/Maths/Linear/Scalar.h>

enum SolverMode
{
	SOLVER_RANDMIZE_ORDER = 1,
	SOLVER_FRICTION_SEPARATE = 2,
	SOLVER_USE_WARMSTARTING = 4,
	SOLVER_USE_2_FRICTION_DIRECTIONS = 16,
	SOLVER_ENABLE_FRICTION_DIRECTION_CACHING = 32,
	SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION = 64,
	SOLVER_CACHE_FRIENDLY = 128,
	SOLVER_SIMD = 256,
	SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS = 512,
	SOLVER_ALLOW_ZERO_LENGTH_FRICTION_DIRECTIONS = 1024,
	SOLVER_DISABLE_IMPLICIT_CONE_FRICTION = 2048,
	SOLVER_USE_ARTICULATED_WARMSTARTING = 4096,
};

struct ContactSolverInfoData
{
	Scalar m_tau;
	Scalar m_damping;  //global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
	Scalar m_friction;
	Scalar m_timeStep;
	Scalar m_restitution;
	i32 m_numIterations;
	Scalar m_maxErrorReduction;
	Scalar m_sor;          //successive over-relaxation term
	Scalar m_erp;          //error reduction for non-contact constraints
	Scalar m_erp2;         //error reduction for contact constraints
	Scalar m_deformable_erp;          //error reduction for deformable constraints
	Scalar m_deformable_cfm;          //constraint force mixing for deformable constraints
	Scalar m_deformable_maxErrorReduction; // maxErrorReduction for deformable contact
	Scalar m_globalCfm;    //constraint force mixing for contacts and non-contacts
	Scalar m_frictionERP;  //error reduction for friction constraints
	Scalar m_frictionCFM;  //constraint force mixing for friction constraints

	i32 m_splitImpulse;
	Scalar m_splitImpulsePenetrationThreshold;
	Scalar m_splitImpulseTurnErp;
	Scalar m_linearSlop;
	Scalar m_warmstartingFactor;
	Scalar m_articulatedWarmstartingFactor;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;
	Scalar m_maxGyroscopicForce;
	Scalar m_singleAxisRollingFrictionThreshold;
	Scalar m_leastSquaresResidualThreshold;
	Scalar m_restitutionVelocityThreshold;
	bool m_jointFeedbackInWorldSpace;
	bool m_jointFeedbackInJointFrame;
	i32 m_reportSolverAnalytics;
	i32 m_numNonContactInnerIterations;
};

struct ContactSolverInfo : public ContactSolverInfoData
{
	inline ContactSolverInfo()
	{
		m_tau = Scalar(0.6);
		m_damping = Scalar(1.0);
		m_friction = Scalar(0.3);
		m_timeStep = Scalar(1.f / 60.f);
		m_restitution = Scalar(0.);
		m_maxErrorReduction = Scalar(20.);
		m_numIterations = 10;
		m_erp = Scalar(0.2);
		m_erp2 = Scalar(0.2);
		m_deformable_erp = Scalar(0.06);
		m_deformable_cfm = Scalar(0.01);
		m_deformable_maxErrorReduction = Scalar(0.1);
		m_globalCfm = Scalar(0.);
		m_frictionERP = Scalar(0.2);  //positional friction 'anchors' are disabled by default
		m_frictionCFM = Scalar(0.);
		m_sor = Scalar(1.);
		m_splitImpulse = true;
		m_splitImpulsePenetrationThreshold = -.04f;
		m_splitImpulseTurnErp = 0.1f;
		m_linearSlop = Scalar(0.0);
		m_warmstartingFactor = Scalar(0.85);
		m_articulatedWarmstartingFactor = Scalar(0.85);
		//m_solverMode =  SOLVER_USE_WARMSTARTING |  SOLVER_SIMD | SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION|SOLVER_USE_2_FRICTION_DIRECTIONS|SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;// | SOLVER_RANDMIZE_ORDER;
		m_solverMode = SOLVER_USE_WARMSTARTING | SOLVER_SIMD;  // | SOLVER_RANDMIZE_ORDER;
		m_restingContactRestitutionThreshold = 2;              //unused as of 2.81
		m_minimumSolverBatchSize = 128;                        //try to combine islands until the amount of constraints reaches this limit
		m_maxGyroscopicForce = 100.f;                          ///it is only used for 'explicit' version of gyroscopic force
		m_singleAxisRollingFrictionThreshold = 1e30f;          ///if the velocity is above this threshold, it will use a single constraint row (axis), otherwise 3 rows.
		m_leastSquaresResidualThreshold = 0.f;
		m_restitutionVelocityThreshold = 0.2f;  //if the relative velocity is below this threshold, there is zero restitution
		m_jointFeedbackInWorldSpace = false;
		m_jointFeedbackInJointFrame = false;
		m_reportSolverAnalytics = 0;
		m_numNonContactInnerIterations = 1;   // the number of inner iterations for solving motor constraint in a single iteration of the constraint solve
	}
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct ContactSolverInfoDoubleData
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
	double m_articulatedWarmstartingFactor;
	double m_maxGyroscopicForce;  ///it is only used for 'explicit' version of gyroscopic force
	double m_singleAxisRollingFrictionThreshold;

	i32 m_numIterations;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;
	i32 m_splitImpulse;
	char m_padding[4];
};
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct ContactSolverInfoFloatData
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
	float m_articulatedWarmstartingFactor;
	float m_maxGyroscopicForce;

	float m_singleAxisRollingFrictionThreshold;
	i32 m_numIterations;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;

	i32 m_minimumSolverBatchSize;
	i32 m_splitImpulse;
	
};

#endif  //DRX3D_CONTACT_SOLVER_INFO
