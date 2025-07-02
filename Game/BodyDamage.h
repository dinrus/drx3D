// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef BODY_DAMAGE_H
#define BODY_DAMAGE_H

#include <drx3D/Game/BodyDefinitions.h>
#include <drx3D/Game/Utility/DrxHash.h>

struct HitInfo;

class CBodyDamageProfile : public _reference_target_t
{
	
	struct JointId : std::unary_function<bool, const JointId&>
	{
		JointId()
			: m_id(0)
		{

		}

		JointId( tukk jointName )
			: m_id(DrxStringUtils::HashString(jointName))
		{

		}

		
		bool operator<( const JointId& otherJoint ) const
		{
			return (m_id < otherJoint.m_id);
		}

		bool operator()( const JointId& otherJoint ) const
		{
			return (m_id < otherJoint.m_id);
		}

		bool operator==( const JointId& otherJoint ) const
		{
			return (m_id == otherJoint.m_id);
		}

		static JointId GetJointIdFromPartId( IEntity& characterEntity, i32k partId );
		static JointId GetJointIdFromPartId(  IDefaultSkeleton& rIDefaultSkeleton, ISkeletonPose& skeletonPose, i32k partId );

	private:
		DrxHash m_id;
	};

	struct MatMappingId : std::unary_function<bool, const MatMappingId&>
	{
		MatMappingId()
			: m_jointId(0)
		{

		}

		MatMappingId( tukk jointName )
			: m_jointName(jointName)
			, m_jointId(jointName)
		{

		}


		bool operator<( const MatMappingId& other ) const
		{
			return (m_jointId < other.m_jointId);
		}

		bool operator()( const MatMappingId& other) const
		{
			return (m_jointId < other.m_jointId);
		}

		tukk GetName() const
		{
			return m_jointName.c_str();
		}

		static MatMappingId GetMatMappingIdFromPartId(  IDefaultSkeleton& rIDefaultSkeleton, ISkeletonPose& skeletonPose, i32k partId );

	private:
		string  m_jointName;
		JointId m_jointId;
	};

	typedef i32 MaterialId;
	typedef i32 PartId;

	typedef std::vector<MaterialId> TMaterialIds;
	typedef std::map<JointId, TMaterialIds> TJointIds;
	typedef std::map<MaterialId, MaterialId> TEffectiveMaterials;
	typedef std::map<JointId, TEffectiveMaterials> TEffectiveMaterialsByBone;

	struct SMaterialMappingEntry
	{
		static i32k MATERIALS_ARRAY_MAX_SIZE = 24;

		SMaterialMappingEntry();

		i32 materialsCount;
		i32 materials[MATERIALS_ARRAY_MAX_SIZE];

		void GetMemoryUsage( IDrxSizer *pSizer ) const{}
	};

	enum EBulletHitClass
	{
		eBHC_Normal = 0,
		eBHC_Aimed	= 1,
		eBHC_Max	= 2
	};

	struct SProjectileMultiplier
	{
		SProjectileMultiplier(u16 _projectileClass, float _multiplier, float _multiplierAimed)
			: projectileClassId(_projectileClass)
		{
			DRX_ASSERT(projectileClassId != (u16)(~0));
			multiplier[eBHC_Normal] = _multiplier;
			multiplier[eBHC_Aimed] = _multiplierAimed;
		}

		u16 projectileClassId;
		float multiplier[eBHC_Max];
	};

	typedef std::vector<SProjectileMultiplier> TProjectileMultipliers;

	struct SBodyPartDamageMultiplier
	{
		SBodyPartDamageMultiplier(float defaultValue)
			: meleeMultiplier(defaultValue)
			, collisionMultiplier(defaultValue)
		{
			defaultMultiplier[eBHC_Normal] = defaultValue;
			defaultMultiplier[eBHC_Aimed]  = defaultValue;
		}

		float defaultMultiplier[eBHC_Max]; 
		float meleeMultiplier;
		float collisionMultiplier;
		TProjectileMultipliers bulletMultipliers;
	};

	class CEffectiveMaterials
	{
	public:
		CEffectiveMaterials(CBodyDamageProfile& bodyDamageProfile, IDefaultSkeleton& rIDefaultSkeleton, ISkeletonPose& skeletonPose, IPhysicalEntity& physicalEntity);

