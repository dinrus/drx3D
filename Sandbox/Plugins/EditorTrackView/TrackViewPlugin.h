// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

#include "IPlugin.h"
#include "TerrainMoveTool.h"

class CTrackViewExporter;
class CAnimationContext;
class CTrackViewSequenceManager;
class CTrackViewAnimNode;

class CTrackViewPlugin : public IPlugin, public ITerrainMoveToolListener
{
public:
	CTrackViewPlugin();
	~CTrackViewPlugin();

	static CTrackViewExporter*        GetExporter();
	static CAnimationContext*         GetAnimationContext();
	static CTrackViewSequenceManager* GetSequenceManager();

	i32                             GetPluginVersion() override                          { return 1; }
	tukk                       GetPluginName() override                             { return "TrackView"; }
	tukk                       GetPluginDescription() override					   { return "Adds the TrackView tool"; }

private:
	void         OnOpenObjectContextMenu(CPopupMenuItem* pMenu, const CBaseObject* pObject);
	void         OnMenuOpenTrackView(CTrackViewAnimNode* pAnimNode);

	virtual void OnMove(const Vec3 targetPos, Vec3 sourcePos, bool bIsCopy) override;

	static CTrackViewExporter*        ms_pExporter;
	static CAnimationContext*         ms_pAnimationContext;
	static CTrackViewSequenceManager* ms_pSequenceManager;
};

