// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __I_DRX_MANNEQUIN_TAG_DEFS_H__
#define __I_DRX_MANNEQUIN_TAG_DEFS_H__

#ifdef DRXACTION_EXPORTS
	#define DRXMANNEQUIN_API DLL_EXPORT
#else
	#define DRXMANNEQUIN_API DLL_IMPORT
#endif

static u32k TAGSTATE_MAX_BYTES = 12;

enum ETagStateFull
{
	TAG_STATE_FULL = 0xff
};

enum ETagStateEmpty
{
	TAG_STATE_EMPTY = 0x00
};

struct STagMask
{
	STagMask()
		: byte(0)
		, mask(0)
	{
	}

	u8 byte;
	u8 mask;
};

struct STagStateBase
{
	STagStateBase(u8* memory, u32 size)
		: state(memory)
		, length(size)
	{
	}

	STagStateBase& operator=(const STagStateBase& source)
	{
		DRX_ASSERT(source.length <= length);

		memcpy(state, source.state, source.length);

		return *this;
	}

	bool operator==(const STagStateBase& comp) const
	{
		for (u32 i = 0; i < length; i++)
		{
			if (state[i] != comp.state[i])
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const STagStateBase& comp) const
	{
		for (u32 i = 0; i < length; i++)
		{
			if (state[i] != comp.state[i])
			{
				return true;
			}
		}
		return false;
	}

	STagStateBase& operator&=(const STagStateBase& comp)
	{
		for (u32 i = 0; i < length; i++)
		{
			state[i] &= comp.state[i];
		}
		return *this;
	}

	STagStateBase& operator|=(const STagStateBase& comp)
	{
		for (u32 i = 0; i < length; i++)
		{
			state[i] |= comp.state[i];
		}
		return *this;
	}

	u8 operator&(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < length);

		return (state[mask.byte] & mask.mask);
	}

	u8 operator|(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < length);

		return (state[mask.byte] | mask.mask);
	}

	ILINE bool IsNull() const
	{
		return (state == NULL);
	}

	ILINE void Set(const STagStateBase& source)
	{
		DRX_ASSERT(source.length <= length);

		memcpy(state, source.state, source.length);
	}

	ILINE void Clear()
	{
		memset(state, 0, length);
	}

	ILINE void Set(const STagMask& mask, bool set)
	{
		DRX_ASSERT(mask.byte < length);

		if (set)
		{
			state[mask.byte] |= mask.mask;
		}
		else
		{
			state[mask.byte] &= ~mask.mask;
		}
	}

	ILINE bool Contains(const STagStateBase& data, const STagStateBase& mask) const
	{
		DRX_ASSERT(length >= data.length);
		DRX_ASSERT(mask.length >= data.length);

		bool ret = true;

		u8k* statePtr = state;
		u8k* maskPtr = mask.state;
		u8k* dataPtr = data.state;
		u8k* endPtr = statePtr + data.length;

		for (; statePtr != endPtr; statePtr++, maskPtr++, dataPtr++)
		{
			ret = ret && ((*statePtr & *maskPtr) == *dataPtr);
		}

		return ret;
	}

	ILINE bool Contains(const STagStateBase& data, const STagStateBase& mask, u32k compLength) const
	{
		DRX_ASSERT(compLength <= length);
		DRX_ASSERT(compLength <= data.length);
		DRX_ASSERT(compLength <= data.length);

		bool ret = true;

		u8k* statePtr = state;
		u8k* maskPtr = mask.state;
		u8k* dataPtr = data.state;
		u8k* endPtr = statePtr + compLength;

		for (; statePtr != endPtr; statePtr++, maskPtr++, dataPtr++)
		{
			ret = ret && ((*statePtr & *maskPtr) == *dataPtr);
		}

		return ret;
	}

	void SetDifference(const STagStateBase& a, const STagStateBase& b)
	{
		DRX_ASSERT(length >= a.length);
		DRX_ASSERT(a.length == b.length);

		u8* statePtr = state;
		u8k* aStatePtr = a.state;
		u8k* bStatePtr = b.state;
		u8k* endPtr = statePtr + a.length;

		for (; statePtr != endPtr; statePtr++, aStatePtr++, bStatePtr++)
		{
			* statePtr = *aStatePtr & ~(*bStatePtr);
		}
	}

	ILINE bool IsSet(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < length);

		return (mask.mask != 0) && ((state[mask.byte] & mask.mask) == mask.mask);
	}

	ILINE bool AreAnySet(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < length);

		return (mask.mask != 0) && ((state[mask.byte] & mask.mask) != 0);
	}

	ILINE bool IsSet(const STagMask& groupMask, const STagMask& tagMask) const
	{
		DRX_ASSERT(groupMask.byte < length);
		DRX_ASSERT(groupMask.byte == tagMask.byte);

		return (tagMask.mask != 0) && ((state[groupMask.byte] & groupMask.mask) == tagMask.mask);
	}

	ILINE bool IsSame(const STagStateBase& a, const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < length);
		DRX_ASSERT(mask.byte < a.length);

		return (state[mask.byte] & mask.mask) == (a.state[mask.byte] & mask.mask);
	}

	u8* state;
	u32 length;
};

template<u32 NUM_BYTES>
struct STagState
{
	STagState()
	{
	}

	ILINE STagState(ETagStateFull pattern)
	{
		memset(state, TAG_STATE_FULL, NUM_BYTES);
	}
	ILINE STagState(ETagStateEmpty pattern)
	{
		memset(state, TAG_STATE_EMPTY, NUM_BYTES);
	}

	STagState(const STagStateBase& copyFrom)
	{
		DRX_ASSERT(copyFrom.length <= NUM_BYTES);
		memcpy(state, copyFrom.state, (std::min)(copyFrom.length, NUM_BYTES));
		memset(state + copyFrom.length, TAG_STATE_EMPTY, NUM_BYTES - copyFrom.length);
	}

	ILINE operator STagStateBase()
	{
		return STagStateBase(state, NUM_BYTES);
	}

	ILINE operator const STagStateBase() const
	{
		//--- Deliberate const case here, the structure as a whole is const
		return STagStateBase(const_cast<u8*>(state), NUM_BYTES);
	}

