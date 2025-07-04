#ifndef STAN_SPRING_NETWORK_H
#define STAN_SPRING_NETWORK_H

#include "vec3n.h"

#define SPRING_STRUCT (0)
#define SPRING_SHEAR (1)
#define SPRING_BEND (2)

class SpringNetwork
{
public:
	class Spring
	{
	public:
		i32 type;  // index into coefficients spring_k[]
		float restlen;
		i32 a, b;      // spring endpoints vector indices
		i32 iab, iba;  // indices into off-diagonal blocks of sparse matrix
		Spring() {}
		Spring(i32 _type, i32 _a, i32 _b, float _restlen) : type(_type), a(_a), b(_b), restlen(_restlen) { iab = iba = -1; }
	};
	Array<Spring> springs;
	float3N X;        // positions of all points
	float3N V;        // velocities
	float3N F;        // force on each point
	float3N dV;       // change in velocity
	float3Nx3N A;     // big matrix we solve system with
	float3Nx3N dFdX;  // big matrix of derivative of force wrt position
	float3Nx3N dFdV;  // big matrix of derivative of force wrt velocity
	float3Nx3N S;     // used for our constraints - contains only some diagonal blocks as needed S[i,i]
	i32 awake;
	float3 bmin, bmax;
	union {
		struct
		{
			float spring_struct;
			float spring_shear;
			float spring_bend;
		};
		float spring_k[3];
	};
	float spring_damp;
	float spring_air;
	float cloth_step;  // delta time for cloth
	float3 cloth_gravity;
	float cloth_sleepthreshold;
	i32 cloth_sleepcount;

	SpringNetwork(i32 _n);
	Spring &AddBlocks(Spring &s);
	Spring &CreateSpring(i32 type, i32 a, i32 b, float restlen) { return AddBlocks(springs.Add(Spring(type, a, b, restlen))); }
	Spring &CreateSpring(i32 type, i32 a, i32 b) { return CreateSpring(type, a, b, magnitude(X[b] - X[a])); }
	void UpdateLimits() { BoxLimits(X.element, X.count, bmin, bmax); }
	void Wake() { awake = cloth_sleepcount; }
	void Simulate(float dt);
	void PreSolveSpring(const Spring &s);
	void CalcForces();
};

#endif  //STAN_SPRING_NETWORK_H
