// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CVarNode_h__
#define __CVarNode_h__
#pragma once

#include <drx3D/Movie/AnimNode.h>

class CAnimCVarNode : public CAnimNode
{
public:
	CAnimCVarNode(i32k id);

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_CVar; }

	virtual void           SetName(tukk name) override;
	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           OnResume() override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	float m_value;
};

#endif // __CVarNode_h__
