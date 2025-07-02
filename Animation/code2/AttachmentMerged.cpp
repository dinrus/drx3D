// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AttachmentMerged.h>

#include <drx3D/Animation/AttachmentMerger.h>
#include <drx3D/Animation/AttachmentUpr.h>
#include <drx3D/Animation/CharacterInstance.h>


CAttachmentMerged::CAttachmentMerged(string strName, CAttachmentUpr* pAttachmentUpr)
	: m_bRequiresIndexUpdate(false)
{
	m_pMergedSkinAttachment = new CAttachmentSKIN();
	m_pMergedSkinAttachment->m_pAttachmentUpr = pAttachmentUpr;
	m_pMergedSkinAttachment->m_strSocketName = strName;
	m_pMergedSkinAttachment->m_nSocketCRC32 = CCrc32::ComputeLowercase(strName.c_str());
}

CAttachmentMerged::~CAttachmentMerged()
{
	CAttachmentMerger::Instance().OnDeleteMergedAttachment(this);
}

DynArray<CAttachmentMerged::MergeInfo>::iterator CAttachmentMerged::FindAttachment(IAttachment* pAttachment)
{
	return std::find_if(m_MergedAttachments.begin(), m_MergedAttachments.end(),
	                    [&](const MergeInfo& attachmentInfo) { return attachmentInfo.pAttachment == pAttachment; });
}

DynArray<CAttachmentMerged::MergeInfo>::const_iterator CAttachmentMerged::FindAttachment(const IAttachment* pAttachment) const
{
	return std::find_if(m_MergedAttachments.begin(), m_MergedAttachments.end(),
	                    [&](const MergeInfo& attachmentInfo) { return attachmentInfo.pAttachment == pAttachment; });
}

bool CAttachmentMerged::HasAttachment(const IAttachment* pAttachment) const
{
	auto it = FindAttachment(pAttachment);
	return it != m_MergedAttachments.end();
}

void CAttachmentMerged::HideMergedAttachment(const IAttachment* pAttachment, bool bHide)
{
	DRX_ASSERT(pAttachment->IsMerged());
	DRX_ASSERT(HasAttachment(pAttachment));

	m_bRequiresIndexUpdate = true;
}

void CAttachmentMerged::AddAttachments(const DynArray<MergeInfo>& attachmentInfos)
{
	for (auto const& attachmentInfo : attachmentInfos)
	{
		DRX_ASSERT(CAttachmentMerger::Instance().CanMerge(m_pMergedSkinAttachment, attachmentInfo.pAttachment));
		DRX_ASSERT(!attachmentInfo.pAttachment->IsMerged());

		for (i32 lod = 0; lod < MAX_STATOBJ_LODS_NUM; ++lod)
		{
			DRX_ASSERT(attachmentInfo.IndexInfo[lod].nFirstIndex == 0 || attachmentInfo.IndexInfo[lod].nFirstIndex + attachmentInfo.IndexInfo[lod].nIndexCount - 1 < m_MergedAttachmentIndices[lod].size());
		}

		uint nFlags = attachmentInfo.pAttachment->GetFlags();
		attachmentInfo.pAttachment->SetFlags(nFlags | FLAGS_ATTACH_MERGED_FOR_SHADOWS);
	}

	m_MergedAttachments.insert(m_MergedAttachments.end(), attachmentInfos.begin(), attachmentInfos.end());
}

bool CAttachmentMerged::AreAttachmentBindingsValid()
{
	for (auto& attachmentInfo : m_MergedAttachments)
	{
		DRX_ASSERT(attachmentInfo.pAttachment);
		if (attachmentInfo.pAttachment->GetIAttachmentObject() != attachmentInfo.pAttachmentObject)
			return false;
	}

	return m_MergedAttachments.size() > 0;
}

void CAttachmentMerged::Invalidate()
{
	for (auto& attachmentInfo : m_MergedAttachments)
	{
		DRX_ASSERT(attachmentInfo.pAttachment->IsMerged());

		uint nFlags = attachmentInfo.pAttachment->GetFlags();
		attachmentInfo.pAttachment->SetFlags(nFlags & ~FLAGS_ATTACH_MERGED_FOR_SHADOWS);

		m_pMergedSkinAttachment->m_pAttachmentUpr->RemoveExtraBone(attachmentInfo.pAttachment);
	}

	m_MergedAttachments.clear();
	m_bRequiresIndexUpdate = false;

	CAttachmentMerger::Instance().OnDeleteMergedAttachment(this);

	u32k loadingFlags = (m_pMergedSkinAttachment->m_pAttachmentUpr->m_pSkelInstance->m_CharEditMode ? CA_CharEditModel : 0);
	m_pMergedSkinAttachment->Immediate_ClearBinding(loadingFlags);
}

void CAttachmentMerged::DrawAttachment(SRendParams& rParams, const SRenderingPassInfo& passInfo, const Matrix34& rWorldMat34, f32 fZoomFactor)
{
	if (m_bRequiresIndexUpdate)
	{
		for (i32 lod = 0; lod < m_pMergedSkinAttachment->m_pModelSkin->GetNumLODs(); ++lod)
		{
			DynArray<std::pair<u32, u32>> indexRanges;
			indexRanges.reserve(m_MergedAttachments.size());

			auto curRange = std::make_pair(0u, 0u);
			for (auto& mergeInfo : m_MergedAttachments)
			{
				if (mergeInfo.pAttachment->IsAttachmentHiddenInShadow())
					continue;

				if (curRange.second == 0)
					curRange.first = mergeInfo.IndexInfo[lod].nFirstIndex;

				// cannot merge: start new range
				if (mergeInfo.IndexInfo[lod].nFirstIndex > curRange.first + curRange.second)
				{
					indexRanges.push_back(curRange);

					curRange.first = mergeInfo.IndexInfo[lod].nFirstIndex;
					curRange.second = mergeInfo.IndexInfo[lod].nIndexCount;
				}
				else
				{
					curRange.second += mergeInfo.IndexInfo[lod].nIndexCount;
				}
			}
			indexRanges.push_back(curRange);

			CAttachmentMerger::Instance().UpdateIndices(this, lod, indexRanges);
		}

		m_bRequiresIndexUpdate = false;
	}

	m_pMergedSkinAttachment->DrawAttachment(rParams, passInfo, rWorldMat34, fZoomFactor);
}