	template<typename T>
	void SetFromInteger(const T& value)
	{
		static_assert(sizeof(T) == NUM_BYTES, "Invalid type size!");

#ifdef NEED_ENDIAN_SWAP
		u8k* pIn = ((u8k*)&value) + NUM_BYTES;
		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			state[i] = *(--pIn);
		}
#else // (!NEED_ENDIAN_SWAP)
		memcpy(state, &value, NUM_BYTES);
#endif // (!NEED_ENDIAN_SWAP)
	}

	template<typename T>
	void GetToInteger(T& value) const
	{
		static_assert(sizeof(T) == NUM_BYTES, "Invalid type size!");
		GetToInteger(value, sizeof(T) * 8);
	}

	template<typename T>
	void GetToInteger(T& value, u32 numBits) const
	{
		DRX_ASSERT_MESSAGE(numBits <= sizeof(T) * 8, "value is not large enough to accommodate the number of bits stored in the TagState");
		u32 minSize = (sizeof(T) <= NUM_BYTES ? sizeof(T) : NUM_BYTES);
#ifdef NEED_ENDIAN_SWAP
		u8* pOut = ((u8*)&value) + sizeof(T);
		for (u32 i = 0; i < minSize; i++)
		{
			*(--pOut) = state[i];
		}
		if (sizeof(T) > NUM_BYTES)
		{
			memset(&value, 0, sizeof(T) - NUM_BYTES);
		}
#else // (!NEED_ENDIAN_SWAP)
		memcpy(&value, state, minSize);
		if (sizeof(T) > NUM_BYTES)
		{
			memset(((u8*)&value) + NUM_BYTES, 0, sizeof(T) - NUM_BYTES);
		}
#endif // (!NEED_ENDIAN_SWAP)
	}

	bool operator==(const STagState<NUM_BYTES>& comp) const
	{
		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			if (state[i] != comp.state[i])
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const STagState<NUM_BYTES>& comp) const
	{
		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			if (state[i] != comp.state[i])
			{
				return true;
			}
		}
		return false;
	}

	STagState<NUM_BYTES> operator&(const STagState<NUM_BYTES>& comp) const
	{
		STagState ret;

		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			ret.state[i] = state[i] & comp.state[i];
		}
		return ret;
	}

	STagState<NUM_BYTES> operator|(const STagState<NUM_BYTES>& comp) const
	{
		STagState ret;

		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			ret.state[i] = state[i] | comp.state[i];
		}
		return ret;
	}

	ILINE bool IsEmpty() const
	{
		bool ret = true;
		for (u32 i = 0; i < NUM_BYTES; i++)
		{
			ret = ret && (state[i] == 0);
		}

		return ret;
	}

	ILINE void Clear()
	{
		memset(state, 0, NUM_BYTES);
	}

	ILINE void Set(const STagMask& mask, bool set)
	{
		DRX_ASSERT(mask.byte < NUM_BYTES);

		if (set)
		{
			state[mask.byte] |= mask.mask;
		}
		else
		{
			state[mask.byte] &= ~mask.mask;
		}
	}

	ILINE bool IsSet(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < NUM_BYTES);

		return ((state[mask.byte] & mask.mask) == mask.mask);
	}

	ILINE bool AreAnySet(const STagMask& mask) const
	{
		DRX_ASSERT(mask.byte < NUM_BYTES);

		return (state[mask.byte] & mask.mask) != 0;
	}

	ILINE bool IsSet(const STagMask& groupMask, const STagMask& tagMask) const
	{
		DRX_ASSERT(groupMask.byte < NUM_BYTES);
		DRX_ASSERT(groupMask.byte == tagMask.byte);

		return (state[groupMask.byte] & groupMask.mask) == tagMask.mask;
	}

	void Serialize(TSerialize serialize)
	{
		for (i32 i = 0; i < NUM_BYTES; ++i)
		{
			serialize.Value("stateBytes", state[i], 'u8');
		}
	}

private:

	u8 state[NUM_BYTES];
};

typedef STagState<TAGSTATE_MAX_BYTES> TagState;

enum ETagDefinitionFlags
{
	eTDF_Tags = 1,
};

class CTagDefinition
{
public:

	struct SPriorityCount
	{
		i32 priority;
		i32 count;
	};

	CTagDefinition()
		: m_hasMasks(false)
	{
	}

	// copy constructor used in TagDef editor
	CTagDefinition(const CTagDefinition& rhs)
		: m_filename(rhs.m_filename)
		, m_tags(rhs.m_tags)
		, m_tagGroups(rhs.m_tagGroups)
		, m_defData(rhs.m_defData)
		, m_priorityTallies(rhs.m_priorityTallies)
		, m_hasMasks(rhs.m_hasMasks)
	{
	}

	CTagDefinition& operator=(const CTagDefinition& rhs)
	{
		m_filename = rhs.m_filename;
		m_tags = rhs.m_tags;
		m_tagGroups = rhs.m_tagGroups;
		m_defData = rhs.m_defData;
		m_priorityTallies = rhs.m_priorityTallies;
		m_hasMasks = rhs.m_hasMasks;

		return *this;
	}

	explicit CTagDefinition(tukk filename)
		: m_hasMasks(false)
	{
		SetFilename(filename);
	}

	struct STagDefData
	{
		DynArray<STagMask> tagMasks;
		DynArray<STagMask> groupMasks;
		u32             numBits;

		ILINE u32       GetNumBytes() const
		{
			return (numBits + 7) >> 3;
		}

		ILINE TagState GenerateMask(const STagStateBase& tagState) const
		{
			TagState ret;
			STagStateBase editRet(ret);
			editRet = tagState;
			const TagGroupID numGroups = (TagGroupID)groupMasks.size();
			for (TagGroupID i = 0; i < numGroups; i++)
			{
				const STagMask& groupMask = groupMasks[i];
				if (groupMask.mask)
				{
					editRet.Set(groupMask, tagState.AreAnySet(groupMask));
				}
			}

			return ret;
		}

		ILINE bool Contains(const STagStateBase& compParent, const STagStateBase& compChild, const STagStateBase& comparisonMask) const
		{
			return compParent.Contains(compChild, comparisonMask, GetNumBytes());
		}

		ILINE bool Contains(const STagStateBase& compParent, const STagStateBase& compChild) const
		{
			if (!compParent.Contains(compChild, compChild, GetNumBytes()))
			{
				//--- Trivial rejection
				return false;
			}
			else
			{
				TagState comparisonMask = GenerateMask(compChild);

				return compParent.Contains(compChild, comparisonMask, GetNumBytes());
			}
		}
	};

	struct STagGroup
	{
		STagGroup(tukk szGroupName)
		{
			m_name.SetByString(szGroupName);
		}

		STagRef m_name;
	};

	struct STag
	{
		STag(tukk szTag, u32 priority, i32 groupID)
		{
			m_name.SetByString(szTag);
			m_priority = priority;
			m_groupID = groupID;
			m_pTagDefinition = NULL;
		}

		u32                m_priority;
		TagGroupID            m_groupID;
		const CTagDefinition* m_pTagDefinition;
		STagRef               m_name;
	};

	ILINE bool IsValidTagID(TagID tagID) const
	{
		return (tagID >= 0) && (tagID < (TagID)m_tags.size());
	}

	ILINE bool IsValidTagGroupID(TagGroupID groupID) const
	{
		return (groupID >= 0) && (groupID < (TagGroupID)m_tagGroups.size());
	}

	TagID GetNum() const
	{
		return (TagID)m_tags.size();
	}

	u32 GetPriority(TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return 0;

		return m_tags[tagID].m_priority;
	}

	void SetPriority(TagID tagID, u32 priority)
	{
		if (!IsValidTagID(tagID))
			return;

		m_tags[tagID].m_priority = priority;
	}

	void Clear()
	{
		m_tags.clear();
		m_tagGroups.clear();
	}

	ILINE bool HasMasks() const
	{
		return m_hasMasks;
	}

	const CTagDefinition* GetSubTagDefinition(TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return NULL;

		return m_tags[tagID].m_pTagDefinition;
	}

	void SetSubTagDefinition(TagID tagID, const CTagDefinition* pTagDef)
	{
		if (IsValidTagID(tagID))
		{
			m_tags[tagID].m_pTagDefinition = pTagDef;
		}
	}

	TagID AddTag(tukk szTag, tukk szGroup = NULL, u32 priority = 0)
	{
		const bool tagExistsAlready = (Find(szTag) != TAG_ID_INVALID);
		if (tagExistsAlready)
			return TAG_ID_INVALID;

		TagGroupID groupID = GROUP_ID_NONE;
		if (szGroup)
			groupID = AddGroup(szGroup);

		m_tags.push_back(STag(szTag, priority, groupID));

		return (TagID)m_tags.size() - 1;
	}

	void RemoveTag(TagID tagID)
	{
		m_tags.erase(m_tags.begin() + tagID);
	}

	TagGroupID AddGroup(tukk szGroup)
	{
		TagGroupID groupID = FindGroup(szGroup);

		if (groupID == GROUP_ID_NONE)
		{
			groupID = m_tagGroups.size();
			m_tagGroups.push_back(STagGroup(szGroup));
		}

		return groupID;
	}

	void RemoveGroup(TagGroupID groupID)
	{
		m_tagGroups.erase(m_tagGroups.begin() + groupID);

		// Adjust GroupID reference in the tags
		DynArray<STag>::iterator itEnd = m_tags.end();
		for (DynArray<STag>::iterator it = m_tags.begin(); it != itEnd; ++it)
		{
			STag& tag = *it;
			if (tag.m_groupID == groupID)
				tag.m_groupID = GROUP_ID_NONE;
			else if (tag.m_groupID > groupID)
				tag.m_groupID--;
		}
	}

