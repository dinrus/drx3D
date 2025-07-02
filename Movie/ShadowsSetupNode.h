// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SHADOWSSETUPNODE_H__
#define __SHADOWSSETUPNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CShadowsSetupNode : public CAnimNode
{
public:
	CShadowsSetupNode(i32k id);
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_ShadowSetup; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;

	virtual void           OnReset() override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;
};

#endif//__SHADOWSSETUPNODE_H__
