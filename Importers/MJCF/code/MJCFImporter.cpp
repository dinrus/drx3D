#include "../MJCFImporter.h"
#include <X/tinyxml2/tinyxml2.h>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Common/b3HashMap.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Importers/URDF/UrdfFindMeshFile.h>
#include <string>
#include <iostream>
#include <fstream>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Importers/URDF/UrdfParser.h>
#include <drx3D/Importers/URDF/urdfStringSplit.h>
#include <drx3D/Importers/URDF/urdfLexicalCast.h>
#include <drx3D/Importers/Obj/LoadMeshFromObj.h>
#include <drx3D/Importers/STL/LoadMeshFromSTL.h>
#include <drx3D/Importers/Collada/LoadMeshFromCollada.h>
#include <drx3D/OpenGLWindow/ShapeData.h>
#include <drx3D/Wavefront/tiny_obj_loader.h>
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexHullShape.h>
#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleMesh.h>

using namespace tinyxml2;

#define mjcf_sphere_indiced textured_detailed_sphere_indices
#define mjcf_sphere_vertices textured_detailed_sphere_vertices

static Vec4 sGoogleColors[4] =
    {
        Vec4(60. / 256., 186. / 256., 84. / 256., 1),
        Vec4(244. / 256., 194. / 256., 13. / 256., 1),
        Vec4(219. / 256., 50. / 256., 54. / 256., 1),
        Vec4(72. / 256., 133. / 256., 237. / 256., 1),
};

#include <vector>

enum ePARENT_LINK_ENUMS
{
    BASE_LINK_INDEX = -1,

    INVALID_LINK_INDEX = -2
};

static i32 gUid = 0;

static bool parseVector4(Vec4& vec4, const STxt& vector_str)
{
    vec4.setZero();
    Array<STxt> pieces;
    Array<float> rgba;
    AlignedObjectArray<STxt> strArray;
    urdfIsAnyOf(" ", strArray);
    urdfStringSplit(pieces, vector_str, strArray);
    for (i32 i = 0; i < pieces.size(); ++i)
    {
        if (!pieces[i].empty())
        {
            rgba.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
        }
    }
    if (rgba.size() != 4)
    {
        return false;
    }
    vec4.setVal(rgba[0], rgba[1], rgba[2], rgba[3]);
    return true;
}

static bool parseVector3(Vec3& vec3, const STxt& vector_str, MJCFErrorLogger* logger, bool lastThree = false)
{
    vec3.setZero();
    Array<STxt> pieces;
    Array<float> rgba;
    AlignedObjectArray<STxt> strArray;
    urdfIsAnyOf(" ", strArray);
    urdfStringSplit(pieces, vector_str, strArray);
    for (i32 i = 0; i < pieces.size(); ++i)
    {
        if (!pieces[i].empty())
        {
            rgba.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
        }
    }
    if (rgba.size() < 3)
    {
        logger->reportWarning(("Couldn't parse vector3 '" + vector_str + "'").c_str());
        return false;
    }
    if (lastThree)
    {
        vec3.setVal(rgba[rgba.size() - 3], rgba[rgba.size() - 2], rgba[rgba.size() - 1]);
    }
    else
    {
        vec3.setVal(rgba[0], rgba[1], rgba[2]);
    }
    return true;
}

static bool parseVector6(Vec3& v0, Vec3& v1, const STxt& vector_str, MJCFErrorLogger* logger)
{
    v0.setZero();
    v1.setZero();

    Array<STxt> pieces;
    Array<float> values;
    AlignedObjectArray<STxt> strArray;
    urdfIsAnyOf(" ", strArray);
    urdfStringSplit(pieces, vector_str, strArray);
    for (i32 i = 0; i < pieces.size(); ++i)
    {
        if (!pieces[i].empty())
        {
            values.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
        }
    }
    if (values.size() < 6)
    {
        logger->reportWarning(("Couldn't parse 6 floats '" + vector_str + "'").c_str());

        return false;
    }
    v0.setVal(values[0], values[1], values[2]);
    v1.setVal(values[3], values[4], values[5]);

    return true;
}

struct MyMJCFAsset
{
    STxt m_fileName;
};

struct MyMJCFDefaults
{
    i32 m_defaultCollisionGroup;
    i32 m_defaultCollisionMask;
    Scalar m_defaultCollisionMargin;

    // joint defaults
    STxt m_defaultJointLimited;

    // geom defaults
    STxt m_defaultGeomRgba;
    i32 m_defaultConDim;
    double m_defaultLateralFriction;
    double m_defaultSpinningFriction;
    double m_defaultRollingFriction;

    MyMJCFDefaults()
        : m_defaultCollisionGroup(1),
          m_defaultCollisionMask(1),
          m_defaultCollisionMargin(0.001),  //assume unit meters, margin is 1mm
          m_defaultConDim(3),
          m_defaultLateralFriction(0.5),
          m_defaultSpinningFriction(0),
          m_defaultRollingFriction(0)
    {
    }
};

struct BulletMJCFImporterInternalData
{
    GUIHelperInterface* m_guiHelper;
    struct UrdfRenderingInterface* m_customVisualShapesConverter;
    char m_pathPrefix[1024];

    STxt m_sourceFileName;  // with path
    STxt m_fileModelName;   // without path
    HashMap<HashString, MyMJCFAsset> m_assets;

    AlignedObjectArray<UrdfModel*> m_models;

    //<compiler angle="radian" meshdir="mesh/" texturedir="texture/" inertiafromgeom="true"/>
    STxt m_meshDir;
    STxt m_textureDir;
    STxt m_angleUnits;
    bool m_inertiaFromGeom;

    i32 m_activeModel;
    i32 m_activeBodyUniqueId;

    //todo: for better MJCF compatibility, we would need a stack of default values
    MyMJCFDefaults m_globalDefaults;
    b3HashMap<b3HashString, MyMJCFDefaults> m_classDefaults;

    //those collision shapes are deleted by caller (todo: make sure this happens!)
    AlignedObjectArray<CollisionShape*> m_allocatedCollisionShapes;
    mutable AlignedObjectArray<TriangleMesh*> m_allocatedMeshInterfaces;

    i32 m_flags;
    i32 m_textureId;

    CommonFileIOInterface* m_fileIO;

    BulletMJCFImporterInternalData()
        : m_inertiaFromGeom(true),
          m_activeModel(-1),
          m_activeBodyUniqueId(-1),
          m_flags(0),
          m_textureId(-1),
          m_fileIO(0)
    {
        m_pathPrefix[0] = 0;
    }

    ~BulletMJCFImporterInternalData()
    {
        for (i32 i = 0; i < m_models.size(); i++)
        {
            delete m_models[i];
        }
    }

    STxt sourceFileLocation(XMLElement* e)
    {
#if 0
    //no C++11 snprintf etc
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s:%i", m_sourceFileName.c_str(), e->Row());
        return buf;
#else
        char row[1024];
#ifdef G3_TINYXML2
        sprintf(row, "unknown line, upgrade tinyxml2 version!");
#else
        sprintf(row, "%d", e->GetLineNum());
#endif
        STxt str = m_sourceFileName.c_str() + STxt(":") + STxt(row);
        return str;
#endif
    }

    const UrdfLink* getLink(i32 modelIndex, i32 linkIndex) const
    {
        if (modelIndex >= 0 && modelIndex < m_models.size())
        {
            UrdfLink** linkPtrPtr = m_models[modelIndex]->m_links.getAtIndex(linkIndex);
            if (linkPtrPtr && *linkPtrPtr)
            {
                UrdfLink* linkPtr = *linkPtrPtr;
                return linkPtr;
            }
        }
        return 0;
    }

    void parseCompiler(XMLElement* root_xml, MJCFErrorLogger* logger)
    {
        tukk meshDirStr = root_xml->Attribute("meshdir");
        if (meshDirStr)
        {
            m_meshDir = meshDirStr;
        }
        tukk textureDirStr = root_xml->Attribute("texturedir");
        if (textureDirStr)
        {
            m_textureDir = textureDirStr;
        }
        tukk angle = root_xml->Attribute("angle");
        m_angleUnits = angle ? angle : "degree";  // degrees by default, http://www.mujoco.org/book/modeling.html#compiler
        tukk inertiaFromGeom = root_xml->Attribute("inertiafromgeom");
        if (inertiaFromGeom && inertiaFromGeom[0] == 'f')  // false, other values assumed `true`.
        {
            m_inertiaFromGeom = false;
        }
    }

    void parseAssets(XMLElement* root_xml, MJCFErrorLogger* logger)
    {
        //      <mesh name="index0"     file="index0.stl"/>
        for (XMLElement* child_xml = root_xml->FirstChildElement(); child_xml; child_xml = child_xml->NextSiblingElement())
        {
            STxt n = child_xml->Val();
            if (n == "mesh")
            {
                tukk assetNameStr = child_xml->Attribute("name");
                tukk fileNameStr = child_xml->Attribute("file");
                if (assetNameStr && fileNameStr)
                {
                    HashString assetName = assetNameStr;
                    MyMJCFAsset asset;
                    asset.m_fileName = m_meshDir + fileNameStr;
                    m_assets.insert(assetName, asset);
                }
            }
        }
    }

