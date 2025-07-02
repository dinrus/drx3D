// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LAYERNODE_H__
#define __LAYERNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CLayerNode : public CAnimNode
{
public:
	CLayerNode(i32k id);
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Layer; }

	virtual void           Animate(SAnimContext& animContext) override;

	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           Activate(bool bActivate) override;
	virtual void           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	bool m_bInit;
	bool m_bPreVisibility;

};

#endif//__LAYERNODE_H__
