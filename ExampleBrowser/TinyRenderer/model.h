#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

namespace TinyRender
{
class Model
{
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_;  // attention, this Vec3i means vertex/uv/normal
	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	Vec4f m_colorRGBA;

	void load_texture(STxt filename, tukk suffix, TGAImage& img);

public:
	Model(tukk filename);
	Model();
	void setColorRGBA(const float rgba[4])
	{
		for (i32 i = 0; i < 4; i++)
			m_colorRGBA[i] = rgba[i];
	}

	const Vec4f& getColorRGBA() const
	{
		return m_colorRGBA;
	}
	void loadDiffuseTexture(tukk relativeFileName);
	void setDiffuseTextureFromData(u8* textureImage, i32 textureWidth, i32 textureHeight);
	void reserveMemory(i32 numVertices, i32 numIndices);
	void addVertex(float x, float y, float z, float normalX, float normalY, float normalZ, float u, float v);
	void addTriangle(i32 vertexposIndex0, i32 normalIndex0, i32 uvIndex0,
					 i32 vertexposIndex1, i32 normalIndex1, i32 uvIndex1,
					 i32 vertexposIndex2, i32 normalIndex2, i32 uvIndex2);

	~Model();
	i32 nverts();
	i32 nnormals()
	{
		return norms_.size();
	}
	i32 nfaces();
	
	Vec3f normal(i32 iface, i32 nthvert);
	Vec3f normal(Vec2f uv);
	Vec3f vert(i32 i);
	Vec3f vert(i32 iface, i32 nthvert);
	Vec3f* readWriteVertices() 
	{
		if (verts_.size() == 0)
			return 0;
		return &verts_[0];
	}

	Vec3f* readWriteNormals()
	{
		if (norms_.size() == 0)
			return 0;
		return &norms_[0];
	}
	

	Vec2f uv(i32 iface, i32 nthvert);
	TGAColor diffuse(Vec2f uv);
	float specular(Vec2f uv);
	std::vector<i32> face(i32 idx);
};
}

#endif  //__MODEL_H__
