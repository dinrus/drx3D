// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ScriptVarNode_h__
#define __ScriptVarNode_h__
#pragma once

#include <drx3D/Movie/AnimNode.h>

class CAnimScriptVarNode : public CAnimNode
{
public:
	CAnimScriptVarNode(i32k id);

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_ScriptVar; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           OnResume() override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;
	virtual bool           GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	void SetScriptValue();
	float m_value;
};

#endif // __ScriptVarNode_h__
