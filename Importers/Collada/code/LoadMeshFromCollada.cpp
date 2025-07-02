#include "../LoadMeshFromCollada.h"
#include <stdio.h>  //fopen
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <string>
#include <X/tinyxml2/tinyxml2.h>
using namespace tinyxml2;


#include <drx3D/Maths/Linear/HashMap.h>
#include <assert.h>
#include <drx3D/Maths/Linear/Matrix4x4.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>

#define MAX_VISUAL_SHAPES 512

struct VertexSource
{
	STxt m_positionArrayId;
	STxt m_normalArrayId;
};

struct TokenFloatArray
{
	AlignedObjectArray<float>& m_values;
	TokenFloatArray(AlignedObjectArray<float>& floatArray)
		: m_values(floatArray)
	{
	}
	inline void add(tukk token)
	{
		float v = atof(token);
		m_values.push_back(v);
	}
};
struct TokenIntArray
{
	AlignedObjectArray<i32>& m_values;
	TokenIntArray(AlignedObjectArray<i32>& intArray)
		: m_values(intArray)
	{
	}
	inline void add(tukk token)
	{
		float v = atoi(token);
		m_values.push_back(v);
	}
};

template <typename AddToken>
void tokenize(const STxt& str, AddToken& tokenAdder, const STxt& delimiters = " \n")
{
	STxt::size_type pos, lastPos = 0;
	while (true)
	{
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == STxt::npos)
		{
			pos = str.length();
			if (pos != lastPos)
			{
				tokenAdder.add(str.data() + lastPos);
			}
			break;
		}
		else
		{
			if (pos != lastPos)
			{
				tokenAdder.add(str.data() + lastPos);
			}
		}
		lastPos = pos + 1;
	}
}

