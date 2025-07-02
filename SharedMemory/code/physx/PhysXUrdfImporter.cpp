#include "PhysXUrdfImporter.h"
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include "../../ThirdPartyLibs/Wavefront/tiny_obj_loader.h"
#include "../../Importers/ImportURDFDemo/URDFImporterInterface.h"

#include "../../Importers/ImportObjDemo/LoadMeshFromObj.h"
#include "../../Importers/ImportSTLDemo/LoadMeshFromSTL.h"
#include "../../Importers/ImportColladaDemo/LoadMeshFromCollada.h"
//#include <drx3D/Physics/Collision/Shapes/ShapeHull.h"  //to create a tesselation of a generic ConvexShape
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Common/b3FileUtils.h>
#include <string>
#include "../../Utils/ResourcePath.h"
#include "../../Utils/DefaultFileIO.h"

#include "../OpenGLWindow/ShapeData.h"


#include "../../Importers/ImportMeshUtility/b3ImportMeshUtility.h"

static Scalar gUrdfDefaultCollisionMargin = 0.001;

#include <iostream>
#include <fstream>
#include <list>
#include "../../Importers/ImportURDFDemo/URDFJointTypes.h"
#include "../../Importers/ImportURDFDemo/UrdfParser.h"


ATTRIBUTE_ALIGNED16(struct)
PhysXURDFInternalData
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();
	DefaultFileIO m_defaultFileIO;
	UrdfParser m_urdfParser;
	struct GUIHelperInterface* m_guiHelper;
	struct CommonFileIOInterface* m_fileIO;
	STxt m_sourceFile;
	char m_pathPrefix[1024];
	i32 m_bodyId;
	HashMap<HashInt, UrdfMaterialColor> m_linkColors;
	AlignedObjectArray<CollisionShape*> m_allocatedCollisionShapes;
	AlignedObjectArray<i32> m_allocatedTextures;
	//mutable AlignedObjectArray<TriangleMesh*> m_allocatedMeshInterfaces;
	HashMap<btHashPtr, UrdfCollision> m_bulletCollisionShape2UrdfCollision;

	UrdfRenderingInterface* m_customVisualShapesConverter;
	bool m_enableTinyRenderer;
	i32 m_flags;

	void setSourceFile(const STxt& relativeFileName, const STxt& prefix)
	{
		m_sourceFile = relativeFileName;
		m_urdfParser.setSourceFile(relativeFileName);
		strncpy(m_pathPrefix, prefix.c_str(), sizeof(m_pathPrefix));
		m_pathPrefix[sizeof(m_pathPrefix) - 1] = 0;  // required, strncpy doesn't write zero on overflow
	}

	PhysXURDFInternalData(CommonFileIOInterface* fileIO)
		:m_urdfParser(fileIO? fileIO : &m_defaultFileIO),
		m_fileIO(fileIO? fileIO : &m_defaultFileIO)
	{
		m_enableTinyRenderer = true;
		m_pathPrefix[0] = 0;
		m_flags = 0;
	}

	void setGlobalScaling(Scalar scaling)
	{
		m_urdfParser.setGlobalScaling(scaling);
	}


};

void PhysXURDFImporter::printTree()
{
	//	Assert(0);
}




PhysXURDFImporter::PhysXURDFImporter(struct CommonFileIOInterface* fileIO,double globalScaling, i32 flags)
{
	m_data = new PhysXURDFInternalData(fileIO);
	m_data->setGlobalScaling(globalScaling);
	m_data->m_flags = flags;
}

struct PhysXErrorLogger : public ErrorLogger
{
	i32 m_numErrors;
	i32 m_numWarnings;

	PhysXErrorLogger()
		: m_numErrors(0),
		  m_numWarnings(0)
	{
	}
	virtual void reportError(tukk error)
	{
		m_numErrors++;
		drx3DError(error);
	}
	virtual void reportWarning(tukk warning)
	{
		m_numWarnings++;
		drx3DWarning(warning);
	}

	virtual void printMessage(tukk msg)
	{
		drx3DPrintf(msg);
	}
};

bool PhysXURDFImporter::loadURDF(tukk fileName, bool forceFixedBase)
{
	if (strlen(fileName) == 0)
		return false;

	//i32 argc=0;
	char relativeFileName[1024];

	b3FileUtils fu;

	//bool fileFound = fu.findFile(fileName, relativeFileName, 1024);
	bool fileFound = m_data->m_fileIO->findResourcePath(fileName, relativeFileName, 1024);

	STxt xml_string;

	if (!fileFound)
	{
		drx3DWarning("URDF file '%s' not found\n", fileName);
		return false;
	}
	else
	{
		char path[1024];
		fu.extractPath(relativeFileName, path, sizeof(path));
		m_data->setSourceFile(relativeFileName, path);

		//read file
		i32 fileId = m_data->m_fileIO->fileOpen(relativeFileName,"r");


		char destBuffer[8192];
		tuk line = 0;
		do
		{
			line = m_data->m_fileIO->readLine(fileId, destBuffer, 8192);
			if (line)
			{
				xml_string += (STxt(destBuffer) + "\n");
			}
		}
		while (line);
		m_data->m_fileIO->fileClose(fileId);
#if 0
		std::fstream xml_file(relativeFileName, std::fstream::in);
		while (xml_file.good())
		{
			STxt line;
			std::getline(xml_file, line);
			xml_string += (line + "\n");
		}
		xml_file.close();
#endif

	}

	PhysXErrorLogger loggie;
	m_data->m_urdfParser.setParseSDF(false);
	bool result = false;

	if (xml_string.length())
	{
			result = m_data->m_urdfParser.loadUrdf(xml_string.c_str(), &loggie, forceFixedBase, (m_data->m_flags & CUF_PARSE_SENSORS));
	}

	return result;
}

i32 PhysXURDFImporter::getNumModels() const
{
	return m_data->m_urdfParser.getNumModels();
}

void PhysXURDFImporter::activateModel(i32 modelIndex)
{
	m_data->m_urdfParser.activateModel(modelIndex);
}

