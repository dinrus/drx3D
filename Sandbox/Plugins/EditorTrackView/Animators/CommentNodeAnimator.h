// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D
// -------------------------------------------------------------------------
//  File name: CommentNodeAnimator.h
//  Created:   09-04-2010 by Dongjoon Kim
//  Description: Comment node animator class
//
////////////////////////////////////////////////////////////////////////////

/** CCommentContext stores information about comment track.
   The Comment Track is activated only in the editor.
 */

#include "Nodes/TrackViewAnimNode.h"

class CTrackViewTrack;

struct CCommentContext
{
	CCommentContext() : m_nLastActiveKeyIndex(-1), m_strComment(0), m_size(1.0f), m_align(0)
	{
		drx_sprintf(m_strFont, "default");
		m_unitPos = Vec2(0.f, 0.f);
		m_color = Vec3(0.f, 0.f, 0.f);
	}

	i32         m_nLastActiveKeyIndex;

	tukk m_strComment;
	char        m_strFont[64];
	Vec2        m_unitPos;
	Vec3        m_color;
	float       m_size;
	i32         m_align;
};

class CCommentNodeAnimator : public IAnimNodeAnimator
{
public:
	CCommentNodeAnimator(CTrackViewAnimNode* pCommentNode);
	virtual void Animate(CTrackViewAnimNode* pNode, const SAnimContext& ac);
	virtual void Render(CTrackViewAnimNode* pNode, const SAnimContext& ac);

private:
	virtual ~CCommentNodeAnimator();

	void                AnimateCommentTextTrack(CTrackViewTrack* pTrack, const SAnimContext& ac);
	CTrackViewKeyHandle GetActiveKeyHandle(CTrackViewTrack* pTrack, SAnimTime fTime);
	Vec2                GetScreenPosFromNormalizedPos(const Vec2& unitPos);
	void                DrawText(tukk szFontName, float fSize, const Vec2& unitPos, const ColorF col, tukk szText, i32 align);

	CTrackViewAnimNode* m_pCommentNode;
	CCommentContext     m_commentContext;
};

