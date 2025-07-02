// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __COMMENTNODE_H__
#define __COMMENTNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CCommentContext;

class CCommentNode : public CAnimNode
{
public:
	CCommentNode(i32k id);
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Comment; }

	virtual void           CreateDefaultTracks() override;
	virtual void           Activate(bool bActivate) override;
	virtual void           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;
};

#endif //__COMMENTNODE_H__