bool PhysXURDFImporter::loadSDF(tukk fileName, bool forceFixedBase)
{
	//i32 argc=0;
	char relativeFileName[1024];

	b3FileUtils fu;

	//bool fileFound = fu.findFile(fileName, relativeFileName, 1024);
	bool fileFound = (m_data->m_fileIO->findResourcePath(fileName, relativeFileName, 1024));

	STxt xml_string;

	if (!fileFound)
	{
		drx3DWarning("SDF file '%s' not found\n", fileName);
		return false;
	}
	else
	{

		char path[1024];
		fu.extractPath(relativeFileName, path, sizeof(path));
		m_data->setSourceFile(relativeFileName, path);

		//read file
		i32 fileId = m_data->m_fileIO->fileOpen(relativeFileName,"r");

		char destBuffer[8192];
		tuk line = 0;
		do
		{
			line = m_data->m_fileIO->readLine(fileId, destBuffer, 8192);
			if (line)
			{
				xml_string += (STxt(destBuffer) + "\n");
			}
		}
		while (line);
		m_data->m_fileIO->fileClose(fileId);
	}

	PhysXErrorLogger loggie;
	//todo: quick test to see if we can re-use the URDF parser for SDF or not
	m_data->m_urdfParser.setParseSDF(true);
	bool result = false;
	if (xml_string.length())
	{
		result = m_data->m_urdfParser.loadSDF(xml_string.c_str(), &loggie);
	}

	return result;
}

tukk PhysXURDFImporter::getPathPrefix()
{
	return m_data->m_pathPrefix;
}

void PhysXURDFImporter::setBodyUniqueId(i32 bodyId)
{
	m_data->m_bodyId = bodyId;
}

i32 PhysXURDFImporter::getBodyUniqueId() const
{
	return m_data->m_bodyId;
}

PhysXURDFImporter::~PhysXURDFImporter()
{
	delete m_data;
}

i32 PhysXURDFImporter::getRootLinkIndex() const
{
	if (m_data->m_urdfParser.getModel().m_rootLinks.size() == 1)
	{
		return m_data->m_urdfParser.getModel().m_rootLinks[0]->m_linkIndex;
	}
	return -1;
};

void PhysXURDFImporter::getLinkChildIndices(i32 linkIndex, AlignedObjectArray<i32>& childLinkIndices) const
{
	childLinkIndices.resize(0);
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	if (linkPtr)
	{
		const UrdfLink* link = *linkPtr;
		//i32 numChildren = m_data->m_urdfParser->getModel().m_links.getAtIndex(linkIndex)->

		for (i32 i = 0; i < link->m_childLinks.size(); i++)
		{
			i32 childIndex = link->m_childLinks[i]->m_linkIndex;
			childLinkIndices.push_back(childIndex);
		}
	}
}

STxt PhysXURDFImporter::getLinkName(i32 linkIndex) const
{
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;
		return link->m_name;
	}
	return "";
}

STxt PhysXURDFImporter::getBodyName() const
{
	return m_data->m_urdfParser.getModel().m_name;
}

STxt PhysXURDFImporter::getJointName(i32 linkIndex) const
{
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;
		if (link->m_parentJoint)
		{
			return link->m_parentJoint->m_name;
		}
	}
	return "";
}

void PhysXURDFImporter::getMassAndInertia2(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame, i32 flags) const
{
	if (flags & CUF_USE_URDF_INERTIA)
	{
		getMassAndInertia(urdfLinkIndex, mass, localInertiaDiagonal, inertialFrame);
	}
	else
	{
		//the link->m_inertia is NOT necessarily aligned with the inertial frame
		//so an additional transform might need to be computed
		UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(urdfLinkIndex);

		Assert(linkPtr);
		if (linkPtr)
		{
			UrdfLink* link = *linkPtr;
			Scalar linkMass;
			if (link->m_parentJoint == 0 && m_data->m_urdfParser.getModel().m_overrideFixedBase)
			{
				linkMass = 0.f;
			}
			else
			{
				linkMass = link->m_inertia.m_mass;
			}
			mass = linkMass;
			localInertiaDiagonal.setVal(0, 0, 0);
			inertialFrame.setOrigin(link->m_inertia.m_linkLocalFrame.getOrigin());
			inertialFrame.setBasis(link->m_inertia.m_linkLocalFrame.getBasis());
		}
		else
		{
			mass = 1.f;
			localInertiaDiagonal.setVal(1, 1, 1);
			inertialFrame.setIdentity();
		}
	}
}

void PhysXURDFImporter::getMassAndInertia(i32 linkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const
{
	//the link->m_inertia is NOT necessarily aligned with the inertial frame
	//so an additional transform might need to be computed
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);

	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;
		Matrix3x3 linkInertiaBasis;
		Scalar linkMass, principalInertiaX, principalInertiaY, principalInertiaZ;
		if (link->m_parentJoint == 0 && m_data->m_urdfParser.getModel().m_overrideFixedBase)
		{
			linkMass = 0.f;
			principalInertiaX = 0.f;
			principalInertiaY = 0.f;
			principalInertiaZ = 0.f;
			linkInertiaBasis.setIdentity();
		}
		else
		{
			linkMass = link->m_inertia.m_mass;
			if (link->m_inertia.m_ixy == 0.0 &&
				link->m_inertia.m_ixz == 0.0 &&
				link->m_inertia.m_iyz == 0.0)
			{
				principalInertiaX = link->m_inertia.m_ixx;
				principalInertiaY = link->m_inertia.m_iyy;
				principalInertiaZ = link->m_inertia.m_izz;
				linkInertiaBasis.setIdentity();
			}
			else
			{
				principalInertiaX = link->m_inertia.m_ixx;
				Matrix3x3 inertiaTensor(link->m_inertia.m_ixx, link->m_inertia.m_ixy, link->m_inertia.m_ixz,
										  link->m_inertia.m_ixy, link->m_inertia.m_iyy, link->m_inertia.m_iyz,
										  link->m_inertia.m_ixz, link->m_inertia.m_iyz, link->m_inertia.m_izz);
				Scalar threshold = 1.0e-6;
				i32 numIterations = 30;
				inertiaTensor.diagonalize(linkInertiaBasis, threshold, numIterations);
				principalInertiaX = inertiaTensor[0][0];
				principalInertiaY = inertiaTensor[1][1];
				principalInertiaZ = inertiaTensor[2][2];
			}
		}
		mass = linkMass;
		if (principalInertiaX < 0 ||
			principalInertiaX > (principalInertiaY + principalInertiaZ) ||
			principalInertiaY < 0 ||
			principalInertiaY > (principalInertiaX + principalInertiaZ) ||
			principalInertiaZ < 0 ||
			principalInertiaZ > (principalInertiaX + principalInertiaY))
		{
			drx3DWarning("Bad inertia tensor properties, setting inertia to zero for link: %s\n", link->m_name.c_str());
			principalInertiaX = 0.f;
			principalInertiaY = 0.f;
			principalInertiaZ = 0.f;
			linkInertiaBasis.setIdentity();
		}
		localInertiaDiagonal.setVal(principalInertiaX, principalInertiaY, principalInertiaZ);
		inertialFrame.setOrigin(link->m_inertia.m_linkLocalFrame.getOrigin());
		inertialFrame.setBasis(link->m_inertia.m_linkLocalFrame.getBasis() * linkInertiaBasis);
	}
	else
	{
		mass = 1.f;
		localInertiaDiagonal.setVal(1, 1, 1);
		inertialFrame.setIdentity();
	}
}

