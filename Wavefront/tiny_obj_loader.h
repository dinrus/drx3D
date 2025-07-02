//
// Copyright 2012-2013, Syoyo Fujita.
//
// Licensed under 2-clause BSD liecense.
//
#ifndef _DRX3D_TINY_OBJ_LOADER_H
#define _DRX3D_TINY_OBJ_LOADER_H

#include <drxtypes.h>
#include <string>
#include <vector>
#include <map>

struct CommonFileIOInterface;

namespace bt_tinyobj
{
struct vertex_index_t
{
	i32 v_idx, vt_idx, vn_idx;
	vertex_index_t() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
	explicit vertex_index_t(i32 idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
	vertex_index_t(i32 vidx, i32 vtidx, i32 vnidx)
		: v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

typedef std::vector<vertex_index_t> face_t;

typedef struct
{
	STxt name;

	float ambient[3];
	float diffuse[3];
	float specular[3];
	float transmittance[3];
	float emission[3];
	float shininess;
	float transparency;  // 1 == opaque; 0 == fully transparent

	STxt ambient_texname;   // map_Ka
	STxt diffuse_texname;   // map_Kd
	STxt specular_texname;  // map_Ks
	STxt normal_texname;
	std::map<STxt, STxt> unknown_parameter;
} material_t;

// Index struct to support different indices for vtx/normal/texcoord.
// -1 means not used.
typedef struct
{
	i32 vertex_index;
	i32 normal_index;
	i32 texcoord_index;
} index_t;

typedef struct
{
	std::vector<index_t> indices;
} mesh_t;

typedef struct
{
	STxt name;
	material_t material;
	mesh_t mesh;
} shape_t;

// Vertex attributes
struct attrib_t
{
	std::vector<float> vertices;   // 'v'(xyz)
	std::vector<float> normals;    // 'vn'
	std::vector<float> texcoords;  // 'vt'(uv)
	attrib_t() {}
};
/// Loads .obj from a file.
/// 'shapes' will be filled with parsed shape data
/// The function returns error string.
/// Returns empty string when loading .obj success.
/// 'mtl_basepath' is optional, and used for base path for .mtl file.
#ifdef USE_STREAM
STxt LoadObj(
	attrib_t& attrib,
	std::vector<shape_t>& shapes,  // [output]
	tukk filename,
	tukk mtl_basepath = NULL);
#else
STxt
LoadObj(
	attrib_t& attrib,
	std::vector<shape_t>& shapes,
	tukk filename,
	tukk mtl_basepath,
	CommonFileIOInterface* fileIO);
#endif

};  // namespace bt_tinyobj

#endif  // _DRX3D_TINY_OBJ_LOADER_H
