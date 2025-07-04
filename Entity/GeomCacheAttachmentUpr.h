// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(USE_GEOM_CACHES)
struct IGeomCacheRenderNode;
class CEntity;

	#include <drx3D/CoreX/Containers/VectorMap.h>

class CGeomCacheAttachmentUpr
{
public:
	void     Update();

	void     RegisterAttachment(CEntity* pChild, CEntity* pParent, u32k targetHash);
	void     UnregisterAttachment(const CEntity* pChild, const CEntity* pParent);

	Matrix34 GetNodeWorldTM(const CEntity* pChild, const CEntity* pParent) const;
	bool     IsAttachmentValid(const CEntity* pChild, const CEntity* pParent) const;

private:
	struct SBinding
	{
		CEntity* pChild;
		CEntity* pParent;

		bool operator<(const SBinding& rhs) const
		{
			return (pChild != rhs.pChild) ? (pChild < rhs.pChild) : (pParent < rhs.pParent);
		}
	};

	struct SAttachmentData
	{
		uint m_nodeIndex;
	};

	VectorMap<SBinding, SAttachmentData> m_attachments;
};

#endif