bool PhysXURDFImporter::getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const
{
	jointLowerLimit = 0.f;
	jointUpperLimit = 0.f;
	jointDamping = 0.f;
	jointFriction = 0.f;
	jointMaxForce = 0.f;
	jointMaxVelocity = 0.f;

	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(urdfLinkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;
		linkTransformInWorld = link->m_linkTransformInWorld;

		if (link->m_parentJoint)
		{
			UrdfJoint* pj = link->m_parentJoint;
			parent2joint = pj->m_parentLinkToJointTransform;
			jointType = pj->m_type;
			jointAxisInJointSpace = pj->m_localJointAxis;
			jointLowerLimit = pj->m_lowerLimit;
			jointUpperLimit = pj->m_upperLimit;
			jointDamping = pj->m_jointDamping;
			jointFriction = pj->m_jointFriction;
			jointMaxForce = pj->m_effortLimit;
			jointMaxVelocity = pj->m_velocityLimit;
			return true;
		}
		else
		{
			parent2joint.setIdentity();
			return false;
		}
	}

	return false;
};

bool PhysXURDFImporter::getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const
{
	Scalar jointMaxForce;
	Scalar jointMaxVelocity;
	return getJointInfo2(urdfLinkIndex, parent2joint, linkTransformInWorld, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction, jointMaxForce, jointMaxVelocity);
}

void PhysXURDFImporter::setRootTransformInWorld(const Transform2& rootTransformInWorld)
{
	m_data->m_urdfParser.getModel().m_rootTransformInWorld = rootTransformInWorld;
}

bool PhysXURDFImporter::getRootTransformInWorld(Transform2& rootTransformInWorld) const
{
	rootTransformInWorld = m_data->m_urdfParser.getModel().m_rootTransformInWorld;
	return true;
}


const struct UrdfLink* PhysXURDFImporter::getUrdfLink(i32 urdfLinkIndex) const
{
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(urdfLinkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		const UrdfLink* link = *linkPtr;
		return link;
	}
	return 0;
}

const struct UrdfModel* PhysXURDFImporter::getUrdfModel() const
{
	return &m_data->m_urdfParser.getModel();
}


i32 PhysXURDFImporter::getUrdfFromCollisionShape(const CollisionShape* collisionShape, UrdfCollision& collision) const
{
	UrdfCollision* col = m_data->m_bulletCollisionShape2UrdfCollision.find(collisionShape);
	if (col)
	{
		collision = *col;
		return 1;
	}
	return 0;
}

