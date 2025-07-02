// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef ITEXTURESTREAMER_H
#define ITEXTURESTREAMER_H

struct STexStreamingInfo;
struct STexStreamPrepState;

typedef DynArray<CTexture*> TStreamerTextureVec;

class ITextureStreamer
{
public:
	enum EApplyScheduleFlags
	{
		eASF_InOut = 1,
		eASF_Prep  = 2,

		eASF_Full  = 3,
	};

public:
	ITextureStreamer();
	virtual ~ITextureStreamer() {}

public:
	virtual void  BeginUpdateSchedule();
	virtual void  ApplySchedule(EApplyScheduleFlags asf);

	virtual bool  BeginPrepare(CTexture* pTexture, tukk sFilename, u32 nFlags) = 0;
	virtual void  EndPrepare(STexStreamPrepState*& pState) = 0;

	virtual void  Precache(CTexture* pTexture) = 0;
	virtual void  UpdateMip(CTexture* pTexture, const float fMipFactor, i32k nFlags, i32k nUpdateId, i32k nCounter) = 0;

	virtual void  OnTextureDestroy(CTexture* pTexture) = 0;

	void          Relink(CTexture* pTexture);
	void          Unlink(CTexture* pTexture);

	virtual void  FlagOutOfMemory() = 0;
	virtual void  Flush() = 0;

	virtual bool  IsOverflowing() const = 0;
	virtual float GetBias() const { return 0.0f; }

	i32           GetMinStreamableMip() const;
	i32           GetMinStreamableMipWithSkip() const;

	void          StatsFetchTextures(std::vector<CTexture*>& out);
	bool          StatsWouldUnload(const CTexture* pTexture);

protected:
	void                 SyncTextureList();

	TStreamerTextureVec& GetTextures()
	{
		return m_textures;
	}

	DrxCriticalSection& GetAccessLock() { return m_accessLock; }

private:
	size_t StatsComputeRequiredMipMemUsage();

private:
	TStreamerTextureVec m_pendingRelinks;
	TStreamerTextureVec m_pendingUnlinks;
	TStreamerTextureVec m_textures;
	
	// MT Locks access to m_pendingRelinks,m_pendingUnlinks,m_textures
	DrxCriticalSection  m_accessLock;
};

#endif
