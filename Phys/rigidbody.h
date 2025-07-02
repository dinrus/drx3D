// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef rigidbody_h
#define rigidbody_h
#pragma once

struct entity_contact;
i32k MAX_CONTACTS = 9984;

enum rbflags { rb_RK4=0x1, rb_articulated=0x10 };

class RigidBody {
public:
	RigidBody(bool) {} // uninitialized; for tmp data
	RigidBody();
	void Create(const Vec3 &center,const Vec3 &Ibody0,const quaternionf &q0, float volume,float mass,
							const quaternionf &qframe,const Vec3 &posframe);
	void Add(const Vec3 &center,const Vec3 Ibodyop,const quaternionf &qop, float volume,float mass);
	void zero();

	void Step(float dt);
	void UpdateState();
	void GetContactMatrix(const Vec3 &r, Matrix33 &K);

	Vec3 pos;
	quaternionf q;
	Vec3 P,L;
	Vec3 w,v;

	float M,Minv; // mass, 1.0/mass (0 for static objects)
	float V; // volume
	Diag33 Ibody; // diagonalized inertia tensor (aligned with body's axes of inertia)
	Diag33 Ibody_inv; // { 1/Ibody.ii }
	quaternionf qfb; // frame->body rotation
	Vec3 offsfb; // frame->body offset
	i32 flags;

	Matrix33 Iinv; // I^-1(t)

	Vec3 Fcollision,Tcollision;
	short bProcessed[MAX_PHYS_THREADS-((MAX_PHYS_THREADS-1)>>31)];
	float Eunproj;
};

struct body_helper {
	Vec3 v,w;
	float Minv,M;
	Matrix33 Iinv;
	Vec3 L;
};

struct ArticulatedBody {
	RigidBody body;
	i32 iParent;
	i32 nChildren,nChildrenTree;
	i32 nPotentialAngles;
	struct featherstone_data *fs;
	Vec3 Pext,Lext;

	void GetContactMatrix(const Vec3& r, Matrix33 &K);
	void ApplyImpulse(const Vec3& dP, const Vec3& dL, body_helper *bodies, i32 iCaller);
};

enum contactflags { contact_count_mask=0x3F, contact_new=0x40, contact_2b_verified=0x80, contact_2b_verified_log2=7,
										contact_angular=0x100, contact_constraint_3dof=0x200, contact_constraint_2dof=0x400,
										contact_constraint_1dof=0x800, contact_solve_for=0x1000,
										contact_constraint=contact_constraint_3dof|contact_constraint_2dof|contact_constraint_1dof,
										contact_angular_log2=8,contact_bidx=0x2000,contact_bidx_log2=13, contact_maintain_count=0x4000,
										contact_wheel=0x8000, contact_use_C_1dof=0x10000, contact_use_C_2dof=0x20000,
										contact_use_C = contact_use_C_1dof|contact_use_C_2dof,
										contact_inexact=0x40000,
										contact_last=0x80000, contact_remove=0x100000, contact_archived=0x200000,
										contact_rope=0x400000, contact_preserve_Pspare=0x800000, contact_area=0x1000000, contact_rope_stretchy=0x2000000
									};

struct rope_solver_vtx {
	Vec3 r,v,P;
	RigidBody *pbody;
	class CPhysicalEntity *pent;
	i32 iBody;
	i32 ivtx;
};


struct entity_contact {
	entity_contact *next,*prev;
	entity_contact *nextAux;

	Vec3 pt[2];
	Vec3 n;
	CPhysicalEntity *pent[2];
	two_ints_in_one ipart;
	RigidBody *pbody[2];
	Vec3 nloc;
	float friction;
	i32 flags;
	float vrel;
	Vec3 vreq;
	float Pspare;
	float penetration;
	u32 iNormal : 2;
	u32 id0     : 15;
	u32 id1     : 15;
	u32 bProcessed  : 16;
	u32 iConstraint : 14;
	u32 bConstraint : 1;
	u32 bChunkStart : 1;
	i32 iCount;
	i32 *pBounceCount;
};

void DisablePreCG();
void InitContactSolver(float time_interval);
void CleanupContactSolvers();
void RegisterContact(entity_contact *pcontact);
i32 InvokeContactSolver(float time_interval, SolverSettings *pss, float Ebefore, entity_contact **&pContacts,i32 &nContacts);
char *AllocSolverTmpBuf(i32 size);

#endif
