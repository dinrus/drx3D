/// obj2sdf will load a Wavefront .obj file that may contain many parts/materials
/// it will split into separate obj files for each part/material and
/// create an sdf file with visuals/collisions pointing to the new obj files
/// this will make it easier to load complex obj files into pybullet
/// see for example export in data/kitchens/fathirmutfak.sdf

///Bullet Continuous Collision Detection and Physics Library
///Erwin Coumans (C) 2018
///http://bulletphysics.org
///
///This software is provided 'as-is', without any express or implied warranty.
///In no event will the authors be held liable for any damages arising from the use of this software.
///Permission is granted to anyone to use this software for any purpose,
///including commercial applications, and to alter it and redistribute it freely,
///subject to the following restrictions:
///
///1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
///2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
///3. This notice may not be removed or altered from any source distribution.
	
#include <string.h>
#include <stdio.h>
#include <assert.h>
#define ASSERT_EQ(a, b) assert((a) == (b));
#include <drx3D/Wavefront/tiny_obj_loader.h>
#include <vector>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/Common/b3HashMap.h>
#include <drx3D/Common/DefaultFileIO.h>

using bt_tinyobj::index_t;

struct ShapeContainer
{
	STxt m_matName;
	STxt m_shapeName;
	bt_tinyobj::material_t material;
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<index_t> indices;
	b3AlignedObjectArray<i32> m_shapeIndices;
};

b3HashMap<b3HashString, ShapeContainer> gMaterialNames;

#define MAX_PATH_LEN 1024

STxt StripExtension(const STxt& sPath)
{
	for (STxt::const_reverse_iterator i = sPath.rbegin(); i != sPath.rend(); i++)
	{
		if (*i == '.')
		{
			return STxt(sPath.begin(), i.base() - 1);
		}

		// if we find a slash there is no extension
		if (*i == '\\' || *i == '/')
			break;
	}

	// we didn't find an extension
	return sPath;
}

