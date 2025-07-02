// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/VertexData.h>
#include <drx3D/Animation/VertexAnimation.h>
#include <drx3D/Animation/ModelSkin.h>
#include <drx3D/Animation/Skeleton.h>
#include <drx3D/Animation/AttachmentFace.h>
#include <drx3D/Animation/AttachmentBone.h>
#include <drx3D/Animation/AttachmentSkin.h>
#include <drx3D/Animation/AttachmentProxy.h>
#include <drx3D/Animation/AttachmentMerged.h>
#include <drx3D/CoreX/Math/GeomQuery.h>

struct CharacterAttachment;
namespace Command {
class CBuffer;
}

class CAttachmentUpr : public IAttachmentUpr
{
	friend class CAttachmentBONE;
	friend class CAttachmentFACE;
	friend class CAttachmentSKIN;

public:
	CAttachmentUpr()
	{
		m_pSkelInstance = NULL;
		m_nHaveEntityAttachments = 0;
		m_numRedirectionWithAttachment = 0;
		m_fZoomDistanceSq = 0;
		m_arrAttachments.reserve(0x20);
		m_TypeSortingRequired = 0;
		m_attachmentMergingRequired = 0;
		m_nDrawProxies = 0;
		memset(&m_sortedRanges, 0, sizeof(m_sortedRanges));
		m_fTurbulenceGlobal = 0;
		m_fTurbulenceLocal = 0;
		m_physAttachIds = 1;
	};

	u32        LoadAttachmentList(tukk pathname);
	static u32 ParseXMLAttachmentList(CharacterAttachment* parrAttachments, u32 numAttachments, XmlNodeRef nodeAttachements);
	void          InitAttachmentList(const CharacterAttachment* parrAttachments, u32 numAttachments, const string pathname, u32 nLoadingFlags, i32 nKeepModelInMemory);

	IAttachment*  CreateAttachment(tukk szName, u32 type, tukk szJointName = 0, bool bCallProject = true);
	IAttachment*  CreateVClothAttachment(const SVClothAttachmentParams& params);

	void          MergeCharacterAttachments();
	void          RequestMergeCharacterAttachments() { ++m_attachmentMergingRequired; }

	i32         GetAttachmentCount() const         { return m_arrAttachments.size(); };
	i32         GetExtraBonesCount() const         { return m_extraBones.size(); }

	i32         AddExtraBone(IAttachment* pAttachment);
	void          RemoveExtraBone(IAttachment* pAttachment);
	i32         FindExtraBone(IAttachment* pAttachment);

	void          UpdateBindings();

	IAttachment*  GetInterfaceByIndex(u32 c) const;
	IAttachment*  GetInterfaceByName(tukk szName) const;
	IAttachment*  GetInterfaceByNameCRC(u32 nameCRC) const;
	IAttachment*  GetInterfaceByPhysId(i32 id) const;

	i32         GetIndexByName(tukk szName) const;
	i32         GetIndexByNameCRC(u32 nameCRC) const;

	void          AddEntityAttachment()
	{
		m_nHaveEntityAttachments++;
	}
	void RemoveEntityAttachment()
	{
		if (m_nHaveEntityAttachments > 0)
		{
			m_nHaveEntityAttachments--;
		}
#ifndef _RELEASE
		else
		{
			DRX_ASSERT_MESSAGE(0, "Removing too many entity attachments");
		}
#endif
	}

	bool NeedsHierarchicalUpdate();
#ifdef EDITOR_PCDEBUGCODE
	void Verification();
#endif

	void UpdateAllRemapTables();
	void PrepareAllRedirectedTransformations(Skeleton::CPoseData& pPoseData);
	void GenerateProxyModelRelativeTransformations(Skeleton::CPoseData& pPoseData);

	// Updates all attachment objects except for those labeled "execute".
	// They all are safe to update in the animation job.
	// These updates get skipped if the character is not visible (as a performance optimization).
	void UpdateLocationsExceptExecute(Skeleton::CPoseData& pPoseData);