#if 0
CollisionShape* PhysXURDFImporter::convertURDFToCollisionShape(const UrdfCollision* collision, tukk urdfPathPrefix) const
{

	D3_PROFILE("convertURDFToCollisionShape");

	CollisionShape* shape = 0;

	switch (collision->m_geometry.m_type)
	{
		case URDF_GEOM_PLANE:
		{
			Vec3 planeNormal = collision->m_geometry.m_planeNormal;
			Scalar planeConstant = 0;  //not available?
			StaticPlaneShape* plane = new StaticPlaneShape(planeNormal, planeConstant);
			shape = plane;
			shape->setMargin(gUrdfDefaultCollisionMargin);
			break;
		}
		case URDF_GEOM_CAPSULE:
		{
			Scalar radius = collision->m_geometry.m_capsuleRadius;
			Scalar height = collision->m_geometry.m_capsuleHeight;
			CapsuleShapeZ* capsuleShape = new CapsuleShapeZ(radius, height);
			shape = capsuleShape;
			shape->setMargin(gUrdfDefaultCollisionMargin);
			break;
		}

		case URDF_GEOM_CYLINDER:
		{
			Scalar cylRadius = collision->m_geometry.m_capsuleRadius;
			Scalar cylHalfLength = 0.5 * collision->m_geometry.m_capsuleHeight;
			if (m_data->m_flags & CUF_USE_IMPLICIT_CYLINDER)
			{
				Vec3 halfExtents(cylRadius, cylRadius, cylHalfLength);
				CylinderShapeZ* cylZShape = new CylinderShapeZ(halfExtents);
				shape = cylZShape;
			}
			else
			{
				AlignedObjectArray<Vec3> vertices;
				//i32 numVerts = sizeof(barrel_vertices)/(9*sizeof(float));
				i32 numSteps = 32;
				for (i32 i = 0; i < numSteps; i++)
				{
					Vec3 vert(cylRadius * Sin(SIMD_2_PI * (float(i) / numSteps)), cylRadius * btCos(SIMD_2_PI * (float(i) / numSteps)), cylHalfLength);
					vertices.push_back(vert);
					vert[2] = -cylHalfLength;
					vertices.push_back(vert);
				}
				ConvexHullShape* cylZShape = new ConvexHullShape(&vertices[0].x(), vertices.size(), sizeof(Vec3));
				cylZShape->setMargin(gUrdfDefaultCollisionMargin);
				cylZShape->recalcLocalAabb();
				if (m_data->m_flags & CUF_INITIALIZE_SAT_FEATURES)
				{
					cylZShape->initializePolyhedralFeatures();
				}
				cylZShape->optimizeConvexHull();
				shape = cylZShape;
			}

			break;
		}
		case URDF_GEOM_BOX:
		{
			Vec3 extents = collision->m_geometry.m_boxSize;
			BoxShape* boxShape = new BoxShape(extents * 0.5f);
			//ConvexShape* boxShape = new ConeShapeX(extents[2]*0.5,extents[0]*0.5);
			if (m_data->m_flags & CUF_INITIALIZE_SAT_FEATURES)
			{
				boxShape->initializePolyhedralFeatures();
			}
			shape = boxShape;
			shape->setMargin(gUrdfDefaultCollisionMargin);
			break;
		}
		case URDF_GEOM_SPHERE:
		{
			Scalar radius = collision->m_geometry.m_sphereRadius;
			SphereShape* sphereShape = new SphereShape(radius);
			shape = sphereShape;
			shape->setMargin(gUrdfDefaultCollisionMargin);
			break;
		}
		case URDF_GEOM_CDF:
		{
			char relativeFileName[1024];
			char pathPrefix[1024];
			pathPrefix[0] = 0;
			if (m_data->m_fileIO->findResourcePath(collision->m_geometry.m_meshFileName.c_str(), relativeFileName, 1024))
			{
				b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);

				AlignedObjectArray<char> sdfData;
				{
					std::streampos fsize = 0;
					std::ifstream file(relativeFileName, std::ios::binary);
					if (file.good())
					{
						fsize = file.tellg();
						file.seekg(0, std::ios::end);
						fsize = file.tellg() - fsize;
						file.seekg(0, std::ios::beg);
						sdfData.resize(fsize);
						i32 bytesRead = file.rdbuf()->sgetn(&sdfData[0], fsize);
						Assert(bytesRead == fsize);
						file.close();
					}
				}

				if (sdfData.size())
				{
					SdfCollisionShape* sdfShape = new SdfCollisionShape();
					bool valid = sdfShape->initializeSDF(&sdfData[0], sdfData.size());
					Assert(valid);

					if (valid)
					{
						shape = sdfShape;
					}
					else
					{
						delete sdfShape;
					}
				}
			}
			break;
		}
		case URDF_GEOM_MESH:
		{
			GLInstanceGraphicsShape* glmesh = 0;
			switch (collision->m_geometry.m_meshFileType)
			{
				case UrdfGeometry::FILE_OBJ:
					if (collision->m_flags & URDF_FORCE_CONCAVE_TRIMESH)
					{
						char relativeFileName[1024];
						char pathPrefix[1024];
						pathPrefix[0] = 0;
						if (m_data->m_fileIO->findResourcePath(collision->m_geometry.m_meshFileName.c_str(), relativeFileName, 1024))
						{
							b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
						}
						glmesh = LoadMeshFromObj(collision->m_geometry.m_meshFileName.c_str(), pathPrefix,m_data->m_fileIO);
					}
					else
					{
						std::vector<tinyobj::shape_t> shapes;
						STxt err = tinyobj::LoadObj(shapes, collision->m_geometry.m_meshFileName.c_str(),"",m_data->m_fileIO);
						//create a convex hull for each shape, and store it in a CompoundShape

						Assert(0);
						
						return shape;
					}
					break;

				case UrdfGeometry::FILE_STL:
					glmesh = LoadMeshFromSTL(collision->m_geometry.m_meshFileName.c_str(), m_data->m_fileIO);
					break;

				case UrdfGeometry::FILE_COLLADA:
				{
					AlignedObjectArray<GLInstanceGraphicsShape> visualShapes;
					AlignedObjectArray<ColladaGraphicsInstance> visualShapeInstances;
					Transform2 upAxisTrans;
					upAxisTrans.setIdentity();
					float unitMeterScaling = 1;
					LoadMeshFromCollada(collision->m_geometry.m_meshFileName.c_str(), visualShapes, visualShapeInstances, upAxisTrans, unitMeterScaling, 2, m_data->m_fileIO);

					glmesh = new GLInstanceGraphicsShape;
					glmesh->m_indices = new b3AlignedObjectArray<i32>();
					glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

					for (i32 i = 0; i < visualShapeInstances.size(); i++)
					{
						ColladaGraphicsInstance* instance = &visualShapeInstances[i];
						GLInstanceGraphicsShape* gfxShape = &visualShapes[instance->m_shapeIndex];

						b3AlignedObjectArray<GLInstanceVertex> verts;
						verts.resize(gfxShape->m_vertices->size());

						i32 baseIndex = glmesh->m_vertices->size();

						for (i32 i = 0; i < gfxShape->m_vertices->size(); i++)
						{
							verts[i].normal[0] = gfxShape->m_vertices->at(i).normal[0];
							verts[i].normal[1] = gfxShape->m_vertices->at(i).normal[1];
							verts[i].normal[2] = gfxShape->m_vertices->at(i).normal[2];
							verts[i].uv[0] = gfxShape->m_vertices->at(i).uv[0];
							verts[i].uv[1] = gfxShape->m_vertices->at(i).uv[1];
							verts[i].xyzw[0] = gfxShape->m_vertices->at(i).xyzw[0];
							verts[i].xyzw[1] = gfxShape->m_vertices->at(i).xyzw[1];
							verts[i].xyzw[2] = gfxShape->m_vertices->at(i).xyzw[2];
							verts[i].xyzw[3] = gfxShape->m_vertices->at(i).xyzw[3];
						}

						i32 curNumIndices = glmesh->m_indices->size();
						i32 additionalIndices = gfxShape->m_indices->size();
						glmesh->m_indices->resize(curNumIndices + additionalIndices);
						for (i32 k = 0; k < additionalIndices; k++)
						{
							glmesh->m_indices->at(curNumIndices + k) = gfxShape->m_indices->at(k) + baseIndex;
						}

						//compensate upAxisTrans and unitMeterScaling here
						Matrix4x4 upAxisMat;
						upAxisMat.setIdentity();
						//upAxisMat.setPureRotation(upAxisTrans.getRotation());
						Matrix4x4 unitMeterScalingMat;
						unitMeterScalingMat.setPureScaling(Vec3(unitMeterScaling, unitMeterScaling, unitMeterScaling));
						Matrix4x4 worldMat = unitMeterScalingMat * instance->m_worldTransform * upAxisMat;
						//Matrix4x4 worldMat = instance->m_worldTransform;
						i32 curNumVertices = glmesh->m_vertices->size();
						i32 additionalVertices = verts.size();
						glmesh->m_vertices->reserve(curNumVertices + additionalVertices);

						for (i32 v = 0; v < verts.size(); v++)
						{
							Vec3 pos(verts[v].xyzw[0], verts[v].xyzw[1], verts[v].xyzw[2]);
							pos = worldMat * pos;
							verts[v].xyzw[0] = float(pos[0]);
							verts[v].xyzw[1] = float(pos[1]);
							verts[v].xyzw[2] = float(pos[2]);
							glmesh->m_vertices->push_back(verts[v]);
						}
					}
					glmesh->m_numIndices = glmesh->m_indices->size();
					glmesh->m_numvertices = glmesh->m_vertices->size();
					//glmesh = LoadMeshFromCollada(success.c_str());
					break;
				}
			}

			if (!glmesh || glmesh->m_numvertices <= 0)
			{
				drx3DWarning("%s: cannot extract mesh from '%s'\n", urdfPathPrefix, collision->m_geometry.m_meshFileName.c_str());
				delete glmesh;
				break;
			}

			AlignedObjectArray<Vec3> convertedVerts;
			convertedVerts.reserve(glmesh->m_numvertices);
			for (i32 i = 0; i < glmesh->m_numvertices; i++)
			{
				convertedVerts.push_back(Vec3(
					glmesh->m_vertices->at(i).xyzw[0] * collision->m_geometry.m_meshScale[0],
					glmesh->m_vertices->at(i).xyzw[1] * collision->m_geometry.m_meshScale[1],
					glmesh->m_vertices->at(i).xyzw[2] * collision->m_geometry.m_meshScale[2]));
			}

			if (collision->m_flags & URDF_FORCE_CONCAVE_TRIMESH)
			{
				DRX3D_PROFILE("convert trimesh");
				TriangleMesh* meshInterface = new TriangleMesh();
				m_data->m_allocatedMeshInterfaces.push_back(meshInterface);
				{
					DRX3D_PROFILE("convert vertices");

					for (i32 i = 0; i < glmesh->m_numIndices / 3; i++)
					{
						const Vec3& v0 = convertedVerts[glmesh->m_indices->at(i * 3)];
						const Vec3& v1 = convertedVerts[glmesh->m_indices->at(i * 3 + 1)];
						const Vec3& v2 = convertedVerts[glmesh->m_indices->at(i * 3 + 2)];
						meshInterface->addTriangle(v0, v1, v2);
					}
				}
				{
					DRX3D_PROFILE("create BvhTriangleMeshShape");
					BvhTriangleMeshShape* trimesh = new BvhTriangleMeshShape(meshInterface, true, true);
					//trimesh->setLocalScaling(collision->m_geometry.m_meshScale);
					shape = trimesh;
				}
			}
			else
			{
				DRX3D_PROFILE("convert ConvexHullShape");
				ConvexHullShape* convexHull = new ConvexHullShape(&convertedVerts[0].getX(), convertedVerts.size(), sizeof(Vec3));
				convexHull->optimizeConvexHull();
				if (m_data->m_flags & CUF_INITIALIZE_SAT_FEATURES)
				{
					convexHull->initializePolyhedralFeatures();
				}
				convexHull->setMargin(gUrdfDefaultCollisionMargin);
				convexHull->recalcLocalAabb();
				//convexHull->setLocalScaling(collision->m_geometry.m_meshScale);
				shape = convexHull;
			}

			delete glmesh;
			break;
		}  // mesh case

		default:
			drx3DWarning("Ошибка: unknown collision geometry type %i\n", collision->m_geometry.m_type);
	}
	if (shape && collision->m_geometry.m_type == URDF_GEOM_MESH)
	{
		m_data->m_bulletCollisionShape2UrdfCollision.insert(shape, *collision);
	}
	return shape;
}


