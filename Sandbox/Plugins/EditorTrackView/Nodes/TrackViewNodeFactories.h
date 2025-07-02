// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

class CTrackViewTrack;
class CTrackViewAnimNode;
class CTrackViewNode;
class CEntityObject;
struct IAnimSequence;
struct IAnimNode;
struct IAnimTrack;

class CTrackViewAnimNodeFactory
{
public:
	CTrackViewAnimNode* BuildAnimNode(IAnimSequence* pSequence, IAnimNode* pAnimNode, CTrackViewNode* pParentNode, CEntityObject* pEntity);
};

class CTrackViewTrackFactory
{
public:
	CTrackViewTrack* BuildTrack(IAnimTrack* pTrack, CTrackViewAnimNode* pTrackAnimNode,
	                            CTrackViewNode* pParentNode, bool bIsSubTrack = false, u32 subTrackIndex = 0);
};