		void Load(const XmlNodeRef& parentNode);

		void UpdateMapping( tukk jointName, i32k physicsJointId );
		void FinalizeMapping();

	private:
		void UpdateMapping(tukk jointName, i32k physicsJointId, const TEffectiveMaterials& effectiveMaterials);
		void UpdateMapping(tukk jointName, i32k physicsJointId, ISurfaceType& sourceMaterial, ISurfaceType& targetMaterial);

		void LoadEffectiveMaterials(const XmlNodeRef& parentNode, tukk boneName = NULL, i32 boneId = -1);
		void LoadEffectiveMaterial(const XmlNodeRef& effectiveMaterial, tukk boneName = NULL, i32 boneId = -1);

		void LogEffectiveMaterialApplied(i32 sourceMaterialId, tukk sourceMaterial, tukk targetMaterial, i32 jointId, i32 materialIndex) const;

		void UpdatePhysicsPartById(const MatMappingId& matMappingId, const pe_params_part& part, SMaterialMappingEntry& mappingEntry, const TMaterialIds& appliedMaterialIds);

		TEffectiveMaterialsByBone m_effectiveMaterialsByBone;
		TEffectiveMaterials m_effectiveMaterials;

		CBodyDamageProfile& m_bodyDamageProfile;
		ISkeletonPose& m_skeletonPose;
		IDefaultSkeleton& m_rICharacterModelSkeleton;
		IPhysicalEntity& m_physicalEntity;

		TJointIds m_jointIdsApplied;
	};

	class CPart
	{
	public:
		CPart(tukk name, u32 flags, i32 id);

		const string& GetName() const { return m_name; }
		PartId GetId() const { return m_id; }
		const TJointIds& GetJointIds() const { return m_jointIds; }
		const TMaterialIds* GetMaterialsByJointId(const JointId& jointId) const;
		u32 GetFlags() const { return m_flags; }

		void LoadElements(const XmlNodeRef& partNode, IDefaultSkeleton& skeletonPose, IAttachmentUpr& attachmentUpr, CEffectiveMaterials& effectiveMaterials, const CBodyDamageProfile& ownerDamageProfile);

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(m_name);
		}
	private:
		void AddBone(const XmlNodeRef& boneNode, tukk boneName, IDefaultSkeleton& rIDefaultSkeleton, CEffectiveMaterials& effectiveMaterials);

		void AddMaterial(const XmlNodeRef& boneNode, tukk boneName, TMaterialIds &materialIds);
		void AddAttachment(tukk attachmentName, IAttachmentUpr& attachmentUpr);

		static i32 GetNextId();

		TJointIds m_jointIds;
		string m_name;
		u32 m_flags;
		PartId m_id;
	};

	class CPartByNameFunctor : std::unary_function<bool, const CPart&>
	{
	public:
		CPartByNameFunctor(tukk name) : m_name(name) {}
		bool operator()(const CPart& part) const { return 0 == strcmp(m_name, part.GetName().c_str()); }
	private:
		tukk m_name;
	};

	class CPartInfo
	{
	public:
		CPartInfo(const CPart& part, const TMaterialIds& materialIds) : m_part(part) , m_materialIds(materialIds) {}

		const TMaterialIds& GetMaterialIds() const { return m_materialIds; }
		const CPart& GetPart() const { return m_part; }

		void GetMemoryUsage( IDrxSizer *pSizer ) const{}
	private:
		const TMaterialIds& m_materialIds;
		const CPart& m_part;
	};

	struct SDefaultMultipliers
	{
		SDefaultMultipliers()
			: m_global(1.0f)
			, m_collision(1.0f)
		{

		}

		float m_global;
		float m_collision;
	};

	typedef std::vector<CPart> TParts;
	typedef std::multimap<JointId, CPartInfo> TPartsByJointId;
	typedef std::map<PartId, SBodyPartDamageMultiplier> TPartIdsToMultipliers;

	typedef TPartsByJointId::const_iterator TPartsByJointIdIterator;
	typedef std::pair<TPartsByJointIdIterator, TPartsByJointIdIterator> TPartsByJointIdRange;

	typedef std::map<MatMappingId, SMaterialMappingEntry> TMaterialMappingEntries;
	typedef std::multimap<PartId,SBodyDamageImpulseFilter> TImpulseFilters;