void PhysXURDFImporter::convertURDFToVisualShapeInternal(const UrdfVisual* visual, tukk urdfPathPrefix, const Transform2& visualTransform, AlignedObjectArray<GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<BulletURDFTexture>& texturesOut, struct b3ImportMeshData& meshData) const
{
	DRX3D_PROFILE("convertURDFToVisualShapeInternal");

	GLInstanceGraphicsShape* glmesh = 0;

	ConvexShape* convexColShape = 0;

	switch (visual->m_geometry.m_type)
	{
        case URDF_GEOM_CAPSULE:
        {
           Scalar radius = visual->m_geometry.m_capsuleRadius;
			Scalar height = visual->m_geometry.m_capsuleHeight;
			CapsuleShapeZ* capsuleShape = new CapsuleShapeZ(radius, height);
			convexColShape = capsuleShape;
			convexColShape->setMargin(gUrdfDefaultCollisionMargin);
            break;
        }
		case URDF_GEOM_CYLINDER:
		{
			AlignedObjectArray<Vec3> vertices;

			//i32 numVerts = sizeof(barrel_vertices)/(9*sizeof(float));
			i32 numSteps = 32;
			for (i32 i = 0; i < numSteps; i++)
			{
				Scalar cylRadius = visual->m_geometry.m_capsuleRadius;
				Scalar cylLength = visual->m_geometry.m_capsuleHeight;

				Vec3 vert(cylRadius * Sin(SIMD_2_PI * (float(i) / numSteps)), cylRadius * btCos(SIMD_2_PI * (float(i) / numSteps)), cylLength / 2.);
				vertices.push_back(vert);
				vert[2] = -cylLength / 2.;
				vertices.push_back(vert);
			}

			ConvexHullShape* cylZShape = new ConvexHullShape(&vertices[0].x(), vertices.size(), sizeof(Vec3));
			cylZShape->setMargin(gUrdfDefaultCollisionMargin);
			cylZShape->recalcLocalAabb();
			convexColShape = cylZShape;
			break;
		}

		case URDF_GEOM_BOX:
		{
			Vec3 extents = visual->m_geometry.m_boxSize;
			i32 strideInBytes = 9 * sizeof(float);
			i32 numVertices = sizeof(cube_vertices_textured) / strideInBytes;
			i32 numIndices = sizeof(cube_indices) / sizeof(i32);
			glmesh = new GLInstanceGraphicsShape;
			glmesh->m_indices = new b3AlignedObjectArray<i32>();
			glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
			glmesh->m_indices->resize(numIndices);
			for (i32 k = 0; k < numIndices; k++)
			{
				glmesh->m_indices->at(k) = cube_indices[k];
			}
			glmesh->m_vertices->resize(numVertices);

			Scalar halfExtentsX = extents[0] * 0.5;
			Scalar halfExtentsY = extents[1] * 0.5;
			Scalar halfExtentsZ = extents[2] * 0.5;
			GLInstanceVertex* verts = &glmesh->m_vertices->at(0);
			Scalar textureScaling = 1;

			for (i32 i = 0; i < numVertices; i++)
			{
				verts[i].xyzw[0] = halfExtentsX * cube_vertices_textured[i * 9];
				verts[i].xyzw[1] = halfExtentsY * cube_vertices_textured[i * 9 + 1];
				verts[i].xyzw[2] = halfExtentsZ * cube_vertices_textured[i * 9 + 2];
				verts[i].xyzw[3] = cube_vertices_textured[i * 9 + 3];
				verts[i].normal[0] = cube_vertices_textured[i * 9 + 4];
				verts[i].normal[1] = cube_vertices_textured[i * 9 + 5];
				verts[i].normal[2] = cube_vertices_textured[i * 9 + 6];
				verts[i].uv[0] = cube_vertices_textured[i * 9 + 7] * textureScaling;
				verts[i].uv[1] = cube_vertices_textured[i * 9 + 8] * textureScaling;
			}
			
			glmesh->m_numIndices = numIndices;
			glmesh->m_numvertices = numVertices;
			break;
		}

		case URDF_GEOM_SPHERE:
		{
			Scalar radius = visual->m_geometry.m_sphereRadius;
			SphereShape* sphereShape = new SphereShape(radius);
			convexColShape = sphereShape;
			convexColShape->setMargin(gUrdfDefaultCollisionMargin);
			break;
		}

		case URDF_GEOM_MESH:
		{
			switch (visual->m_geometry.m_meshFileType)
			{
				case UrdfGeometry::FILE_OBJ:
				{

					if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(visual->m_geometry.m_meshFileName, meshData, m_data->m_fileIO))
					{
						if (meshData.m_textureImage1)
						{
							BulletURDFTexture texData;
							texData.m_width = meshData.m_textureWidth;
							texData.m_height = meshData.m_textureHeight;
							texData.textureData1 = meshData.m_textureImage1;
							texData.m_isCached = meshData.m_isCached;
							texturesOut.push_back(texData);
						}
						glmesh = meshData.m_gfxShape;
					}
					break;
				}

				case UrdfGeometry::FILE_STL:
				{
					glmesh = LoadMeshFromSTL(visual->m_geometry.m_meshFileName.c_str(),m_data->m_fileIO);
					break;
				}

				case UrdfGeometry::FILE_COLLADA:
				{
					AlignedObjectArray<GLInstanceGraphicsShape> visualShapes;
					AlignedObjectArray<ColladaGraphicsInstance> visualShapeInstances;
					Transform2 upAxisTrans;
					upAxisTrans.setIdentity();
					float unitMeterScaling = 1;
					i32 upAxis = 2;

					LoadMeshFromCollada(visual->m_geometry.m_meshFileName.c_str(),
										visualShapes,
										visualShapeInstances,
										upAxisTrans,
										unitMeterScaling,
										upAxis,
										m_data->m_fileIO);

					glmesh = new GLInstanceGraphicsShape;
					//		i32 index = 0;
					glmesh->m_indices = new b3AlignedObjectArray<i32>();
					glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

					for (i32 i = 0; i < visualShapeInstances.size(); i++)
					{
						ColladaGraphicsInstance* instance = &visualShapeInstances[i];
						GLInstanceGraphicsShape* gfxShape = &visualShapes[instance->m_shapeIndex];

						b3AlignedObjectArray<GLInstanceVertex> verts;
						verts.resize(gfxShape->m_vertices->size());

						i32 baseIndex = glmesh->m_vertices->size();

						for (i32 i = 0; i < gfxShape->m_vertices->size(); i++)
						{
							verts[i].normal[0] = gfxShape->m_vertices->at(i).normal[0];
							verts[i].normal[1] = gfxShape->m_vertices->at(i).normal[1];
							verts[i].normal[2] = gfxShape->m_vertices->at(i).normal[2];
							verts[i].uv[0] = gfxShape->m_vertices->at(i).uv[0];
							verts[i].uv[1] = gfxShape->m_vertices->at(i).uv[1];
							verts[i].xyzw[0] = gfxShape->m_vertices->at(i).xyzw[0];
							verts[i].xyzw[1] = gfxShape->m_vertices->at(i).xyzw[1];
							verts[i].xyzw[2] = gfxShape->m_vertices->at(i).xyzw[2];
							verts[i].xyzw[3] = gfxShape->m_vertices->at(i).xyzw[3];
						}

						i32 curNumIndices = glmesh->m_indices->size();
						i32 additionalIndices = gfxShape->m_indices->size();
						glmesh->m_indices->resize(curNumIndices + additionalIndices);
						for (i32 k = 0; k < additionalIndices; k++)
						{
							glmesh->m_indices->at(curNumIndices + k) = gfxShape->m_indices->at(k) + baseIndex;
						}

						//compensate upAxisTrans and unitMeterScaling here
						Matrix4x4 upAxisMat;
						upAxisMat.setIdentity();
						//								upAxisMat.setPureRotation(upAxisTrans.getRotation());
						Matrix4x4 unitMeterScalingMat;
						unitMeterScalingMat.setPureScaling(Vec3(unitMeterScaling, unitMeterScaling, unitMeterScaling));
						Matrix4x4 worldMat = unitMeterScalingMat * upAxisMat * instance->m_worldTransform;
						//Matrix4x4 worldMat = instance->m_worldTransform;
						i32 curNumVertices = glmesh->m_vertices->size();
						i32 additionalVertices = verts.size();
						glmesh->m_vertices->reserve(curNumVertices + additionalVertices);

						for (i32 v = 0; v < verts.size(); v++)
						{
							Vec3 pos(verts[v].xyzw[0], verts[v].xyzw[1], verts[v].xyzw[2]);
							pos = worldMat * pos;
							verts[v].xyzw[0] = float(pos[0]);
							verts[v].xyzw[1] = float(pos[1]);
							verts[v].xyzw[2] = float(pos[2]);
							glmesh->m_vertices->push_back(verts[v]);
						}
					}
					glmesh->m_numIndices = glmesh->m_indices->size();
					glmesh->m_numvertices = glmesh->m_vertices->size();
					//glmesh = LoadMeshFromCollada(visual->m_geometry.m_meshFileName);

					break;
				}
			}  // switch file type

			if (!glmesh || !glmesh->m_vertices || glmesh->m_numvertices <= 0)
			{
				drx3DWarning("%s: cannot extract anything useful from mesh '%s'\n", urdfPathPrefix, visual->m_geometry.m_meshFileName.c_str());
				break;
			}

			//apply the geometry scaling
			for (i32 i = 0; i < glmesh->m_vertices->size(); i++)
			{
				glmesh->m_vertices->at(i).xyzw[0] *= visual->m_geometry.m_meshScale[0];
				glmesh->m_vertices->at(i).xyzw[1] *= visual->m_geometry.m_meshScale[1];
				glmesh->m_vertices->at(i).xyzw[2] *= visual->m_geometry.m_meshScale[2];
			}
			break;
		}
		case URDF_GEOM_PLANE:
		{
			drx3DWarning("No default visual for URDF_GEOM_PLANE");
			break;
		}
		default:
		{
			drx3DWarning("Ошибка: unknown visual geometry type %i\n", visual->m_geometry.m_type);
		}
	}

	//if we have a convex, tesselate into localVertices/localIndices
	if ((glmesh == 0) && convexColShape)
	{
		DRX3D_PROFILE("convexColShape");

		ShapeHull* hull = new ShapeHull(convexColShape);
		hull->buildHull(0.0);
		{
			//	i32 strideInBytes = 9*sizeof(float);
			i32 numVertices = hull->numVertices();
			i32 numIndices = hull->numIndices();

			glmesh = new GLInstanceGraphicsShape;
			//	i32 index = 0;
			glmesh->m_indices = new b3AlignedObjectArray<i32>();
			glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

			for (i32 i = 0; i < numVertices; i++)
			{
				GLInstanceVertex vtx;
				Vec3 pos = hull->getVertexPointer()[i];
				vtx.xyzw[0] = pos.x();
				vtx.xyzw[1] = pos.y();
				vtx.xyzw[2] = pos.z();
				vtx.xyzw[3] = 1.f;
				pos.normalize();
				vtx.normal[0] = pos.x();
				vtx.normal[1] = pos.y();
				vtx.normal[2] = pos.z();
				Scalar u = Atan2(vtx.normal[0], vtx.normal[2]) / (2 * SIMD_PI) + 0.5;
				Scalar v = vtx.normal[1] * 0.5 + 0.5;
				vtx.uv[0] = u;
				vtx.uv[1] = v;
				glmesh->m_vertices->push_back(vtx);
			}

			AlignedObjectArray<i32> indices;
			for (i32 i = 0; i < numIndices; i++)
			{
				glmesh->m_indices->push_back(hull->getIndexPointer()[i]);
			}

			glmesh->m_numvertices = glmesh->m_vertices->size();
			glmesh->m_numIndices = glmesh->m_indices->size();
		}
		delete hull;
		delete convexColShape;
		convexColShape = 0;
	}

	if (glmesh && glmesh->m_numIndices > 0 && glmesh->m_numvertices > 0)
	{
		DRX3D_PROFILE("glmesh");
		i32 baseIndex = verticesOut.size();

		for (i32 i = 0; i < glmesh->m_indices->size(); i++)
		{
			indicesOut.push_back(glmesh->m_indices->at(i) + baseIndex);
		}

		for (i32 i = 0; i < glmesh->m_vertices->size(); i++)
		{
			GLInstanceVertex& v = glmesh->m_vertices->at(i);
			Vec3 vert(v.xyzw[0], v.xyzw[1], v.xyzw[2]);
			Vec3 vt = visualTransform * vert;
			v.xyzw[0] = vt[0];
			v.xyzw[1] = vt[1];
			v.xyzw[2] = vt[2];
			Vec3 triNormal(v.normal[0], v.normal[1], v.normal[2]);
			triNormal = visualTransform.getBasis() * triNormal;
			v.normal[0] = triNormal[0];
			v.normal[1] = triNormal[1];
			v.normal[2] = triNormal[2];
			verticesOut.push_back(v);
		}
	}
	delete glmesh;
}