void readFloatArray(XMLElement* source, AlignedObjectArray<float>& floatArray, i32& componentStride)
{
	i32 numVals, stride;
	XMLElement* array = source->FirstChildElement("float_array");
	if (array)
	{
		componentStride = 1;
		if (source->FirstChildElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride", &stride) != XML_NO_ATTRIBUTE)
		{
			componentStride = stride;
		}
		array->QueryIntAttribute("count", &numVals);
		TokenFloatArray adder(floatArray);
		floatArray.reserve(numVals);
		STxt txt = array->GetText();
		tokenize(array->GetText(), adder);
		assert(floatArray.size() == numVals);
	}
}

Vec3 getVector3FromXmlText(tukk text)
{
	Vec3 vec(0, 0, 0);
	AlignedObjectArray<float> floatArray;
	TokenFloatArray adder(floatArray);
	floatArray.reserve(3);
	tokenize(text, adder);
	assert(floatArray.size() == 3);
	if (floatArray.size() == 3)
	{
		vec.setVal(floatArray[0], floatArray[1], floatArray[2]);
	}
	return vec;
}

Vec4 getVector4FromXmlText(tukk text)
{
	Vec4 vec(0, 0, 0, 0);
	AlignedObjectArray<float> floatArray;
	TokenFloatArray adder(floatArray);
	floatArray.reserve(4);
	tokenize(text, adder);
	assert(floatArray.size() == 4);
	if (floatArray.size() == 4)
	{
		vec.setVal(floatArray[0], floatArray[1], floatArray[2], floatArray[3]);
	}
	return vec;
}

void readLibraryGeometries(XMLDocument& doc, AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes, HashMap<HashString, i32>& name2Shape, float extraScaling)
{
	HashMap<HashString, XMLElement*> allSources;
	HashMap<HashString, VertexSource> vertexSources;
	for (XMLElement* geometry = doc.RootElement()->FirstChildElement("library_geometries")->FirstChildElement("geometry");
		 geometry != NULL; geometry = geometry->NextSiblingElement("geometry"))
	{
		AlignedObjectArray<Vec3> vertexPositions;
		AlignedObjectArray<Vec3> vertexNormals;
		AlignedObjectArray<i32> indices;

		tukk geometryName = geometry->Attribute("id");
		for (XMLElement* mesh = geometry->FirstChildElement("mesh"); (mesh != NULL); mesh = mesh->NextSiblingElement("mesh"))
		{
			XMLElement* vertices2 = mesh->FirstChildElement("vertices");

			for (XMLElement* source = mesh->FirstChildElement("source"); source != NULL; source = source->NextSiblingElement("source"))
			{
				tukk srcId = source->Attribute("id");
				//				printf("source id=%s\n",srcId);
				allSources.insert(srcId, source);
			}
			tukk vertexId = vertices2->Attribute("id");
			//printf("vertices id=%s\n",vertexId);
			VertexSource vs;
			for (XMLElement* input = vertices2->FirstChildElement("input"); input != NULL; input = input->NextSiblingElement("input"))
			{
				tukk sem = input->Attribute("semantic");
				STxt semName(sem);
				//					printf("sem=%s\n",sem);
				//		tukk src = input->Attribute("source");
				//					printf("src=%s\n",src);
				tukk srcIdRef = input->Attribute("source");
				STxt source_name;
				source_name = STxt(srcIdRef);
				source_name = source_name.erase(0, 1);
				if (semName == "POSITION")
				{
					vs.m_positionArrayId = source_name;
				}
				if (semName == "NORMAL")
				{
					vs.m_normalArrayId = source_name;
				}
			}
			vertexSources.insert(vertexId, vs);

			AlignedObjectArray<XMLElement*> trianglesAndPolylists;

			for (XMLElement* primitive = mesh->FirstChildElement("triangles"); primitive; primitive = primitive->NextSiblingElement("triangles"))
			{
				trianglesAndPolylists.push_back(primitive);
			}
			for (XMLElement* primitive = mesh->FirstChildElement("polylist"); primitive; primitive = primitive->NextSiblingElement("polylist"))
			{
				trianglesAndPolylists.push_back(primitive);
			}

			for (i32 i = 0; i < trianglesAndPolylists.size(); i++)
			{
				XMLElement* primitive = trianglesAndPolylists[i];
				STxt positionSourceName;
				STxt normalSourceName;
				i32 primitiveCount;
				primitive->QueryIntAttribute("count", &primitiveCount);
				i32 indexStride = 1;
				i32 posOffset = 0;
				i32 normalOffset = 0;
				i32 numIndices = 0;
				{
					for (XMLElement* input = primitive->FirstChildElement("input"); input != NULL; input = input->NextSiblingElement("input"))
					{
						tukk sem = input->Attribute("semantic");
						STxt semName(sem);
						i32 offset = atoi(input->Attribute("offset"));
						if ((offset + 1) > indexStride)
							indexStride = offset + 1;
						//printf("sem=%s\n",sem);
						//	tukk src = input->Attribute("source");

						//printf("src=%s\n",src);
						tukk srcIdRef = input->Attribute("source");
						STxt source_name;
						source_name = STxt(srcIdRef);
						source_name = source_name.erase(0, 1);

						if (semName == "VERTEX")
						{
							//now we have POSITION and possibly NORMAL too, using same index array (<p>)
							VertexSource* vs = vertexSources[source_name.c_str()];
							if (vs->m_positionArrayId.length())
							{
								positionSourceName = vs->m_positionArrayId;
								posOffset = offset;
							}
							if (vs->m_normalArrayId.length())
							{
								normalSourceName = vs->m_normalArrayId;
								normalOffset = offset;
							}
						}
						if (semName == "NORMAL")
						{
							Assert(normalSourceName.length() == 0);
							normalSourceName = source_name;
							normalOffset = offset;
						}
					}
					numIndices = primitiveCount * 3;
				}
				AlignedObjectArray<float> positionFloatArray;
				i32 posStride = 1;
				XMLElement** sourcePtr = allSources[positionSourceName.c_str()];
				if (sourcePtr)
				{
					readFloatArray(*sourcePtr, positionFloatArray, posStride);
				}
				AlignedObjectArray<float> normalFloatArray;
				i32 normalStride = 1;
				sourcePtr = allSources[normalSourceName.c_str()];
				if (sourcePtr)
				{
					readFloatArray(*sourcePtr, normalFloatArray, normalStride);
				}
				AlignedObjectArray<i32> curIndices;
				curIndices.reserve(numIndices * indexStride);
				TokenIntArray adder(curIndices);
				STxt txt = primitive->FirstChildElement("p")->GetText();
				tokenize(txt, adder);
				assert(curIndices.size() == numIndices * indexStride);
				i32 indexOffset = vertexPositions.size();

				for (i32 index = 0; index < numIndices; index++)
				{
					i32 posIndex = curIndices[index * indexStride + posOffset];
					i32 normalIndex = curIndices[index * indexStride + normalOffset];
					vertexPositions.push_back(Vec3(extraScaling * positionFloatArray[posIndex * 3 + 0],
														extraScaling * positionFloatArray[posIndex * 3 + 1],
														extraScaling * positionFloatArray[posIndex * 3 + 2]));

					if (normalFloatArray.size() && (normalFloatArray.size() > normalIndex))
					{
						vertexNormals.push_back(Vec3(normalFloatArray[normalIndex * 3 + 0],
														  normalFloatArray[normalIndex * 3 + 1],
														  normalFloatArray[normalIndex * 3 + 2]));
					}
					else
					{
						//add a dummy normal of length zero, so it is easy to detect that it is an invalid normal
						vertexNormals.push_back(Vec3(0, 0, 0));
					}
				}
				i32 curNumIndices = indices.size();
				indices.resize(curNumIndices + numIndices);
				for (i32 index = 0; index < numIndices; index++)
				{
					indices[curNumIndices + index] = index + indexOffset;
				}
			}  //if(primitive != NULL)
		}      //for each mesh

		i32 shapeIndex = visualShapes.size();
		if (shapeIndex < MAX_VISUAL_SHAPES)
		{
			GLInstanceGraphicsShape& visualShape = visualShapes.expand();
			{
				visualShape.m_vertices = new b3AlignedObjectArray<GLInstanceVertex>;
				visualShape.m_indices = new b3AlignedObjectArray<i32>;
				i32 indexBase = 0;

				Assert(vertexNormals.size() == vertexPositions.size());
				for (i32 v = 0; v < vertexPositions.size(); v++)
				{
					GLInstanceVertex vtx;
					vtx.xyzw[0] = vertexPositions[v].x();
					vtx.xyzw[1] = vertexPositions[v].y();
					vtx.xyzw[2] = vertexPositions[v].z();
					vtx.xyzw[3] = 1.f;
					vtx.normal[0] = vertexNormals[v].x();
					vtx.normal[1] = vertexNormals[v].y();
					vtx.normal[2] = vertexNormals[v].z();
					vtx.uv[0] = 0.5f;
					vtx.uv[1] = 0.5f;
					visualShape.m_vertices->push_back(vtx);
				}

				for (i32 index = 0; index < indices.size(); index++)
				{
					visualShape.m_indices->push_back(indices[index] + indexBase);
				}

				//drx3DPrintf(" index_count =%dand vertexPositions.size=%d\n",indices.size(), vertexPositions.size());
				indexBase = visualShape.m_vertices->size();
				visualShape.m_numIndices = visualShape.m_indices->size();
				visualShape.m_numvertices = visualShape.m_vertices->size();
			}
			//drx3DPrintf("geometry name=%s\n",geometryName);
			name2Shape.insert(geometryName, shapeIndex);
		}
		else
		{
			drx3DWarning("DAE exceeds number of visual shapes (%d/%d)", shapeIndex, MAX_VISUAL_SHAPES);
		}

	}  //for each geometry
}

void readNodeHierarchy(XMLElement* node, HashMap<HashString, i32>& name2Shape, AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances, const Matrix4x4& parentTransMat)
{
	Matrix4x4 nodeTrans;
	nodeTrans.setIdentity();

	///todo(erwincoumans) we probably have to read the elements 'translate', 'scale', 'rotate' and 'matrix' in-order and accumulate them...
	{
		for (XMLElement* transElem = node->FirstChildElement("matrix"); transElem; transElem = node->NextSiblingElement("matrix"))
		{
			if (transElem->GetText())
			{
				AlignedObjectArray<float> floatArray;
				TokenFloatArray adder(floatArray);
				tokenize(transElem->GetText(), adder);
				if (floatArray.size() == 16)
				{
					Matrix4x4 t(floatArray[0], floatArray[1], floatArray[2], floatArray[3],
								  floatArray[4], floatArray[5], floatArray[6], floatArray[7],
								  floatArray[8], floatArray[9], floatArray[10], floatArray[11],
								  floatArray[12], floatArray[13], floatArray[14], floatArray[15]);

					nodeTrans = nodeTrans * t;
				}
				else
				{
					drx3DWarning("Ошибка: expected 16 elements in a <matrix> element, skipping\n");
				}
			}
		}
	}

	{
		for (XMLElement* transElem = node->FirstChildElement("translate"); transElem; transElem = node->NextSiblingElement("translate"))
		{
			if (transElem->GetText())
			{
				Vec3 pos = getVector3FromXmlText(transElem->GetText());
				//nodePos+= unitScaling*parentScaling*pos;
				Matrix4x4 t;
				t.setPureTranslation(pos);
				nodeTrans = nodeTrans * t;
			}
		}
	}
	{
		for (XMLElement* scaleElem = node->FirstChildElement("scale");
			 scaleElem != NULL; scaleElem = node->NextSiblingElement("scale"))
		{
			if (scaleElem->GetText())
			{
				Vec3 scaling = getVector3FromXmlText(scaleElem->GetText());
				Matrix4x4 t;
				t.setPureScaling(scaling);
				nodeTrans = nodeTrans * t;
			}
		}
	}
	{
		for (XMLElement* rotateElem = node->FirstChildElement("rotate");
			 rotateElem != NULL; rotateElem = node->NextSiblingElement("rotate"))
		{
			if (rotateElem->GetText())
			{
				//accumulate orientation
				Vec4 rotate = getVector4FromXmlText(rotateElem->GetText());
				Quat orn(Vec3(rotate), Radians(rotate[3]));  //COLLADA DAE rotate is in degrees, convert to radians
				Matrix4x4 t;
				t.setPureRotation(orn);
				nodeTrans = nodeTrans * t;
			}
		}
	}

	nodeTrans = parentTransMat * nodeTrans;

	for (XMLElement* instanceGeom = node->FirstChildElement("instance_geometry");
		 instanceGeom != 0;
		 instanceGeom = instanceGeom->NextSiblingElement("instance_geometry"))
	{
		tukk geomUrl = instanceGeom->Attribute("url");
		//printf("node referring to geom %s\n", geomUrl);
		geomUrl++;
		i32* shapeIndexPtr = name2Shape[geomUrl];
		if (shapeIndexPtr)
		{
			//	i32 index = *shapeIndexPtr;
			//printf("found geom with index %d\n", *shapeIndexPtr);
			ColladaGraphicsInstance& instance = visualShapeInstances.expand();
			instance.m_shapeIndex = *shapeIndexPtr;
			instance.m_worldTransform = nodeTrans;
		}
		else
		{
			drx3DWarning("geom not found\n");
		}
	}

	for (XMLElement* childNode = node->FirstChildElement("node");
		 childNode != NULL; childNode = childNode->NextSiblingElement("node"))
	{
		readNodeHierarchy(childNode, name2Shape, visualShapeInstances, nodeTrans);
	}
}
void readVisualSceneInstanceGeometries(XMLDocument& doc, HashMap<HashString, i32>& name2Shape, AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances)
{
	HashMap<HashString, XMLElement*> allVisualScenes;

	XMLElement* libVisualScenes = doc.RootElement()->FirstChildElement("library_visual_scenes");
	if (libVisualScenes == 0)
		return;

	{
		for (XMLElement* scene = libVisualScenes->FirstChildElement("visual_scene");
			 scene != NULL; scene = scene->NextSiblingElement("visual_scene"))
		{
			tukk sceneName = scene->Attribute("id");
			allVisualScenes.insert(sceneName, scene);
		}
	}

	XMLElement* scene = 0;
	{
		XMLElement* scenes = doc.RootElement()->FirstChildElement("scene");
		if (scenes)
		{
			XMLElement* instanceSceneReference = scenes->FirstChildElement("instance_visual_scene");
			if (instanceSceneReference)
			{
				tukk instanceSceneUrl = instanceSceneReference->Attribute("url");
				XMLElement** sceneInstancePtr = allVisualScenes[instanceSceneUrl + 1];  //skip #
				if (sceneInstancePtr)
				{
					scene = *sceneInstancePtr;
				}
			}
		}
	}

	if (scene)
	{
		for (XMLElement* node = scene->FirstChildElement("node");
			 node != NULL; node = node->NextSiblingElement("node"))
		{
			Matrix4x4 identity;
			identity.setIdentity();
			Vec3 identScaling(1, 1, 1);
			readNodeHierarchy(node, name2Shape, visualShapeInstances, identity);
		}
	}
}

void getUnitMeterScalingAndUpAxisTransform(XMLDocument& doc, Transform2& tr, float& unitMeterScaling, i32 clientUpAxis)
{
	///todo(erwincoumans) those up-axis transformations have been quickly coded without rigorous testing

	XMLElement* unitMeter = doc.RootElement()->FirstChildElement("asset")->FirstChildElement("unit");
	if (unitMeter)
	{
		tukk meterText = unitMeter->Attribute("meter");
		//printf("meterText=%s\n", meterText);
		unitMeterScaling = atof(meterText);
	}

	XMLElement* upAxisElem = doc.RootElement()->FirstChildElement("asset")->FirstChildElement("up_axis");
	if (upAxisElem)
	{
		switch (clientUpAxis)
		{
			case 1:
			{
				STxt upAxisTxt = upAxisElem->GetText();
				if (upAxisTxt == "X_UP")
				{
					Quat x2y(Vec3(0, 0, 1), SIMD_HALF_PI);
					tr.setRotation(x2y);
				}
				if (upAxisTxt == "Y_UP")
				{
					//assume Y_UP for now, to be compatible with assimp?
					//client and COLLADA are both Z_UP so no transform needed (identity)
				}
				if (upAxisTxt == "Z_UP")
				{
					Quat z2y(Vec3(1, 0, 0), -SIMD_HALF_PI);
					tr.setRotation(z2y);
				}
				break;
			}
			case 2:
			{
				STxt upAxisTxt = upAxisElem->GetText();
				if (upAxisTxt == "X_UP")
				{
					Quat x2z(Vec3(0, 1, 0), -SIMD_HALF_PI);
					tr.setRotation(x2z);
				}
				if (upAxisTxt == "Y_UP")
				{
					Quat y2z(Vec3(1, 0, 0), SIMD_HALF_PI);
					tr.setRotation(y2z);
				}
				if (upAxisTxt == "Z_UP")
				{
					//client and COLLADA are both Z_UP so no transform needed (identity)
				}
				break;
			}
			case 0:
			default:
			{
				//we don't support X or other up axis
				Assert(0);
			}
		};
	}
}

void LoadMeshFromCollada(tukk relativeFileName, AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes, AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances, Transform2& upAxisTransform, float& unitMeterScaling, i32 clientUpAxis, struct CommonFileIOInterface* fileIO)
{
	//	GLInstanceGraphicsShape* instance = 0;

	//usually COLLADA files don't have that many visual geometries/shapes
	visualShapes.reserve(MAX_VISUAL_SHAPES);

	float extraScaling = 1;  //0.01;
	HashMap<HashString, i32> name2ShapeIndex;
	
	char filename[1024];
	if (!fileIO->findResourcePath(relativeFileName, filename, 1024))
	{
		drx3DWarning("File not found: %s\n", filename);
		return;
	}

	XMLDocument doc;
	//doc.Parse((tukk)filedata, 0, TIXML_ENCODING_UTF8);
	b3AlignedObjectArray<char> xmlString;
	i32 fileHandle = fileIO->fileOpen(filename,"r");
	if (fileHandle>=0)
	{
		i32 size = fileIO->getFileSize(fileHandle);
		xmlString.resize(size);
		i32 actual = fileIO->fileRead(fileHandle, &xmlString[0],size);
		if (actual==size)
		{
		}
		fileIO->fileClose(fileHandle);
	}
	if (xmlString.size()==0)
		return;

	if (doc.Parse(&xmlString[0], xmlString.size()) != XML_SUCCESS)
	//if (doc.LoadFile(filename) != XML_SUCCESS)
		return;

	//We need units to be in meter, so apply a scaling using the asset/units meter
	unitMeterScaling = 1;
	upAxisTransform.setIdentity();

	//Also we can optionally compensate all transforms using the asset/up_axis as well as unit meter scaling
	getUnitMeterScalingAndUpAxisTransform(doc, upAxisTransform, unitMeterScaling, clientUpAxis);

	Matrix4x4 ident;
	ident.setIdentity();

	readLibraryGeometries(doc, visualShapes, name2ShapeIndex, extraScaling);

	readVisualSceneInstanceGeometries(doc, name2ShapeIndex, visualShapeInstances);
}

#ifdef COMPARE_WITH_ASSIMP

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "assimp/ColladaLoader.h"
//#	include "STLLoader.h"
#include "assimp/SortByPTypeProcess.h"
#include "assimp/LimitBoneWeightsProcess.h"
#include "assimp/TriangulateProcess.h"
#include "assimp/JoinVerticesProcess.h"
#include "assimp/RemoveVCProcess.h"

namespace Assimp
{
// ------------------------------------------------------------------------------------------------
void GetImporterInstanceList(std::vector<BaseImporter*>& out)
{
	out.push_back(new ColladaLoader());
}
// ------------------------------------------------------------------------------------------------
void GetPostProcessingStepInstanceList(std::vector<BaseProcess*>& out)
{
	out.push_back(new SortByPTypeProcess());
	out.push_back(new LimitBoneWeightsProcess());
	out.push_back(new TriangulateProcess());
	out.push_back(new JoinVerticesProcess());
	//out.push_back( new RemoveVCProcess());
}

}  // namespace Assimp

static void addMeshParts(const aiScene* scene, const aiNode* node, GLInstanceGraphicsShape* outverts, const aiMatrix4x4& parentTr)
{
	aiMatrix4x4 const& nodeTrans(node->mTransformation);

	aiMatrix4x4 trans;
	trans = parentTr * nodeTrans;

	for (size_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];
		size_t num_vertices = mesh->mNumVertices;
		if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			i32 curVertexBase = outverts->m_vertices->size();

			for (i32 v = 0; v < mesh->mNumVertices; v++)
			{
				GLInstanceVertex vtx;
				aiVector3D vWorld = trans * mesh->mVertices[v];
				vtx.xyzw[0] = vWorld.x;
				vtx.xyzw[1] = vWorld.y;
				vtx.xyzw[2] = vWorld.z;
				vtx.xyzw[3] = 1;
				if (mesh->HasNormals())
				{
					vtx.normal[0] = mesh->mNormals[v].x;
					vtx.normal[1] = mesh->mNormals[v].y;
					vtx.normal[2] = mesh->mNormals[v].z;
				}
				else
				{
					vtx.normal[0] = 0;
					vtx.normal[1] = 0;
					vtx.normal[2] = 1;
				}
				if (mesh->HasTextureCoords(0))
				{
					vtx.uv[0] = mesh->mTextureCoords[0][v].x;
					vtx.uv[1] = mesh->mTextureCoords[0][v].y;
				}
				else
				{
					vtx.uv[0] = 0.5f;
					vtx.uv[1] = 0.5f;
				}
				outverts->m_vertices->push_back(vtx);
			}
			for (i32 f = 0; f < mesh->mNumFaces; f++)
			{
				drx3DAssert(mesh->mFaces[f].mNumIndices == 3);
				i32 i0 = mesh->mFaces[f].mIndices[0];
				i32 i1 = mesh->mFaces[f].mIndices[1];
				i32 i2 = mesh->mFaces[f].mIndices[2];
				outverts->m_indices->push_back(i0 + curVertexBase);
				outverts->m_indices->push_back(i1 + curVertexBase);
				outverts->m_indices->push_back(i2 + curVertexBase);
			}
		}
	}
	for (size_t i = 0; i < node->mNumChildren; ++i)
	{
		addMeshParts(scene, node->mChildren[i], outverts, trans);
	}
}

