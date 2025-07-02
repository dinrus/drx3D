// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

#include <DrxMovie/IMovieSystem.h>
#include "TrackViewTrack.h"

// Small specialization for sequence tracks
class CTrackViewSequenceTrack : public CTrackViewTrack
{
public:
	CTrackViewSequenceTrack(IAnimTrack* pTrack, CTrackViewAnimNode* pTrackAnimNode,
	                        CTrackViewNode* pParentNode, bool bIsSubTrack = false, uint subTrackIndex = 0)
		: CTrackViewTrack(pTrack, pTrackAnimNode, pParentNode, bIsSubTrack, subTrackIndex) {}

private:
	virtual string                      GetKeyDescription(const uint index) const override;

	virtual void                        SetKey(uint keyIndex, const STrackKey* pKey) override;
	virtual _smart_ptr<IAnimKeyWrapper> GetWrappedKey(i32 key) override;

	virtual tukk                 GetKeyType() const override;

	virtual bool                        KeysHaveDuration() const override { return true; }

	virtual SAnimTime                   GetKeyDuration(const uint index) const override;
	virtual SAnimTime                   GetKeyAnimDuration(const uint index) const override;
	virtual SAnimTime                   GetKeyAnimStart(const uint index) const override;
};

