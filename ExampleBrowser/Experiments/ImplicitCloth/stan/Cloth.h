#ifndef STAN_CLOTH_H
#define STAN_CLOTH_H

#include "SpringNetwork.h"

class Cloth : public SpringNetwork
{
public:
	i32 w, h;

	float3 color;  // for debug rendering
	Cloth(tukk _name, i32 _n);
	~Cloth();
};

Cloth* ClothCreate(i32 w, i32 h, float size);

#endif  //STAN_CLOTH_H
