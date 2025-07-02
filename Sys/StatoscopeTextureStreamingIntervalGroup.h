// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef STATOSCOPETEXTURESTREAMINGINTERVALGROUP_H
#define STATOSCOPETEXTURESTREAMINGINTERVALGROUP_H

#include <drx3D/Sys/Statoscope.h>

#if ENABLE_STATOSCOPE

class CStatoscopeTextureStreamingIntervalGroup : public CStatoscopeIntervalGroup, public ITextureStreamListener
{
public:
	CStatoscopeTextureStreamingIntervalGroup();

	void Enable_Impl();
	void Disable_Impl();

public: // ITextureStreamListener Members
	virtual void OnCreatedStreamedTexture(uk pHandle, tukk name, i32 nMips, i32 nMinMipAvailable);
	virtual void OnUploadedStreamedTexture(uk pHandle);
	virtual void OnDestroyedStreamedTexture(uk pHandle);
	virtual void OnTextureWantsMip(uk pHandle, i32 nMinMip);
	virtual void OnTextureHasMip(uk pHandle, i32 nMinMip);
	virtual void OnBegunUsingTextures(uk * pHandles, size_t numHandles);
	virtual void OnEndedUsingTextures(uk * pHandle, size_t numHandles);

private:
	void OnChangedTextureUse(uk * pHandles, size_t numHandles, i32 inUse);
	void OnChangedMip(uk pHandle, i32 field, i32 mip);
};

#endif

#endif
