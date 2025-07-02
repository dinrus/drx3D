// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIMPOSTFXNODE_H__
#define __ANIMPOSTFXNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CFXNodeDescription;

class CAnimPostFXNode : public CAnimNode
{
public:
	static void       Initialize();

	static CAnimNode* CreateNode(i32k id, EAnimNodeType nodeType);

	CAnimPostFXNode(i32k id, CFXNodeDescription* pDesc);

	virtual void           SerializeAnims(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;
	virtual EAnimNodeType  GetType() const override;
	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;
	virtual void           CreateDefaultTracks() override;
	virtual void           Activate(bool activate) override;
	virtual void           Animate(SAnimContext& animContext) override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

	typedef std::map<EAnimNodeType, _smart_ptr<CFXNodeDescription>> FxNodeDescriptionMap;
	static FxNodeDescriptionMap s_fxNodeDescriptions;
	static bool                 s_initialized;

	CFXNodeDescription*         m_pDescription;
};

#endif //__ANIMPOSTFXNODE_H__
