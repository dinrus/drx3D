// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/Skeleton.h>

class CAttachmentUpr;

struct SAttachmentBase : public IAttachment
{
	SAttachmentBase()
		: m_AttFlags(0)
		, m_nSocketCRC32(0)
		, m_nRefCounter(0)
		, m_pIAttachmentObject(nullptr)
		, m_pAttachmentUpr(nullptr)
	{
	}

	// these functions will queue commands in the attachment manager
	// and they are exposed in the interface, since they are safe
	// to call from the main thread at any time
	virtual void AddBinding(IAttachmentObject* pModel, ISkin* pISkin = 0, u32 nLoadingFlags = 0) override;
	virtual void ClearBinding(u32 nLoadingFlags = 0) override;
	virtual void SwapBinding(IAttachment* pNewAttachment) override;

	// these are the actual implementations and will get overloaded by the actual classes
	virtual u32 Immediate_AddBinding(IAttachmentObject* pModel, ISkin* pISkin = 0, u32 nLoadingFlags = 0) = 0;
	virtual void   Immediate_ClearBinding(u32 nLoadingFlags = 0) = 0;
	virtual u32 Immediate_SwapBinding(IAttachment* pNewAttachment) = 0;

	//shared members
	u32              m_AttFlags;
	string              m_strSocketName;
	u32              m_nSocketCRC32;
	i32               m_nRefCounter;
	IAttachmentObject*  m_pIAttachmentObject;
	CAttachmentUpr* m_pAttachmentUpr;
};
