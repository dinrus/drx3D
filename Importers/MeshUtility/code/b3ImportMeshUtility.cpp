#include "../b3ImportMeshUtility.h"

#include <vector>
#include <drx3D/Wavefront/tiny_obj_loader.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Importers/Obj/Wavefront2GLInstanceGraphicsShape.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3FileUtils.h>
#include <X/stb/stb_image.h>
#include <drx3D/Importers/Obj/LoadMeshFromObj.h>
#include <drx3D/Common/b3HashMap.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx/Core/Core.h>

struct CachedTextureResult
{
	STxt m_textureName;

	i32 m_width;
	i32 m_height;
	u8* m_pixels;
	CachedTextureResult()
		: m_width(0),
		  m_height(0),
		  m_pixels(0)
	{
	}
};

static b3HashMap<b3HashString, CachedTextureResult> gCachedTextureResults;
struct CachedTextureManager
{
	CachedTextureManager()
	{
	}
	virtual ~CachedTextureManager()
	{
		for (i32 i = 0; i < gCachedTextureResults.size(); i++)
		{
			CachedTextureResult* res = gCachedTextureResults.getAtIndex(i);
			if (res)
			{
				free(res->m_pixels);
			}
		}
	}
};
static CachedTextureManager sTexCacheMgr;

bool b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(const STxt& fileName, b3ImportMeshData& meshData, struct CommonFileIOInterface* fileIO)
{
	D3_PROFILE("loadAndRegisterMeshFromFileInternal");
	meshData.m_gfxShape = 0;
	meshData.m_textureImage1 = 0;
	meshData.m_textureHeight = 0;
	meshData.m_textureWidth = 0;
	meshData.m_flags = 0;
	meshData.m_isCached = false;

	char relativeFileName[1024];
	drx::Txt fname =  drx::GetHomeDir() + "/dinrus/dev/drx3D/data/" + fileName;
	if (fileIO->findResourcePath(fname, relativeFileName, 1024))
	{
		char pathPrefix[1024];

		b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
		Vec3 shift(0, 0, 0);

		std::vector<bt_tinyobj::shape_t> shapes;
		bt_tinyobj::attrib_t attribute;
		{
			D3_PROFILE("tinyobj::LoadObj");
			STxt err = LoadFromCachedOrFromObj(attribute, shapes, relativeFileName, pathPrefix, fileIO);
			//STxt err = tinyobj::LoadObj(shapes, relativeFileName, pathPrefix);
		}

		GLInstanceGraphicsShape* gfxShape = btgCreateGraphicsShapeFromWavefrontObj(attribute, shapes);
		{
			D3_PROFILE("Load Texture");
			//i32 textureIndex = -1;
			//try to load some texture
			for (i32 i = 0; meshData.m_textureImage1 == 0 && i < shapes.size(); i++)
			{
				const bt_tinyobj::shape_t& shape = shapes[i];
				meshData.m_rgbaColor[0] = shape.material.diffuse[0];
				meshData.m_rgbaColor[1] = shape.material.diffuse[1];
				meshData.m_rgbaColor[2] = shape.material.diffuse[2];
				meshData.m_rgbaColor[3] = shape.material.transparency;
				meshData.m_flags |= D3_IMPORT_MESH_HAS_RGBA_COLOR;

				meshData.m_specularColor[0] = shape.material.specular[0];
				meshData.m_specularColor[1] = shape.material.specular[1];
				meshData.m_specularColor[2] = shape.material.specular[2];
				meshData.m_specularColor[3] = 1;
				meshData.m_flags |= D3_IMPORT_MESH_HAS_SPECULAR_COLOR;
				
				if (shape.material.diffuse_texname.length() > 0)
				{
					i32 width, height, n;
					tukk filename = shape.material.diffuse_texname.c_str();
					u8* image = 0;

					tukk prefix[] = {pathPrefix, "./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
					i32 numprefix = sizeof(prefix) / sizeof(tukk);

					for (i32 i = 0; !image && i < numprefix; i++)
					{
						char relativeFileName[1024];
						sprintf(relativeFileName, "%s%s", prefix[i], filename);
						char relativeFileName2[1024];
						if (fileIO->findResourcePath(relativeFileName, relativeFileName2, 1024))
						{
							if (b3IsFileCachingEnabled())
							{
								CachedTextureResult* texture = gCachedTextureResults[relativeFileName];
								if (texture)
								{
									image = texture->m_pixels;
									width = texture->m_width;
									height = texture->m_height;
									meshData.m_textureWidth = width;
									meshData.m_textureHeight = height;
									meshData.m_textureImage1 = image;
									meshData.m_isCached = true;
								}
							}

							if (image == 0)
							{

								b3AlignedObjectArray<char> buffer;
								buffer.reserve(1024);
								i32 fileId = fileIO->fileOpen(relativeFileName,"rb");
								if (fileId>=0)
								{
									i32 size = fileIO->getFileSize(fileId);
									if (size>0)
									{
										buffer.resize(size);
										i32 actual = fileIO->fileRead(fileId,&buffer[0],size);
										if (actual != size)
										{
											drx3DWarning("Несоотвествие размера файла STL!\n");
											buffer.resize(0);
										}
									}
									fileIO->fileClose(fileId);
								}

								if (buffer.size())
								{
									image = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
								}
								//image = stbi_load(relativeFileName, &width, &height, &n, 3);

								meshData.m_textureImage1 = image;

								if (image)
								{
									meshData.m_textureWidth = width;
									meshData.m_textureHeight = height;

									if (b3IsFileCachingEnabled())
									{
										CachedTextureResult result;
										result.m_textureName = relativeFileName;
										result.m_width = width;
										result.m_height = height;
										result.m_pixels = image;
										meshData.m_isCached = true;
										gCachedTextureResults.insert(relativeFileName, result);
									}
								}
								else
								{
									drx3DWarning("Неподдерживаемый формат изображения текстуры [%s]\n", relativeFileName);

									break;
								}
							}
						}
						else
						{
							drx3DWarning("не найден [%s]\n", relativeFileName);
						}
					}
				}
			}
		}
		meshData.m_gfxShape = gfxShape;
		return true;
	}
	else
	{
		drx3DWarning("Cannot find %s\n", fileName.c_str());
	}

	return false;
}
