// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <DrxAnimation/IDrxAnimation.h>
#include <DrxAnimation/IAttachment.h>
#include <vector>
#include <array>
#include <drx3D/CoreX/Serialization/Forward.h>

struct ICharacterInstance;

namespace CharacterTool
{

using std::vector;

struct CharacterDefinition;

struct SJointPhysics
{
	enum class EType
	{
		None  = -1, // chosen to match GetPhysInfoProperties_ROPE parameter
		Rope  = 0,
		Cloth = 1,
	};

	enum EPropertySetIndex
	{
		ePropertySetIndex_Alive,
		ePropertySetIndex_Ragdoll,

		ePropertySetIndex_COUNT
	};

	typedef std::array<DynArray<SJointProperty>, ePropertySetIndex_COUNT> JointProperties;

	EType           type;
	JointProperties ropeProperties;
	JointProperties clothProperties;

	SJointPhysics()
		: type(EType::None)
		, ropeProperties()
		, clothProperties()
	{
	}

	void Serialize(Serialization::IArchive& ar);
	void LoadFromXml(const XmlNodeRef& node);
	void SaveToXml(XmlNodeRef node) const;
};

struct CharacterAttachment
{

	enum TransformSpace
	{
		SPACE_CHARACTER,
		SPACE_JOINT
	};

	enum ProxyPurpose
	{
		AUXILIARY,
		CLOTH,
		RAGDOLL
	};

	AttachmentTypes      m_attachmentType;
	string               m_strSocketName;
	TransformSpace       m_positionSpace;
	TransformSpace       m_rotationSpace;
	QuatT                m_characterSpacePosition;
	QuatT                m_jointSpacePosition;
	string               m_strJointName;
	string               m_strGeometryFilepath;
	string               m_strMaterial;
	Vec4                 m_ProxyParams;
	ProxyPurpose         m_ProxyPurpose;
	SimulationParams     m_simulationParams;
	geomtypes            m_ProxyType, m_prevProxyType;
	Vec4                 m_prevProxyParams;
	i32                  m_meshSmooth, m_prevMeshSmooth;
	Vec3i                m_limits[2];
	Ang3                 m_frame0;
	Vec3                 m_tension, m_damping;
	string               m_strJointNameMirror;
	bool                 m_updateMirror;
	QuatT                m_boneTrans, m_boneTransMirror;
	Vec3                 m_dirChild;

	struct ProxySource
	{
		i32  m_refCount = 0;
		void AddRef()  { ++m_refCount; }
		void Release() { if (!--m_refCount) delete this; }

		_smart_ptr<IGeometry> m_hullMesh;
		_smart_ptr<IGeometry> m_proxyMesh;
		
	};
	_smart_ptr<ProxySource> m_proxySrc;

	string               m_strRowJointName;
	RowSimulationParams  m_rowSimulationParams;

	SVClothParams        m_vclothParams;
	float                m_viewDistanceMultiplier;
	i32                  m_nFlags;

	SJointPhysics        m_jointPhysics;

	CharacterDefinition* m_definition;

	CharacterAttachment()
		: m_attachmentType(CA_BONE)
		, m_positionSpace(SPACE_JOINT)
		, m_rotationSpace(SPACE_JOINT)
		, m_characterSpacePosition(IDENTITY)
		, m_jointSpacePosition(IDENTITY)
		, m_ProxyParams(0, 0, 0, 0)
		, m_ProxyPurpose(AUXILIARY)
		, m_viewDistanceMultiplier(1.0f)
		, m_nFlags(0)
		, m_definition()
		, m_ProxyType(GEOM_BOX)
		, m_prevProxyType(GEOM_BOX)
		, m_prevProxyParams(0, 0, 0, 0)
		, m_meshSmooth(0)
		, m_prevMeshSmooth(0)
		, m_limits { ZERO, ZERO }
		, m_frame0(0, 0, 0)
		, m_tension(0)
		, m_damping(0)
		, m_updateMirror(false)
		, m_boneTrans(IDENTITY)
		, m_boneTransMirror(IDENTITY)
		, m_dirChild(ZERO)
	{
	}

	void Serialize(Serialization::IArchive& ar);

	IGeometry* CreateProxyGeom() const;
	void       ChangeProxyType();
	void       GenerateMesh();
	void       UpdateMirrorInfo(i32 idBone, const IDefaultSkeleton& skel);
};

struct IDrxAnimation;

struct CharacterDefinition
{
	bool                        m_initialized;
	bool                        m_physEdit, m_physNeedsApply;
	string                      skeleton;
	string                      materialPath;
	string                      physics;
	string                      rig;
	vector<CharacterAttachment> attachments;
	vector<CharacterAttachment>	origBonePhysAtt; // for reverting
	DynArray<string>            m_arrAllProcFunctions;  //all procedural functions

	IAnimationPoseModifierSetupPtr modifiers;

	static std::map<INT_PTR,_smart_ptr<CharacterAttachment::ProxySource>> g_meshArchive;
	static i32 g_meshArchiveUsed;

	CharacterDefinition()
	{
		m_initialized = m_physEdit = m_physNeedsApply = false;
		++g_meshArchiveUsed;
	}
	~CharacterDefinition()
	{
		if (!--g_meshArchiveUsed)
			g_meshArchive.clear();
	}


	bool        LoadFromXml(const XmlNodeRef& root);
	bool        LoadFromXmlFile(tukk filename);
	void        ApplyToCharacter(bool* skinSetChanged, ICharacterInstance* character, ICharacterManager* cryAnimation, bool showDebug, bool applyPhys = false);
	void        LoadPhysProxiesFromCharacter(ICharacterInstance *character);
	void        SavePhysProxiesToCGF(tukk fname);
	void        ApplyBoneAttachment(IAttachment* pIAttachment, ICharacterManager* characterManager, const CharacterAttachment& desc, ICharacterInstance* pICharacterInstance, bool showDebug) const;
	void        ApplyFaceAttachment(IAttachment* pIAttachment, ICharacterManager* characterManager, const CharacterAttachment& desc, bool showDebug) const;
	void        ApplySkinAttachment(IAttachment* pIAttachment, ICharacterManager* characterManager, const CharacterAttachment& desc, ICharacterInstance* pICharacterInstance, bool* skinChanged) const;
	void        ApplyPhysAttachments(ICharacterInstance* character);

	void        SynchModifiers(ICharacterInstance& character);

	void        Serialize(Serialization::IArchive& ar);
	bool        Save(tukk filename);
	XmlNodeRef  SaveToXml();
	static void ExportBoneAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportFaceAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportSkinAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportProxyAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportPClothAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportVClothAttachment(const CharacterAttachment& attach, XmlNodeRef nodeAttachments);
	static void ExportSimulation(const CharacterAttachment& attach, XmlNodeRef nodeAttach);

	bool        SaveToMemory(vector<char>* buffer);

	void        RemoveRagdollAttachments() 
	{ 
		attachments.erase(std::remove_if(attachments.begin(), attachments.end(), 
			[](const auto& att) -> bool { return att.m_attachmentType == CA_PROX && att.m_ProxyPurpose == CharacterAttachment::RAGDOLL; }),
			attachments.end());
	}
};

}

