// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __EVENTNODE_H__
#define __EVENTNODE_H__

#include <drx3D/Movie/AnimNode.h>

class CAnimEventNode : public CAnimNode
{
public:
	CAnimEventNode(i32k id);

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Event; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;
	virtual bool           GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	void SetScriptValue();

private:
	//! Last animated key in track.
	i32 m_lastEventKey;
};

#endif //__EVENTNODE_H__
