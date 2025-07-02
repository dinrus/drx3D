#include "../LoadMeshFromObj.h"

#include  <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <stdio.h>  //fopen
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <string>
#include <vector>
#include "../Wavefront2GLInstanceGraphicsShape.h"
#include <drx3D/Common/b3HashMap.h>

struct CachedObjResult
{
	STxt m_msg;
	std::vector<bt_tinyobj::shape_t> m_shapes;
	bt_tinyobj::attrib_t m_attribute;
};

static b3HashMap<b3HashString, CachedObjResult> gCachedObjResults;
static i32 gEnableFileCaching = 1;

i32 b3IsFileCachingEnabled()
{
	return gEnableFileCaching;
}
void b3EnableFileCaching(i32 enable)
{
	gEnableFileCaching = enable;
	if (enable == 0)
	{
		gCachedObjResults.clear();
	}
}

STxt LoadFromCachedOrFromObj(
	bt_tinyobj::attrib_t& attribute,
	std::vector<bt_tinyobj::shape_t>& shapes,  // [output]
	tukk filename,
	tukk mtl_basepath,
	struct CommonFileIOInterface* fileIO)
{
	CachedObjResult* resultPtr = gCachedObjResults[filename];
	if (resultPtr)
	{
		const CachedObjResult& result = *resultPtr;
		shapes = result.m_shapes;
		attribute = result.m_attribute;
		return result.m_msg;
	}

	STxt err = bt_tinyobj::LoadObj(attribute, shapes, filename, mtl_basepath, fileIO);
	CachedObjResult result;
	result.m_msg = err;
	result.m_shapes = shapes;
	result.m_attribute = attribute;
	if (gEnableFileCaching)
	{
		gCachedObjResults.insert(filename, result);
	}
	return err;
}

GLInstanceGraphicsShape* LoadMeshFromObj(tukk relativeFileName, tukk materialPrefixPath, struct CommonFileIOInterface* fileIO)
{
	D3_PROFILE("LoadMeshFromObj");
	std::vector<bt_tinyobj::shape_t> shapes;
	bt_tinyobj::attrib_t attribute;
	{
		D3_PROFILE("drx3D_tinyobj::LoadObj2");
		STxt err = LoadFromCachedOrFromObj(attribute, shapes, relativeFileName, materialPrefixPath, fileIO);
	}

	{
		D3_PROFILE("gCreateGraphicsShapeFromWavefrontObj");
		GLInstanceGraphicsShape* gfxShape = btgCreateGraphicsShapeFromWavefrontObj(attribute, shapes);
		return gfxShape;
	}
}