    bool parseDefaults(MyMJCFDefaults& defaults, XMLElement* root_xml, MJCFErrorLogger* logger)
    {
        bool handled = false;
        //rudimentary 'default' support, would need more work for better feature coverage
        for (XMLElement* child_xml = root_xml->FirstChildElement(); child_xml; child_xml = child_xml->NextSiblingElement())
        {
            STxt n = child_xml->Val();

            if (n.find("default") != STxt::npos)
            {
                tukk className = child_xml->Attribute("class");

                if (className)
                {
                    MyMJCFDefaults* curDefaultsPtr = m_classDefaults[className];
                    if (!curDefaultsPtr)
                    {
                        MyMJCFDefaults def;
                        m_classDefaults.insert(className, def);
                        curDefaultsPtr = m_classDefaults[className];
                    }
                    if (curDefaultsPtr)
                    {
                        MyMJCFDefaults& curDefaults = *curDefaultsPtr;
                        parseDefaults(curDefaults, child_xml, logger);
                    }
                }
            }

            if (n == "inertial")
            {
            }
            if (n == "asset")
            {
                parseAssets(child_xml, logger);
            }
            if (n == "joint")
            {
                // Other attributes here:
                // armature="1"
                // damping="1"
                // limited="true"
                if (tukk conTypeStr = child_xml->Attribute("limited"))
                {
                    defaults.m_defaultJointLimited = child_xml->Attribute("limited");
                }
            }
            if (n == "geom")
            {
                //contype, conaffinity
                tukk conTypeStr = child_xml->Attribute("contype");
                if (conTypeStr)
                {
                    defaults.m_defaultCollisionGroup = urdfLexicalCast<i32>(conTypeStr);
                }
                tukk conAffinityStr = child_xml->Attribute("conaffinity");
                if (conAffinityStr)
                {
                    defaults.m_defaultCollisionMask = urdfLexicalCast<i32>(conAffinityStr);
                }
                tukk rgba = child_xml->Attribute("rgba");
                if (rgba)
                {
                    defaults.m_defaultGeomRgba = rgba;
                }

                tukk conDimS = child_xml->Attribute("condim");
                if (conDimS)
                {
                    defaults.m_defaultConDim = urdfLexicalCast<i32>(conDimS);
                }
                i32 conDim = defaults.m_defaultConDim;

                tukk frictionS = child_xml->Attribute("friction");
                if (frictionS)
                {
                    Array<STxt> pieces;
                    Array<float> frictions;
                    AlignedObjectArray<STxt> strArray;
                    urdfIsAnyOf(" ", strArray);
                    urdfStringSplit(pieces, frictionS, strArray);
                    for (i32 i = 0; i < pieces.size(); ++i)
                    {
                        if (!pieces[i].empty())
                        {
                            frictions.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
                        }
                    }
                    if (frictions.size() > 0)
                    {
                        defaults.m_defaultLateralFriction = frictions[0];
                    }
                    if (frictions.size() > 1)
                    {
                        defaults.m_defaultSpinningFriction = frictions[1];
                    }
                    if (frictions.size() > 2)
                    {
                        defaults.m_defaultRollingFriction = frictions[2];
                    }
                }
            }
        }
        handled = true;
        return handled;
    }
    bool parseRootLevel(MyMJCFDefaults& defaults, XMLElement* root_xml, MJCFErrorLogger* logger)
    {
        for (XMLElement* rootxml = root_xml->FirstChildElement(); rootxml; rootxml = rootxml->NextSiblingElement())
        {
            bool handled = false;
            STxt n = rootxml->Val();

            if (n == "body")
            {
                i32 modelIndex = m_models.size();
                UrdfModel* model = new UrdfModel();
                m_models.push_back(model);
                parseBody(defaults, rootxml, modelIndex, INVALID_LINK_INDEX, logger);
                initTreeAndRoot(*model, logger);
                handled = true;
            }

            if (n == "geom")
            {
                i32 modelIndex = m_models.size();
                UrdfModel* modelPtr = new UrdfModel();
                m_models.push_back(modelPtr);

                UrdfLink* linkPtr = new UrdfLink();
                linkPtr->m_name = "anonymous";
                tukk namePtr = rootxml->Attribute("name");
                if (namePtr)
                {
                    linkPtr->m_name = namePtr;
                }
                i32 linkIndex = modelPtr->m_links.size();
                linkPtr->m_linkIndex = linkIndex;
                modelPtr->m_links.insert(linkPtr->m_name.c_str(), linkPtr);

                //don't parse geom transform here, it will be inside 'parseGeom'
                linkPtr->m_linkTransformInWorld.setIdentity();

                //              modelPtr->m_rootLinks.push_back(linkPtr);

                Vec3 inertialShift(0, 0, 0);
                parseGeom(defaults, rootxml, modelIndex, linkIndex, logger, inertialShift);
                initTreeAndRoot(*modelPtr, logger);

                handled = true;
            }

            if (n == "site")
            {
                handled = true;
            }
            if (!handled)
            {
                logger->reportWarning((sourceFileLocation(rootxml) + ": unhandled root element '" + n + "'").c_str());
            }
        }
        return true;
    }

