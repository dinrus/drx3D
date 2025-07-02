#ifndef DRX3D_SOFT_BODY_HELPERS_H
#define DRX3D_SOFT_BODY_HELPERS_H

#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <fstream>
#include <string>
//
// Helpers
//

/* fDrawFlags															*/
struct fDrawFlags
{
	enum _
	{
		Nodes = 0x0001,
		Links = 0x0002,
		Faces = 0x0004,
		Tetras = 0x0008,
		Normals = 0x0010,
		Contacts = 0x0020,
		Anchors = 0x0040,
		Notes = 0x0080,
		Clusters = 0x0100,
		NodeTree = 0x0200,
		FaceTree = 0x0400,
		ClusterTree = 0x0800,
		Joints = 0x1000,
		/* presets	*/
		Std = Links + Faces + Tetras + Anchors + Notes + Joints,
		StdTetra = Std - Faces + Tetras
	};
};

struct SoftBodyHelpers
{
	/* Draw body															*/
	static void Draw(SoftBody* psb,
					 IDebugDraw* idraw,
					 i32 drawflags = fDrawFlags::Std);
	/* Draw body infos														*/
	static void DrawInfos(SoftBody* psb,
						  IDebugDraw* idraw,
						  bool masses,
						  bool areas,
						  bool stress);
	/* Draw node tree														*/
	static void DrawNodeTree(SoftBody* psb,
							 IDebugDraw* idraw,
							 i32 mindepth = 0,
							 i32 maxdepth = -1);
	/* Draw face tree														*/
	static void DrawFaceTree(SoftBody* psb,
							 IDebugDraw* idraw,
							 i32 mindepth = 0,
							 i32 maxdepth = -1);
	/* Draw cluster tree													*/
	static void DrawClusterTree(SoftBody* psb,
								IDebugDraw* idraw,
								i32 mindepth = 0,
								i32 maxdepth = -1);
	/* Draw rigid frame														*/
	static void DrawFrame(SoftBody* psb,
						  IDebugDraw* idraw);
	/* Create a rope														*/
	static SoftBody* CreateRope(SoftBodyWorldInfo& worldInfo,
								  const Vec3& from,
								  const Vec3& to,
								  i32 res,
								  i32 fixeds);
	/* Create a patch														*/
	static SoftBody* CreatePatch(SoftBodyWorldInfo& worldInfo,
								   const Vec3& corner00,
								   const Vec3& corner10,
								   const Vec3& corner01,
								   const Vec3& corner11,
								   i32 resx,
								   i32 resy,
								   i32 fixeds,
								   bool gendiags,
								   Scalar perturbation = 0.);
	/* Create a patch with UV Texture Coordinates	*/
	static SoftBody* CreatePatchUV(SoftBodyWorldInfo& worldInfo,
									 const Vec3& corner00,
									 const Vec3& corner10,
									 const Vec3& corner01,
									 const Vec3& corner11,
									 i32 resx,
									 i32 resy,
									 i32 fixeds,
									 bool gendiags,
									 float* tex_coords = 0);
	static float CalculateUV(i32 resx, i32 resy, i32 ix, i32 iy, i32 id);
	/* Create an ellipsoid													*/
	static SoftBody* CreateEllipsoid(SoftBodyWorldInfo& worldInfo,
									   const Vec3& center,
									   const Vec3& radius,
									   i32 res);
	/* Create from trimesh													*/
	static SoftBody* CreateFromTriMesh(SoftBodyWorldInfo& worldInfo,
										 const Scalar* vertices,
										 i32k* triangles,
										 i32 ntriangles,
										 bool randomizeConstraints = true);
	/* Create from convex-hull												*/
	static SoftBody* CreateFromConvexHull(SoftBodyWorldInfo& worldInfo,
											const Vec3* vertices,
											i32 nvertices,
											bool randomizeConstraints = true);

	/* Export TetGen compatible .smesh file									*/
	//	static void				ExportAsSMeshFile(	SoftBody* psb,
	//												tukk filename);
	/* Create from TetGen .ele, .face, .node files							*/
	//	static SoftBody*		CreateFromTetGenFile(	SoftBodyWorldInfo& worldInfo,
	//													tukk ele,
	//													tukk face,
	//													tukk node,
	//													bool bfacelinks,
	//													bool btetralinks,
	//													bool bfacesfromtetras);
	/* Create from TetGen .ele, .face, .node data							*/
	static SoftBody* CreateFromTetGenData(SoftBodyWorldInfo& worldInfo,
											tukk ele,
											tukk face,
											tukk node,
											bool bfacelinks,
											bool btetralinks,
											bool bfacesfromtetras);
	static SoftBody* CreateFromVtkFile(SoftBodyWorldInfo& worldInfo, tukk vtk_file);

	static void writeObj(tukk file, const SoftBody* psb);

	static void writeState(tukk file, const SoftBody* psb);

  //this code cannot be here, dependency on example code are not allowed
	//static STxt loadDeformableState(AlignedObjectArray<Vec3>& qs, AlignedObjectArray<Vec3>& vs, tukk filename, CommonFileIOInterface* fileIO);

	static void getBarycentricWeights(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& p, Vec4& bary);

	static void getBarycentricWeights(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p, Vec4& bary);

	static void interpolateBarycentricWeights(SoftBody* psb);

	static void extrapolateBarycentricWeights(SoftBody* psb);

	static void generateBoundaryFaces(SoftBody* psb);

	static void duplicateFaces(tukk filename, const SoftBody* psb);
	/// Sort the list of links to move link calculations that are dependent upon earlier
	/// ones as far as possible away from the calculation of those values
	/// This tends to make adjacent loop iterations not dependent upon one another,
	/// so out-of-order processors can execute instructions from multiple iterations at once
	static void ReoptimizeLinkOrder(SoftBody* psb);
};

#endif  //DRX3D_SOFT_BODY_HELPERS_H
