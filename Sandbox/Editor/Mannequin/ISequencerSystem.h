// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ISEQUENCER_SYSTEM__H__
#define __ISEQUENCER_SYSTEM__H__
#pragma once

class CSequencerTrack;
class CSequencerNode;
class CSequencerSequence;

class XmlNodeRef;

enum ESequencerParamType
{
	SEQUENCER_PARAM_UNDEFINED = 0,
	SEQUENCER_PARAM_FRAGMENTID,
	SEQUENCER_PARAM_TAGS,
	SEQUENCER_PARAM_PARAMS,
	SEQUENCER_PARAM_ANIMLAYER,
	SEQUENCER_PARAM_PROCLAYER,
	SEQUENCER_PARAM_TRANSITIONPROPS,
	SEQUENCER_PARAM_TOTAL
};

enum ESequencerNodeType
{
	SEQUENCER_NODE_UNDEFINED,
	SEQUENCER_NODE_SEQUENCE_ANALYZER_GLOBAL,
	SEQUENCER_NODE_SCOPE,
	SEQUENCER_NODE_FRAGMENT_EDITOR_GLOBAL,
	SEQUENCER_NODE_FRAGMENT,
	SEQUENCER_NODE_FRAGMENT_CLIPS,
	SEQUENCER_NODE_TOTAL
};

enum ESequencerKeyFlags
{
	AKEY_SELECTED = 0x01
};

struct SKeyColour
{
	u8 base[3];
	u8 high[3];
	u8 back[3];
};

struct CSequencerKey
{
	CSequencerKey()
		: m_time()
		, flags()
		, m_fileState()
		, m_filePath()
		, m_fileName()
	{
	}

	virtual bool IsFileInsidePak() const { return m_fileState & eIsInsidePak; }
	virtual bool IsFileInsideDB() const  { return m_fileState & eIsInsideDB; }
	virtual bool IsFileOnDisk() const    { return m_fileState & eIsOnDisk; }
	virtual bool HasValidFile() const    { return m_fileState & eHasFile; }
	virtual bool HasFileRepresentation() const
	{
		std::vector<CString> extensions;
		CString editableExtension;
		GetExtensions(extensions, editableExtension);
		return extensions.size() > 0;
	}

	virtual const CString GetFileName() const { return m_fileName; }
	virtual const CString GetFilePath() const { return m_filePath; }

	// Returns a list of all paths relevant to this file
	// E.G. for i_cafs will return paths to .ma, .icaf and .animsetting
	// N.B. path is relative to game root
	virtual void GetFilePaths(std::vector<CString>& paths, CString& relativePath, CString& fileName, CString& editableExtension) const
	{
		std::vector<CString> extensions;
		GetExtensions(extensions, editableExtension);

		relativePath = m_filePath;
		fileName = m_fileName;

		CString fullPath = PathUtil::AddSlash(m_filePath.GetString());
		fullPath += m_fileName;

		paths.reserve(extensions.size());
		for (std::vector<CString>::const_iterator iter = extensions.begin(), itEnd = extensions.end(); iter != itEnd; ++iter)
		{
			paths.push_back(fullPath + (*iter));
		}
	}

	virtual void UpdateFlags()
	{
		m_fileState = eNone;
	};

	float m_time;
	i32   flags;

	bool operator<(const CSequencerKey& key) const  { return m_time < key.m_time; }
	bool operator==(const CSequencerKey& key) const { return m_time == key.m_time; }
	bool operator>(const CSequencerKey& key) const  { return m_time > key.m_time; }
	bool operator<=(const CSequencerKey& key) const { return m_time <= key.m_time; }
	bool operator>=(const CSequencerKey& key) const { return m_time >= key.m_time; }
	bool operator!=(const CSequencerKey& key) const { return m_time != key.m_time; }

protected:

	// Provides all extensions relevant to the clip in extensions, and the one used to edit the clip
	// in editableExtension...
	// E.G. for i_cafs will return .ma, .animsetting, .i_caf and return .ma also as editableExtension
	virtual void GetExtensions(std::vector<CString>& extensions, CString& editableExtension) const
	{
		extensions.clear();
		editableExtension.Empty();
	};

	enum ESequencerKeyFileState
	{
		eNone        = 0,
		eHasFile     = BIT(0),
		eIsInsidePak = BIT(1),
		eIsInsideDB  = BIT(2),
		eIsOnDisk    = BIT(3),
	};

	ESequencerKeyFileState m_fileState; // Flags to represent the state of (m_filePath + m_fileName + m_fileExtension)
	CString                m_filePath;  // The path relative to game root
	CString                m_fileName;  // The filename of the key file (e.g. filename.i_caf for CClipKey)
};

class CSequencerTrack
	: public _i_reference_target_t
{
public:
	enum ESequencerTrackFlags
	{
		SEQUENCER_TRACK_HIDDEN   = BIT(1), //!< Set when track is hidden in track view.
		SEQUENCER_TRACK_SELECTED = BIT(2), //!< Set when track is selected in track view.
	};

public:
	CSequencerTrack()
		: m_nParamType(SEQUENCER_PARAM_UNDEFINED)
		, m_bModified(false)
		, m_flags(0)
		, m_changeCount(0)
		, m_muted(false)
	{
	}

	virtual ~CSequencerTrack() {}

	virtual void                 SetNumKeys(i32 numKeys) = 0;
	virtual i32                  GetNumKeys() const = 0;

	virtual i32                  CreateKey(float time) = 0;

	virtual void                 SetKey(i32 index, CSequencerKey* key) = 0;
	virtual void                 GetKey(i32 index, CSequencerKey* key) const = 0;

	virtual const CSequencerKey* GetKey(i32 index) const = 0;
	virtual CSequencerKey*       GetKey(i32 index) = 0;

	virtual void                 RemoveKey(i32 num) = 0;

	virtual void                 GetKeyInfo(i32 key, tukk & description, float& duration) = 0;
	virtual void                 GetTooltip(i32 key, tukk & description, float& duration) { return GetKeyInfo(key, description, duration); }
	virtual float                GetKeyDuration(i32k key) const = 0;
	virtual const SKeyColour& GetKeyColour(i32 key) const = 0;
	virtual const SKeyColour& GetBlendColour(i32 key) const = 0;

	virtual bool              CanEditKey(i32 key) const   { return true; }
	virtual bool              CanMoveKey(i32 key) const   { return true; }

	virtual bool              CanAddKey(float time) const { return true; }
	virtual bool              CanRemoveKey(i32 key) const { return true; }

	virtual i32               CloneKey(i32 key) = 0;
	virtual i32               CopyKey(CSequencerTrack* pFromTrack, i32 nFromKey) = 0;

	virtual i32               GetNumSecondarySelPts(i32 key) const { return 0; }

	// Look for a secondary selection point in the key & time range specified.
	//
	// Returns an id that can be passed into the other secondary selection functions.
	// Returns 0 when it could not find a secondary selection point.
	virtual i32 GetSecondarySelectionPt(i32 key, float timeMin, float timeMax) const
	{
		return 0;
	}

	// Look for a secondary selection point in the selected keys, in the time range specified.
	//
	// Returns the key index as well as an id that can be passed into the other secondary selection functions.
	// Returns 0 when it could not find a secondary selection point.
	virtual i32 FindSecondarySelectionPt(i32& key, float timeMin, float timeMax) const
	{
		return 0;
	}

	virtual void        SetSecondaryTime(i32 key, i32 id, float time)    {}
	virtual float       GetSecondaryTime(i32 key, i32 id) const          { return 0.0f; }
	virtual CString     GetSecondaryDescription(i32 key, i32 id) const   { return ""; }
	virtual bool        CanMoveSecondarySelection(i32 key, i32 id) const { return true; }

	virtual void        InsertKeyMenuOptions(CMenu& menu, i32 keyID)     {}
	virtual void        ClearKeyMenuOptions(CMenu& menu, i32 keyID)      {}
	virtual void        OnKeyMenuOption(i32 menuOption, i32 keyID)       {}

	virtual ColorB      GetColor() const                                 { return ColorB(220, 220, 220); }

	ESequencerParamType GetParameterType() const                         { return m_nParamType; }
	void                SetParameterType(ESequencerParamType type)       { m_nParamType = type; }

	void                UpdateKeys()                                     { MakeValid(); }

	i32                 NextKeyByTime(i32 key) const
	{
		if (key + 1 < GetNumKeys())
			return key + 1;
		else
			return -1;
	}

	i32 FindKey(float time) const
	{
		i32k keyCount = GetNumKeys();
		for (i32 i = 0; i < keyCount; i++)
		{
			const CSequencerKey* pKey = GetKey(i);
			assert(pKey);
			if (pKey->m_time == time)
				return i;
		}
		return -1;
	}

	virtual void SetKeyTime(i32 index, float time)
	{
		CSequencerKey* key = GetKey(index);
		assert(key);
		if (CanMoveKey(index))
		{
			key->m_time = time;
			Invalidate();
		}
	}

	float GetKeyTime(i32 index) const
	{
		const CSequencerKey* key = GetKey(index);
		assert(key);
		return key->m_time;
	}

	void SetKeyFlags(i32 index, i32 flags)
	{
		CSequencerKey* key = GetKey(index);
		assert(key);
		key->flags = flags;
		Invalidate();
	}

	i32 GetKeyFlags(i32 index) const
	{
		const CSequencerKey* key = GetKey(index);
		assert(key);
		return key->flags;
	}

	virtual void SelectKey(i32 index, bool select)
	{
		CSequencerKey* key = GetKey(index);
		assert(key);
		if (select)
			key->flags |= AKEY_SELECTED;
		else
			key->flags &= ~AKEY_SELECTED;
	}

	bool IsKeySelected(i32 index) const
	{
		const CSequencerKey* key = GetKey(index);
		assert(key);
		const bool isKeySelected = (key->flags & AKEY_SELECTED);
		return isKeySelected;
	}

	void SortKeys()
	{
		DoSortKeys();
		m_bModified = false;
	}

	void SetFlags(i32 flags) { m_flags = flags; }
	i32  GetFlags()          { return m_flags; }

	void SetSelected(bool bSelect)
	{
		if (bSelect)
			m_flags |= SEQUENCER_TRACK_SELECTED;
		else
			m_flags &= ~SEQUENCER_TRACK_SELECTED;
	}

	void         SetTimeRange(const Range& timeRange) { m_timeRange = timeRange; }
	const Range& GetTimeRange() const                 { return m_timeRange; }

	void         OnChange()
	{
		m_changeCount++;
		OnChangeCallback();
	}
	u32       GetChangeCount() const { return m_changeCount; }
	void         ResetChangeCount()     { m_changeCount = 0; }

	virtual bool Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks = true) = 0;
	virtual bool SerializeSelection(XmlNodeRef& xmlNode, bool bLoading, bool bCopySelected = false, float fTimeOffset = 0) = 0;

	void         Mute(bool bMute) { m_muted = bMute; }
	bool         IsMuted() const  { return m_muted; }

protected:
	virtual void OnChangeCallback() {}
	virtual void DoSortKeys() = 0;

	void         MakeValid()
	{
		if (m_bModified)
			SortKeys();
		assert(!m_bModified);
	}

	void Invalidate() { m_bModified = true; }

private:
	ESequencerParamType m_nParamType;
	bool                m_bModified;
	Range               m_timeRange;
	i32                 m_flags;
	u32              m_changeCount;
	bool                m_muted;
};

#endif // __ISequencerSystem_h__