    bool parseJoint(MyMJCFDefaults& defaults, XMLElement* link_xml, i32 modelIndex, i32 parentLinkIndex, i32 linkIndex, MJCFErrorLogger* logger, const Transform2& parentToLinkTrans, Transform2& jointTransOut)
    {
        bool jointHandled = false;
        tukk jType = link_xml->Attribute("type");
        tukk limitedStr = link_xml->Attribute("limited");
        tukk axisStr = link_xml->Attribute("axis");
        tukk posStr = link_xml->Attribute("pos");
        tukk ornStr = link_xml->Attribute("quat");
        tukk nameStr = link_xml->Attribute("name");
        tukk rangeStr = link_xml->Attribute("range");

        Transform2 jointTrans;
        jointTrans.setIdentity();
        if (posStr)
        {
            Vec3 pos;
            STxt p = posStr;
            if (parseVector3(pos, p, logger))
            {
                jointTrans.setOrigin(pos);
            }
        }
        if (ornStr)
        {
            STxt o = ornStr;
            Vec4 o4;
            if (parseVector4(o4, o))
            {
                Quat orn(o4[3], o4[0], o4[1], o4[2]);
                jointTrans.setRotation(orn);
            }
        }

        Vec3 jointAxis(1, 0, 0);
        if (axisStr)
        {
            STxt ax = axisStr;
            parseVector3(jointAxis, ax, logger);
        }
        else
        {
            logger->reportWarning((sourceFileLocation(link_xml) + ": joint without axis attribute").c_str());
        }

        double range[2] = {1, 0};
        STxt lim = m_globalDefaults.m_defaultJointLimited;
        if (limitedStr)
        {
            lim = limitedStr;
        }
        bool isLimited = lim == "true";

        UrdfJointTypes ejtype;
        if (jType)
        {
            STxt jointType = jType;
            if (jointType == "fixed")
            {
                ejtype = URDFFixedJoint;
                jointHandled = true;
            }
            if (jointType == "slide")
            {
                ejtype = URDFPrismaticJoint;
                jointHandled = true;
            }
            if (jointType == "hinge")
            {
                if (isLimited)
                {
                    ejtype = URDFRevoluteJoint;
                }
                else
                {
                    ejtype = URDFContinuousJoint;
                }
                jointHandled = true;
            }
        }
        else
        {
            logger->reportWarning((sourceFileLocation(link_xml) + ": expected 'type' attribute for joint").c_str());
        }

        if (isLimited)
        {
            //parse the 'range' field
            Array<STxt> pieces;
            Array<float> limits;
            AlignedObjectArray<STxt> strArray;
            urdfIsAnyOf(" ", strArray);
            urdfStringSplit(pieces, rangeStr, strArray);
            for (i32 i = 0; i < pieces.size(); ++i)
            {
                if (!pieces[i].empty())
                {
                    limits.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
                }
            }
            if (limits.size() == 2)
            {
                range[0] = limits[0];
                range[1] = limits[1];
                if (m_angleUnits == "degree" && ejtype == URDFRevoluteJoint)
                {
                    range[0] = limits[0] * D3_PI / 180;
                    range[1] = limits[1] * D3_PI / 180;
                }
            }
            else
            {
                logger->reportWarning((sourceFileLocation(link_xml) + ": cannot parse 'range' attribute (units='" + m_angleUnits + "'')").c_str());
            }
        }

        // TODO armature : real, "0" Armature inertia (or rotor inertia) of all
        // degrees of freedom created by this joint. These are constants added to the
        // diagonal of the inertia matrix in generalized coordinates. They make the
        // simulation more stable, and often increase physical realism. This is because
        // when a motor is attached to the system with a transmission that amplifies
        // the motor force by c, the inertia of the rotor (i.e. the moving part of the
        // motor) is amplified by c*c. The same holds for gears in the early stages of
        // planetary gear boxes. These extra inertias often dominate the inertias of
        // the robot parts that are represented explicitly in the model, and the
        // armature attribute is the way to model them.

        // TODO damping : real, "0" Damping applied to all degrees of
        // freedom created by this joint. Unlike friction loss
        // which is computed by the constraint solver, damping is
        // simply a force linear in velocity. It is included in
        // the passive forces. Despite this simplicity, larger
        // damping values can make numerical integrators unstable,
        // which is why our Euler integrator handles damping
        // implicitly. See Integration in the Computation chapter.

        const UrdfLink* linkPtr = getLink(modelIndex, linkIndex);

        Transform2 parentLinkToJointTransform;
        parentLinkToJointTransform.setIdentity();
        parentLinkToJointTransform = parentToLinkTrans * jointTrans;
        jointTransOut = jointTrans;

        if (jointHandled)
        {
            UrdfJoint* jointPtr = new UrdfJoint();
            jointPtr->m_childLinkName = linkPtr->m_name;
            const UrdfLink* parentLink = getLink(modelIndex, parentLinkIndex);
            jointPtr->m_parentLinkName = parentLink->m_name;
            jointPtr->m_localJointAxis = jointAxis;
            jointPtr->m_parentLinkToJointTransform = parentLinkToJointTransform;
            jointPtr->m_type = ejtype;
            i32 numJoints = m_models[modelIndex]->m_joints.size();

            //range
            jointPtr->m_lowerLimit = range[0];
            jointPtr->m_upperLimit = range[1];

            if (nameStr)
            {
                jointPtr->m_name = nameStr;
            }
            else
            {
                char jointName[1024];
                sprintf(jointName, "joint%d_%d_%d", gUid++, linkIndex, numJoints);
                jointPtr->m_name = jointName;
            }
            m_models[modelIndex]->m_joints.insert(jointPtr->m_name.c_str(), jointPtr);
            return true;
        }
        /*
        URDFRevoluteJoint=1,
        URDFPrismaticJoint,
        URDFContinuousJoint,
        URDFFloatingJoint,
        URDFPlanarJoint,
        URDFFixedJoint,
        */
        return false;
    }
    bool parseGeom(MyMJCFDefaults& defaults, XMLElement* link_xml, i32 modelIndex, i32 linkIndex, MJCFErrorLogger* logger, Vec3& inertialShift)
    {
        UrdfLink** linkPtrPtr = m_models[modelIndex]->m_links.getAtIndex(linkIndex);
        if (linkPtrPtr == 0)
        {
            // XXX: should it be assert?
            logger->reportWarning("Invalide linkindex");
            return false;
        }
        UrdfLink* linkPtr = *linkPtrPtr;

        Transform2 linkLocalFrame;
        linkLocalFrame.setIdentity();

        bool handledGeomType = false;
        UrdfGeometry geom;

        tukk sz = link_xml->Attribute("size");
        i32 conDim = defaults.m_defaultConDim;

        tukk conDimS = link_xml->Attribute("condim");
        {
            if (conDimS)
            {
                conDim = urdfLexicalCast<i32>(conDimS);
            }
        }

        double lateralFriction = defaults.m_defaultLateralFriction;
        double spinningFriction = defaults.m_defaultSpinningFriction;
        double rollingFriction = defaults.m_defaultRollingFriction;

        tukk frictionS = link_xml->Attribute("friction");
        if (frictionS)
        {
            Array<STxt> pieces;
            Array<float> frictions;
            AlignedObjectArray<STxt> strArray;
            urdfIsAnyOf(" ", strArray);
            urdfStringSplit(pieces, frictionS, strArray);
            for (i32 i = 0; i < pieces.size(); ++i)
            {
                if (!pieces[i].empty())
                {
                    frictions.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
                }
            }
            if (frictions.size() > 0)
            {
                lateralFriction = frictions[0];
            }
            if (frictions.size() > 1 && conDim > 3)
            {
                spinningFriction = frictions[1];
            }
            if (frictions.size() > 2 && conDim > 4)
            {
                rollingFriction = frictions[2];
            }
        }

        linkPtr->m_contactInfo.m_lateralFriction = lateralFriction;
        linkPtr->m_contactInfo.m_spinningFriction = spinningFriction;
        linkPtr->m_contactInfo.m_rollingFriction = rollingFriction;

        if (conDim > 3)
        {
            linkPtr->m_contactInfo.m_spinningFriction = defaults.m_defaultSpinningFriction;
            linkPtr->m_contactInfo.m_flags |= URDF_CONTACT_HAS_SPINNING_FRICTION;
        }
        if (conDim > 4)
        {
            linkPtr->m_contactInfo.m_rollingFriction = defaults.m_defaultRollingFriction;
            linkPtr->m_contactInfo.m_flags |= URDF_CONTACT_HAS_ROLLING_FRICTION;
        }

        {
            if (m_flags & CUF_GOOGLEY_UNDEFINED_COLORS)
            {
                geom.m_localMaterial.m_matColor.m_rgbaColor = sGoogleColors[linkIndex & 3];
            }
            else
            {
                geom.m_localMaterial.m_matColor.m_rgbaColor.setVal(1, 1, 1, 1);
            }
            geom.m_localMaterial.m_matColor.m_specularColor.setVal(1, 1, 1);
            geom.m_hasLocalMaterial = true;
        }

        STxt rgba = defaults.m_defaultGeomRgba;
        if (tukk rgbattr = link_xml->Attribute("rgba"))
        {
            rgba = rgbattr;
        }
        if (!rgba.empty())
        {
            // "0 0.7 0.7 1"
            if ((m_flags & CUF_MJCF_COLORS_FROM_FILE))
            {
                parseVector4(geom.m_localMaterial.m_matColor.m_rgbaColor, rgba);
                geom.m_hasLocalMaterial = true;
                geom.m_localMaterial.m_name = rgba;
            }
        }

        tukk posS = link_xml->Attribute("pos");
        if (posS)
        {
            Vec3 pos(0, 0, 0);
            STxt p = posS;
            if (parseVector3(pos, p, logger))
            {
                linkLocalFrame.setOrigin(pos);
            }
        }

        tukk ornS = link_xml->Attribute("quat");
        if (ornS)
        {
            Quat orn(0, 0, 0, 1);
            Vec4 o4;
            if (parseVector4(o4, ornS))
            {
                orn.setVal(o4[1], o4[2], o4[3], o4[0]);
                linkLocalFrame.setRotation(orn);
            }
        }

        tukk axis_and_angle = link_xml->Attribute("axisangle");
        if (axis_and_angle)
        {
            Quat orn(0, 0, 0, 1);
            Vec4 o4;
            if (parseVector4(o4, axis_and_angle))
            {
                orn.setRotation(Vec3(o4[0], o4[1], o4[2]), o4[3]);
                linkLocalFrame.setRotation(orn);
            }
        }

        tukk gType = link_xml->Attribute("type");
        if (gType)
        {
            STxt geomType = gType;

            if (geomType == "plane")
            {
                geom.m_type = URDF_GEOM_PLANE;
                geom.m_planeNormal.setVal(0, 0, 1);
                Vec3 size(1, 1, 1);
                if (sz)
                {
                    STxt sizeStr = sz;
                    bool lastThree = false;
                    parseVector3(size, sizeStr, logger, lastThree);
                }
                geom.m_boxSize = 2*size;
                handledGeomType = true;
            }
            if (geomType == "box")
            {
                Vec3 size(1, 1, 1);
                if (sz)
                {
                    STxt sizeStr = sz;
                    bool lastThree = false;
                    parseVector3(size, sizeStr, logger, lastThree);
                }
                geom.m_type = URDF_GEOM_BOX;
                geom.m_boxSize = 2*size;
                handledGeomType = true;
            }

            if (geomType == "sphere")
            {
                geom.m_type = URDF_GEOM_SPHERE;
                if (sz)
                {
                    geom.m_sphereRadius = urdfLexicalCast<double>(sz);
                }
                else
                {
                    logger->reportWarning((sourceFileLocation(link_xml) + ": no size field (scalar) in sphere geom").c_str());
                }
                handledGeomType = true;
            }

            if (geomType == "capsule" || geomType == "cylinder")
            {
                // <geom conaffinity="0" contype="0" fromto="0 0 0 0 0 0.02" name="root" rgba="0.9 0.4 0.6 1" size=".011" type="cylinder"/>
                geom.m_type = geomType == "cylinder" ? URDF_GEOM_CYLINDER : URDF_GEOM_CAPSULE;

                Array<STxt> pieces;
                Array<float> sizes;
                AlignedObjectArray<STxt> strArray;
                urdfIsAnyOf(" ", strArray);
                urdfStringSplit(pieces, sz, strArray);
                for (i32 i = 0; i < pieces.size(); ++i)
                {
                    if (!pieces[i].empty())
                    {
                        sizes.push_back(urdfLexicalCast<double>(pieces[i].c_str()));
                    }
                }

                geom.m_capsuleRadius = 2.00f;  // 2 to make it visible if something is wrong
                geom.m_capsuleHeight = 2.00f;

                if (sizes.size() > 0)
                {
                    geom.m_capsuleRadius = sizes[0];
                    if (sizes.size() > 1)
                    {
                        geom.m_capsuleHeight = 2 * sizes[1];
                    }
                }
                else
                {
                    logger->reportWarning((sourceFileLocation(link_xml) + ": couldn't convert 'size' attribute of capsule geom").c_str());
                }
                tukk fromtoStr = link_xml->Attribute("fromto");
                geom.m_hasFromTo = false;

                if (fromtoStr)
                {
                    geom.m_hasFromTo = true;
                    STxt fromto = fromtoStr;
                    parseVector6(geom.m_capsuleFrom, geom.m_capsuleTo, fromto, logger);
                    inertialShift = 0.5 * (geom.m_capsuleFrom + geom.m_capsuleTo);
                    handledGeomType = true;
                }
                else
                {
                    if (sizes.size() < 2)
                    {
                        logger->reportWarning((sourceFileLocation(link_xml) + ": capsule without fromto attribute requires 2 sizes (radius and halfheight)").c_str());
                    }
                    else
                    {
                        handledGeomType = true;
                    }
                }
            }
            if (geomType == "mesh")
            {
                tukk meshStr = link_xml->Attribute("mesh");
                if (meshStr)
                {
                    MyMJCFAsset* assetPtr = m_assets[meshStr];
                    if (assetPtr)
                    {
                        geom.m_type = URDF_GEOM_MESH;
                        geom.m_meshFileName = assetPtr->m_fileName;
                        bool exists = UrdfFindMeshFile(m_fileIO,
                            m_sourceFileName, assetPtr->m_fileName, sourceFileLocation(link_xml),
                            &geom.m_meshFileName,
                            &geom.m_meshFileType);
                        handledGeomType = exists;

                        geom.m_meshScale.setVal(1, 1, 1);
                        //todo: parse mesh scale
                        if (sz)
                        {
                        }
                    }
                }
            }
            if (handledGeomType)
            {
                {
                    UrdfCollision col;
                    col.m_flags |= URDF_HAS_COLLISION_GROUP;
                    col.m_collisionGroup = defaults.m_defaultCollisionGroup;

                    col.m_flags |= URDF_HAS_COLLISION_MASK;
                    col.m_collisionMask = defaults.m_defaultCollisionMask;

                    //contype, conaffinity
                    tukk conTypeStr = link_xml->Attribute("contype");
                    if (conTypeStr)
                    {
                        col.m_flags |= URDF_HAS_COLLISION_GROUP;
                        col.m_collisionGroup = urdfLexicalCast<i32>(conTypeStr);
                    }
                    tukk conAffinityStr = link_xml->Attribute("conaffinity");
                    if (conAffinityStr)
                    {
                        col.m_flags |= URDF_HAS_COLLISION_MASK;
                        col.m_collisionMask = urdfLexicalCast<i32>(conAffinityStr);
                    }

                    col.m_geometry = geom;
                    col.m_linkLocalFrame = linkLocalFrame;
                    col.m_sourceFileLocation = sourceFileLocation(link_xml);
                    linkPtr->m_collisionArray.push_back(col);
                }
                {
                    UrdfVisual vis;
                    vis.m_geometry = geom;
                    vis.m_linkLocalFrame = linkLocalFrame;
                    vis.m_sourceFileLocation = sourceFileLocation(link_xml);
                    linkPtr->m_visualArray.push_back(vis);
                }
            }
            else
            {
                logger->reportWarning((sourceFileLocation(link_xml) + ": unhandled geom type '" + geomType + "'").c_str());
            }
        }
        else
        {
            logger->reportWarning((sourceFileLocation(link_xml) + ": geom requires type").c_str());
        }

        return handledGeomType;
    }

