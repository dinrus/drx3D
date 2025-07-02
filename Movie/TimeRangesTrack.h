// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TIMERANGESTRACK_H__
#define __TIMERANGESTRACK_H__

#pragma once

//forward declarations.
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Movie/AnimTrack.h>

/** CTimeRangesTrack contains keys that represent generic time ranges
 */
class CTimeRangesTrack : public TAnimTrack<STimeRangeKey>
{
public:
	virtual void           SerializeKey(STimeRangeKey& key, XmlNodeRef& keyNode, bool bLoading) override;

	virtual CAnimParamType GetParameterType() const override { return eAnimParamType_TimeRanges; }

	i32                    GetActiveKeyIndexForTime(const SAnimTime time);
};

#endif
