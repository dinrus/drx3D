// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANIMGEOMCACHENODE_H__
#define __ANIMGEOMCACHENODE_H__

#pragma once

#if defined(USE_GEOM_CACHES)
	#include <drx3D/Movie/EntityNode.h>

class CAnimGeomCacheNode : public CAnimEntityNode
{
public:
	CAnimGeomCacheNode(i32k id);
	~CAnimGeomCacheNode();
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_GeomCache; }
	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           OnReset() override;
	virtual void           Activate(bool bActivate) override;
	virtual void           PrecacheDynamic(SAnimTime startTime) override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

private:
	virtual bool          GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

	IGeomCacheRenderNode* GetGeomCacheRenderNode();

	CAnimGeomCacheNode(const CAnimGeomCacheNode&);
	CAnimGeomCacheNode& operator=(const CAnimGeomCacheNode&);

	bool m_bActive;
};

#endif
#endif
