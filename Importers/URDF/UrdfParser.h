#ifndef URDF_PARSER_H
#define URDF_PARSER_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>
#include "URDFJointTypes.h"
#include "SDFAudioTypes.h"
#include <string>

#define Array AlignedObjectArray

struct ErrorLogger
{
	virtual ~ErrorLogger() {}
	virtual void reportError(tukk error) = 0;
	virtual void reportWarning(tukk warning) = 0;
	virtual void printMessage(tukk msg) = 0;
};

struct UrdfMaterial
{
	STxt m_name;
	STxt m_textureFilename;
	UrdfMaterialColor m_matColor;

	UrdfMaterial()
	{
	}
};

struct UrdfInertia
{
	Transform2 m_linkLocalFrame;
	bool m_hasLinkLocalFrame;

	double m_mass;
	double m_ixx, m_ixy, m_ixz, m_iyy, m_iyz, m_izz;

	UrdfInertia()
	{
		m_hasLinkLocalFrame = false;
		m_linkLocalFrame.setIdentity();
		m_mass = 0.f;
		m_ixx = m_ixy = m_ixz = m_iyy = m_iyz = m_izz = 0.f;
	}
};

enum UrdfGeomTypes
{
	URDF_GEOM_SPHERE = 2,
	URDF_GEOM_BOX,
	URDF_GEOM_CYLINDER,
	URDF_GEOM_MESH,
	URDF_GEOM_PLANE,
	URDF_GEOM_CAPSULE,  //non-standard URDF
	URDF_GEOM_CDF,      //signed-distance-field, non-standard URDF
	URDF_GEOM_HEIGHTFIELD,   //heightfield, non-standard URDF
	URDF_GEOM_UNKNOWN,
};

struct UrdfGeometry
{
	UrdfGeomTypes m_type;

	double m_sphereRadius;

	Vec3 m_boxSize;

	double m_capsuleRadius;
	double m_capsuleHeight;
	i32 m_hasFromTo;
	Vec3 m_capsuleFrom;
	Vec3 m_capsuleTo;

	Vec3 m_planeNormal;

	enum
	{
		FILE_STL = 1,
		FILE_COLLADA = 2,
		FILE_OBJ = 3,
		FILE_CDF = 4,
		MEMORY_VERTICES = 5,
	        FILE_VTK = 6,

	};
	i32 m_meshFileType;
	STxt m_meshFileName;
	Vec3 m_meshScale;

	Array<Vec3> m_vertices;
	Array<Vec3> m_uvs;
	Array<Vec3> m_normals;
	Array<i32> m_indices;


	UrdfMaterial m_localMaterial;
	bool m_hasLocalMaterial;

	UrdfGeometry()
		: m_type(URDF_GEOM_UNKNOWN),
		  m_sphereRadius(1),
		  m_boxSize(1, 1, 1),
		  m_capsuleRadius(1),
		  m_capsuleHeight(1),
		  m_hasFromTo(0),
		  m_capsuleFrom(0, 1, 0),
		  m_capsuleTo(1, 0, 0),
		  m_planeNormal(0, 0, 1),
		  m_meshFileType(0),
		  m_meshScale(1, 1, 1),
		  m_hasLocalMaterial(false)
	{
	}
};


struct UrdfShape
{
	STxt m_sourceFileLocation;
	Transform2 m_linkLocalFrame;
	UrdfGeometry m_geometry;
	STxt m_name;
};

struct UrdfVisual : UrdfShape
{
	STxt m_materialName;
	// Maps user data keys to user data values.
	HashMap<HashString, STxt> m_userData;
};

struct UrdfCollision : UrdfShape
{
	i32 m_flags;
	i32 m_collisionGroup;
	i32 m_collisionMask;
	UrdfCollision()
		: m_flags(0)
	{
	}
};

struct UrdfJoint;

struct UrdfLink
{
	STxt m_name;
	UrdfInertia m_inertia;
	Transform2 m_linkTransformInWorld;
	Array<UrdfVisual> m_visualArray;
	Array<UrdfCollision> m_collisionArray;
	UrdfLink* m_parentLink;
	UrdfJoint* m_parentJoint;

	Array<UrdfJoint*> m_childJoints;
	Array<UrdfLink*> m_childLinks;

	i32 m_linkIndex;

	URDFLinkContactInfo m_contactInfo;

