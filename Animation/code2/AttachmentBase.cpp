// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AttachmentBase.h>

#include <drx3D/Animation/AttachmentUpr.h>

void SAttachmentBase::AddBinding(IAttachmentObject* pModel, ISkin* pISkin /*= 0*/, u32 nLoadingFlags /*= 0*/)
{
	if (nLoadingFlags & CA_CharEditModel || nLoadingFlags & CA_ImmediateMode)
	{
		// The reason for introducing these special cases is twofold:
		// - Certain modification commands (such as CAddAttachmentObject) contain specialized control paths for CA_CharEditModel
		//   that may end up in a recursive CModificationCommandBuffer flush. This can cause a whole range of various mean things,
		//   including but not limited to stack overflows and calling methods on dangling pointers.
		// - CModificationCommandBuffer is currently not flushed when user interacts continuously with the CT model
		//   (e.g. dragging attachments around), which ends up with buffer overflows.
		//
		// There's no point in buffering modification commands in the character edit mode anyway, so we simply reverted to the old synchronous behavior as an ad-hoc fix.
		// Ideally, the attachment management code should be redesigned from the ground up to get rid of the massive technical debt and properly account for current, more dynamic use cases.
		Immediate_AddBinding(pModel, pISkin, nLoadingFlags);
		return;
	}

	m_pAttachmentUpr->AddAttachmentObject(this, pModel, pISkin, nLoadingFlags);
	// Immediate_AddBinding(pModel, pISkin, nLoadingFlags);
}

void SAttachmentBase::ClearBinding(u32 nLoadingFlags /*= 0*/)
{
	if (nLoadingFlags & CA_CharEditModel || nLoadingFlags & CA_ImmediateMode)
	{
		Immediate_ClearBinding(nLoadingFlags);
		return;
	}

	m_pAttachmentUpr->ClearAttachmentObject(this, nLoadingFlags);
	// Immediate_ClearBinding(nLoadingFlags);
}

void SAttachmentBase::SwapBinding(IAttachment* pNewAttachment)
{
	m_pAttachmentUpr->SwapAttachmentObject(this, pNewAttachment);
	// Immediate_SwapBinding(pNewAttachment);
}
