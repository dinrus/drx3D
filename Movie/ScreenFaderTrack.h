// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCREENFADERTRACK_H__
#define __SCREENFADERTRACK_H__

#pragma once

#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Movie/AnimTrack.h>

class CScreenFaderTrack : public TAnimTrack<SScreenFaderKey>
{
public:
	CScreenFaderTrack();
	~CScreenFaderTrack();

	virtual void           SerializeKey(SScreenFaderKey& key, XmlNodeRef& keyNode, bool bLoading) override;

	virtual CAnimParamType GetParameterType() const override { return eAnimParamType_ScreenFader; }

public:
	void      PreloadTextures();
	ITexture* GetActiveTexture() const;
	void      SetScreenFaderTrackDefaults();

	bool      IsTextureVisible() const         { return m_bTextureVisible; }
	void      SetTextureVisible(bool bVisible) { m_bTextureVisible = bVisible; }
	Vec4      GetDrawColor() const             { return m_drawColor; }
	void      SetDrawColor(Vec4 vDrawColor)    { m_drawColor = vDrawColor; }
	i32       GetLastTextureID() const         { return m_lastTextureID; }
	void      SetLastTextureID(i32 nTextureID) { m_lastTextureID = nTextureID; }
	bool      SetActiveTexture(i32 index);

private:
	void ReleasePreloadedTextures();

	std::vector<ITexture*> m_preloadedTextures;
	bool                   m_bTextureVisible;
	Vec4                   m_drawColor;
	i32                    m_lastTextureID;
};

#endif//__SCREENFADERTRACK_H__
