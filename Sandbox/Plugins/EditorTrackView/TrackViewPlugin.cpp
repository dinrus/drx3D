// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Source File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#include "StdAfx.h"
#include "TrackViewPlugin.h"

#include <drx3D/CoreX/Platform/platform_impl.inl>

#include "AnimationContext.h"
#include "TrackViewWindow.h"
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "Controls/DynamicPopupMenu.h"
#include "TrackViewSequenceManager.h"
#include "Objects/SequenceObject.h"
#include "Objects/EntityObject.h"
#include "TrackViewExporter.h"

REGISTER_VIEWPANE_FACTORY(CTrackViewWindow, "Track View", "Tools", false)

REGISTER_PLUGIN(CTrackViewPlugin)

CTrackViewExporter* CTrackViewPlugin::ms_pExporter;
CAnimationContext* CTrackViewPlugin::ms_pAnimationContext;
CTrackViewSequenceManager* CTrackViewPlugin::ms_pSequenceManager;

CTrackViewPlugin::CTrackViewPlugin()
{
	GetIEditor()->RegisterObjectContextMenuExtension(functor(*this, &CTrackViewPlugin::OnOpenObjectContextMenu));

	ms_pSequenceManager = new CTrackViewSequenceManager();
	ms_pAnimationContext = new CAnimationContext();
	ms_pExporter = new CTrackViewExporter();
}

CTrackViewPlugin::~CTrackViewPlugin()
{
	delete ms_pAnimationContext;
	delete ms_pSequenceManager;
	delete ms_pExporter;
}

CAnimationContext* CTrackViewPlugin::GetAnimationContext()
{
	return ms_pAnimationContext;
}

CTrackViewSequenceManager* CTrackViewPlugin::GetSequenceManager()
{
	return ms_pSequenceManager;
}

CTrackViewExporter* CTrackViewPlugin::GetExporter()
{
	return ms_pExporter;
}

void CTrackViewPlugin::OnOpenObjectContextMenu(CPopupMenuItem* pMenu, const CBaseObject* pObject)
{
	if (!pObject->IsKindOf(RUNTIME_CLASS(CEntityObject)))
	{
		return;
	}

	const CEntityObject* pEntityObject = static_cast<const CEntityObject*>(pObject);

	// TrackView sequences
	CTrackViewAnimNodeBundle bundle = CTrackViewPlugin::GetSequenceManager()->GetAllRelatedAnimNodes(pEntityObject);

	if (bundle.GetCount() > 0)
	{
		pMenu->AddSeparator();

		CPopupMenuItem& sequenceMenu = pMenu->Add("Track View Sequences");

		u32k nodeListCount = bundle.GetCount();
		for (u32 nodeIndex = 0; nodeIndex < nodeListCount; ++nodeIndex)
		{
			CTrackViewSequence* pSequence = bundle.GetNode(nodeIndex)->GetSequence();

			if (pSequence)
			{
				sequenceMenu.Add(pSequence->GetName(), functor(*this, &CTrackViewPlugin::OnMenuOpenTrackView), bundle.GetNode(nodeIndex));
			}
		}
	}
}

void CTrackViewPlugin::OnMenuOpenTrackView(CTrackViewAnimNode* pAnimNode)
{
	GetIEditor()->OpenView("Track View");
	std::vector<CTrackViewWindow*> windows = CTrackViewWindow::GetTrackViewWindows();
	for (CTrackViewWindow* pWindow : windows)
	{
		// TODO
		//pWindow->ShowNode(pAnimNode);
	}
}

void CTrackViewPlugin::OnMove(const Vec3 targetPos, Vec3 sourcePos, bool bIsCopy)
{
	if (bIsCopy)
	{
		return;
	}

	const Vec3 offset = targetPos - sourcePos;

	const CTrackViewSequenceManager* pSequenceManager = CTrackViewPlugin::GetSequenceManager();
	u32k numSequences = pSequenceManager->GetCount();

	for (u32 sequenceIndex = 0; sequenceIndex < numSequences; ++sequenceIndex)
	{
		CTrackViewSequence* pSequence = pSequenceManager->GetSequenceByIndex(sequenceIndex);
		CTrackViewTrackBundle trackBundle = pSequence->GetAllTracks();

		u32k numTracks = trackBundle.GetCount();
		for (u32 trackIndex = 0; trackIndex < numTracks; ++trackIndex)
		{
			CTrackViewTrack* pTrack = trackBundle.GetTrack(trackIndex);
			if (pTrack->GetParameterType() != eAnimParamType_Position)
			{
				continue;
			}

			CTrackViewAnimNode* pAnimNode = pTrack->GetAnimNode();
			if (pAnimNode)
			{
				CEntityObject* pEntityObject = pAnimNode->GetNodeEntity();
				if (pEntityObject && pEntityObject->GetParent())
				{
					// If entity object of the node is parented to something then do not move the keys,
					// because the parent will be moved and therefore the object with it.
					continue;
				}
			}

			pTrack->OffsetKeys(TMovieSystemValue(offset));
		}
	}
}