	bool AssignBits()
	{
		const bool allAssigned = AssignBits(m_defData);
		CalculatePriorityTallies();

		m_hasMasks = true;

		return allAssigned;
	}

	//-----------------------------------------------------------------------------
	// Assign bits
	//-----------------------------------------------------------------------------
	// Assigns bits for the tags & groups.
	// - Shares memory between groups and ensures that the groups do not straddle byte boundaries for simple masking.
	// - [optional] Pass in a usedTags mask to filter which tags are assigned bits
	//   (this requires internal tags to be setup first!)
	// - Returns false if not all tags could be assigned because of size constraints, true otherwise
	//-----------------------------------------------------------------------------
	bool AssignBits(STagDefData& tagDefData, const STagStateBase& usedTags = STagStateBase(0, 0)) const
	{
		DynArray<STagMask>& tagMasks = tagDefData.tagMasks;
		DynArray<STagMask>& groupMasks = tagDefData.groupMasks;

		const TagID numTags = (TagID)m_tags.size();
		const TagGroupID numGroups = (TagGroupID)m_tagGroups.size();

		tagMasks.resize(numTags, STagMask());
		groupMasks.resize(numGroups, STagMask());

		TagID tagsMapped = 0;
		TagID totalTagsToMap = numTags;
		u32 totalBits = 0;

		for (TagID i = 0; (i < numTags); i++)
		{
			tagMasks[i].mask = 0;
		}
		for (TagGroupID i = 0; (i < numGroups); i++)
		{
			groupMasks[i].mask = 0;
		}

		if (!usedTags.IsNull())
		{
			totalTagsToMap = 0;

			for (TagID i = 0; (i < numTags); i++)
			{
				if (usedTags.IsSet(m_defData.tagMasks[i]))
				{
					totalTagsToMap++;
				}
			}
		}

		for (u32 curByte = 0; (curByte < TAGSTATE_MAX_BYTES) && (tagsMapped < totalTagsToMap); curByte++)
		{
			u8 curBit = 0;

			for (TagGroupID g = 0; (g < numGroups) && (curBit < 8); g++)
			{
				u32 numTagsInGroup = 0;

				if ((groupMasks[g].mask == 0)
				    && (usedTags.IsNull() || usedTags.AreAnySet(m_defData.groupMasks[g])))
				{
					for (TagID i = 0; i < numTags; i++)
					{
						const STag& tag = m_tags[i];
						if (tag.m_groupID == g)
						{
							numTagsInGroup++;
						}
					}

					u32 numBits = 32 - countLeadingZeros32(numTagsInGroup);
					if (numBits <= (u32)(8 - curBit))
					{
						groupMasks[g].byte = curByte;
						groupMasks[g].mask = 0;

						numTagsInGroup = 0;
						for (TagID i = 0; i < numTags; i++)
						{
							const STag& tag = m_tags[i];
							if (tag.m_groupID == g)
							{
								numTagsInGroup++;
								tagMasks[i].byte = curByte;
								tagMasks[i].mask = numTagsInGroup << curBit;
							}
						}

						for (u32 i = 0, bit = 1 << curBit; i < numBits; i++, bit <<= 1)
						{
							groupMasks[g].mask |= bit;
						}
						curBit += numBits;
						tagsMapped += numTagsInGroup;
					}
				}
			}

			for (TagID i = 0; (i < numTags) && (curBit < 8); i++)
			{
				const STag& tag = m_tags[i];

				if ((tagMasks[i].mask == 0)
				    && (tag.m_groupID == GROUP_ID_NONE)
				    && (usedTags.IsNull() || usedTags.IsSet(m_defData.tagMasks[i])))
				{
					tagMasks[i].byte = curByte;
					tagMasks[i].mask = (1 << curBit);
					curBit++;
					tagsMapped++;
				}
			}

			if (tagsMapped < totalTagsToMap)
				totalBits += 8;
			else
				totalBits += curBit;
		}

		tagDefData.numBits = totalBits;

		const bool allAssigned = totalTagsToMap == tagsMapped;
		return allAssigned;
	}

	//---------------------------------------------------------------------------------------------------------------
	// Maps the original tag state via the modified tag def (allows remapping of indices when a tag is deleted)
	//---------------------------------------------------------------------------------------------------------------
	bool MapTagState(const STagStateBase& originalTags, STagStateBase mappedTags, CTagDefinition& modifiedTagDef) const
	{
		u32k numTags = m_tags.size();
		bool allMapped = true;
		mappedTags.Clear();

		for (u32 tagIndex = 0; tagIndex < numTags; ++tagIndex)
		{
			if (IsSet(originalTags, tagIndex))
			{
				u32 mappedTagID = modifiedTagDef.Find(GetTagCRC(tagIndex));
				if (mappedTagID != TAG_ID_INVALID)
				{
					if (tagIndex != mappedTagID)
					{
						//DrxLog("[TAGDEF]: MapTagState() (%s): mapped tag %i [%s] to %i [%s]", m_filename, tagIndex, GetTagName(tagIndex), mappedTagID, modifiedTagDef.GetTagName(mappedTagID));
					}
					modifiedTagDef.Set(mappedTags, mappedTagID, true);
				}
				else
				{
					//DrxLog("[TAGDEF]: MapTagState() (%s): Cannot map tag %i [%s] - will be erased", m_filename, tagIndex, GetTagName(tagIndex));
					allMapped = false;
				}
			}
		}

		return allMapped;
	}

	//---------------------------------------------------------------------------------------------------------------
	// Checks to see if a given tag mask can be represented by an input set of tag definition data
	//---------------------------------------------------------------------------------------------------------------
	bool CanRepresent(const STagStateBase& sourceTags, const STagDefData& data) const
	{
		u32k numTags = m_tags.size();

		for (u32 i = 0; i < numTags; i++)
		{
			if (IsSet(sourceTags, i) && (data.tagMasks[i].mask == 0))
			{
				return false;
			}
		}

		return true;
	}

	//---------------------------------------------------------------------------------------------------------------
	// Converts a tag state from the default format to the format specified by the input STagDefData, dropping
	// unmapped tags and shrinking down the memory footprint
	//---------------------------------------------------------------------------------------------------------------
	void CompressTagState(STagStateBase targetTags, const STagStateBase& sourceTags, const STagDefData& data) const
	{
		u32k numTags = m_tags.size();
		targetTags.Clear();

		if (!data.tagMasks.empty())
		{
			const STagMask* pTagMaskDest = &data.tagMasks[0];
			for (u32 i = 0; i < numTags; i++)
			{
				if (pTagMaskDest->mask != 0)
				{
					TagGroupID groupID = m_tags[i].m_groupID;
					if (groupID != GROUP_ID_NONE)
					{
						if (sourceTags.IsSet(m_defData.groupMasks[groupID], m_defData.tagMasks[i]))
						{
							targetTags.Set(*pTagMaskDest, true);
						}
					}
					else
					{
						if (sourceTags.IsSet(m_defData.tagMasks[i]))
						{
							targetTags.Set(*pTagMaskDest, true);
						}
					}
				}

				pTagMaskDest++;
			}
		}
	}

	//---------------------------------------------------------------------------------------------------------------
	// Converts a tag state from the format specified by the input STagDefData to the default format,
	// expanding the memory footprint to the full uncompressed size
	//---------------------------------------------------------------------------------------------------------------
	void UncompressTagState(STagStateBase targetTags, const STagStateBase& sourceTags, const STagDefData& data) const
	{
		u32k numTags = m_tags.size();
		targetTags.Clear();

		if (!data.tagMasks.empty())
		{
			const STagMask* pTagMaskDest = &data.tagMasks[0];
			for (u32 i = 0; i < numTags; i++)
			{
				if (pTagMaskDest->mask != 0)
				{
					TagGroupID groupID = m_tags[i].m_groupID;
					if (groupID != GROUP_ID_NONE)
					{
						if (sourceTags.IsSet(data.groupMasks[groupID], *pTagMaskDest))
						{
							targetTags.Set(m_defData.tagMasks[i], true);
						}
					}
					else
					{
						if (sourceTags.IsSet(*pTagMaskDest))
						{
							targetTags.Set(m_defData.tagMasks[i], true);
						}
					}
				}

				pTagMaskDest++;
			}
		}
	}

