// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIMSCREENFADERNODE_H__
#define __ANIMSCREENFADERNODE_H__

#pragma once

#include <drx3D/Movie/AnimNode.h>

class CScreenFaderTrack;

class CAnimScreenFaderNode : public CAnimNode
{
public:
	CAnimScreenFaderNode(i32k id);
	~CAnimScreenFaderNode();
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_ScreenFader; }
	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           Activate(bool bActivate) override;
	virtual void           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

	virtual void           Render() override;

	bool                   IsAnyTextureVisible() const;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;
	virtual bool NeedToRender() const override { return true; }

private:
	CAnimScreenFaderNode(const CAnimScreenFaderNode&);
	CAnimScreenFaderNode& operator=(const CAnimScreenFaderNode&);

	void Deactivate();

private:
	void PrecacheTexData();

	Vec4  m_startColor;
	bool  m_bActive;
	i32   m_lastActivatedKey;
	bool  m_texPrecached;
};

#endif //__ANIMSCREENFADERNODE_H__
