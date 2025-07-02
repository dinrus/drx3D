// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   TextureUpr.h : Common texture manager declarations.

   Revision история:
* Created by Kenzo ter Elst
   =============================================================================*/

#ifndef _TEXTUREMANAGER_H
#define _TEXTUREMANAGER_H

#include <drx3D/CoreX/String/DrxName.h>

class CTexture;

class CTextureUpr
{
public:

	CTextureUpr() {}
	virtual ~CTextureUpr();

	void            PreloadDefaultTextures();
	void            ReleaseDefaultTextures();

	const CTexture* GetDefaultTexture(const string& sTextureName) const;
	const CTexture* GetDefaultTexture(const CDrxNameTSCRC& sTextureNameID) const;

private:

	typedef std::map<CDrxNameTSCRC, CTexture*> TTextureMap;
	TTextureMap m_DefaultTextures;
};

#endif
