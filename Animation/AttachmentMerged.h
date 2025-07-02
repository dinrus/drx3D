// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/AttachmentSkin.h>

class CAttachmentMerger;

class CAttachmentMerged : public _reference_target_t
{
	friend class CAttachmentMerger;

public:
	struct MergeInfo
	{
		_smart_ptr<IAttachment> pAttachment;
		IAttachmentObject*      pAttachmentObject;

		struct
		{
			u32 nFirstIndex;
			u32 nIndexCount;
		} IndexInfo[MAX_STATOBJ_LODS_NUM];
	};

	CAttachmentMerged(string strName, CAttachmentUpr* pAttachmentUpr);
	virtual ~CAttachmentMerged();

	void Invalidate();
	bool AreAttachmentBindingsValid();

	void HideMergedAttachment(const IAttachment* pAttachment, bool bHide);
	bool HasAttachment(const IAttachment* pAttachment) const;
	void AddAttachments(const DynArray<MergeInfo>& attachmentInfos);

	void DrawAttachment(SRendParams& rParams, const SRenderingPassInfo& passInfo, const Matrix34& rWorldMat34, f32 fZoomFactor = 1);

private:
	DynArray<MergeInfo>::iterator       FindAttachment(IAttachment* pAttachment);
	DynArray<MergeInfo>::const_iterator FindAttachment(const IAttachment* pAttachment) const;

	DynArray<MergeInfo>         m_MergedAttachments;
	DynArray<vtx_idx>           m_MergedAttachmentIndices[MAX_STATOBJ_LODS_NUM];
	bool                        m_bRequiresIndexUpdate;

	_smart_ptr<CAttachmentSKIN> m_pMergedSkinAttachment;
};
