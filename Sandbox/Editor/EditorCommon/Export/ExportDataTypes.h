// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Objects/BaseObject.h"

#include <DrxMovie/IMovieSystem.h>

#define EXP_NAMESIZE 32
struct IStatObj;

namespace Export
{
struct Vector3D
{
	float x, y, z;
};

struct Quat
{
	Vector3D v;
	float    w;
};

struct UV
{
	float u, v;
};

struct Face
{
	u32 idx[3];
};

struct Color
{
	float r, g, b, a;
};

typedef char TPath[MAX_PATH];

struct Material
{
	Color diffuse;
	Color specular;
	float opacity;
	float smoothness;
	char  name[EXP_NAMESIZE];
	TPath mapDiffuse;
	TPath mapSpecular;
	TPath mapOpacity;
	TPath mapNormals;
	TPath mapDecal;
	TPath mapDisplacement;
};

enum EEntityObjectType
{
	eEntity       = 0,
	eCamera       = 1,
	eCameraTarget = 2,
};

struct EntityAnimData
{
	EAnimParamType dataType;
	float          keyTime;
	float          keyValue;
	float          leftTangent;
	float          rightTangent;
	float          leftTangentWeight;
	float          rightTangentWeight;
};

namespace
{

inline Vector3D Vec3ToVector3D(const Vec3& vec)
{
	Vector3D ret;
	ret.x = vec.x;
	ret.y = vec.y;
	ret.z = vec.z;
	return ret;
}

const float kTangentDelta = 0.01f;
const float kAspectRatio = 1.777778f;
i32k kReserveCount = 7;             // x,y,z,rot_x,rot_y,rot_z,fov
const string kMasterCameraName = "MasterCamera";
}   // namespace
}

struct EDITOR_COMMON_API SExportMesh : public _i_reference_target_t
{
public:
	SExportMesh();

	i32                 GetFaceCount() const  { return m_faces.size(); }
	const Export::Face* GetFaceBuffer() const { return m_faces.size() ? &m_faces[0] : 0; }
	void                SetMaterial(IEditorMaterial* pMtl, CBaseObject* pBaseObj);

	std::vector<Export::Face> m_faces;
	Export::Material          material;
};

struct EDITOR_COMMON_API SExportObject : public _i_reference_target_t
{
public:
	SExportObject(tukk pName);

	i32                           GetVertexCount() const    { return m_vertices.size(); }
	const Export::Vector3D*       GetVertexBuffer() const   { return m_vertices.size() ? &m_vertices[0] : 0; }

	i32                           GetNormalCount() const    { return m_normals.size(); }
	const Export::Vector3D*       GetNormalBuffer() const   { return m_normals.size() ? &m_normals[0] : 0; }

	i32                           GetTexCoordCount() const  { return m_texCoords.size(); }
	const Export::UV*             GetTexCoordBuffer() const { return m_texCoords.size() ? &m_texCoords[0] : 0; }

	i32                           GetColorCount() const     { return m_colors.size(); }
	const Export::Color*          GetColorBuffer() const    { return m_colors.size() ? &m_colors[0] : 0; }

	i32                           GetMeshCount() const      { return m_meshes.size(); }
	SExportMesh*                  GetMesh(i32 index) const  { return m_meshes[index]; }

	size_t                        MeshHash() const          { return m_MeshHash; }

	void                          SetMaterialName(tukk pName);
	void                          SetObjectEntityType(Export::EEntityObjectType type)       { entityType = type; }
	Export::EEntityObjectType     GetObjectEntityType() const                               { return entityType; }
	size_t                        GetEntityAnimationDataCount() const                       { return m_entityAnimData.size(); }
	const Export::EntityAnimData* GetEntityAnimationData(i32 index) const                   { return &m_entityAnimData[index]; }
	void                          AddEntityAnimationData(Export::EntityAnimData entityData) { m_entityAnimData.push_back(entityData); };
	void                          SetLastPtr(CBaseObject* pObject)                          { m_pLastObject = pObject; }
	CBaseObject*                  GetLastObjectPtr()                                        { return m_pLastObject; }

	Export::Quat                         rot;
	Export::Vector3D                     pos;
	Export::Vector3D                     scale;

	CBaseObject*                         m_pLastObject;
	std::vector<Export::Vector3D>        m_vertices;
	std::vector<Export::Vector3D>        m_normals;
	std::vector<Export::UV>              m_texCoords;
	std::vector<Export::Color>           m_colors;
	std::vector<_smart_ptr<SExportMesh>> m_meshes;
	std::vector<Export::EntityAnimData>  m_entityAnimData;

	i32                       nParent;
	size_t                    m_MeshHash;
	bool                      bIsCameraTarget;
	char                      name[EXP_NAMESIZE];
	char                      materialName[EXP_NAMESIZE];
	Export::EEntityObjectType entityType;
	char                      cameraTargetNodeName[EXP_NAMESIZE];
};

struct EDITOR_COMMON_API SExportData
{
	i32            GetObjectCount() const           { return m_objects.size(); }
	SExportObject* GetExportObject(i32 index) const { return m_objects[index]; }
	SExportObject* AddObject(tukk objectName);
	SExportObject* AddObject(SExportObject* pObject);
	void           Clear();

	std::vector<_smart_ptr<SExportObject>> m_objects;
};

