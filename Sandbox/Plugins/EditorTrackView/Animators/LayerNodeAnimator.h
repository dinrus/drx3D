// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D, 1999-2010.
// -------------------------------------------------------------------------
//  File name: LayerNodeAnimator.h
//  Version:   v1.00
//  Created:   22-03-2010 by Dongjoon Kim
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Nodes/TrackViewAnimNode.h"

class CLayerNodeAnimator : public IAnimNodeAnimator
{
public:
	void Animate(CTrackViewAnimNode* pNode, const SAnimContext& ac);

protected:
	virtual ~CLayerNodeAnimator() {}
};

