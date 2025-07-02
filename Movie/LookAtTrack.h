// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LOOKATTRACK_H__
#define __LOOKATTRACK_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Movie/AnimTrack.h>

/** Look at target track, keys represent new lookat targets for entity.
 */
class CLookAtTrack : public TAnimTrack<SLookAtKey>
{
public:
	CLookAtTrack() : m_iAnimationLayer(-1) {}

	virtual bool           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual void           SerializeKey(SLookAtKey& key, XmlNodeRef& keyNode, bool bLoading) override;

	virtual CAnimParamType GetParameterType() const override          { return eAnimParamType_LookAt; }

	virtual i32            GetAnimationLayerIndex() const override    { return m_iAnimationLayer; }
	virtual void           SetAnimationLayerIndex(i32 index) override { m_iAnimationLayer = index; }

private:
	i32 m_iAnimationLayer;
};

#endif // __LOOKATTRACK_H__