	TagGroupID GetGroupID(TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return GROUP_ID_NONE;

		return m_tags[tagID].m_groupID;
	}

	TagGroupID GetNumGroups() const
	{
		return (TagGroupID)m_tagGroups.size();
	}

	u32 GetGroupCRC(TagGroupID groupID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return 0;

		return m_tagGroups[groupID].m_name.crc;
	}

#if STORE_TAG_STRINGS

	tukk GetGroupName(TagGroupID groupID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return "<invalid>";

		return m_tagGroups[groupID].m_name.c_str();
	}

	void SetGroupName(TagGroupID groupID, tukk szGroup)
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return;

		m_tagGroups[groupID].m_name.SetByString(szGroup);
	}

#endif //STORE_TAG_STRINGS

	TagID Find(tukk szTag) const
	{
		if (szTag)
		{
			u32 crc = MannGenCRC(szTag);

			return Find(crc);
		}
		else
		{
			return TAG_ID_INVALID;
		}
	}

	TagID Find(u32 crc) const
	{
		// TODO: Use hash/map
		TagID numTags = (TagID)m_tags.size();
		for (TagID i = 0; i < numTags; i++)
		{
			if (m_tags[i].m_name.crc == crc)
			{
				return i;
			}
		}

		return TAG_ID_INVALID;
	}

	TagGroupID FindGroup(tukk szTag) const
	{
		u32 crc = MannGenCRC(szTag);

		return FindGroup(crc);
	}

	TagGroupID FindGroup(u32 crc) const
	{
		// TODO: Use hash/map
		TagGroupID numGroups = (TagGroupID)m_tagGroups.size();
		for (TagGroupID i = 0; i < numGroups; i++)
		{
			if (m_tagGroups[i].m_name.crc == crc)
			{
				return i;
			}
		}

		return GROUP_ID_NONE;
	}

	void SetFilename(tukk filename)
	{
		m_filename = filename;
	}
	tukk GetFilename() const
	{
		return m_filename.c_str();
	}

	u32 GetTagCRC(TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return 0;

		return m_tags[tagID].m_name.crc;
	}

	tukk GetTagName(TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return "<invalid>";

		return m_tags[tagID].m_name.c_str();
	}

	void SetTagName(TagID tagID, tukk szTag)
	{
		if (!IsValidTagID(tagID))
			return;

		m_tags[tagID].m_name.SetByString(szTag);
	}

	void SetTagGroup(TagID tagID, TagGroupID groupID)
	{
		if (!IsValidTagID(tagID))
			return;

		m_tags[tagID].m_groupID = groupID;
	}

	template<typename T>
	bool TagListToIntegerFlags(tukk tagList, T& tagStateInteger) const
	{
		STagState<sizeof(T)> tagState;

		bool ret = TagListToFlags(tagList, tagState);

		tagState.GetToInteger(tagStateInteger);

		return ret;
	}

	bool TagListToFlags(tukk tagList, STagStateBase tagState, bool append = false, bool verbose = true) const
	{
		bool success = true;
		if (!append)
		{
			tagState.Clear();
		}

		if (tagList && (tagList[0] != '\0'))
		{
			tukk cur = tagList;
			tukk nxt = strstr(cur, "+");
			char tagBuffer[128];
			while (nxt)
			{
				drx_strcpy(tagBuffer, cur, (size_t)(nxt - cur));
				i32 tag = Find(tagBuffer);
				if (tag < 0)
				{
					if (verbose)
					{
						DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "[TagListToFlags] Invalid tag '%s'", tagBuffer);
					}
					success = false;
				}
				else
				{
					tagState.Set(m_defData.tagMasks[tag], true);
				}

				cur = nxt + 1;
				nxt = strstr(nxt + 1, "+");
			}

			drx_strcpy(tagBuffer, cur);
			TagID tag = Find(tagBuffer);
			if (tag == TAG_ID_INVALID)
			{
				if (verbose)
				{
					DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "[TagListToFlags] Invalid tag '%s'", tagBuffer);
				}
				success = false;
			}
			else
			{
				tagState.Set(m_defData.tagMasks[tag], true);
			}
		}

		return success;
	}

	template<typename T>
	bool IsGroupSet(const T& state, const TagGroupID groupID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return false;

		return state.AreAnySet(m_defData.groupMasks[groupID]);
	}

	TagID GetTagInGroup(const STagStateBase& state, const TagGroupID& groupID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return TAG_ID_INVALID;

		u8k groupMask = state & m_defData.groupMasks[groupID];
		const TagID numTags = m_tags.size();
		for (TagID itTags = 0; itTags < numTags; ++itTags)
		{
			const STag& tag = m_tags[itTags];
			const STagMask& tagMask = m_defData.tagMasks[itTags];
			if ((tag.m_groupID == groupID) && (groupMask == tagMask.mask))
			{
				return itTags;
			}
		}
		return TAG_ID_INVALID;
	}

	void SetGroup(STagStateBase state, const TagGroupID groupID, const TagID tagID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return;

		const STagMask& groupMask = m_defData.groupMasks[groupID];
		state.Set(groupMask, false);

		if (IsValidTagID(tagID))
		{
			const STag& tag = m_tags[tagID];
			DRX_ASSERT(tag.m_groupID == groupID);
			if (tag.m_groupID != groupID)
				return;

			state.Set(m_defData.tagMasks[tagID], true);
		}
	}

	template<typename T>
	void ClearGroup(T& state, const TagGroupID groupID) const
	{
		DRX_ASSERT(IsValidTagGroupID(groupID));
		if (!IsValidTagGroupID(groupID))
			return;

		state.Set(m_defData.groupMasks[groupID], false);
	}

	template<typename T>
	bool IsSet(const T& state, const TagID tagID) const
	{
		if (!IsValidTagID(tagID))
			return false;

		return IsSetInternal(state, tagID);
	}

	template<typename T>
	void Set(T& state, const TagID tagID, bool set) const
	{
		if (!IsValidTagID(tagID))
			return;

		const STag& tag = m_tags[tagID];
		const STagMask& tagMask = m_defData.tagMasks[tagID];

		if (tag.m_groupID != GROUP_ID_NONE)
		{
			const STagMask& groupMask = m_defData.groupMasks[tag.m_groupID];

			const bool isSet = state.IsSet(groupMask, tagMask);

			if (isSet != set)
			{
				state.Set(groupMask, false);

				if (set)
				{
					state.Set(tagMask, true);
				}
			}
		}
		else
		{
			state.Set(tagMask, set);
		}
	}

	//-----------------------------------------------------------------
	// Returns a tagState which has set bits for all the tags
	// that are also set in the passed in tagDef
	//-----------------------------------------------------------------
	TagState GetSharedTags(const CTagDefinition& tagDef) const
	{
		const TagGroupID numGroups = (TagGroupID)m_tagGroups.size();
		TagState ret;
		ret.Clear();
		for (TagGroupID i = 0; i < numGroups; i++)
		{
			if (tagDef.FindGroup(m_tagGroups[i].m_name.crc) != GROUP_ID_NONE)
			{
				ret.Set(m_defData.groupMasks[i], true);
			}
		}

		u32k numTags = GetNum();
		for (u32 i = 0; i < numTags; i++)
		{
			if ((m_tags[i].m_groupID == GROUP_ID_NONE) && (tagDef.Find(m_tags[i].m_name.crc) != TAG_ID_INVALID))
			{
				ret.Set(m_defData.tagMasks[i], true);
			}
		}

		return ret;
	}

	// Returns the union of the tags in a and b.
	// When a and b contain different tags within the same tag group,
	// b gets precedence.
	TagState GetUnion(const STagStateBase& a, const STagStateBase& b) const
	{
		TagState ret = a;
		STagStateBase retBase = ret;

		const TagGroupID numGroups = (TagGroupID)m_tagGroups.size();
		for (TagGroupID i = 0; i < numGroups; i++)
		{
			const STagMask& groupMask = m_defData.groupMasks[i];
			const bool aSet = a.AreAnySet(groupMask);
			const bool bSet = b.AreAnySet(groupMask);
			if (aSet && bSet)
			{
				retBase.Set(groupMask, false);
			}
		}

		retBase |= b;

		return ret;
	}

	TagState GetIntersection(const STagStateBase& a, const STagStateBase& b) const
	{
		TagState ret = a;
		STagStateBase retBase = ret;

		const TagGroupID numGroups = (TagGroupID)m_tagGroups.size();
		for (TagGroupID i = 0; i < numGroups; i++)
		{
			const STagMask& groupMask = m_defData.groupMasks[i];
			if ((a & groupMask) != (b & groupMask))
				retBase.Set(groupMask, false);
		}

		retBase &= b;

		return ret;
	}

	// Returns a - b (all tags set in a which are not set in b)
	//
	// Set wipeOverlappedGroups to clear any groups that have a tag set in b.
	TagState GetDifference(const STagStateBase& a, const STagStateBase& b, const bool wipeOverlappedGroups = false) const
	{
		TagState clearMaskB;
		if (wipeOverlappedGroups)
		{
			clearMaskB = m_defData.GenerateMask(b);
		}
		else
		{
			const TagGroupID numGroups = (TagGroupID)m_tagGroups.size();
			clearMaskB = b;
			for (TagGroupID i = 0; i < numGroups; i++)
			{
				const STagMask& groupMask = m_defData.groupMasks[i];
				if (!a.IsSame(b, groupMask))
				{
					clearMaskB.Set(groupMask, false);
				}
			}
		}

		TagState ret;
		STagStateBase retBase = ret;
		retBase.SetDifference(a, clearMaskB);

		return ret;
	}

	ILINE TagState GenerateMask(const STagStateBase& tagState, const STagDefData& defData) const
	{
		return defData.GenerateMask(tagState);
	}

	ILINE TagState GenerateMask(const STagStateBase& tagState) const
	{
		return m_defData.GenerateMask(tagState);
	}

	ILINE bool Contains(const STagStateBase& compParent, const STagStateBase& compChild, const STagStateBase& comparisonMask) const
	{
		return compParent.Contains(compChild, comparisonMask);
	}

	ILINE bool Contains(const STagStateBase& compParent, const STagStateBase& compChild) const
	{
		if (!compParent.Contains(compChild, compChild))
		{
			//--- Trivial rejection
			return false;
		}
		else
		{
			TagState comparisonMask = GenerateMask(compChild);

			return compParent.Contains(compChild, comparisonMask);//((compParent & comparisonMask) == compChild);
		}
	}

	ILINE bool Contains(const STagStateBase& compParent, const STagStateBase& compChild, const STagDefData& defData) const
	{
		if (!compParent.Contains(compChild, compChild))
		{
			//--- Trivial rejection
			return false;
		}
		else
		{
			TagState comparisonMask = GenerateMask(compChild, defData);

			return compParent.Contains(compChild, comparisonMask);
		}
	}

	u32 RateTagState(const STagStateBase& tagState, const DynArray<SPriorityCount>* pPriorityTallies = NULL) const
	{
		if (pPriorityTallies == NULL)
		{
			pPriorityTallies = &m_priorityTallies;
		}
		u32k numPriorities = pPriorityTallies->size();

		u32 score = 0;
		u32k numTags = GetNum();
		for (u32 i = 0; i < numTags; i++)
		{
			if (IsSetInternal(tagState, i))
			{
				i32 priority = m_tags[i].m_priority;
				u32 priorityTally = 1;
				for (u32 p = 0; (p < numPriorities); p++)
				{
					const SPriorityCount& priorityCount = m_priorityTallies[p];

					if (priority > priorityCount.priority)
					{
						u32 newTally = priorityTally * (priorityCount.count + 1);
						//						DRX_ASSERT_MESSAGE(newTally >= priorityTally, "TagState rating overflow - too many distinct priority levels!");
						priorityTally = newTally;
					}
					else
					{
						break;
					}
				}

				score += priorityTally;
			}
		}

		return score;
	}

	template<typename T, size_t N>
	void IntegerFlagsToTagList(const T& tagStateInteger, DrxStackStringT<char, N>& tagList) const
	{
		STagState<sizeof(T)> tagState;
		tagState.SetFromInteger(tagStateInteger);

		FlagsToTagList(tagState, tagList);
	}

	template<size_t N>
	void FlagsToTagList(const STagStateBase& tagState, DrxStackStringT<char, N>& tagList) const
	{
		const TagID numTags = GetNum();
		bool isFirst = true;

		tagList.clear();
		for (TagID i = 0; i < numTags; i++)
		{
			if (m_defData.tagMasks[i].mask && IsSetInternal(tagState, i))
			{
				tukk tagName = GetTagName(i);
				if (!isFirst)
				{
					tagList += '+';
				}
				isFirst = false;
				tagList += tagName;
			}
		}
	}

	ILINE u32 GetNumBits() const
	{
		return m_defData.numBits;
	}

	ILINE u32 GetNumBytes() const
	{
		return m_defData.GetNumBytes();
	}

	ILINE bool HasTooManyBits() const
	{
		return m_defData.numBits > (sizeof(TagState) << 3);
	}

	ILINE const STagDefData& GetDefData() const
	{
		return m_defData;
	}

	void CombinePriorities(const CTagDefinition& tagDef, DynArray<SPriorityCount>& combinedPriorities) const
	{
		u32k numPriorities1 = m_priorityTallies.size();
		u32k numPriorities2 = tagDef.m_priorityTallies.size();

		std::set<i32> combPrioritySet;

		for (u32 i = 0; i < numPriorities1; i++)
		{
			combPrioritySet.insert(m_priorityTallies[i].priority);
		}
		for (u32 i = 0; i < numPriorities2; i++)
		{
			combPrioritySet.insert(tagDef.m_priorityTallies[i].priority);
		}

		u32k numCombinedPriorities = combPrioritySet.size();
		combinedPriorities.resize(numCombinedPriorities);
		std::set<i32>::iterator iter = combPrioritySet.begin();
		for (u32 i = 0; i < numCombinedPriorities; ++i, ++iter)
		{
			i32k priority = *iter;
			SPriorityCount& priorityCount = combinedPriorities[i];

			priorityCount.priority = priority;
			priorityCount.count = 0;

			for (u32 p = 0; p < numPriorities1; p++)
			{
				if (m_priorityTallies[p].priority == priority)
				{
					priorityCount.count += m_priorityTallies[p].count;
					break;
				}
			}
			for (u32 p = 0; p < numPriorities2; p++)
			{
				if (tagDef.m_priorityTallies[p].priority == priority)
				{
					priorityCount.count += tagDef.m_priorityTallies[p].count;
					break;
				}
			}
		}
	}

