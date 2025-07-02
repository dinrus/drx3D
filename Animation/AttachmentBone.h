// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/AttachmentBase.h>
#include <drx3D/Animation/SocketSimulation.h>

class CAttachmentBONE : public SAttachmentBase
{
public:

	CAttachmentBONE()
	{
		m_nJointID = -1;
		m_AttRelativeDefault.SetIdentity();
		m_AttAbsoluteDefault.SetIdentity();
		m_AttModelRelative.SetIdentity();
		m_addTransformation.SetIdentity();
	};

	~CAttachmentBONE() {};

	virtual void AddRef()
	{
		++m_nRefCounter;
	}

	virtual void Release()
	{
		if (--m_nRefCounter == 0)
			delete this;
	}

	virtual u32             GetType() const { return CA_BONE; }
	virtual u32             SetJointName(tukk szJointName);

	virtual tukk        GetName() const    { return m_strSocketName; };
	virtual u32             GetNameCRC() const { return m_nSocketCRC32; }
	virtual u32             ReName(tukk strSocketName, u32 crc);

	virtual u32             GetFlags() const       { return m_AttFlags; }
	virtual void               SetFlags(u32 flags) { m_AttFlags = flags; }

	virtual u32             Immediate_AddBinding(IAttachmentObject* pIAttachmentObject, ISkin* pISkin = 0, u32 nLoadingFlags = 0);
	virtual void               Immediate_ClearBinding(u32 nLoadingFlags = 0);
	virtual u32             Immediate_SwapBinding(IAttachment* pNewAttachment);

	virtual IAttachmentObject* GetIAttachmentObject() const { return m_pIAttachmentObject;  }
	virtual IAttachmentSkin*   GetIAttachmentSkin()         { return 0; }

	virtual void               HideAttachment(u32 x);
	virtual u32             IsAttachmentHidden() const             { return m_AttFlags & FLAGS_ATTACH_HIDE_MAIN_PASS; }
	virtual void               HideInRecursion(u32 x);
	virtual u32             IsAttachmentHiddenInRecursion() const  { return m_AttFlags & FLAGS_ATTACH_HIDE_RECURSION; }
	virtual void               HideInShadow(u32 x);
	virtual u32             IsAttachmentHiddenInShadow() const     { return m_AttFlags & FLAGS_ATTACH_HIDE_SHADOW_PASS; }

	virtual void               SetAttAbsoluteDefault(const QuatT& qt) { m_AttAbsoluteDefault = qt;  m_AttFlags &= (~FLAGS_ATTACH_PROJECTED);  assert(m_AttAbsoluteDefault.q.IsValid()); };
	virtual const QuatT&      GetAttAbsoluteDefault() const          { return m_AttAbsoluteDefault;  };
	virtual void              SetAttRelativeDefault(const QuatT& qt) { m_AttRelativeDefault = qt; assert(m_AttRelativeDefault.q.IsValid());  };
	virtual const QuatT&      GetAttRelativeDefault() const          { return m_AttRelativeDefault; };
	virtual const QuatT&      GetAttModelRelative() const;
	virtual const QuatTS      GetAttWorldAbsolute() const;
	virtual const QuatT&      GetAdditionalTransformation() const { return m_addTransformation; }
	virtual void              UpdateAttModelRelative();

	virtual u32            GetJointID() const { return m_nJointID; };

	virtual void              AlignJointAttachment();

	virtual SimulationParams& GetSimulationParams() { return m_Simulation;  };
	virtual void              PostUpdateSimulationParams(bool bAttachmentSortingRequired, tukk pJointName = 0);

	u32                    ProjectAttachment(const Skeleton::CPoseData* pPoseData = 0);
	void                      Update_Redirected(Skeleton::CPoseData& pPoseData); //only bones can have this feature
	void                      Update_Empty(Skeleton::CPoseData& pPoseData);
	void                      Update_Static(Skeleton::CPoseData& pPoseData);
	void                      Update_Execute(Skeleton::CPoseData& pPoseData);

	virtual void              Serialize(TSerialize ser);
	virtual size_t            SizeOfThis() const;
	virtual void              GetMemoryUsage(IDrxSizer* pSizer) const;

	QuatT       m_AttRelativeDefault; //attachment location relative to the facematrix/bonematrix in default pose;
	QuatT       m_AttAbsoluteDefault; //attachment location relative to the model when character is in default-pose
	QuatTS      m_AttModelRelative;   //the position, orientation and scale of the attachment in animated relative model-space (=relative to Vec3(0,0,0))
	QuatT       m_addTransformation;  //additional rotation applied during rendering.
	i32       m_nJointID;
	string      m_strJointName;
	CSimulation m_Simulation;
protected:
	void ClearBinding_Internal(bool release);
};
