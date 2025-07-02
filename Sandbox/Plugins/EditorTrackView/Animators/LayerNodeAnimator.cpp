// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Source File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#include "StdAfx.h"
#include "LayerNodeAnimator.h"
#include "Nodes/TrackViewTrack.h"
#include "IObjectManager.h"
#include "Objects/IObjectLayer.h"
#include "Objects/IObjectLayerManager.h"

void CLayerNodeAnimator::Animate(CTrackViewAnimNode* pNode, const SAnimContext& ac)
{
	if (GetIEditor()->IsInGameMode())
	{
		return;
	}

	CTrackViewTrack* pTrack = pNode->GetTrackForParameter(eAnimParamType_Visibility);
	if (pTrack)
	{
		bool visible = stl::get<bool>(pTrack->GetValue(ac.time));

		IObjectLayerManager* pLayerManager =
		  GetIEditor()->GetObjectManager()->GetIObjectLayerManager();

		if (pLayerManager)
		{
			IObjectLayer* pLayer = pLayerManager->FindLayerByName(pNode->GetName());
			if (pLayer)
			{
				pLayer->SetVisible(visible);
			}
		}
	}
}