public:
	CBodyDamageProfile(TBodyDamageProfileId id);

	void LoadXmlInfo(const SBodyDamageDef &bodyDamageDef, bool bReload = false);
	bool Init(const SBodyCharacterInfo& characterInfo, bool loadEffectiveMaterials = true, bool bReload = false);
	bool Reload(const SBodyCharacterInfo& characterInfo, const SBodyDamageDef &bodyDamageDef, TBodyDamageProfileId id);

	TBodyDamageProfileId GetId() const { return m_id; }
	bool IsInitialized() const { return m_bInitialized; }

	bool PhysicalizeEntity(IPhysicalEntity* pPhysicalEntity, IDefaultSkeleton* pIDefaultSkeleton) const;

	float  GetDamageMultiplier(IEntity& characterEntity, const HitInfo& hitInfo) const;
	float  GetExplosionDamageMultiplier(IEntity& characterEntity, const HitInfo& hitInfo) const;	
	u32 GetPartFlags(IEntity& characterEntity, const HitInfo& hitInfo) const;

	SMaterialMappingEntry& InsertMappingEntry( const MatMappingId& matMappingId, const pe_params_part& part);
	void RemoveMappingEntry(const MatMappingId& matMappingId);

	void GetMemoryUsage(IDrxSizer *pSizer) const;

	void GetHitImpulseFilter( IEntity& characterEntity, const HitInfo &hitInfo, SBodyDamageImpulseFilter &impulseFilter) const;

	const CPart* FindPartWithBoneName(tukk boneName) const;

private:
	XmlNodeRef LoadXml(tukk fileName) const;
	void LoadDamage(tukk bodyDamageFileName);
	void LoadParts(const XmlNodeRef& rootNode, IDefaultSkeleton& skeletonPose, IAttachmentUpr& attachmentUpr, CEffectiveMaterials& effectiveMaterials);
	u32 LoadPartFlags(const XmlNodeRef& partNode) const;
	void LoadMultipliers(const XmlNodeRef& rootNode);
	void LoadMultiplier(const XmlNodeRef& multiplierNode);
	void LoadExplosionMultipliers(const XmlNodeRef& rootNode);
	void LoadExplosionMultiplier(const XmlNodeRef& multiplierNode);
	void LoadImpulseFilters(const XmlNodeRef& rootNode, IDefaultSkeleton& skeletonPose);
	void LoadImpulseFilter(const XmlNodeRef& filterNode, IDefaultSkeleton& skeletonPose);
	void LoadImpulse( const XmlNodeRef& filterNode, IDefaultSkeleton& skeletonPose, const PartId partID );
	void IndexParts();
	const CPart* FindPart( IEntity& characterEntity, i32k partId, i32 material) const;
	void LogDamageMultiplier(IEntity& characterEntity, const HitInfo& hitInfo, tukk partName, const float multiplierValue) const;
	void LogExplosionDamageMultiplier(IEntity& characterEntity, const float multiplierValue) const;
	void LogFoundMaterial(i32 materialId, const CPartInfo& part, i32k partId) const;

	bool FindDamageMultiplierForBullet(const TProjectileMultipliers& bulletMultipliers, u16 projectileClassId, EBulletHitClass hitClass, float& multiplier) const;
	float GetBestMultiplierForHitType(const SBodyPartDamageMultiplier& damageMultipliers, i32 hitType, EBulletHitClass hitClass) const;

	float GetDefaultDamageMultiplier( const HitInfo& hitInfo ) const;

private:
	bool m_bInitialized;
	TBodyDamageProfileId m_id;

	// Caching Xml info for later initialization
	XmlNodeRef m_partsRootNode;
	XmlNodeRef m_damageRootNode;

	TParts m_parts;
	TPartsByJointId m_partsByJointId;
	TPartIdsToMultipliers m_partIdsToMultipliers;
	TMaterialMappingEntries m_effectiveMaterialsMapping;
	TImpulseFilters m_impulseFilters;
	TProjectileMultipliers m_explosionMultipliers;

	SDefaultMultipliers m_defaultMultipliers;

};

#endif