private:

	template<typename T>
	bool IsSetInternal(const T& state, const TagID tagID) const
	{
		const STag& tag = m_tags[tagID];
		if ((tag.m_groupID != GROUP_ID_NONE))
		{
			return state.IsSet(m_defData.groupMasks[tag.m_groupID], m_defData.tagMasks[tagID]);
		}
		else
		{
			return state.IsSet(m_defData.tagMasks[tagID]);
		}
	}

	void CalculatePriorityTallies()
	{
		std::set<i32> priorities;
		i32k numTags = m_tags.size();
		for (TagID tagID = 0; tagID < numTags; tagID++)
		{
			const STag& tag = m_tags[tagID];
			priorities.insert(tag.m_priority);
		}

		u32k numPriorities = priorities.size();
		m_priorityTallies.resize(numPriorities);
		std::set<i32>::iterator iter = priorities.begin();
		for (u32 i = 0; i < numPriorities; ++i, ++iter)
		{
			i32k priority = *iter;
			SPriorityCount& priorityCount = m_priorityTallies[i];

			priorityCount.priority = priority;
			priorityCount.count = 0;

			for (TagID tagID = 0; tagID < numTags; tagID++)
			{
				const STag& tag = m_tags[tagID];
				if (tag.m_priority == priority)
				{
					priorityCount.count++;
				}
			}
		}
	}

	string                   m_filename;
	DynArray<STag>           m_tags;
	DynArray<STagGroup>      m_tagGroups;
	STagDefData              m_defData;
	DynArray<SPriorityCount> m_priorityTallies;
	bool                     m_hasMasks;
};