i32 PhysXURDFImporter::convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const
{
	i32 graphicsIndex = -1;
	AlignedObjectArray<GLInstanceVertex> vertices;
	AlignedObjectArray<i32> indices;
	Transform2 startTrans;
	startTrans.setIdentity();
	AlignedObjectArray<PhysXURDFTexture> textures;

	const UrdfModel& model = m_data->m_urdfParser.getModel();
	UrdfLink* const* linkPtr = model.m_links.getAtIndex(linkIndex);
	if (linkPtr)
	{
		const UrdfLink* link = *linkPtr;

		for (i32 v = 0; v < link->m_visualArray.size(); v++)
		{
			const UrdfVisual& vis = link->m_visualArray[v];
			Transform2 childTrans = vis.m_linkLocalFrame;
			btHashString matName(vis.m_materialName.c_str());
			UrdfMaterial* const* matPtr = model.m_materials[matName];
			b3ImportMeshData meshData;

			convertURDFToVisualShapeInternal(&vis, pathPrefix, localInertiaFrame.inverse() * childTrans, vertices, indices, textures,meshData);

			if (m_data->m_flags&CUF_USE_MATERIAL_COLORS_FROM_MTL)
			{
				if ((meshData.m_flags & D3_IMPORT_MESH_HAS_RGBA_COLOR) &&
						(meshData.m_flags & D3_IMPORT_MESH_HAS_SPECULAR_COLOR))
				{
					UrdfMaterialColor matCol;

					if (m_data->m_flags&CUF_USE_MATERIAL_TRANSPARANCY_FROM_MTL)
					{
						matCol.m_rgbaColor.setVal(meshData.m_rgbaColor[0],
									meshData.m_rgbaColor[1],
									meshData.m_rgbaColor[2],
									meshData.m_rgbaColor[3]);
					} else
					{
						matCol.m_rgbaColor.setVal(meshData.m_rgbaColor[0],
									meshData.m_rgbaColor[1],
									meshData.m_rgbaColor[2],
									1);
					}

					matCol.m_specularColor.setVal(meshData.m_specularColor[0],
						meshData.m_specularColor[1],
						meshData.m_specularColor[2]);
					m_data->m_linkColors.insert(linkIndex, matCol);
				}
			} else
			{
				if (matPtr)
				{
					UrdfMaterial* const mat = *matPtr;
					//printf("UrdfMaterial %s, rgba = %f,%f,%f,%f\n",mat->m_name.c_str(),mat->m_rgbaColor[0],mat->m_rgbaColor[1],mat->m_rgbaColor[2],mat->m_rgbaColor[3]);
					UrdfMaterialColor matCol;
					matCol.m_rgbaColor = mat->m_matColor.m_rgbaColor;
					matCol.m_specularColor = mat->m_matColor.m_specularColor;
					m_data->m_linkColors.insert(linkIndex, matCol);
				}
			}

		}
	}
	if (vertices.size() && indices.size())
	{
		//		graphicsIndex  = m_data->m_guiHelper->registerGraphicsShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size());
		//graphicsIndex  = m_data->m_guiHelper->registerGraphicsShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size());

		//CommonRenderInterface* renderer = m_data->m_guiHelper->getRenderInterface();

		if (1)
		{
			i32 textureIndex = -1;
			if (textures.size())
			{
				textureIndex = m_data->m_guiHelper->registerTexture(textures[0].textureData1, textures[0].m_width, textures[0].m_height);
				if (textureIndex >= 0)
				{
					m_data->m_allocatedTextures.push_back(textureIndex);
				}
			}
			{
				D3_PROFILE("registerGraphicsShape");
				graphicsIndex = m_data->m_guiHelper->registerGraphicsShape(&vertices[0].xyzw[0], vertices.size(), &indices[0], indices.size(), D3_GL_TRIANGLES, textureIndex);
			}
		}
	}

	//delete textures
	for (i32 i = 0; i < textures.size(); i++)
	{
		D3_PROFILE("free textureData");
		if (!textures[i].m_isCached)
		{
			free(textures[i].textureData1);
		}
	}
	return graphicsIndex;
}
#endif
bool PhysXURDFImporter::getLinkColor(i32 linkIndex, Vec4& colorRGBA) const
{
	const UrdfMaterialColor* matColPtr = m_data->m_linkColors[linkIndex];
	if (matColPtr)
	{
		colorRGBA = matColPtr->m_rgbaColor;
		return true;
	}
	return false;
}

