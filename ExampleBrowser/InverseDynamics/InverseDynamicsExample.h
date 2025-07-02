#ifndef INVERSE_DYNAMICS_EXAMPLE_H
#define INVERSE_DYNAMICS_EXAMPLE_H

enum drx3d_inverseExampleOptions
{
	DRX3D_ID_LOAD_URDF = 0,
	DRX3D_ID_PROGRAMMATICALLY = 1
};

class CommonExampleInterface* drx3d_inverseExampleCreateFunc(struct CommonExampleOptions& options);

#endif  //INVERSE_DYNAMICS_EXAMPLE_H