struct SFragTagState
{
	explicit SFragTagState(const TagState& _globalTags = TagState(TAG_STATE_EMPTY), const TagState& _fragmentTags = TagState(TAG_STATE_EMPTY))
		:
		globalTags(_globalTags),
		fragmentTags(_fragmentTags)
	{
	}

	bool operator==(const SFragTagState& fragTagState) const
	{
		return (globalTags == fragTagState.globalTags) && (fragmentTags == fragTagState.fragmentTags);
	}
	bool operator!=(const SFragTagState& fragTagState) const
	{
		return (globalTags != fragTagState.globalTags) || (fragmentTags != fragTagState.fragmentTags);
	}

	TagState globalTags;
	TagState fragmentTags;
};

//! Helper to store the boolean state of tags for an action controller
//! \see IActionController::GetContext().state
class CTagState
{
public:
	CTagState(const CTagDefinition& defs, TagState state = TAG_STATE_EMPTY)
		: m_defs(defs)
		, m_state(state)
	{
#ifndef _RELEASE
		const size_t maxSupportedBits = sizeof(TagState) * 8;
		const size_t definitionBits = defs.GetNumBits();
		if (maxSupportedBits < definitionBits)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "!Number of bits required for tag definition '%s' (%" PRISIZE_T " bits) is greater than %" PRISIZE_T " bits. To fix this, group mutually exclusive tags together, remove unnecessary tags or increase the size of TagState.", defs.GetFilename(), definitionBits, maxSupportedBits);
		}
#endif
	}

	CTagState(const CTagState& other) 
		: m_defs(other.m_defs)
		, m_state(other.m_state)
	{
	}

	void Clear()
	{
		m_state = TAG_STATE_EMPTY;
	}

	void SetByCRC(u32 CRC, bool enable)
	{
		const TagID id = m_defs.Find(CRC);
		if (m_defs.IsValidTagID(id))
		{
			Set(id, enable);
		}
	}

	//! Sets a tag to the specified state
	//! \par Example
	//! \include DinrusXAnimation/Examples/SetTag.cpp
	void Set(TagID id, bool enable)
	{
		m_defs.Set(m_state, id, enable);
	}

	void SetGroup(TagGroupID groupID, TagID tagID)
	{
		m_defs.SetGroup(m_state, groupID, tagID);
	}

	void ClearGroup(TagGroupID groupID)
	{
		m_defs.ClearGroup(m_state, groupID);
	}

	ILINE bool IsSet(TagID tagID) const
	{
		return m_defs.IsSet(m_state, tagID);
	}

	ILINE bool IsGroupSet(TagGroupID tagGroupID) const
	{
		return m_defs.IsGroupSet(m_state, tagGroupID);
	}

	ILINE bool Contains(const TagState& comp) const
	{
		return m_defs.Contains(m_state, comp);
	}
	ILINE bool Contains(const CTagState& comp) const
	{
		return m_defs.Contains(m_state, comp.m_state);
	}

	const TagState& GetMask() const
	{
		return m_state;
	}

	const CTagDefinition& GetDef() const
	{
		return m_defs;
	}

	CTagState& operator=(const TagState& tagState)
	{
		m_state = tagState;
		return *this;
	}

	CTagState& operator=(const CTagState& cTagState)
	{
		m_state = cTagState.m_state;
		return *this;
	}

private:

	const CTagDefinition& m_defs;
	TagState              m_state;

};

//--- Just expose this??
template<typename T>
class TOptimisedTagSortedList
{
public:

	friend class CAnimationDatabaseUpr;
	friend class CAnimationDatabase;
	template<typename TFRIEND> friend class TTagSortedList;

	TOptimisedTagSortedList(u32 size, u32 stride)
		:
		m_pTagDef(NULL),
		m_pFragTagDef(NULL),
		m_pDefData(NULL),
		m_pFragDefData(NULL),
		m_size(size),
		m_keySize(stride)
	{
		m_keys = new u8[size * stride];
		m_values = new T[size];
	}

	~TOptimisedTagSortedList()
	{
		delete[] m_keys;
		delete[] m_values;
		delete m_pDefData;
	}

	ILINE u32 Size() const
	{
		return m_size;
	}
	ILINE u32 GetKeySize() const
	{
		return m_keySize;
	}

	void GetKey(SFragTagState& fragTagState, u32 idx) const
	{
		DRX_ASSERT(idx < m_size);

		if (idx < m_size)
		{
			u32k numBytesGlobal = m_pDefData->GetNumBytes();
			u32k numBytesFrag = m_pFragDefData ? m_pFragDefData->GetNumBytes() : 0;

			m_pTagDef->UncompressTagState(fragTagState.globalTags, STagStateBase(m_keys + (idx * m_keySize), numBytesGlobal), *m_pDefData);
			fragTagState.fragmentTags = STagStateBase(m_keys + (idx * m_keySize) + numBytesGlobal, numBytesFrag);
		}
	}