void LoadMeshFromColladaAssimp(tukk relativeFileName, AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes, AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances, Transform2& upAxisTrans, float& unitMeterScaling)
{
	upAxisTrans.setIdentity();
	unitMeterScaling = 1;

	GLInstanceGraphicsShape* shape = 0;

	FILE* file = fopen(relativeFileName, "rb");
	if (file)
	{
		i32 size = 0;
		if (fseek(file, 0, SEEK_END) || (size = ftell(file)) == EOF || fseek(file, 0, SEEK_SET))
		{
			drx3DWarning("Ошибка: Cannot access file to determine size of %s\n", relativeFileName);
		}
		else
		{
			if (size)
			{
				//printf("Open DAE file of %d bytes\n",size);

				Assimp::Importer importer;
				//importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_COLORS);
				importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
				//	importer.SetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, 1);
				aiScene const* scene = importer.ReadFile(relativeFileName,
														 aiProcess_JoinIdenticalVertices |
															 //aiProcess_RemoveComponent |
															 aiProcess_SortByPType |
															 aiProcess_Triangulate);
				if (scene)
				{
					shape = &visualShapes.expand();
					shape->m_scaling[0] = 1;
					shape->m_scaling[1] = 1;
					shape->m_scaling[2] = 1;
					shape->m_scaling[3] = 1;
					i32 index = 0;
					shape->m_indices = new b3AlignedObjectArray<i32>();
					shape->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

					aiMatrix4x4 ident;
					addMeshParts(scene, scene->mRootNode, shape, ident);
					shape->m_numIndices = shape->m_indices->size();
					shape->m_numvertices = shape->m_vertices->size();
					ColladaGraphicsInstance& instance = visualShapeInstances.expand();
					instance.m_shapeIndex = visualShapes.size() - 1;
				}
			}
		}
	}
}

#endif  //COMPARE_WITH_ASSIMP
