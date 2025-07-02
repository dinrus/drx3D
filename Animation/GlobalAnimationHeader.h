// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef RESOURCE_COMPILER
struct GlobalAnimationHeader
{
	GlobalAnimationHeader()
		: m_nFlags(0)
		, m_FilePath()
		, m_FilePathCRC32(0)
	{}

	ILINE u32      IsAimpose() const           { return m_nFlags & CA_AIMPOSE; }
	ILINE void        OnAimpose()                 { m_nFlags |= CA_AIMPOSE; }

	ILINE u32      IsAimposeUnloaded() const   { return m_nFlags & CA_AIMPOSE_UNLOADED; }
	ILINE void        OnAimposeUnloaded()         { m_nFlags |= CA_AIMPOSE_UNLOADED; }

	ILINE u32      IsAssetCreated() const      { return m_nFlags & CA_ASSET_CREATED; }
	ILINE void        OnAssetCreated()            { m_nFlags |= CA_ASSET_CREATED; }
	ILINE void        InvalidateAssetCreated()    { m_nFlags &= (CA_ASSET_CREATED ^ -1); }

	ILINE u32      IsAssetAdditive() const     { return m_nFlags & CA_ASSET_ADDITIVE; }
	ILINE void        OnAssetAdditive()           { m_nFlags |= CA_ASSET_ADDITIVE; }

	ILINE u32      IsAssetCycle() const        { return m_nFlags & CA_ASSET_CYCLE; }
	ILINE void        OnAssetCycle()              { m_nFlags |= CA_ASSET_CYCLE; }

	ILINE u32      IsAssetLMG() const          { return m_nFlags & CA_ASSET_LMG; }
	ILINE void        OnAssetLMG()                { m_nFlags |= CA_ASSET_LMG; }

	ILINE u32      IsAssetLMGValid() const     { return m_nFlags & CA_ASSET_LMG_VALID; }
	ILINE void        OnAssetLMGValid()           { m_nFlags |= CA_ASSET_LMG_VALID; }
	ILINE void        InvalidateAssetLMG()        { m_nFlags &= (CA_ASSET_LMG_VALID ^ -1); }

	ILINE u32      IsAssetRequested() const    { return m_nFlags & CA_ASSET_REQUESTED; }
	ILINE void        OnAssetRequested()          { m_nFlags |= CA_ASSET_REQUESTED; }
	ILINE void        ClearAssetRequested()       { m_nFlags &= ~CA_ASSET_REQUESTED; }

	ILINE u32      IsAssetNotFound() const     { return m_nFlags & CA_ASSET_NOT_FOUND; }
	ILINE void        OnAssetNotFound()           { m_nFlags |= CA_ASSET_NOT_FOUND; }
	ILINE void        ClearAssetNotFound()        { m_nFlags &= ~CA_ASSET_NOT_FOUND; }

	ILINE u32      IsAssetTCB() const          { return m_nFlags & CA_ASSET_TCB; }

	ILINE u32      IsAssetInternalType() const { return m_nFlags & CA_ASSET_INTERNALTYPE; }
	ILINE void        OnAssetInternalType()       { m_nFlags |= CA_ASSET_INTERNALTYPE; }

	ILINE u32      GetFlags() const            { return m_nFlags; }
	ILINE void        SetFlags(u32 nFlags)     { m_nFlags = nFlags; }

	ILINE tukk GetFilePath() const         { return m_FilePath.c_str(); }
	ILINE void        SetFilePath(const string& name);

	ILINE i32         GetFilePathCRC32() const { return m_FilePathCRC32; }

protected:

	u32 m_nFlags;
	string m_FilePath;        //low-case path-name - unique per animation asset
	u32 m_FilePathCRC32;   //hash value for searching animation-paths (lower-case)
};

inline void GlobalAnimationHeader::SetFilePath(const string& name)
{
	m_FilePath = name;
	m_FilePathCRC32 = CCrc32::ComputeLowercase(name.c_str());
}
#endif