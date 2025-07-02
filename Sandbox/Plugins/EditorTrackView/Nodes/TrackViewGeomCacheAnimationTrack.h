// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

#if defined(USE_GEOM_CACHES)
	#include <DrxMovie/IMovieSystem.h>
	#include "TrackViewTrack.h"

////////////////////////////////////////////////////////////////////////////
// This class represents a time range track of a geom cache node in TrackView
////////////////////////////////////////////////////////////////////////////
class CTrackViewGeomCacheAnimationTrack : public CTrackViewTrack
{
public:
	CTrackViewGeomCacheAnimationTrack(IAnimTrack* pTrack, CTrackViewAnimNode* pTrackAnimNode,
	                                  CTrackViewNode* pParentNode, bool bIsSubTrack = false, u32 subTrackIndex = 0)
		: CTrackViewTrack(pTrack, pTrackAnimNode, pParentNode, bIsSubTrack, subTrackIndex) {}

private:
	virtual CTrackViewKeyHandle CreateKey(const SAnimTime time) override;

	virtual bool                KeysHaveDuration() const override { return true; }

	virtual SAnimTime           GetKeyDuration(const uint index) const override;
	virtual SAnimTime           GetKeyAnimDuration(const uint index) const override;
	virtual SAnimTime           GetKeyAnimStart(const uint index) const override;
	virtual bool                IsKeyAnimLoopable(const uint index) const override;
};
#endif