	// for attachment objects labeled "execute", we have to distinguish between attachment
	// objects which are safe to update in the animation job and those who are not (e. g. entities)
	// Updates only "execute" attachment objects which are safe to update in the animation job
	// "execute" attachment objects are always updated (regardless of visibility)
	void           UpdateLocationsExecute(Skeleton::CPoseData& pPoseData);
	// Updates only "execute" attachments which are unsafe to update in the animation job
	void           UpdateLocationsExecuteUnsafe(Skeleton::CPoseData& pPoseData);

	void           ProcessAllAttachedObjectsFast();

	void           DrawAttachments(SRendParams& rRendParams, const Matrix34& m, const SRenderingPassInfo& passInfo, const f32 fZoomFactor, const f32 fZoomDistanceSq);
	void           DrawMergedAttachments(SRendParams& rRendParams, const Matrix34& m, const SRenderingPassInfo& passInfo, const f32 fZoomFactor, const f32 fZoomDistanceSq);

	virtual i32  RemoveAttachmentByInterface(const IAttachment* pAttachment, u32 loadingFlags = 0);
	virtual i32  RemoveAttachmentByName(tukk szName, u32 loadingFlags = 0);
	virtual i32  RemoveAttachmentByNameCRC(u32 nameCrc, u32 loadingFlags = 0);
	virtual u32 ProjectAllAttachment();

	void           RemoveAttachmentByIndex(u32 index, u32 loadingFlags = 0);
	u32         RemoveAllAttachments();

	void           CreateCommands(Command::CBuffer& buffer);

	// Attachment Object Binding
	virtual void                ClearAttachmentObject(SAttachmentBase* pAttachment, u32 nLoadingFlags);
	virtual void                AddAttachmentObject(SAttachmentBase* pAttachment, IAttachmentObject* pModel, ISkin* pISkin = 0, u32 nLoadingFlags = 0);
	virtual void                SwapAttachmentObject(SAttachmentBase* pAttachment, IAttachment* pNewAttachment);

	void                        PhysicalizeAttachment(i32 idx, i32 nLod, IPhysicalEntity* pent, const Vec3& offset);
	i32                         UpdatePhysicalizedAttachment(i32 idx, IPhysicalEntity* pent, const QuatT& offset);
	i32                         UpdatePhysAttachmentHideState(i32 idx, IPhysicalEntity* pent, const Vec3& offset);

	virtual void                PhysicalizeAttachment(i32 idx, IPhysicalEntity* pent = 0, i32 nLod = 0);
	virtual void                DephysicalizeAttachment(i32 idx, IPhysicalEntity* pent = 0);

	void                        OnHideAttachment(const IAttachment* pAttachment, u32 nHideType, bool bHide);
	void                        Serialize(TSerialize ser);

	virtual ICharacterInstance* GetSkelInstance() const;
	ILINE bool                  IsFastUpdateType(IAttachmentObject::EType eAttachmentType) const
	{
		if (eAttachmentType == IAttachmentObject::eAttachment_Entity)
			return true;
		if (eAttachmentType == IAttachmentObject::eAttachment_Effect)
			return true;
		return false;
	}

	virtual IProxy*     CreateProxy(tukk szName, tukk szJointName);
	virtual i32       GetProxyCount() const { return m_arrProxies.size(); }

	virtual IProxy*     GetProxyInterfaceByIndex(u32 c) const;
	virtual IProxy*     GetProxyInterfaceByName(tukk szName) const;
	virtual IProxy*     GetProxyInterfaceByCRC(u32 nameCRC) const;

	virtual i32       GetProxyIndexByName(tukk szName) const;
	virtual i32       GetProxyIndexByCRC(u32 nameCRC) const;

	virtual i32       RemoveProxyByInterface(const IProxy* ptr);
	virtual i32       RemoveProxyByName(tukk szName);
	virtual i32       RemoveProxyByNameCRC(u32 nameCRC);
	void                RemoveProxyByIndex(u32 n);
	virtual void        DrawProxies(u32 enable) { (enable & 0x80) ? m_nDrawProxies &= enable : m_nDrawProxies |= enable; }
	void                VerifyProxyLinks();

