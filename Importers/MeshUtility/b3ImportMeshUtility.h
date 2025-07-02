#ifndef D3_IMPORT_MESH_UTILITY_H
#define D3_IMPORT_MESH_UTILITY_H

#include <drxtypes.h>
#include <string>

enum b3ImportMeshDataFlags
{
	D3_IMPORT_MESH_HAS_RGBA_COLOR=1,
	D3_IMPORT_MESH_HAS_SPECULAR_COLOR=2,
};

struct b3ImportMeshData
{
	struct GLInstanceGraphicsShape* m_gfxShape;

	u8* m_textureImage1;  //in 3 component 8-bit RGB data
	bool m_isCached;
	i32 m_textureWidth;
	i32 m_textureHeight;
	double m_rgbaColor[4];
	double m_specularColor[4];
	i32 m_flags;

	b3ImportMeshData()
		:m_gfxShape(0),
		m_textureImage1(0),
		m_isCached(false),
		m_textureWidth(0),
		m_textureHeight(0),
		m_flags(0)
	{
	}

};

class b3ImportMeshUtility
{
public:
	static bool loadAndRegisterMeshFromFileInternal(const STxt& fileName, b3ImportMeshData& meshData, struct CommonFileIOInterface* fileIO);
};

#endif  //D3_IMPORT_MESH_UTILITY_H