    Transform2 parseTransform(XMLElement* link_xml, MJCFErrorLogger* logger)
    {
        Transform2 tr;
        tr.setIdentity();

        tukk p = link_xml->Attribute("pos");
        if (p)
        {
            Vec3 pos(0, 0, 0);
            STxt pstr = p;
            if (parseVector3(pos, pstr, logger))
            {
                tr.setOrigin(pos);
            }
        }
        else
        {
            //          logger->reportWarning("body should have pos attribute");
        }
        tukk o = link_xml->Attribute("quat");
        if (o)
        {
            STxt ornstr = o;
            Vec4 o4;
            Quat orn(0, 0, 0, 1);
            if (parseVector4(o4, ornstr))
            {
                orn.setVal(o4[1], o4[2], o4[3], o4[0]);  //MuJoCo quats are [w,x,y,z], drx3D uses [x,y,z,w]
                tr.setRotation(orn);
            }
        }
        else
        {
            //          logger->reportWarning("body doesn't have quat (orientation) attribute");
        }
        return tr;
    }

    double computeVolume(const UrdfLink* linkPtr, MJCFErrorLogger* logger) const
    {
        double totalVolume = 0;

        for (i32 i = 0; i < linkPtr->m_collisionArray.size(); i++)
        {
            const UrdfCollision* col = &linkPtr->m_collisionArray[i];
            switch (col->m_geometry.m_type)
            {
                case URDF_GEOM_SPHERE:
                {
                    double r = col->m_geometry.m_sphereRadius;
                    totalVolume += 4. / 3. * SIMD_PI * r * r * r;
                    break;
                }
                case URDF_GEOM_BOX:
                {
                    totalVolume += col->m_geometry.m_boxSize[0] *
                                   col->m_geometry.m_boxSize[1] *
                                   col->m_geometry.m_boxSize[2];
                    break;
                }
                case URDF_GEOM_MESH:
                {
                    //todo (based on mesh bounding box?)
                    break;
                }
                case URDF_GEOM_PLANE:
                {
                    //todo
                    break;
                }
                case URDF_GEOM_CDF:
                {
                    //todo
                    break;
                }
                case URDF_GEOM_CYLINDER:
                case URDF_GEOM_CAPSULE:
                {
                    //one sphere
                    double r = col->m_geometry.m_capsuleRadius;
                    if (col->m_geometry.m_type == URDF_GEOM_CAPSULE)
                    {
                        totalVolume += 4. / 3. * SIMD_PI * r * r * r;
                    }
                    Scalar h(0);
                    if (col->m_geometry.m_hasFromTo)
                    {
                        //and one cylinder of 'height'
                        h = (col->m_geometry.m_capsuleFrom - col->m_geometry.m_capsuleTo).length();
                    }
                    else
                    {
                        h = col->m_geometry.m_capsuleHeight;
                    }
                    totalVolume += SIMD_PI * r * r * h;
                    break;
                }
                default:
                {
                }
            }
        }

        return totalVolume;
    }
    UrdfLink* getLink(i32 modelIndex, i32 linkIndex)
    {
        UrdfLink** linkPtrPtr = m_models[modelIndex]->m_links.getAtIndex(linkIndex);
        if (linkPtrPtr && *linkPtrPtr)
        {
            return *linkPtrPtr;
        }
        Assert(0);
        return 0;
    }

    i32 createBody(i32 modelIndex, tukk namePtr)
    {
        UrdfModel* modelPtr = m_models[modelIndex];
        i32 orgChildLinkIndex = modelPtr->m_links.size();
        UrdfLink* linkPtr = new UrdfLink();
        char linkn[1024];
        sprintf(linkn, "link%d_%d", modelIndex, orgChildLinkIndex);
        linkPtr->m_name = linkn;
        if (namePtr)
        {
            linkPtr->m_name = namePtr;
        }
        linkPtr->m_linkIndex = orgChildLinkIndex;
        modelPtr->m_links.insert(linkPtr->m_name.c_str(), linkPtr);

        return orgChildLinkIndex;
    }

    bool parseBody(MyMJCFDefaults& defaults, XMLElement* link_xml, i32 modelIndex, i32 orgParentLinkIndex, MJCFErrorLogger* logger)
    {
        MyMJCFDefaults curDefaults = defaults;

        i32 newParentLinkIndex = orgParentLinkIndex;

        tukk childClassName = link_xml->Attribute("childclass");
        if (childClassName)
        {
            MyMJCFDefaults* classDefaults = m_classDefaults[childClassName];
            if (classDefaults)
            {
                curDefaults = *classDefaults;
            }
        }
        tukk bodyName = link_xml->Attribute("name");
        i32 orgChildLinkIndex = createBody(modelIndex, bodyName);
        Transform2 localInertialFrame;
        localInertialFrame.setIdentity();

        //      i32 curChildLinkIndex = orgChildLinkIndex;
        STxt bodyN;

        if (bodyName)
        {
            bodyN = bodyName;
        }
        else
        {
            char anon[1024];
            sprintf(anon, "anon%d", gUid++);
            bodyN = anon;
        }

        //      Transform2 orgLinkTransform = parseTransform(link_xml,logger);

        Transform2 linkTransform = parseTransform(link_xml, logger);
        UrdfLink* linkPtr = getLink(modelIndex, orgChildLinkIndex);

        bool massDefined = false;

        Scalar mass = 0.f;
        Vec3 localInertiaDiag(0, 0, 0);
        //  i32 thisLinkIndex = -2;
        bool hasJoint = false;
        Transform2 jointTrans;
        jointTrans.setIdentity();
        bool skipFixedJoint = false;

        for (XMLElement* xml = link_xml->FirstChildElement(); xml; xml = xml->NextSiblingElement())
        {
            bool handled = false;
            STxt n = xml->Val();
            if (n == "inertial")
            {
                //   <inertial pos="0 0 0" quat="0.5 0.5 -0.5 0.5" mass="297.821" diaginertia="109.36 69.9714 69.9714" />

                tukk p = xml->Attribute("pos");
                if (p)
                {
                    STxt posStr = p;
                    Vec3 inertialPos(0, 0, 0);
                    if (parseVector3(inertialPos, posStr, logger))
                    {
                        localInertialFrame.setOrigin(inertialPos);
                    }
                }
                tukk o = xml->Attribute("quat");
                if (o)
                {
                    STxt ornStr = o;
                    Quat orn(0, 0, 0, 1);
                    Vec4 o4;
                    if (parseVector4(o4, ornStr))
                    {
                        orn.setVal(o4[1], o4[2], o4[3], o4[0]);
                        localInertialFrame.setRotation(orn);
                    }
                }
                tukk m = xml->Attribute("mass");
                if (m)
                {
                    mass = urdfLexicalCast<double>(m);
                }
                tukk i = xml->Attribute("diaginertia");
                if (i)
                {
                    STxt istr = i;
                    parseVector3(localInertiaDiag, istr, logger);
                }

                massDefined = true;
                handled = true;

                if (!m_inertiaFromGeom)
                {
                    linkPtr->m_inertia.m_mass = mass;
                    linkPtr->m_inertia.m_linkLocalFrame = localInertialFrame;
                    linkPtr->m_inertia.m_ixx = localInertiaDiag[0];
                    linkPtr->m_inertia.m_iyy = localInertiaDiag[1];
                    linkPtr->m_inertia.m_izz = localInertiaDiag[2];
                }
            }

            if (n == "joint")
            {
                if (!hasJoint)
                {
                    tukk jType = xml->Attribute("type");
                    STxt jointType = jType ? jType : "";

                    if (newParentLinkIndex != INVALID_LINK_INDEX || jointType != "free")
                    {
                        if (newParentLinkIndex == INVALID_LINK_INDEX)
                        {
                            i32 newRootLinkIndex = createBody(modelIndex, 0);
                            UrdfLink* rootLink = getLink(modelIndex, newRootLinkIndex);
                            rootLink->m_inertia.m_mass = 0;
                            rootLink->m_linkTransformInWorld.setIdentity();
                            newParentLinkIndex = newRootLinkIndex;
                        }

                        i32 newLinkIndex = createBody(modelIndex, 0);
                        parseJoint(curDefaults, xml, modelIndex, newParentLinkIndex, newLinkIndex, logger, linkTransform, jointTrans);

                        //getLink(modelIndex,newLinkIndex)->m_linkTransformInWorld = jointTrans*linkTransform;

                        linkTransform = jointTrans.inverse();
                        newParentLinkIndex = newLinkIndex;
                        //newParentLinkIndex, curChildLinkIndex
                        hasJoint = true;
                        handled = true;
                    }
                }
                else
                {
                    i32 newLinkIndex = createBody(modelIndex, 0);
                    Transform2 joint2nextjoint = jointTrans.inverse();
                    Transform2 unused;
                    parseJoint(curDefaults, xml, modelIndex, newParentLinkIndex, newLinkIndex, logger, joint2nextjoint, unused);
                    newParentLinkIndex = newLinkIndex;
                    //todo: compute relative joint transforms (if any) and append to linkTransform
                    hasJoint = true;
                    handled = true;
                }
            }
            if (n == "geom")
            {
                Vec3 inertialShift(0, 0, 0);
                parseGeom(curDefaults, xml, modelIndex, orgChildLinkIndex, logger, inertialShift);
                if (!massDefined)
                {
                    localInertialFrame.setOrigin(inertialShift);
                }
                handled = true;
            }

            //recursive
            if (n == "body")
            {
                parseBody(curDefaults, xml, modelIndex, orgChildLinkIndex, logger);
                handled = true;
            }

            if (n == "light")
            {
                handled = true;
            }
            if (n == "site")
            {
                handled = true;
            }

            if (!handled)
            {
                logger->reportWarning((sourceFileLocation(xml) + ": unknown field '" + n + "'").c_str());
            }
        }

        linkPtr->m_linkTransformInWorld = linkTransform;

        if ((newParentLinkIndex != INVALID_LINK_INDEX) && !skipFixedJoint)
        {
            //linkPtr->m_linkTransformInWorld.setIdentity();
            //default to 'fixed' joint
            UrdfJoint* jointPtr = new UrdfJoint();
            jointPtr->m_childLinkName = linkPtr->m_name;
            const UrdfLink* parentLink = getLink(modelIndex, newParentLinkIndex);
            jointPtr->m_parentLinkName = parentLink->m_name;
            jointPtr->m_localJointAxis.setVal(1, 0, 0);
            jointPtr->m_parentLinkToJointTransform = linkTransform;
            jointPtr->m_type = URDFFixedJoint;
            char jointName[1024];
            sprintf(jointName, "jointfix_%d_%d", gUid++, newParentLinkIndex);
            jointPtr->m_name = jointName;
            m_models[modelIndex]->m_joints.insert(jointPtr->m_name.c_str(), jointPtr);
        }

        //check mass/inertia
        if (!massDefined)
        {
            double density = 1000;
            double volume = computeVolume(linkPtr, logger);
            mass = density * volume;
        }
        linkPtr->m_inertia.m_linkLocalFrame = localInertialFrame;  // = jointTrans.inverse();
        linkPtr->m_inertia.m_mass = mass;
        return true;
    }

