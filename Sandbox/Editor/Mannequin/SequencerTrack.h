// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SEQUENCER_TRACK_h__
#define __SEQUENCER_TRACK_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "ISequencerSystem.h"

i32k KEY_MENU_CMD_BASE = 1000;

template<class KeyType>
class TSequencerTrack
	: public CSequencerTrack
{
public:
	//! Return number of keys in track.
	virtual i32 GetNumKeys() const { return m_keys.size(); }

	//! Set number of keys in track.
	//! If needed adds empty keys at end or remove keys from end.
	virtual void SetNumKeys(i32 numKeys) { m_keys.resize(numKeys); }

	//! Remove specified key.
	virtual void RemoveKey(i32 num);

	i32          CreateKey(float time);
	i32          CloneKey(i32 fromKey);
	i32          CopyKey(CSequencerTrack* pFromTrack, i32 nFromKey);

	//! Get key at specified location.
	//! @param key Must be valid pointer to compatible key structure, to be filled with specified key location.
	virtual void                 GetKey(i32 index, CSequencerKey* key) const;

	virtual const CSequencerKey* GetKey(i32 index) const;
	virtual CSequencerKey*       GetKey(i32 index);

	//! Set key at specified location.
	//! @param key Must be valid pointer to compatible key structure.
	virtual void SetKey(i32 index, CSequencerKey* key);

	/** Serialize this animation track to XML.
	    Do not override this method, prefer to override SerializeKey.
	 */
	virtual bool Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks = true);

	virtual bool SerializeSelection(XmlNodeRef& xmlNode, bool bLoading, bool bCopySelected = false, float fTimeOffset = 0);

	/** Serialize single key of this track.
	    Override this in derived classes.
	    Do not save time attribute, it is already saved in Serialize of the track.
	 */
	virtual void SerializeKey(KeyType& key, XmlNodeRef& keyNode, bool bLoading) = 0;

protected:
	//! Sort keys in track (after time of keys was modified).
	virtual void DoSortKeys();

protected:
	typedef std::vector<KeyType> Keys;
	Keys m_keys;
};

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline void TSequencerTrack<KeyType >::RemoveKey(i32 index)
{
	assert(index >= 0 && index < (i32)m_keys.size());
	m_keys.erase(m_keys.begin() + index);
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline void TSequencerTrack<KeyType >::GetKey(i32 index, CSequencerKey* key) const
{
	assert(index >= 0 && index < (i32)m_keys.size());
	assert(key != 0);
	*(KeyType*)key = m_keys[index];
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline const CSequencerKey* TSequencerTrack<KeyType >::GetKey(i32 index) const
{
	assert(index >= 0 && index < (i32)m_keys.size());
	return &m_keys[index];
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline CSequencerKey* TSequencerTrack<KeyType >::GetKey(i32 index)
{
	assert(index >= 0 && index < (i32)m_keys.size());
	return &m_keys[index];
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline void TSequencerTrack<KeyType >::SetKey(i32 index, CSequencerKey* key)
{
	assert(index >= 0 && index < (i32)m_keys.size());
	assert(key != 0);
	m_keys[index] = *(KeyType*)key;
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline void TSequencerTrack<KeyType >::DoSortKeys()
{
	std::sort(m_keys.begin(), m_keys.end());
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline bool TSequencerTrack<KeyType >::Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks)
{
	if (bLoading)
	{
		i32 num = xmlNode->getChildCount();

		Range timeRange;
		i32 flags = GetFlags();
		xmlNode->getAttr("Flags", flags);
		xmlNode->getAttr("StartTime", timeRange.start);
		xmlNode->getAttr("EndTime", timeRange.end);
		SetFlags(flags);
		SetTimeRange(timeRange);

		SetNumKeys(num);
		for (i32 i = 0; i < num; i++)
		{
			XmlNodeRef keyNode = xmlNode->getChild(i);
			keyNode->getAttr("time", m_keys[i].m_time);

			SerializeKey(m_keys[i], keyNode, bLoading);
		}

		if ((!num) && (!bLoadEmptyTracks))
			return false;
	}
	else
	{
		i32 num = GetNumKeys();
		MakeValid();
		xmlNode->setAttr("Flags", GetFlags());
		xmlNode->setAttr("StartTime", GetTimeRange().start);
		xmlNode->setAttr("EndTime", GetTimeRange().end);

		for (i32 i = 0; i < num; i++)
		{
			XmlNodeRef keyNode = xmlNode->newChild("Key");
			keyNode->setAttr("time", m_keys[i].m_time);

			SerializeKey(m_keys[i], keyNode, bLoading);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline bool TSequencerTrack<KeyType >::SerializeSelection(XmlNodeRef& xmlNode, bool bLoading, bool bCopySelected, float fTimeOffset)
{
	if (bLoading)
	{
		i32 numCur = GetNumKeys();
		i32 num = xmlNode->getChildCount();

		i32 type;
		xmlNode->getAttr("TrackType", type);

		SetNumKeys(num + numCur);
		for (i32 i = 0; i < num; i++)
		{
			XmlNodeRef keyNode = xmlNode->getChild(i);
			keyNode->getAttr("time", m_keys[i + numCur].m_time);
			m_keys[i + numCur].m_time += fTimeOffset;

			SerializeKey(m_keys[i + numCur], keyNode, bLoading);
			if (bCopySelected)
			{
				m_keys[i + numCur].flags |= AKEY_SELECTED;
			}
		}
		SortKeys();
	}
	else
	{
		i32 num = GetNumKeys();
		//CheckValid();

		for (i32 i = 0; i < num; i++)
		{
			if (!bCopySelected || GetKeyFlags(i) & AKEY_SELECTED)
			{
				XmlNodeRef keyNode = xmlNode->newChild("Key");
				keyNode->setAttr("time", m_keys[i].m_time + fTimeOffset);

				SerializeKey(m_keys[i], keyNode, bLoading);
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline i32 TSequencerTrack<KeyType >::CreateKey(float time)
{
	KeyType key, akey;
	i32 nkey = GetNumKeys();
	SetNumKeys(nkey + 1);
	key.m_time = time;
	SetKey(nkey, &key);

	return nkey;
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline i32 TSequencerTrack<KeyType >::CloneKey(i32 fromKey)
{
	KeyType key;
	GetKey(fromKey, &key);
	i32 nkey = GetNumKeys();
	SetNumKeys(nkey + 1);
	SetKey(nkey, &key);
	return nkey;
}

//////////////////////////////////////////////////////////////////////////
template<class KeyType>
inline i32 TSequencerTrack<KeyType >::CopyKey(CSequencerTrack* pFromTrack, i32 nFromKey)
{
	KeyType key;
	pFromTrack->GetKey(nFromKey, &key);
	i32 nkey = GetNumKeys();
	SetNumKeys(nkey + 1);
	SetKey(nkey, &key);
	return nkey;
}

#endif // __SEQUENCER_TRACK_h__

