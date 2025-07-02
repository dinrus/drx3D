#ifndef LOAD_MESH_FROM_OBJ_H
#define LOAD_MESH_FROM_OBJ_H

struct GLInstanceGraphicsShape;

#include <drx3D/Wavefront/tiny_obj_loader.h>

i32 b3IsFileCachingEnabled();
void b3EnableFileCaching(i32 enable);

STxt LoadFromCachedOrFromObj(
	bt_tinyobj::attrib_t& attribute,
	std::vector<bt_tinyobj::shape_t>& shapes,  // [output]
	tukk filename,
	tukk mtl_basepath,
	struct CommonFileIOInterface* fileIO);

GLInstanceGraphicsShape* LoadMeshFromObj(tukk relativeFileName, tukk materialPrefixPath,struct CommonFileIOInterface* fileIO);

#endif  //LOAD_MESH_FROM_OBJ_H