    void recurseAddChildLinks(UrdfModel* model, UrdfLink* link)
    {
        for (i32 i = 0; i < link->m_childLinks.size(); i++)
        {
            i32 linkIndex = model->m_links.size();
            link->m_childLinks[i]->m_linkIndex = linkIndex;
            tukk linkName = link->m_childLinks[i]->m_name.c_str();
            model->m_links.insert(linkName, link->m_childLinks[i]);
        }
        for (i32 i = 0; i < link->m_childLinks.size(); i++)
        {
            recurseAddChildLinks(model, link->m_childLinks[i]);
        }
    }

    bool initTreeAndRoot(UrdfModel& model, MJCFErrorLogger* logger)
    {
        // every link has children links and joints, but no parents, so we create a
        // local convenience data structure for keeping child->parent relations
        HashMap<HashString, HashString> parentLinkTree;

        // loop through all joints, for every link, assign children links and children joints
        for (i32 i = 0; i < model.m_joints.size(); i++)
        {
            UrdfJoint** jointPtr = model.m_joints.getAtIndex(i);
            if (jointPtr)
            {
                UrdfJoint* joint = *jointPtr;
                STxt parent_link_name = joint->m_parentLinkName;
                STxt child_link_name = joint->m_childLinkName;
                if (parent_link_name.empty() || child_link_name.empty())
                {
                    logger->reportError("parent link or child link is empty for joint");
                    logger->reportError(joint->m_name.c_str());
                    return false;
                }

                UrdfLink** childLinkPtr = model.m_links.find(joint->m_childLinkName.c_str());
                if (!childLinkPtr)
                {
                    logger->reportError("Cannot find child link for joint ");
                    logger->reportError(joint->m_name.c_str());

                    return false;
                }
                UrdfLink* childLink = *childLinkPtr;

                UrdfLink** parentLinkPtr = model.m_links.find(joint->m_parentLinkName.c_str());
                if (!parentLinkPtr)
                {
                    logger->reportError("Cannot find parent link for a joint");
                    logger->reportError(joint->m_name.c_str());
                    return false;
                }
                UrdfLink* parentLink = *parentLinkPtr;

                childLink->m_parentLink = parentLink;

                childLink->m_parentJoint = joint;
                parentLink->m_childJoints.push_back(joint);
                parentLink->m_childLinks.push_back(childLink);
                parentLinkTree.insert(childLink->m_name.c_str(), parentLink->m_name.c_str());
            }
        }

        //search for children that have no parent, those are 'root'
        for (i32 i = 0; i < model.m_links.size(); i++)
        {
            UrdfLink** linkPtr = model.m_links.getAtIndex(i);
            Assert(linkPtr);
            if (linkPtr)
            {
                UrdfLink* link = *linkPtr;
                link->m_linkIndex = i;

                if (!link->m_parentLink)
                {
                    model.m_rootLinks.push_back(link);
                }
            }
        }

        if (model.m_rootLinks.size() > 1)
        {
            logger->reportWarning("URDF file with multiple root links found");
        }

        if (model.m_rootLinks.size() == 0)
        {
            logger->reportError("URDF without root link found");
            return false;
        }

        //re-index the link indices so parent indices are always smaller than child indices
        AlignedObjectArray<UrdfLink*> links;
        links.resize(model.m_links.size());
        for (i32 i = 0; i < model.m_links.size(); i++)
        {
            links[i] = *model.m_links.getAtIndex(i);
        }
        model.m_links.clear();
        for (i32 i = 0; i < model.m_rootLinks.size(); i++)
        {
            UrdfLink* rootLink = model.m_rootLinks[i];
            i32 linkIndex = model.m_links.size();
            rootLink->m_linkIndex = linkIndex;
            model.m_links.insert(rootLink->m_name.c_str(), rootLink);
            recurseAddChildLinks(&model, rootLink);
        }
        return true;
    }
};

BulletMJCFImporter::BulletMJCFImporter(struct GUIHelperInterface* helper, UrdfRenderingInterface* customConverter, CommonFileIOInterface* fileIO, i32 flags)
{
    m_data = new BulletMJCFImporterInternalData();
    m_data->m_guiHelper = helper;
    m_data->m_customVisualShapesConverter = customConverter;
    m_data->m_flags = flags;
    m_data->m_textureId = -1;
    m_data->m_fileIO = fileIO;
}

BulletMJCFImporter::~BulletMJCFImporter()
{
    delete m_data;
}

bool BulletMJCFImporter::loadMJCF(tukk fileName, MJCFErrorLogger* logger, bool forceFixedBase)
{
    if (strlen(fileName) == 0)
        return false;

    //i32 argc=0;
    char relativeFileName[1024];

    b3FileUtils fu;

    //bool fileFound = fu.findFile(fileName, relativeFileName, 1024);
    bool fileFound = (m_data->m_fileIO->findResourcePath(fileName, relativeFileName, 1024) > 0);
    m_data->m_sourceFileName = relativeFileName;

    STxt xml_string;
    m_data->m_pathPrefix[0] = 0;

    if (!fileFound)
    {
        std::cerr << "MJCF file not found" << std::endl;
        return false;
    }
    else
    {
        //read file
        i32 fileId = m_data->m_fileIO->fileOpen(relativeFileName,"r");

        char destBuffer[8192];
        while (m_data->m_fileIO->readLine(fileId, destBuffer, 8192))
        {
            xml_string += (STxt(destBuffer) + "\n");
        }
        m_data->m_fileIO->fileClose(fileId);

        if (parseMJCFString(xml_string.c_str(), logger))
        {
            return true;
        }
    }

    return false;
}

bool BulletMJCFImporter::parseMJCFString(tukk xmlText, MJCFErrorLogger* logger)
{
    XMLDocument xml_doc;
    xml_doc.Parse(xmlText);
    if (xml_doc.Error())
    {
#ifdef G3_TINYXML2
        logger->reportError("xml reading error (upgrade tinyxml2 version!");
#else
        logger->reportError(xml_doc.ErrorStr());
        xml_doc.ClearError();
#endif
        return false;
    }

    XMLElement* mujoco_xml = xml_doc.FirstChildElement("mujoco");
    if (!mujoco_xml)
    {
        logger->reportWarning("Cannot find <mujoco> root element");
        return false;
    }

    tukk modelName = mujoco_xml->Attribute("model");
    if (modelName)
    {
        m_data->m_fileModelName = modelName;
    }

    //<compiler>,<option>,<size>,<default>,<body>,<keyframe>,<contactpair>,
    //<light>, <camera>,<constraint>,<tendon>,<actuator>,<customfield>,<textfield>

    for (XMLElement* link_xml = mujoco_xml->FirstChildElement("default"); link_xml; link_xml = link_xml->NextSiblingElement("default"))
    {
        m_data->parseDefaults(m_data->m_globalDefaults, link_xml, logger);
    }

    for (XMLElement* link_xml = mujoco_xml->FirstChildElement("compiler"); link_xml; link_xml = link_xml->NextSiblingElement("compiler"))
    {
        m_data->parseCompiler(link_xml, logger);
    }

    for (XMLElement* link_xml = mujoco_xml->FirstChildElement("asset"); link_xml; link_xml = link_xml->NextSiblingElement("asset"))
    {
        m_data->parseAssets(link_xml, logger);
    }

    for (XMLElement* link_xml = mujoco_xml->FirstChildElement("body"); link_xml; link_xml = link_xml->NextSiblingElement("body"))
    {
        m_data->parseRootLevel(m_data->m_globalDefaults, link_xml, logger);
    }

    for (XMLElement* link_xml = mujoco_xml->FirstChildElement("worldbody"); link_xml; link_xml = link_xml->NextSiblingElement("worldbody"))
    {
        m_data->parseRootLevel(m_data->m_globalDefaults, link_xml, logger);
    }

    return true;
}

tukk BulletMJCFImporter::getPathPrefix()
{
    return m_data->m_pathPrefix;
}