	virtual u32      GetProcFunctionCount() const { return 5; }
	virtual tukk GetProcFunctionName(u32 idx) const;
	tukk         ExecProcFunction(u32 nCRC32, Skeleton::CPoseData* pPoseData, tukk pstrFunction = 0) const;

	float               GetExtent(EGeomForm eForm);
	void                GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;
#if !defined(_RELEASE)
	float               DebugDrawAttachment(IAttachment* pAttachment, ISkin* pSkin, Vec3 drawLoc, IMaterial* pMaterial, float drawScale,const SRenderingPassInfo &passInfo);
#endif

public:
	u32 m_TypeSortingRequired;
	void         SortByType();
	size_t       SizeOfAllAttachments() const;
	void         GetMemoryUsage(IDrxSizer* pSizer) const;
	ILINE u32 GetMinJointAttachments() const  { return m_sortedRanges[eRange_BoneStatic].begin; }
	ILINE u32 GetMaxJointAttachments() const  { return m_sortedRanges[eRange_BoneExecute].end; }
	ILINE u32 GetRedirectedJointCount() const { return m_sortedRanges[eRange_BoneRedirect].GetNumElements(); }

	// Generates a context in the Character Upr for each attached instance
	// and returns the number of instance contexts generated
	i32 GenerateAttachedInstanceContexts();

	CGeomExtents                            m_Extents;
	CCharInstance*                          m_pSkelInstance;
	DynArray<_smart_ptr<SAttachmentBase>>   m_arrAttachments;
	DynArray<_smart_ptr<CAttachmentMerged>> m_mergedAttachments;
	DynArray<IAttachment*>                  m_extraBones;
	DynArray<CProxy>                        m_arrProxies;
	u32 m_nDrawProxies;
	f32    m_fTurbulenceGlobal, m_fTurbulenceLocal;
private:
	class CModificationCommand
	{
	public:
		CModificationCommand(SAttachmentBase* pAttachment) : m_pAttachment(pAttachment) {}
		virtual ~CModificationCommand() {}
		virtual void Execute() = 0;
	protected:
		_smart_ptr<SAttachmentBase> m_pAttachment;
	};

	class CModificationCommandBuffer
	{
	public:
		static u32k kMaxOffsets = 4096;
		static u32k kMaxMemory = 4096;

		CModificationCommandBuffer() { Clear(); }
		~CModificationCommandBuffer();
		void  Execute();
		template<class T>
		T*    Alloc() { return static_cast<T*>(Alloc(sizeof(T), alignof(T))); }
	private:
		uk Alloc(size_t size, size_t align);
		void  Clear();
		stl::aligned_vector<char, 32> m_memory;
		std::vector<u32>           m_commandOffsets;
	};

#define CMD_BUF_PUSH_COMMAND(buf, command, ...) \
  new((buf).Alloc<command>())command(__VA_ARGS__)

	f32    m_fZoomDistanceSq;
	u32 m_nHaveEntityAttachments;
	u32 m_numRedirectionWithAttachment;
	u32 m_physAttachIds; // bitmask for used physicalized attachment ids
	u32 m_attachmentMergingRequired;

	struct SRange
	{
		u16 begin;
		u16 end;
		u32 GetNumElements() const { return end - begin; };
	};

	// The eRange_BoneExecute and eRange_BoneExecuteUnsafe need to stay next
	// to each other.
	// Generally, the algorithms make some assumptions on the order of
	// the ranges, so it is best not to touch them, currently.
	enum ERange
	{
		eRange_BoneRedirect,
		eRange_BoneEmpty,
		eRange_BoneStatic,
		eRange_BoneExecute,
		eRange_BoneExecuteUnsafe,
		eRange_FaceEmpty,
		eRange_FaceStatic,
		eRange_FaceExecute,
		eRange_FaceExecuteUnsafe,
		eRange_SkinMesh,
		eRange_VertexClothOrPendulumRow,

		eRange_COUNT
	};

	SRange                     m_sortedRanges[eRange_COUNT];

	CModificationCommandBuffer m_modificationCommandBuffer;
};
