// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#if defined(USE_GEOM_CACHES)
	#include <drx3D/Entity/GeomCacheAttachmentUpr.h>
	#include <drx3D/Entity/Entity.h>

void CGeomCacheAttachmentUpr::Update()
{
	for (VectorMap<SBinding, SAttachmentData>::iterator iter = m_attachments.begin(); iter != m_attachments.end(); ++iter)
	{
		iter->first.pChild->InvalidateTM(ENTITY_XFORM_FROM_PARENT);
	}
}

void CGeomCacheAttachmentUpr::RegisterAttachment(CEntity* pChild, CEntity* pParent, u32k targetHash)
{
	IGeomCacheRenderNode* pGeomCacheRenderNode = const_cast<CEntity*>(pParent)->GetGeomCacheRenderNode(0);

	if (pGeomCacheRenderNode)
	{
		const uint nodeCount = pGeomCacheRenderNode->GetNodeCount();

		for (uint i = 0; i < nodeCount; ++i)
		{
			u32k nodeNameHash = pGeomCacheRenderNode->GetNodeNameHash(i);
			if (nodeNameHash == targetHash)
			{
				SBinding binding;
				binding.pChild = pChild;
				binding.pParent = pParent;

				SAttachmentData attachment;
				attachment.m_nodeIndex = i;

				m_attachments[binding] = attachment;
				break;
			}
		}
	}
}

void CGeomCacheAttachmentUpr::UnregisterAttachment(const CEntity* pChild, const CEntity* pParent)
{
	SBinding binding;
	binding.pChild = const_cast<CEntity*>(pChild);
	binding.pParent = const_cast<CEntity*>(pParent);

	m_attachments.erase(binding);
}

Matrix34 CGeomCacheAttachmentUpr::GetNodeWorldTM(const CEntity* pChild, const CEntity* pParent) const
{
	SBinding binding;
	binding.pChild = const_cast<CEntity*>(pChild);
	binding.pParent = const_cast<CEntity*>(pParent);

	VectorMap<SBinding, SAttachmentData>::const_iterator findIter = m_attachments.find(binding);

	if (findIter != m_attachments.end())
	{
		const SAttachmentData data = findIter->second;
		IGeomCacheRenderNode* pGeomCacheRenderNode = const_cast<CEntity*>(binding.pParent)->GetGeomCacheRenderNode(0);
		if (pGeomCacheRenderNode)
		{
			Matrix34 nodeTransform = pGeomCacheRenderNode->GetNodeTransform(data.m_nodeIndex);
			nodeTransform.OrthonormalizeFast();
			return pParent->GetWorldTM() * nodeTransform;
		}
	}

	return pParent->GetWorldTM();
}

bool CGeomCacheAttachmentUpr::IsAttachmentValid(const CEntity* pChild, const CEntity* pParent) const
{
	SBinding binding;
	binding.pChild = const_cast<CEntity*>(pChild);
	binding.pParent = const_cast<CEntity*>(pParent);

	VectorMap<SBinding, SAttachmentData>::const_iterator findIter = m_attachments.find(binding);

	if (findIter != m_attachments.end())
	{
		const SAttachmentData data = findIter->second;
		IGeomCacheRenderNode* pGeomCacheRenderNode = const_cast<CEntity*>(binding.pParent)->GetGeomCacheRenderNode(0);
		return pGeomCacheRenderNode->IsNodeDataValid(data.m_nodeIndex);
	}

	return false;
}
#endif