bool PhysXURDFImporter::getLinkColor2(i32 linkIndex, UrdfMaterialColor& matCol) const
{
	UrdfMaterialColor* matColPtr = m_data->m_linkColors[linkIndex];
	if (matColPtr)
	{
		matCol = *matColPtr;
		return true;
	}
	return false;
}

void PhysXURDFImporter::setLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const
{
	m_data->m_linkColors.insert(linkIndex, matCol);
}

bool PhysXURDFImporter::getLinkContactInfo(i32 urdflinkIndex, URDFLinkContactInfo& contactInfo) const
{
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(urdflinkIndex);
	if (linkPtr)
	{
		const UrdfLink* link = *linkPtr;
		contactInfo = link->m_contactInfo;
		return true;
	}
	return false;
}

bool PhysXURDFImporter::getLinkAudioSource(i32 linkIndex, SDFAudioSource& audioSource) const
{
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	if (linkPtr)
	{
		const UrdfLink* link = *linkPtr;
		if (link->m_audioSource.m_flags & SDFAudioSource::SDFAudioSourceValid)
		{
			audioSource = link->m_audioSource;
			return true;
		}
	}
	return false;
}

void PhysXURDFImporter::setEnableTinyRenderer(bool enable)
{
	m_data->m_enableTinyRenderer = enable;
}