	const T* Get(u32 idx) const
	{
		u32k numEntries = m_size;
		DRX_ASSERT(idx < numEntries);

		if (idx < numEntries)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	i32 FindIdx(const SFragTagState& fragTags) const
	{
		u32k numEntries = m_size;
		u32k numBytesGlobal = m_pDefData->GetNumBytes();
		u32k numBytesFrag = m_pFragDefData ? m_pFragDefData->GetNumBytes() : 0;

		TagState compressedGlobalTags;
		m_pTagDef->CompressTagState(compressedGlobalTags, fragTags.globalTags, *m_pDefData);

		STagStateBase globalTags(m_keys, numBytesGlobal);
		STagStateBase fragmentTags(m_keys + numBytesGlobal, numBytesFrag);

		for (u32 i = 0; i < numEntries; i++)
		{
			if ((globalTags == compressedGlobalTags)
			    && (fragmentTags == fragTags.fragmentTags))
			{
				return i;
			}

			globalTags.state += m_keySize;
			fragmentTags.state += m_keySize;
		}

		return -1;
	}

	const T* Find(const SFragTagState& fragTags) const
	{
		i32 idx = FindIdx(fragTags);

		if (idx >= 0)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	T* Find(const SFragTagState& fragTags)
	{
		i32 idx = FindIdx(fragTags);

		if (idx >= 0)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	const T& GetDefault() const
	{
		u32k size = m_size;
		if (size == 0)
		{
			DrxFatalError("[TTagSortedList] No default in list, this should not happen");
		}

		return m_values[size - 1];
	}

	const T* GetBestMatch(const SFragTagState& fragTags, SFragTagState* pFragTagsMatched = NULL, u32* pTagSetIdx = NULL) const
	{
		u32k numEntries = m_size;
		u32k numBytesGlobal = m_pDefData->GetNumBytes();
		u32k numBytesFrag = m_pFragDefData ? m_pFragDefData->GetNumBytes() : NULL;

		TagState compressedGlobalTags;
		m_pTagDef->CompressTagState(compressedGlobalTags, fragTags.globalTags, *m_pDefData);

		STagStateBase globalTags(m_keys, numBytesGlobal);
		STagStateBase fragmentTags(m_keys + numBytesGlobal, numBytesFrag);

		for (u32 i = 0; i < numEntries; i++)
		{
			if (m_pDefData->Contains(compressedGlobalTags, globalTags)
			    && (!m_pFragDefData || m_pFragDefData->Contains(fragTags.fragmentTags, fragmentTags)))
			{
				if (pFragTagsMatched)
				{
					m_pTagDef->UncompressTagState(pFragTagsMatched->globalTags, globalTags, *m_pDefData);
					pFragTagsMatched->fragmentTags = fragmentTags;
				}
				if (pTagSetIdx)
				{
					*pTagSetIdx = i;
				}

				return &m_values[i];
			}

			globalTags.state += m_keySize;
			fragmentTags.state += m_keySize;
		}

		return NULL;
	}

	const T* GetBestMatch(const SFragTagState& fragTags, const STagStateBase& requiredTagState, SFragTagState* pFragTagsMatched = NULL, u32* pTagSetIdx = NULL) const
	{
		if (m_pTagDef->CanRepresent(requiredTagState, *m_pDefData))
		{
			u32k numEntries = m_size;
			u32k numBytesGlobal = m_pDefData->GetNumBytes();
			u32k numBytesFrag = m_pFragDefData ? m_pFragDefData->GetNumBytes() : 0;

			TagState compressedGlobalTags, compressedReqTags;
			m_pTagDef->CompressTagState(compressedGlobalTags, fragTags.globalTags, *m_pDefData);
			m_pTagDef->CompressTagState(compressedReqTags, requiredTagState, *m_pDefData);

			STagStateBase globalTags(m_keys, numBytesGlobal);
			STagStateBase fragmentTags(m_keys + numBytesGlobal, numBytesFrag);

			const TagState requiredComparisonMask = m_pDefData->GenerateMask(compressedReqTags);

			for (u32 i = 0; i < numEntries; i++)
			{
				if (m_pDefData->Contains(globalTags, compressedReqTags, requiredComparisonMask)
				    && m_pDefData->Contains(compressedGlobalTags, globalTags)
				    && (!m_pFragDefData || m_pFragDefData->Contains(fragTags.fragmentTags, fragmentTags)))
				{
					if (pFragTagsMatched)
					{
						m_pTagDef->UncompressTagState(pFragTagsMatched->globalTags, globalTags, *m_pDefData);
						pFragTagsMatched->fragmentTags = fragmentTags;
					}
					if (pTagSetIdx)
					{
						*pTagSetIdx = i;
					}

					return &m_values[i];
				}

				globalTags.state += m_keySize;
				fragmentTags.state += m_keySize;
			}
		}

		return NULL;
	}

	ILINE const CTagDefinition::STagDefData* GetDefData() const
	{
		return m_pDefData;
	}
	ILINE const CTagDefinition::STagDefData* GetFragDefData() const
	{
		return m_pFragDefData;
	}

private:

	const CTagDefinition*              m_pTagDef;
	const CTagDefinition*              m_pFragTagDef;
	const CTagDefinition::STagDefData* m_pDefData;
	const CTagDefinition::STagDefData* m_pFragDefData;

	u8*                             m_keys;
	T* m_values;
	u32                             m_size;
	u32                             m_keySize;
};

template<typename T>
class TTagSortedList
{
public:

	friend class CAnimationDatabaseUpr;
	friend class CAnimationDatabase;

	struct SSortStruct
	{
		u32 priority;
		u16 originalIdx;

		bool operator<(const SSortStruct& compare) const
		{
			return priority > compare.priority;
		}
	};

	TTagSortedList<T>()
	{
	}

	~TTagSortedList<T>()
	{
	}

	T& Insert(const SFragTagState& fragTags, const T& data)
	{
		u32k numEntries = m_keys.size();
		for (u32 i = 0; i < numEntries; i++)
		{
			if (m_keys[i] == fragTags)
			{
				m_values[i] = data;
				return m_values[i];
			}
		}

		Resize(numEntries + 1);
		m_keys[numEntries] = fragTags;
		m_values[numEntries] = data;
		return m_values[numEntries];
	}

	void Erase(u32 idx)
	{
		u32k numEntries = m_keys.size();
		DRX_ASSERT(idx < numEntries);

		if (idx < numEntries)
		{
			m_keys.erase(m_keys.begin() + idx);
			m_values.erase(m_values.begin() + idx);
		}
	}

	void Resize(u32k newSize)
	{
		if (newSize != m_keys.size())
		{
			m_keys.resize(newSize);
			m_values.resize(newSize);
		}
	}

	u32 Size() const
	{
		return m_keys.size();
	}

	const T* Get(u32 idx) const
	{
		u32k numEntries = m_keys.size();
		DRX_ASSERT(idx < numEntries);

		if (idx < numEntries)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	i32 FindIdx(const SFragTagState& fragTags) const
	{
		u32k numEntries = m_keys.size();
		for (u32 i = 0; i < numEntries; i++)
		{
			if (m_keys[i] == fragTags)
			{
				return i;
			}
		}

		return -1;
	}

	const T* Find(const SFragTagState& fragTags) const
	{
		i32 idx = FindIdx(fragTags);

		if (idx >= 0)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	T* Find(const SFragTagState& fragTags)
	{
		i32 idx = FindIdx(fragTags);

		if (idx >= 0)
		{
			return &m_values[idx];
		}

		return NULL;
	}

	void Sort(const CTagDefinition& tagDefs, const CTagDefinition* pFragTagDefs)
	{
		DynArray<CTagDefinition::SPriorityCount>* pCombinedPriorities = NULL;

		if (pFragTagDefs)
		{
			pCombinedPriorities = new DynArray<CTagDefinition::SPriorityCount>();
			tagDefs.CombinePriorities(*pFragTagDefs, *pCombinedPriorities);
		}

		u32k size = m_keys.size();
		SSortStruct* pSortKeys = new SSortStruct[size];
		for (u32 i = 0; i < size; i++)
		{
			pSortKeys[i].priority = tagDefs.RateTagState(m_keys[i].globalTags, pCombinedPriorities);
			pSortKeys[i].originalIdx = i;
		}
		if (pFragTagDefs)
		{
			for (u32 i = 0; i < size; i++)
			{
				pSortKeys[i].priority += pFragTagDefs->RateTagState(m_keys[i].fragmentTags, pCombinedPriorities);
			}
		}

		delete(pCombinedPriorities);

		std::stable_sort(pSortKeys, pSortKeys + size);

		for (u32 i = 0; i < size; i++)
		{
			u32k originalIdx = pSortKeys[i].originalIdx;
			if (originalIdx != i)
			{
				SFragTagState bufferedTS = m_keys[i];
				T bufferedValue = m_values[i];
				m_keys[i] = m_keys[originalIdx];
				m_values[i] = m_values[originalIdx];
				m_keys[originalIdx] = bufferedTS;
				m_values[originalIdx] = bufferedValue;

				for (u32 k = i + 1; k < size; k++)
				{
					if (pSortKeys[k].originalIdx == i)
					{
						pSortKeys[k].originalIdx = originalIdx;
						break;
					}
				}
			}
		}

		delete[] pSortKeys;
	}

	TOptimisedTagSortedList<T>* Compress(const CTagDefinition& tagDef, const CTagDefinition* pFragTagDef = NULL) const
	{
		u32k size = m_keys.size();

		//--- Assess usage
		SFragTagState usedTags;
		QueryUsedTags(usedTags, &tagDef, NULL);

		//--- Generate buffers
		CTagDefinition::STagDefData* pTagDefData = new CTagDefinition::STagDefData();
		TagState tagStateFilter(usedTags.globalTags);
		tagDef.AssignBits(*pTagDefData, tagStateFilter);
		const CTagDefinition::STagDefData* pFragData = (pFragTagDef ? &pFragTagDef->GetDefData() : NULL);

		u32k bytesGlobal = pTagDefData->GetNumBytes();
		u32k bytesFrag = (pFragData ? pFragData->GetNumBytes() : 0);
		u32k bytesTotal = bytesGlobal + bytesFrag;

		//--- Create & initialise new optimised DB
		TOptimisedTagSortedList<T>* pOptimisedList = new TOptimisedTagSortedList<T>(size, bytesTotal);
		pOptimisedList->m_pTagDef = &tagDef;
		pOptimisedList->m_pDefData = pTagDefData;
		pOptimisedList->m_pFragTagDef = pFragTagDef;
		pOptimisedList->m_pFragDefData = pFragData;

		STagStateBase globalKeys = STagStateBase(pOptimisedList->m_keys, bytesGlobal);
		u8* keyBuffer = pOptimisedList->m_keys;
		T* valueBuffer = pOptimisedList->m_values;

		//--- Assign the globalTags & data
		for (u32 i = 0; i < size; i++)
		{
			tagDef.CompressTagState(globalKeys, m_keys[i].globalTags, *pTagDefData);
			valueBuffer[i] = m_values[i];
			globalKeys.state += bytesTotal;
		}

		//--- Now optionally assign the fragTags
		if (pFragTagDef)
		{
			STagStateBase fragKeys = STagStateBase(pOptimisedList->m_keys + bytesGlobal, bytesFrag);

			for (u32 i = 0; i < size; i++)
			{
				pFragTagDef->CompressTagState(fragKeys, m_keys[i].fragmentTags, *pFragData);
				fragKeys.state += bytesTotal;
			}
		}

		return pOptimisedList;
	}

	const T& GetDefault() const
	{
		u32k size = m_keys.size();
		if (size == 0)
		{
			DrxFatalError("[TTagSortedList] No default in list, this should not happen");
		}

		return m_values[size - 1];
	}

	const T* GetBestMatch(const SFragTagState& fragTags, const CTagDefinition* pGlobalTagDef, const CTagDefinition* pFragTagDef, SFragTagState* pMatchedFragTags = NULL, u32* pTagSetIdx = NULL) const
	{
		STagStateBase tagGlobal((const STagStateBase)(fragTags.globalTags));
		STagStateBase tagFragment((const STagStateBase)(fragTags.fragmentTags));

		u32k numEntries = m_keys.size();
		for (u32 i = 0; i < numEntries; i++)
		{
			const SFragTagState& fragTagState = m_keys[i];

			if (pGlobalTagDef->Contains(fragTags.globalTags, fragTagState.globalTags)
			    && (!pFragTagDef || pFragTagDef->Contains(fragTags.fragmentTags, fragTagState.fragmentTags)))
			{
				if (pMatchedFragTags)
				{
					*pMatchedFragTags = fragTagState;
				}
				if (pTagSetIdx)
				{
					*pTagSetIdx = i;
				}
				return &m_values[i];
			}
		}

		if (pTagSetIdx)
		{
			*pTagSetIdx = TAG_SET_IDX_INVALID;
		}
		return NULL;
	}

	const T* GetBestMatch(const SFragTagState& fragTags, const TagState& requiredTagState, const CTagDefinition* pGlobalTagDef, const CTagDefinition* pFragTagDef, SFragTagState* pMatchedFragTags = NULL) const
	{
		TagState requiredComparisonMask = pGlobalTagDef->GenerateMask(requiredTagState);

		u32k numEntries = m_keys.size();
		for (u32 i = 0; i < numEntries; i++)
		{
			const SFragTagState& fragTagState = m_keys[i];

			if (pGlobalTagDef->Contains(fragTagState.globalTags, requiredTagState, requiredComparisonMask)
			    && pGlobalTagDef->Contains(fragTags.globalTags, fragTagState.globalTags)
			    && (!pFragTagDef || pFragTagDef->Contains(fragTags.fragmentTags, fragTagState.fragmentTags)))
			{
				if (pMatchedFragTags)
				{
					*pMatchedFragTags = fragTagState;
				}
				return &m_values[i];
			}
		}

		return NULL;
	}

	void QueryUsedTags(SFragTagState& usedTags, const CTagDefinition* pGlobalTagDef, const CTagDefinition* pFragTagDef) const
	{
		usedTags.globalTags.Clear();
		usedTags.fragmentTags.Clear();

		u32k numEntries = m_keys.size();

		for (u32 t = 0; t < numEntries; t++)
		{
			const SFragTagState& tagState = m_keys[t];

			usedTags.globalTags = pGlobalTagDef->GetUnion(usedTags.globalTags, tagState.globalTags);
			if (pFragTagDef)
			{
				usedTags.fragmentTags = pFragTagDef->GetUnion(usedTags.fragmentTags, tagState.fragmentTags);
			}
		}

		//--- Expand to include whole group in mask
		usedTags.globalTags = pGlobalTagDef->GenerateMask(usedTags.globalTags);
		if (pFragTagDef)
		{
			usedTags.fragmentTags = pFragTagDef->GenerateMask(usedTags.fragmentTags);
		}
	}

	void QueryUsedTags(SFragTagState& usedTags, const SFragTagState& filter, const CTagDefinition* pGlobalTagDef, const CTagDefinition* pFragTagDef) const
	{
		usedTags.globalTags.Clear();
		usedTags.fragmentTags.Clear();

		u32k numEntries = m_keys.size();

		TagState filterGroupMaskGlobal = pGlobalTagDef->GenerateMask(filter.globalTags);
		TagState filterGroupMaskFrag;
		if (pFragTagDef)
		{
			filterGroupMaskFrag = pFragTagDef->GenerateMask(filter.fragmentTags);
		}
		else
		{
			filterGroupMaskFrag.Clear();
		}

		//--- The best pure match is the last one we should consider
		u32 matchID;
		if (!GetBestMatch(filter, pGlobalTagDef, pFragTagDef, NULL, &matchID))
		{
			matchID = numEntries;
		}

		for (u32 t = 0; t < matchID; t++)
		{
			const SFragTagState& tagState = m_keys[t];

			const TagState groupMaskGlobal = pGlobalTagDef->GenerateMask(tagState.globalTags);
			const TagState combinedGlobal(filterGroupMaskGlobal & groupMaskGlobal);
			const TagState filterTagsAdjusted(filter.globalTags & combinedGlobal);
			const TagState tagsAdjusted(tagState.globalTags & combinedGlobal);

			bool valid = false;
			if (filterTagsAdjusted == tagsAdjusted)
			{
				if (pFragTagDef)
				{
					const TagState groupMaskFrag = pFragTagDef->GenerateMask(tagState.fragmentTags);
					const TagState combinedFrag(filterGroupMaskFrag & groupMaskFrag);
					const TagState filterFragTagAdjusted(filter.fragmentTags & combinedFrag);
					const TagState fragTagsAdjusted(tagState.fragmentTags & combinedFrag);

					valid = (filterFragTagAdjusted == fragTagsAdjusted);
				}
				else
				{
					valid = true;
				}
			}

			if (valid)
			{
				usedTags.globalTags = pGlobalTagDef->GetUnion(usedTags.globalTags, tagState.globalTags);
				if (pFragTagDef)
				{
					usedTags.fragmentTags = pFragTagDef->GetUnion(usedTags.fragmentTags, tagState.fragmentTags);
				}
			}
		}

		//--- Expand to include whole group in mask
		usedTags.globalTags = pGlobalTagDef->GenerateMask(usedTags.globalTags);
		if (pFragTagDef)
		{
			usedTags.fragmentTags = pFragTagDef->GenerateMask(usedTags.fragmentTags);
		}
	}

private:

	DynArray<SFragTagState> m_keys;
	DynArray<T>             m_values;
};

#endif //__I_DRX_MANNEQUIN_TAG_DEFS_H__