i32 BulletMJCFImporter::getRootLinkIndex() const
{
    if (m_data->m_activeModel >= 0 && m_data->m_activeModel < m_data->m_models.size())
    {
        if (m_data->m_models[m_data->m_activeModel]->m_rootLinks.size())
        {
            return 0;
        }
    }
    return -1;
}

STxt BulletMJCFImporter::getLinkName(i32 linkIndex) const
{
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
    if (link)
    {
        return link->m_name;
    }
    return "";
}

STxt BulletMJCFImporter::getBodyName() const
{
    return m_data->m_fileModelName;
}

bool BulletMJCFImporter::getLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const
{
    bool hasLinkColor = false;
    {
        const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
        if (link)
        {
            for (i32 i = 0; i < link->m_visualArray.size(); i++)
            {
                if (link->m_visualArray[i].m_geometry.m_hasLocalMaterial)
                {
                    matCol = link->m_visualArray[i].m_geometry.m_localMaterial.m_matColor;
                    hasLinkColor = true;
                    break;
                }
            }

            if (!hasLinkColor)
            {
                for (i32 i = 0; i < link->m_collisionArray.size(); i++)
                {
                    if (link->m_collisionArray[i].m_geometry.m_hasLocalMaterial)
                    {
                        matCol = link->m_collisionArray[0].m_geometry.m_localMaterial.m_matColor;
                        hasLinkColor = true;
                    }
                    break;
                }
            }
        }
    }

    if (!hasLinkColor)
    {

        matCol.m_rgbaColor = (m_data->m_flags & CUF_GOOGLEY_UNDEFINED_COLORS) ? sGoogleColors[linkIndex & 3] : Vec4(1,1,1,1);
        matCol.m_specularColor.setVal(1, 1, 1);
        hasLinkColor = true;
    }
    return hasLinkColor;
}

bool BulletMJCFImporter::getLinkColor(i32 linkIndex, Vec4& colorRGBA) const
{
    //  UrdfLink* link = m_data->getLink(linkIndex);
    return false;
}

//todo: placeholder implementation
//MuJoCo type/affinity is different from drx3D group/mask, so we should implement a custom collision filter instead
//(contype1 & conaffinity2) || (contype2 & conaffinity1)
i32 BulletMJCFImporter::getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const
{
    i32 flags = 0;

    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
    if (link)
    {
        for (i32 i = 0; i < link->m_collisionArray.size(); i++)
        {
            const UrdfCollision& col = link->m_collisionArray[i];
            colGroup = col.m_collisionGroup;
            flags |= URDF_HAS_COLLISION_GROUP;
            colMask = col.m_collisionMask;
            flags |= URDF_HAS_COLLISION_MASK;
        }
    }

    return flags;
}

STxt BulletMJCFImporter::getJointName(i32 linkIndex) const
{
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
    if (link)
    {
        if (link->m_parentJoint)
        {
            return link->m_parentJoint->m_name;
        }
        return link->m_name;
    }
    return "";
}

//fill mass and inertial data. If inertial data is missing, please initialize mass, inertia to sensitive values, and inertialFrame to identity.
void BulletMJCFImporter::getMassAndInertia(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const
{
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, urdfLinkIndex);
    if (link)
    {
        mass = link->m_inertia.m_mass;
        localInertiaDiagonal.setVal(link->m_inertia.m_ixx,
                                      link->m_inertia.m_iyy,
                                      link->m_inertia.m_izz);
        inertialFrame.setIdentity();
        inertialFrame = link->m_inertia.m_linkLocalFrame;
    }
    else
    {
        mass = 0;
        localInertiaDiagonal.setZero();
        inertialFrame.setIdentity();
    }
}

///fill an array of child link indices for this link, AlignedObjectArray behaves like a std::vector so just use push_back and resize(0) if needed
void BulletMJCFImporter::getLinkChildIndices(i32 urdfLinkIndex, AlignedObjectArray<i32>& childLinkIndices) const
{
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, urdfLinkIndex);
    if (link)
    {
        for (i32 i = 0; i < link->m_childLinks.size(); i++)
        {
            i32 childIndex = link->m_childLinks[i]->m_linkIndex;
            childLinkIndices.push_back(childIndex);
        }
    }
}

bool BulletMJCFImporter::getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const
{
    jointLowerLimit = 0.f;
    jointUpperLimit = 0.f;
    jointDamping = 0.f;
    jointFriction = 0.f;
    jointMaxForce = 0;
    jointMaxVelocity = 0;

    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, urdfLinkIndex);
    if (link)
    {
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
}

bool BulletMJCFImporter::getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const
{
    //backwards compatibility for custom file importers
    Scalar jointMaxForce = 0;
    Scalar jointMaxVelocity = 0;
    return getJointInfo2(urdfLinkIndex, parent2joint, linkTransformInWorld, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction, jointMaxForce, jointMaxVelocity);
}

bool BulletMJCFImporter::getRootTransformInWorld(Transform2& rootTransformInWorld) const
{
    rootTransformInWorld.setIdentity();
    /*
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel,0);
    if (link)
    {
        rootTransformInWorld = link->m_linkTransformInWorld;
    }
    */
    return true;
}

