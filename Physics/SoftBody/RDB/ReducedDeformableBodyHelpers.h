#ifndef DRX3D_REDUCED_SOFT_BODY_HELPERS_H
#define DRX3D_REDUCED_SOFT_BODY_HELPERS_H

#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>
#include <string>

struct ReducedDeformableBodyHelpers
{
	// create a reduced deformable object
	static ReducedDeformableBody* createReducedDeformableObject(SoftBodyWorldInfo& worldInfo, const STxt& file_path, const STxt& vtk_file, i32k num_modes, bool rigid_only);
	// read in geometry info from Vtk file
  static ReducedDeformableBody* createFromVtkFile(SoftBodyWorldInfo& worldInfo, tukk vtk_file);
	// read in all reduced files
	static void readReducedDeformableInfoFromFiles(ReducedDeformableBody* rsb, tukk file_path);
	// read in a binary vector
	static void readBinaryVec(ReducedDeformableBody::tDenseArray& vec, u32k n_size, tukk file);
	// read in a binary matrix
	static void readBinaryMat(ReducedDeformableBody::tDenseMatrix& mat, u32k n_modes, u32k n_full, tukk file);
	
	// calculate the local inertia tensor for a box shape reduced deformable object
	static void calculateLocalInertia(Vec3& inertia, const Scalar mass, const Vec3& half_extents, const Vec3& margin);
};


#endif // DRX3D_REDUCED_SOFT_BODY_HELPERS_H