i32 PhysXURDFImporter::convertLinkVisualShapes3(
	i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame,
	const UrdfLink* linkPtr, const UrdfModel* model,
	i32 collisionObjectUniqueId, i32 bodyUniqueId, struct  CommonFileIOInterface* fileIO) const
{
	return 0;
}

void PhysXURDFImporter::convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& localInertiaFrame, class CollisionObject2* colObj, i32 bodyUniqueId) const
{
	if (m_data->m_enableTinyRenderer && m_data->m_customVisualShapesConverter)
	{
		const UrdfModel& model = m_data->m_urdfParser.getModel();
		UrdfLink* const* linkPtr = model.m_links.getAtIndex(urdfIndex);
		if (linkPtr)
		{
			m_data->m_customVisualShapesConverter->setFlags(m_data->m_flags);
			m_data->m_customVisualShapesConverter->convertVisualShapes(linkIndex, pathPrefix, localInertiaFrame, *linkPtr, &model, 0, bodyUniqueId, m_data->m_fileIO);
		}
	}
}

i32 PhysXURDFImporter::getNumAllocatedCollisionShapes() const
{
	return m_data->m_allocatedCollisionShapes.size();
}

CollisionShape* PhysXURDFImporter::getAllocatedCollisionShape(i32 index)
{
	return m_data->m_allocatedCollisionShapes[index];
}

i32 PhysXURDFImporter::getNumAllocatedMeshInterfaces() const
{
	return 0;// m_data->m_allocatedMeshInterfaces.size();
}

StridingMeshInterface* PhysXURDFImporter::getAllocatedMeshInterface(i32 index)
{
	return 0;// m_data->m_allocatedMeshInterfaces[index];
}

i32 PhysXURDFImporter::getNumAllocatedTextures() const
{
	return m_data->m_allocatedTextures.size();
}

i32 PhysXURDFImporter::getAllocatedTexture(i32 index) const
{
	return m_data->m_allocatedTextures[index];
}

i32 PhysXURDFImporter::getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const
{
	i32 result = 0;
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;
		for (i32 v = 0; v < link->m_collisionArray.size(); v++)
		{
			const UrdfCollision& col = link->m_collisionArray[v];
			if (col.m_flags & URDF_HAS_COLLISION_GROUP)
			{
				colGroup = col.m_collisionGroup;
				result |= URDF_HAS_COLLISION_GROUP;
			}
			if (col.m_flags & URDF_HAS_COLLISION_MASK)
			{
				colMask = col.m_collisionMask;
				result |= URDF_HAS_COLLISION_MASK;
			}
		}
	}
	return result;
}

#if 0
class CompoundShape* PhysXURDFImporter::convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const
{

	CompoundShape* compoundShape = new CompoundShape();
	m_data->m_allocatedCollisionShapes.push_back(compoundShape);

	compoundShape->setMargin(gUrdfDefaultCollisionMargin);
	UrdfLink* const* linkPtr = m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{
		UrdfLink* link = *linkPtr;

		for (i32 v = 0; v < link->m_collisionArray.size(); v++)
		{
			const UrdfCollision& col = link->m_collisionArray[v];
			CollisionShape* childShape = convertURDFToCollisionShape(&col, pathPrefix);
			if (childShape)
			{
				m_data->m_allocatedCollisionShapes.push_back(childShape);
				if (childShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
				{
					CompoundShape* compound = (CompoundShape*)childShape;
					for (i32 i = 0; i < compound->getNumChildShapes(); i++)
					{
						m_data->m_allocatedCollisionShapes.push_back(compound->getChildShape(i));
					}
				}

				Transform2 childTrans = col.m_linkLocalFrame;

				compoundShape->addChildShape(localInertiaFrame.inverse() * childTrans, childShape);
			}
		}
	}

	return compoundShape;
}
#endif
