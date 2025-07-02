// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright 2001-2016 DinrusPro 3D GmbH. All rights reserved.

#pragma once

#include <DrxMovie/IMovieSystem.h>
#include "TrackViewTrack.h"

// Small specialization for animation tracks
class CTrackViewAnimationTrack : public CTrackViewTrack
{
public:
	CTrackViewAnimationTrack(IAnimTrack* pTrack, CTrackViewAnimNode* pTrackAnimNode, CTrackViewNode* pParentNode, bool bIsSubTrack = false, uint subTrackIndex = 0);

	virtual bool      KeysHaveDuration() const override { return true; }
	virtual SAnimTime GetKeyDuration(const uint index) const override;
	virtual SAnimTime GetKeyAnimDuration(const uint index) const override;
	virtual SAnimTime GetKeyAnimStart(const uint index) const override;
	virtual SAnimTime GetKeyAnimEnd(const uint index) const override;

private:
	float GetKeyDurationFromAnimationData(const SCharacterKey& key) const;
};