void BulletMJCFImporter::convertURDFToVisualShapeInternal(const UrdfVisual* visual, tukk urdfPathPrefix, const Transform2& visualTransform, AlignedObjectArray<GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<MJCFURDFTexture>& texturesOut) const
{
    GLInstanceGraphicsShape* glmesh = 0;
    i32 strideInBytes = 9 * sizeof(float);

    ConvexShape* convexColShape = 0;

    switch (visual->m_geometry.m_type)
    {
        case URDF_GEOM_CAPSULE:
        {
#if 1

            Scalar height = visual->m_geometry.m_capsuleHeight;

            Transform2 capsuleTrans;
            capsuleTrans.setIdentity();
            if (visual->m_geometry.m_hasFromTo)
            {
                Vec3 f = visual->m_geometry.m_capsuleFrom;
                Vec3 t = visual->m_geometry.m_capsuleTo;

                //compute the local 'fromto' transform
                Vec3 localPosition = Scalar(0.5) * (t + f);
                Quat localOrn;
                localOrn = Quat::getIdentity();

                Vec3 diff = t - f;
                Scalar lenSqr = diff.length2();
                height = 0.f;

                if (lenSqr > SIMD_EPSILON)
                {
                    height = Sqrt(lenSqr);
                    Vec3 ax = diff / height;

                    Vec3 zAxis(0, 0, 1);
                    localOrn = shortestArcQuat(zAxis, ax);
                }
                capsuleTrans.setOrigin(localPosition);
                capsuleTrans.setRotation(localOrn);
            }

            Scalar diam = 2. * visual->m_geometry.m_capsuleRadius;
            b3AlignedObjectArray<GLInstanceVertex> transformedVertices;
            i32 numVertices = sizeof(mjcf_sphere_vertices) / strideInBytes;
            transformedVertices.resize(numVertices);
            for (i32 i = 0; i < numVertices; i++)
            {
                Vec3 vert;
                vert.setVal(mjcf_sphere_vertices[i * 9 + 0],
                              mjcf_sphere_vertices[i * 9 + 1],
                              mjcf_sphere_vertices[i * 9 + 2]);

                Scalar halfHeight = 0.5 * height;
                Vec3 trVer = (diam * vert);
                i32 up = 2;  //default to z axis up for capsule
                if (trVer[up] > 0)
                    trVer[up] += halfHeight;
                else
                    trVer[up] -= halfHeight;

                trVer = capsuleTrans * trVer;

                transformedVertices[i].xyzw[0] = trVer[0];
                transformedVertices[i].xyzw[1] = trVer[1];
                transformedVertices[i].xyzw[2] = trVer[2];
                transformedVertices[i].xyzw[3] = 0;
                transformedVertices[i].normal[0] = mjcf_sphere_vertices[i * 9 + 4];
                transformedVertices[i].normal[1] = mjcf_sphere_vertices[i * 9 + 5];
                transformedVertices[i].normal[2] = mjcf_sphere_vertices[i * 9 + 6];
                //transformedVertices[i].uv[0] = mjcf_sphere_vertices[i * 9 + 7];
                //transformedVertices[i].uv[1] = mjcf_sphere_vertices[i * 9 + 8];

                Scalar u = Atan2(transformedVertices[i].normal[0], transformedVertices[i].normal[2]) / (2 * SIMD_PI) + 0.5;
                Scalar v = transformedVertices[i].normal[1] * 0.5 + 0.5;
                transformedVertices[i].uv[0] = u;
                transformedVertices[i].uv[1] = v;
            }

            glmesh = new GLInstanceGraphicsShape;
            //      i32 index = 0;
            glmesh->m_indices = new b3AlignedObjectArray<i32>();
            glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();

            i32 numIndices = sizeof(mjcf_sphere_indiced) / sizeof(i32);
            for (i32 i = 0; i < numIndices; i++)
            {
                glmesh->m_indices->push_back(mjcf_sphere_indiced[i]);
            }
            for (i32 i = 0; i < transformedVertices.size(); i++)
            {
                glmesh->m_vertices->push_back(transformedVertices[i]);
            }
            glmesh->m_numIndices = numIndices;
            glmesh->m_numvertices = transformedVertices.size();
            glmesh->m_scaling[0] = 1;
            glmesh->m_scaling[1] = 1;
            glmesh->m_scaling[2] = 1;
            glmesh->m_scaling[3] = 1;
#else
            if (visual->m_geometry.m_hasFromTo)
            {
                Vec3 f = visual->m_geometry.m_capsuleFrom;
                Vec3 t = visual->m_geometry.m_capsuleTo;
                Vec3 fromto[2] = {f, t};
                Scalar radii[2] = {Scalar(visual->m_geometry.m_capsuleRadius), Scalar(visual->m_geometry.m_capsuleRadius)};

                btMultiSphereShape* ms = new btMultiSphereShape(fromto, radii, 2);
                convexColShape = ms;
            }
            else
            {
                CapsuleShapeZ* cap = new CapsuleShapeZ(visual->m_geometry.m_capsuleRadius,
                                                           visual->m_geometry.m_capsuleHeight);
                convexColShape = cap;
            }
#endif

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

                Vec3 vert(cylRadius * Sin(SIMD_2_PI * (float(i) / numSteps)), cylRadius * Cos(SIMD_2_PI * (float(i) / numSteps)), cylLength / 2.);
                vertices.push_back(vert);
                vert[2] = -cylLength / 2.;
                vertices.push_back(vert);
            }

            ConvexHullShape* cylZShape = new ConvexHullShape(&vertices[0].x(), vertices.size(), sizeof(Vec3));
            cylZShape->setMargin(m_data->m_globalDefaults.m_defaultCollisionMargin);
            cylZShape->recalcLocalAabb();
            convexColShape = cylZShape;
            break;
        }

        case URDF_GEOM_BOX:
        {
            Vec3 extents = 0.5*visual->m_geometry.m_boxSize;
            BoxShape* boxShape = new BoxShape(extents * 0.5f);
            //ConvexShape* boxShape = new ConeShapeX(extents[2]*0.5,extents[0]*0.5);
            convexColShape = boxShape;
            convexColShape->setMargin(m_data->m_globalDefaults.m_defaultCollisionMargin);
            break;
        }

        case URDF_GEOM_SPHERE:
        {
#if 1
            Scalar sphereSize = 2. * visual->m_geometry.m_sphereRadius;
            b3AlignedObjectArray<GLInstanceVertex> transformedVertices;
            i32 numVertices = sizeof(mjcf_sphere_vertices) / strideInBytes;
            transformedVertices.resize(numVertices);

            glmesh = new GLInstanceGraphicsShape;
            //      i32 index = 0;
            glmesh->m_indices = new b3AlignedObjectArray<i32>();
            glmesh->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
            printf("vertices:\n");
            for (i32 i = 0; i < numVertices; i++)
            {
                Vec3 vert;
                vert.setVal(mjcf_sphere_vertices[i * 9 + 0],
                              mjcf_sphere_vertices[i * 9 + 1],
                              mjcf_sphere_vertices[i * 9 + 2]);

                Vec3 trVer = sphereSize * vert;
                transformedVertices[i].xyzw[0] = trVer[0];
                transformedVertices[i].xyzw[1] = trVer[1];
                transformedVertices[i].xyzw[2] = trVer[2];
                transformedVertices[i].xyzw[3] = 0;
                transformedVertices[i].normal[0] = mjcf_sphere_vertices[i * 9 + 4];
                transformedVertices[i].normal[1] = mjcf_sphere_vertices[i * 9 + 5];
                transformedVertices[i].normal[2] = mjcf_sphere_vertices[i * 9 + 6];
                //transformedVertices[i].uv[0] = mjcf_sphere_vertices[i * 9 + 7];
                //transformedVertices[i].uv[1] = mjcf_sphere_vertices[i * 9 + 8];

                Scalar u = Atan2(transformedVertices[i].normal[0], transformedVertices[i].normal[2]) / (2 * SIMD_PI) + 0.5;
                Scalar v = transformedVertices[i].normal[1] * 0.5 + 0.5;
                transformedVertices[i].uv[0] = u;
                transformedVertices[i].uv[1] = v;
            }
            i32 numIndices = sizeof(mjcf_sphere_indiced) / sizeof(i32);
            for (i32 i = 0; i < numIndices; i++)
            {
                glmesh->m_indices->push_back(mjcf_sphere_indiced[i]);
            }
            for (i32 i = 0; i < transformedVertices.size(); i++)
            {
                glmesh->m_vertices->push_back(transformedVertices[i]);
            }
            glmesh->m_numIndices = numIndices;
            glmesh->m_numvertices = transformedVertices.size();
            glmesh->m_scaling[0] = 1;
            glmesh->m_scaling[1] = 1;
            glmesh->m_scaling[2] = 1;
            glmesh->m_scaling[3] = 1;

#else

            Scalar radius = visual->m_geometry.m_sphereRadius;
            SphereShape* sphereShape = new SphereShape(radius);
            convexColShape = sphereShape;
            convexColShape->setMargin(m_data->m_globalDefaults.m_defaultCollisionMargin);
#endif
            break;
        }

        case URDF_GEOM_MESH:
        {
            switch (visual->m_geometry.m_meshFileType)
            {
                case UrdfGeometry::FILE_OBJ:
                {
                    b3ImportMeshData meshData;
                    if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(visual->m_geometry.m_meshFileName, meshData, m_data->m_fileIO))
                    {
                        if (meshData.m_textureImage1)
                        {
                            MJCFURDFTexture texData;
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
                    glmesh = LoadMeshFromSTL(visual->m_geometry.m_meshFileName.c_str(), m_data->m_fileIO);
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
                    //      i32 index = 0;
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
                        //                              upAxisMat.setPureRotation(upAxisTrans.getRotation());
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
            //  i32 strideInBytes = 9*sizeof(float);
            i32 numVertices = hull->numVertices();
            i32 numIndices = hull->numIndices();

            glmesh = new GLInstanceGraphicsShape;
            //  i32 index = 0;
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
                Vec3 normal = pos.normalized();
                vtx.normal[0] = normal.x();
                vtx.normal[1] = normal.y();
                vtx.normal[2] = normal.z();
                Scalar u = Atan2(normal[0], normal[2]) / (2 * SIMD_PI) + 0.5;
                Scalar v = normal[1] * 0.5 + 0.5;
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

i32 BulletMJCFImporter::convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& inertialFrame) const
{
    i32 graphicsIndex = -1;
    if (m_data->m_flags & CUF_MJCF_COLORS_FROM_FILE)
    {
        AlignedObjectArray<GLInstanceVertex> vertices;
        AlignedObjectArray<i32> indices;
        Transform2 startTrans;
        startTrans.setIdentity();
        AlignedObjectArray<MJCFURDFTexture> textures;

        const UrdfModel& model = *m_data->m_models[m_data->m_activeModel];
        UrdfLink* const* linkPtr = model.m_links.getAtIndex(linkIndex);
        if (linkPtr)
        {
            const UrdfLink* link = *linkPtr;

            for (i32 v = 0; v < link->m_visualArray.size(); v++)
            {
                const UrdfVisual& vis = link->m_visualArray[v];
                Transform2 childTrans = vis.m_linkLocalFrame;
                HashString matName(vis.m_materialName.c_str());
                UrdfMaterial* const* matPtr = model.m_materials[matName];

                convertURDFToVisualShapeInternal(&vis, pathPrefix, inertialFrame.inverse() * childTrans, vertices, indices, textures);
            }
        }
        if (vertices.size() && indices.size())
        {
            if (1)
            {
                i32 textureIndex = -2;
                if (textures.size())
                {
                    textureIndex = m_data->m_guiHelper->registerTexture(textures[0].textureData1, textures[0].m_width, textures[0].m_height);
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
    }
    return graphicsIndex;
}

bool BulletMJCFImporter::getLinkContactInfo(i32 linkIndex, URDFLinkContactInfo& contactInfo) const
{
    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
    if (link)
    {
        contactInfo = link->m_contactInfo;
        return true;
    }
    return false;
}

void BulletMJCFImporter::convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& inertialFrame, class CollisionObject2* colObj, i32 objectIndex) const
{
    if (m_data->m_customVisualShapesConverter)
    {
        const UrdfLink* link = m_data->getLink(m_data->m_activeModel, urdfIndex);
        i32 uid3 = m_data->m_customVisualShapesConverter->convertVisualShapes(linkIndex, pathPrefix, inertialFrame, link, 0, colObj->getBroadphaseHandle()->getUid(), objectIndex, m_data->m_fileIO);
        colObj->setUserIndex3(uid3);
    }
}

void BulletMJCFImporter::setBodyUniqueId(i32 bodyId)
{
    m_data->m_activeBodyUniqueId = bodyId;
}

i32 BulletMJCFImporter::getBodyUniqueId() const
{
    drx3DAssert(m_data->m_activeBodyUniqueId != -1);
    return m_data->m_activeBodyUniqueId;
}

static CollisionShape* MjcfCreateConvexHullFromShapes(const bt_tinyobj::attrib_t& attribute, std::vector<bt_tinyobj::shape_t>& shapes, const Vec3& geomScale, Scalar collisionMargin)
{
    CompoundShape* compound = new CompoundShape();
    compound->setMargin(collisionMargin);

    Transform2 identity;
    identity.setIdentity();

    for (i32 s = 0; s < (i32)shapes.size(); s++)
    {
        ConvexHullShape* convexHull = new ConvexHullShape();
        convexHull->setMargin(collisionMargin);
        bt_tinyobj::shape_t& shape = shapes[s];

        i32 faceCount = shape.mesh.indices.size();

        for (i32 f = 0; f < faceCount; f += 3)
        {
            Vec3 pt;
            pt.setVal(attribute.vertices[3 * shape.mesh.indices[f].vertex_index + 0],
                        attribute.vertices[3 * shape.mesh.indices[f].vertex_index + 1],
                        attribute.vertices[3 * shape.mesh.indices[f].vertex_index + 2]);

            convexHull->addPoint(pt * geomScale, false);

            pt.setVal(attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 0],
                        attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 1],
                        attribute.vertices[3 * shape.mesh.indices[f + 1].vertex_index + 2]);
            convexHull->addPoint(pt * geomScale, false);

            pt.setVal(attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 0],
                        attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 1],
                        attribute.vertices[3 * shape.mesh.indices[f + 2].vertex_index + 2]);
            convexHull->addPoint(pt * geomScale, false);
        }

        convexHull->recalcLocalAabb();
        convexHull->optimizeConvexHull();
        compound->addChildShape(identity, convexHull);
    }

    return compound;
}

class CompoundShape* BulletMJCFImporter::convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const
{
    CompoundShape* compound = new CompoundShape();
    m_data->m_allocatedCollisionShapes.push_back(compound);

    const UrdfLink* link = m_data->getLink(m_data->m_activeModel, linkIndex);
    if (link)
    {
        for (i32 i = 0; i < link->m_collisionArray.size(); i++)
        {
            const UrdfCollision* col = &link->m_collisionArray[i];
            CollisionShape* childShape = 0;

            switch (col->m_geometry.m_type)
            {
                case URDF_GEOM_PLANE:
                {
                    childShape = new StaticPlaneShape(col->m_geometry.m_planeNormal, 0);
                    break;
                }
                case URDF_GEOM_SPHERE:
                {
                    childShape = new SphereShape(col->m_geometry.m_sphereRadius);
                    break;
                }
                case URDF_GEOM_BOX:
                {
                    childShape = new BoxShape(0.5*col->m_geometry.m_boxSize);
                    break;
                }
                case URDF_GEOM_CYLINDER:
                {
                    if (col->m_geometry.m_hasFromTo)
                    {
                        Vec3 f = col->m_geometry.m_capsuleFrom;
                        Vec3 t = col->m_geometry.m_capsuleTo;

                        //compute the local 'fromto' transform
                        Vec3 localPosition = Scalar(0.5) * (t + f);
                        Quat localOrn;
                        localOrn = Quat::getIdentity();

                        Vec3 diff = t - f;
                        Scalar lenSqr = diff.length2();
                        Scalar height = 0.f;

                        if (lenSqr > SIMD_EPSILON)
                        {
                            height = Sqrt(lenSqr);
                            Vec3 ax = diff / height;

                            Vec3 zAxis(0, 0, 1);
                            localOrn = shortestArcQuat(zAxis, ax);
                        }
                        CylinderShapeZ* cyl = new CylinderShapeZ(Vec3(col->m_geometry.m_capsuleRadius, col->m_geometry.m_capsuleRadius, Scalar(0.5) * height));

                        CompoundShape* compound = new CompoundShape();
                        Transform2 localTransform(localOrn, localPosition);
                        compound->addChildShape(localTransform, cyl);
                        childShape = compound;
                    }
                    else
                    {
                        CylinderShapeZ* cap = new CylinderShapeZ(Vec3(col->m_geometry.m_capsuleRadius,
                                                                               col->m_geometry.m_capsuleRadius, Scalar(0.5) * col->m_geometry.m_capsuleHeight));
                        childShape = cap;
                    }
                    break;
                }
                case URDF_GEOM_MESH:
                {
                    GLInstanceGraphicsShape* glmesh = 0;
                    switch (col->m_geometry.m_meshFileType)
                    {
                        case UrdfGeometry::FILE_OBJ:
                        {
                            if (col->m_flags & URDF_FORCE_CONCAVE_TRIMESH)
                            {
                                glmesh = LoadMeshFromObj(col->m_geometry.m_meshFileName.c_str(), 0,m_data->m_fileIO);
                            }
                            else
                            {
                                std::vector<bt_tinyobj::shape_t> shapes;
                                bt_tinyobj::attrib_t attribute;
                                STxt err = bt_tinyobj::LoadObj(attribute, shapes, col->m_geometry.m_meshFileName.c_str(), "", m_data->m_fileIO);
                                //create a convex hull for each shape, and store it in a CompoundShape

                                childShape = MjcfCreateConvexHullFromShapes(attribute, shapes, col->m_geometry.m_meshScale, m_data->m_globalDefaults.m_defaultCollisionMargin);
                            }
                            break;
                        }
                        case UrdfGeometry::FILE_STL:
                        {
                            glmesh = LoadMeshFromSTL(col->m_geometry.m_meshFileName.c_str(), m_data->m_fileIO);
                            break;
                        }
                        default:
                            drx3DWarning("%s: Unsupported file type in Collision: %s (maybe .dae?)\n", col->m_sourceFileLocation.c_str(), col->m_geometry.m_meshFileType);
                    }

                    if (childShape)
                    {
                        // okay!
                    }
                    else if (!glmesh || glmesh->m_numvertices <= 0)
                    {
                        drx3DWarning("%s: cannot extract anything useful from mesh '%s'\n", col->m_sourceFileLocation.c_str(), col->m_geometry.m_meshFileName.c_str());
                    }
                    else
                    {
                        //drx3DPrintf("extracted %d verticed from STL file %s\n", glmesh->m_numvertices,fullPath);
                        //i32 shapeId = m_glApp->m_instancingRenderer->registerShape(&gvertices[0].pos[0],gvertices.size(),&indices[0],indices.size());
                        //convex->setUserIndex(shapeId);
                        AlignedObjectArray<Vec3> convertedVerts;
                        convertedVerts.reserve(glmesh->m_numvertices);
                        for (i32 i = 0; i < glmesh->m_numvertices; i++)
                        {
                            convertedVerts.push_back(Vec3(
                                glmesh->m_vertices->at(i).xyzw[0] * col->m_geometry.m_meshScale[0],
                                glmesh->m_vertices->at(i).xyzw[1] * col->m_geometry.m_meshScale[1],
                                glmesh->m_vertices->at(i).xyzw[2] * col->m_geometry.m_meshScale[2]));
                        }

                        if (col->m_flags & URDF_FORCE_CONCAVE_TRIMESH)
                        {
                            TriangleMesh* meshInterface = new TriangleMesh();
                            m_data->m_allocatedMeshInterfaces.push_back(meshInterface);

                            for (i32 i = 0; i < glmesh->m_numIndices / 3; i++)
                            {
                                float* v0 = glmesh->m_vertices->at(glmesh->m_indices->at(i * 3)).xyzw;
                                float* v1 = glmesh->m_vertices->at(glmesh->m_indices->at(i * 3 + 1)).xyzw;
                                float* v2 = glmesh->m_vertices->at(glmesh->m_indices->at(i * 3 + 2)).xyzw;
                                meshInterface->addTriangle(Vec3(v0[0], v0[1], v0[2]),
                                                           Vec3(v1[0], v1[1], v1[2]),
                                                           Vec3(v2[0], v2[1], v2[2]));
                            }

                            BvhTriangleMeshShape* trimesh = new BvhTriangleMeshShape(meshInterface, true, true);
                            childShape = trimesh;
                        }
                        else
                        {
                            ConvexHullShape* convexHull = new ConvexHullShape(&convertedVerts[0].getX(), convertedVerts.size(), sizeof(Vec3));
                            convexHull->optimizeConvexHull();
                            //convexHull->initializePolyhedralFeatures();
                            convexHull->setMargin(m_data->m_globalDefaults.m_defaultCollisionMargin);
                            childShape = convexHull;
                        }
                    }

                    delete glmesh;
                    break;
                }
                case URDF_GEOM_CAPSULE:
                {
                    if (col->m_geometry.m_hasFromTo)
                    {
                        if (m_data->m_flags & CUF_USE_IMPLICIT_CYLINDER)
                        {
                            Vec3 f = col->m_geometry.m_capsuleFrom;
                            Vec3 t = col->m_geometry.m_capsuleTo;

                            //compute the local 'fromto' transform
                            Vec3 localPosition = Scalar(0.5) * (t + f);
                            Quat localOrn;
                            localOrn = Quat::getIdentity();

                            Vec3 diff = t - f;
                            Scalar lenSqr = diff.length2();
                            Scalar height = 0.f;

                            if (lenSqr > SIMD_EPSILON)
                            {
                                height = Sqrt(lenSqr);
                                Vec3 ax = diff / height;

                                Vec3 zAxis(0, 0, 1);
                                localOrn = shortestArcQuat(zAxis, ax);
                            }
                            CapsuleShapeZ* capsule = new CapsuleShapeZ(col->m_geometry.m_capsuleRadius, height);

                            CompoundShape* compound = new CompoundShape();
                            Transform2 localTransform(localOrn, localPosition);
                            compound->addChildShape(localTransform, capsule);
                            childShape = compound;
                        }
                        else
                        {
                            Vec3 f = col->m_geometry.m_capsuleFrom;
                            Vec3 t = col->m_geometry.m_capsuleTo;
                            Vec3 fromto[2] = {f, t};
                            Scalar radii[2] = {Scalar(col->m_geometry.m_capsuleRadius), Scalar(col->m_geometry.m_capsuleRadius)};

                            MultiSphereShape* ms = new MultiSphereShape(fromto, radii, 2);
                            childShape = ms;
                        }
                    }
                    else
                    {
                        CapsuleShapeZ* cap = new CapsuleShapeZ(col->m_geometry.m_capsuleRadius,
                                                                   col->m_geometry.m_capsuleHeight);
                        childShape = cap;
                    }
                    break;
                }
                case URDF_GEOM_CDF:
                {
                    //todo
                    break;
                }
                case URDF_GEOM_UNKNOWN:
                {
                    break;
                }
                default:
                {
                }

            }  // switch geom

            if (childShape)
            {
                m_data->m_allocatedCollisionShapes.push_back(childShape);
                compound->addChildShape(localInertiaFrame.inverse() * col->m_linkLocalFrame, childShape);
            }
        }
    }
    return compound;
}

i32 BulletMJCFImporter::getNumAllocatedCollisionShapes() const
{
    return m_data->m_allocatedCollisionShapes.size();
}
class CollisionShape* BulletMJCFImporter::getAllocatedCollisionShape(i32 index)
{
    return m_data->m_allocatedCollisionShapes[index];
}

i32 BulletMJCFImporter::getNumAllocatedMeshInterfaces() const
{
    return m_data->m_allocatedMeshInterfaces.size();
}

StridingMeshInterface* BulletMJCFImporter::getAllocatedMeshInterface(i32 index)
{
    return m_data->m_allocatedMeshInterfaces[index];
}

i32 BulletMJCFImporter::getNumModels() const
{
    return m_data->m_models.size();
}

void BulletMJCFImporter::activateModel(i32 modelIndex)
{
    if ((modelIndex >= 0) && (modelIndex < m_data->m_models.size()))
    {
        m_data->m_activeModel = modelIndex;
    }
}