	SDFAudioSource m_audioSource;
	// Maps user data keys to user data values.
	HashMap<HashString, STxt> m_userData;

	UrdfLink()
		: m_parentLink(0),
		  m_parentJoint(0),
		  m_linkIndex(-2)
	{
	}
};
struct UrdfJoint
{
	STxt m_name;
	UrdfJointTypes m_type;
	Transform2 m_parentLinkToJointTransform;
	STxt m_parentLinkName;
	STxt m_childLinkName;
	Vec3 m_localJointAxis;

	double m_lowerLimit;
	double m_upperLimit;

	double m_effortLimit;
	double m_velocityLimit;

	double m_jointDamping;
	double m_jointFriction;
	double m_twistLimit;
	UrdfJoint()
		: m_lowerLimit(0),
		  m_upperLimit(-1),
		  m_effortLimit(0),
		  m_velocityLimit(0),
		  m_jointDamping(0),
		  m_jointFriction(0),
		  m_twistLimit(-1)
	{
	}
};

struct SpringCoeffcients
{
	double elastic_stiffness;
	double damping_stiffness;
	double bending_stiffness;
	i32 damp_all_directions;
	i32 bending_stride;
	SpringCoeffcients() : elastic_stiffness(0.),
						  damping_stiffness(0.),
						  bending_stiffness(0.),
						  damp_all_directions(0),
						  bending_stride(2) {}
};

struct LameCoefficients
{
	double mu;
	double lambda;
	double damping;
	LameCoefficients() : mu(0.), lambda(0.), damping(0.) {}
};

struct UrdfDeformable
{
	STxt m_name;
	double m_mass;
	double m_collisionMargin;
	double m_friction;
	double m_repulsionStiffness;
	double m_gravFactor;
	bool m_cache_barycenter;

	SpringCoeffcients m_springCoefficients;
	LameCoefficients m_corotatedCoefficients;
	LameCoefficients m_neohookeanCoefficients;

	STxt m_visualFileName;
	STxt m_simFileName;
	HashMap<HashString, STxt> m_userData;

	UrdfDeformable() : m_mass(1.), m_collisionMargin(0.02), m_friction(1.), m_repulsionStiffness(0.5), m_gravFactor(1.), m_cache_barycenter(false), m_visualFileName(""), m_simFileName("")
	{
	}
};

struct UrdfReducedDeformable
{
	STxt m_name;
	i32 m_numModes;

	double m_mass;
	double m_stiffnessScale;
	double m_erp;
	double m_cfm;
	double m_friction;
	double m_collisionMargin;
	double m_damping;

	STxt m_visualFileName;
	STxt m_simFileName;
	HashMap<HashString, STxt> m_userData;

	UrdfReducedDeformable()
		:	m_numModes(1),
			m_mass(1),
			m_stiffnessScale(100),
			m_erp(0.2),					// generally, 0.2 is a good value for erp and cfm
			m_cfm(0.2),
			m_friction(0),
			m_collisionMargin(0.02),
			m_damping(0),
			m_visualFileName(""),
			m_simFileName("")
	{}
};

struct UrdfModel
{
	STxt m_name;
	STxt m_sourceFile;
	Transform2 m_rootTransformInWorld;
	HashMap<HashString, UrdfMaterial*> m_materials;
	HashMap<HashString, UrdfLink*> m_links;
	HashMap<HashString, UrdfJoint*> m_joints;
	UrdfDeformable m_deformable;
	UrdfReducedDeformable m_reducedDeformable;
	// Maps user data keys to user data values.
	HashMap<HashString, STxt> m_userData;

	Array<UrdfLink*> m_rootLinks;
	bool m_overrideFixedBase;

	UrdfModel()
		: m_overrideFixedBase(false)
	{
		m_rootTransformInWorld.setIdentity();
	}

	~UrdfModel()
	{
		for (i32 i = 0; i < m_materials.size(); i++)
		{
			UrdfMaterial** ptr = m_materials.getAtIndex(i);
			if (ptr)
			{
				UrdfMaterial* t = *ptr;
				delete t;
			}
		}
		for (i32 i = 0; i < m_links.size(); i++)
		{
			UrdfLink** ptr = m_links.getAtIndex(i);
			if (ptr)
			{
				UrdfLink* t = *ptr;
				delete t;
			}
		}
		for (i32 i = 0; i < m_joints.size(); i++)
		{
			UrdfJoint** ptr = m_joints.getAtIndex(i);
			if (ptr)
			{
				UrdfJoint* t = *ptr;
				delete t;
			}
		}
	}
};