i32 main(i32 argc, tuk argv[])
{
	b3CommandLineArgs args(argc, argv);
	tuk fileName;
	args.GetCmdLineArgument("fileName", fileName);
	if (fileName == 0)
	{
		printf("required --fileName=\"name\"");
		exit(0);
	}
	STxt matLibName = StripExtension(fileName);

	printf("fileName = %s\n", fileName);
	if (fileName == 0)
	{
		printf("Please use --fileName=\"pathToObj\".");
		exit(0);
	}
	bool mergeMaterials = args.CheckCmdLineFlag("mergeMaterials");

	char fileNameWithPath[MAX_PATH_LEN];
	bool fileFound = (ResourcePath::findResourcePath(fileName, fileNameWithPath, MAX_PATH_LEN,0)) > 0;
	char materialPrefixPath[MAX_PATH_LEN];
	b3FileUtils::extractPath(fileNameWithPath, materialPrefixPath, MAX_PATH_LEN);

	std::vector<bt_tinyobj::shape_t> shapes;
	bt_tinyobj::attrib_t attribute;

	DefaultFileIO fileIO;
	STxt err = bt_tinyobj::LoadObj(attribute, shapes, fileNameWithPath, materialPrefixPath, &fileIO);

	char sdfFileName[MAX_PATH_LEN];
	sprintf(sdfFileName, "%s%s.sdf", materialPrefixPath, "newsdf");
	FILE* sdfFile = fopen(sdfFileName, "w");
	if (sdfFile == 0)
	{
		printf("Фатальная ошибка: не удаётся создать файл sdf %s\n", sdfFileName);
		exit(0);
	}

	fprintf(sdfFile, "<sdf version='1.6'>\n\t<world name='default'>\n\t<gravity>0 0 -9.8</gravity>\n");

	for (i32 s = 0; s < (i32)shapes.size(); s++)
	{
		bt_tinyobj::shape_t& shape = shapes[s];
		bt_tinyobj::material_t mat = shape.material;

		b3HashString key = mat.name.length() ? mat.name.c_str() : "";
		if (!gMaterialNames.find(key))
		{
			ShapeContainer container;
			container.m_matName = mat.name;
			container.m_shapeName = shape.name;
			container.material = mat;
			gMaterialNames.insert(key, container);
		}

		ShapeContainer* shapeC = gMaterialNames.find(key);
		if (shapeC)
		{
			shapeC->m_shapeIndices.push_back(s);

			i32 curPositions = shapeC->positions.size() / 3;
			i32 curNormals = shapeC->normals.size() / 3;
			i32 curTexcoords = shapeC->texcoords.size() / 2;

			i32 faceCount = shape.mesh.indices.size();
			i32 vertexCount = attribute.vertices.size();
			for (i32 v = 0; v < vertexCount; v++)
			{
				shapeC->positions.push_back(attribute.vertices[v]);
			}
			i32 numNormals = i32(attribute.normals.size());
			for (i32 vn = 0; vn < numNormals; vn++)
			{
				shapeC->normals.push_back(attribute.normals[vn]);
			}
			i32 numTexCoords = i32(attribute.texcoords.size());
			for (i32 vt = 0; vt < numTexCoords; vt++)
			{
				shapeC->texcoords.push_back(attribute.texcoords[vt]);
			}

			for (i32 face = 0; face < faceCount; face += 3)
			{
				if (face < 0 && face >= i32(shape.mesh.indices.size()))
				{
					continue;
				}

				index_t index;
				for (i32 ii = 0; ii < 3; ii++)
				{
					index.vertex_index = shape.mesh.indices[face + ii].vertex_index + curPositions;
					index.normal_index = shape.mesh.indices[face + ii].normal_index + curNormals;
					index.texcoord_index = shape.mesh.indices[face + ii].texcoord_index + curTexcoords;
					shapeC->indices.push_back(index);
				}
			}
		}
	}

	printf("unique materials=%d\n", gMaterialNames.size());

	if (mergeMaterials)
	{
		for (i32 m = 0; m < gMaterialNames.size(); m++)
		{
			if (gMaterialNames.getAtIndex(m)->m_shapeIndices.size() == 0)
				continue;

			ShapeContainer* shapeCon = gMaterialNames.getAtIndex(m);

			printf("object name = %s\n", shapeCon->m_shapeName.c_str());

			char objSdfPartFileName[MAX_PATH_LEN];
			sprintf(objSdfPartFileName, "part%d.obj", m);

			char objFileName[MAX_PATH_LEN];
			if (strlen(materialPrefixPath) > 0)
			{
				sprintf(objFileName, "%s/part%d.obj", materialPrefixPath, m);
			}
			else
			{
				sprintf(objFileName, "part%d.obj", m);
			}

			FILE* f = fopen(objFileName, "w");
			if (f == 0)
			{
				printf("Fatal error: cannot create part obj file %s\n", objFileName);
				exit(0);
			}
			fprintf(f, "# Exported using automatic converter by Erwin Coumans\n");
			if (matLibName.length())
			{
				fprintf(f, "mtllib %s.mtl\n", matLibName.c_str());
			}
			else
			{
				fprintf(f, "mtllib bedroom.mtl\n");
			}

			i32 faceCount = shapeCon->indices.size();
			i32 vertexCount = shapeCon->positions.size();
			bt_tinyobj::material_t mat = shapeCon->material;
			if (shapeCon->m_matName.length())
			{
				tukk objName = shapeCon->m_matName.c_str();
				printf("mat.name = %s\n", objName);
				fprintf(f, "#object %s\n\n", objName);
			}
			for (i32 v = 0; v < vertexCount / 3; v++)
			{
				fprintf(f, "v %f %f %f\n", shapeCon->positions[v * 3 + 0], shapeCon->positions[v * 3 + 1], shapeCon->positions[v * 3 + 2]);
			}

			if (mat.name.length())
			{
				fprintf(f, "usemtl %s\n", mat.name.c_str());
			}
			else
			{
				fprintf(f, "usemtl wire_028089177\n");
			}

			fprintf(f, "\n");
			i32 numNormals = i32(shapeCon->normals.size());

			for (i32 vn = 0; vn < numNormals / 3; vn++)
			{
				fprintf(f, "vn %f %f %f\n", shapeCon->normals[vn * 3 + 0], shapeCon->normals[vn * 3 + 1], shapeCon->normals[vn * 3 + 2]);
			}

			fprintf(f, "\n");
			i32 numTexCoords = i32(shapeCon->texcoords.size());
			for (i32 vt = 0; vt < numTexCoords / 2; vt++)
			{
				fprintf(f, "vt %f %f\n", shapeCon->texcoords[vt * 2 + 0], shapeCon->texcoords[vt * 2 + 1]);
			}

			fprintf(f, "s off\n");

			for (i32 face = 0; face < faceCount; face += 3)
			{
				if (face < 0 && face >= i32(shapeCon->indices.size()))
				{
					continue;
				}
				fprintf(f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						shapeCon->indices[face].vertex_index + 1, shapeCon->indices[face].texcoord_index + 1, shapeCon->indices[face].normal_index + 1,
						shapeCon->indices[face + 1].vertex_index + 1, shapeCon->indices[face + 1].texcoord_index + 1, shapeCon->indices[face + 1].normal_index + 1,
						shapeCon->indices[face + 2].vertex_index + 1, shapeCon->indices[face + 2].texcoord_index + 1, shapeCon->indices[face + 2].normal_index + 1);
			}
			fclose(f);

			float kdRed = mat.diffuse[0];
			float kdGreen = mat.diffuse[1];
			float kdBlue = mat.diffuse[2];
			float transparency = mat.transparency;

			fprintf(sdfFile,
					"\t\t<model name='%s'>\n"
					"\t\t\t<static>1</static>\n"
					"\t\t\t<pose >0 0 0 0 0 0</pose>\n"
					"\t\t\t<link name='link_d%d'>\n"
					"\t\t\t<inertial>\n"
					"\t\t\t<mass>0</mass>\n"
					"\t\t\t<inertia>\n"
					"\t\t\t<ixx>0.166667</ixx>\n"
					"\t\t\t<ixy>0</ixy>\n"
					"\t\t\t<ixz>0</ixz>\n"
					"\t\t\t<iyy>0.166667</iyy>\n"
					"\t\t\t<iyz>0</iyz>\n"
					"\t\t\t<izz>0.166667</izz>\n"
					"\t\t\t</inertia>\n"
					"\t\t\t</inertial>\n"
					"\t\t\t<collision concave='yes' name='collision_%d'>\n"
					"\t\t\t<geometry>\n"
					"\t\t\t<mesh>\n"
					"\t\t\t<scale>1 1 1</scale>\n"
					"\t\t\t\t<uri>%s</uri>\n"
					"\t\t\t</mesh>\n"
					"\t\t\t</geometry>\n"
					"\t\t\t  </collision>\n"
					"\t\t\t<visual name='visual'>\n"
					"\t\t\t\t<geometry>\n"
					"\t\t\t\t<mesh>\n"
					"\t\t\t\t\t<scale>1 1 1</scale>\n"
					"\t\t\t\t\t<uri>%s</uri>\n"
					"\t\t\t\t</mesh>\n"
					"\t\t\t\t</geometry>\n"
					"\t\t\t<material>\n"
					"\t\t\t\t<ambient>1 0 0 1</ambient>\n"
					"\t\t\t\t<diffuse>%f %f %f %f</diffuse>\n"
					"\t\t\t\t<specular>0.1 0.1 0.1 1</specular>\n"
					"\t\t\t\t<emissive>0 0 0 0</emissive>\n"
					"\t\t\t </material>\n"
					"\t\t\t </visual>\n"
					"\t\t\t </link>\n"
					"\t\t\t</model>\n",
					objSdfPartFileName, m, m,
					objSdfPartFileName, objSdfPartFileName,
					kdRed, kdGreen, kdBlue, transparency);
		}
	}
	else
	{
		for (i32 s = 0; s < (i32)shapes.size(); s++)
		{
			bt_tinyobj::shape_t& shape = shapes[s];

			if (shape.name.length())
			{
				printf("object name = %s\n", shape.name.c_str());
			}

			char objFileName[MAX_PATH_LEN];
			if (strlen(materialPrefixPath) > 0)
			{
				sprintf(objFileName, "%s/part%d.obj", materialPrefixPath, s);
			}
			else
			{
				sprintf(objFileName, "part%d.obj", s);
			}
			FILE* f = fopen(objFileName, "w");
			if (f == 0)
			{
				printf("Fatal error: cannot create part obj file %s\n", objFileName);
				exit(0);
			}
			fprintf(f, "# Exported using automatic converter by Erwin Coumans\n");
			if (matLibName.length())
			{
				fprintf(f, "mtllib %s.mtl\n", matLibName.c_str());
			}
			else
			{
				fprintf(f, "mtllib bedroom.mtl\n");
			}

			i32 faceCount = shape.mesh.indices.size();
			i32 vertexCount = attribute.vertices.size();
			bt_tinyobj::material_t mat = shape.material;
			if (shape.name.length())
			{
				tukk objName = shape.name.c_str();
				printf("mat.name = %s\n", objName);
				fprintf(f, "#object %s\n\n", objName);
			}
			for (i32 v = 0; v < vertexCount / 3; v++)
			{
				fprintf(f, "v %f %f %f\n", attribute.vertices[v * 3 + 0], attribute.vertices[v * 3 + 1], attribute.vertices[v * 3 + 2]);
			}

			if (mat.name.length())
			{
				fprintf(f, "usemtl %s\n", mat.name.c_str());
			}
			else
			{
				fprintf(f, "usemtl wire_028089177\n");
			}

			fprintf(f, "\n");
			i32 numNormals = i32(attribute.normals.size());

			for (i32 vn = 0; vn < numNormals / 3; vn++)
			{
				fprintf(f, "vn %f %f %f\n", attribute.normals[vn * 3 + 0], attribute.normals[vn * 3 + 1], attribute.normals[vn * 3 + 2]);
			}

			fprintf(f, "\n");
			i32 numTexCoords = i32(attribute.texcoords.size());
			for (i32 vt = 0; vt < numTexCoords / 2; vt++)
			{
				fprintf(f, "vt %f %f\n", attribute.texcoords[vt * 2 + 0], attribute.texcoords[vt * 2 + 1]);
			}

			fprintf(f, "s off\n");

			for (i32 face = 0; face < faceCount; face += 3)
			{
				if (face < 0 && face >= i32(shape.mesh.indices.size()))
				{
					continue;
				}
				fprintf(f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                        shape.mesh.indices[face].vertex_index + 1,     shape.mesh.indices[face].texcoord_index + 1,     shape.mesh.indices[face].normal_index + 1,
                                        shape.mesh.indices[face + 1].vertex_index + 1, shape.mesh.indices[face + 1].texcoord_index + 1, shape.mesh.indices[face + 1].normal_index + 1,
                                        shape.mesh.indices[face + 2].vertex_index + 1, shape.mesh.indices[face + 2].texcoord_index + 1, shape.mesh.indices[face + 2].normal_index + 1);
			}
                        fclose(f);

			float kdRed = mat.diffuse[0];
			float kdGreen = mat.diffuse[1];
			float kdBlue = mat.diffuse[2];
			float transparency = mat.transparency;
			char objSdfPartFileName[MAX_PATH_LEN];
			sprintf(objSdfPartFileName, "part%d.obj", s);
			fprintf(sdfFile,
					"\t\t<model name='%s'>\n"
					"\t\t\t<static>1</static>\n"
					"\t\t\t<pose>0 0 0 0 0 0</pose>\n"
					"\t\t\t<link name='link_d%d'>\n"
					"\t\t\t<inertial>\n"
					"\t\t\t<mass>0</mass>\n"
					"\t\t\t<inertia>\n"
					"\t\t\t<ixx>0.166667</ixx>\n"
					"\t\t\t<ixy>0</ixy>\n"
					"\t\t\t<ixz>0</ixz>\n"
					"\t\t\t<iyy>0.166667</iyy>\n"
					"\t\t\t<iyz>0</iyz>\n"
					"\t\t\t<izz>0.166667</izz>\n"
					"\t\t\t</inertia>\n"
					"\t\t\t</inertial>\n"
					"\t\t\t<collision name='collision_%d'>\n"
					"\t\t\t<geometry>\n"
					"\t\t\t<mesh>\n"
					"\t\t\t<scale>1 1 1</scale>\n"
					"\t\t\t\t<uri>%s</uri>\n"
					"\t\t\t</mesh>\n"
					"\t\t\t</geometry>\n"
					"\t\t\t  </collision>\n"
					"\t\t\t<visual name='visual'>\n"
					"\t\t\t\t<geometry>\n"
					"\t\t\t\t<mesh>\n"
					"\t\t\t\t\t<scale>1 1 1</scale>\n"
					"\t\t\t\t\t<uri>%s</uri>\n"
					"\t\t\t\t</mesh>\n"
					"\t\t\t\t</geometry>\n"
					"\t\t\t<material>\n"
					"\t\t\t\t<ambient>1 0 0 1</ambient>\n"
					"\t\t\t\t<diffuse>%f %f %f %f</diffuse>\n"
					"\t\t\t\t<specular>0.1 0.1 0.1 1</specular>\n"
					"\t\t\t\t<emissive>0 0 0 0</emissive>\n"
					"\t\t\t </material>\n"
					"\t\t\t </visual>\n"
					"\t\t\t </link>\n"
					"\t\t\t</model>\n",
					objSdfPartFileName, s, s,
					objSdfPartFileName, objSdfPartFileName,
					kdRed, kdGreen, kdBlue, transparency);
		}
	}
	fprintf(sdfFile, "\t</world>\n</sdf>\n");

	fclose(sdfFile);

	return 0;
}