namespace tinyxml2
{
class XMLElement;
};

class UrdfParser
{
protected:
	UrdfModel m_urdf2Model;
	AlignedObjectArray<UrdfModel*> m_sdfModels;
	AlignedObjectArray<UrdfModel*> m_tmpModels;

	bool m_parseSDF;
	i32 m_activeSdfModel;

	Scalar m_urdfScaling;

	struct CommonFileIOInterface* m_fileIO;

	bool parseTransform(Transform2& tr, tinyxml2::XMLElement* xml, ErrorLogger* logger, bool parseSDF = false);
	bool parseInertia(UrdfInertia& inertia, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseGeometry(UrdfGeometry& geom, tinyxml2::XMLElement* g, ErrorLogger* logger);
	bool parseVisual(UrdfModel& model, UrdfVisual& visual, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseCollision(UrdfCollision& collision, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool initTreeAndRoot(UrdfModel& model, ErrorLogger* logger);
	bool parseMaterial(UrdfMaterial& material, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseJointLimits(UrdfJoint& joint, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseJointDynamics(UrdfJoint& joint, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseJoint(UrdfJoint& joint, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseLink(UrdfModel& model, UrdfLink& link, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseSensor(UrdfModel& model, UrdfLink& link, UrdfJoint& joint, tinyxml2::XMLElement* config, ErrorLogger* logger);
  bool parseLameCoefficients(LameCoefficients& lameCoefficients, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseDeformable(UrdfModel& model, tinyxml2::XMLElement* config, ErrorLogger* logger);
	bool parseReducedDeformable(UrdfModel& model, tinyxml2::XMLElement* config, ErrorLogger* logger);

public:
	UrdfParser(struct CommonFileIOInterface* fileIO);
	virtual ~UrdfParser();

	void setParseSDF(bool useSDF)
	{
		m_parseSDF = useSDF;
	}
	bool getParseSDF() const
	{
		return m_parseSDF;
	}
	void setGlobalScaling(Scalar scaling)
	{
		m_urdfScaling = scaling;
	}

	bool loadUrdf(tukk urdfText, ErrorLogger* logger, bool forceFixedBase, bool parseSensors);

	bool loadUrdf(tukk urdfText, ErrorLogger* logger, bool forceFixedBase)
	{
		return loadUrdf(urdfText, logger, forceFixedBase, false);
	}

	bool loadSDF(tukk sdfText, ErrorLogger* logger);

	i32 getNumModels() const
	{
		//user should have loaded an SDF when calling this method
		if (m_parseSDF)
		{
			return m_sdfModels.size();
		}
		return 1;
	}

	void activateModel(i32 modelIndex);

	UrdfModel& getModelByIndex(i32 index)
	{
		//user should have loaded an SDF when calling this method
		Assert(m_parseSDF);

		return *m_sdfModels[index];
	}

	const UrdfModel& getModelByIndex(i32 index) const
	{
		//user should have loaded an SDF when calling this method
		Assert(m_parseSDF);

		return *m_sdfModels[index];
	}

	const UrdfModel& getModel() const
	{
		if (m_parseSDF)
		{
			return *m_sdfModels[m_activeSdfModel];
		}

		return m_urdf2Model;
	}

	UrdfModel& getModel()
	{
		if (m_parseSDF)
		{
			return *m_sdfModels[m_activeSdfModel];
		}
		return m_urdf2Model;
	}

	const UrdfDeformable& getDeformable() const
	{
		return m_urdf2Model.m_deformable;
	}

	const UrdfReducedDeformable& getReducedDeformable() const
	{
		return m_urdf2Model.m_reducedDeformable;
	}

	bool mergeFixedLinks(UrdfModel& model, UrdfLink* link, ErrorLogger* logger, bool forceFixedBase, i32 level);
	bool printTree(UrdfLink* link, ErrorLogger* logger, i32 level);
	bool recreateModel(UrdfModel& model, UrdfLink* link, ErrorLogger* logger);

	STxt sourceFileLocation(tinyxml2::XMLElement* e);

	void setSourceFile(const STxt& sourceFile)
	{
		m_urdf2Model.m_sourceFile = sourceFile;
	}
};

#endif
