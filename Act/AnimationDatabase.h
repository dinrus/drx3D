// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ANIMATIONDATABASE_H__
#define __ANIMATIONDATABASE_H__

#include <drx3D/Act/IDrxMannequin.h>

class CAnimationDatabase : public IAnimationDatabase
{
public:

	friend class CAnimationDatabaseUpr;
	friend class CAnimationDatabaseLibrary;

	CAnimationDatabase()
		: m_pFragDef(NULL)
		, m_pTagDef(NULL)
		, m_autoSort(true)
	{
	}

	explicit CAnimationDatabase(tukk normalizedFilename)
		: m_filename(normalizedFilename)
		, m_pFragDef(NULL)
		, m_pTagDef(NULL)
		, m_autoSort(true)
	{
	}

	virtual ~CAnimationDatabase();

	virtual bool Validate(const IAnimationSet* animSet, MannErrorCallback errorCallback, MannErrorCallback warningCallback, uk errorCallbackContext) const;
	virtual void EnumerateAnimAssets(const IAnimationSet* animSet, MannAssetCallback assetCallback, uk callbackContext) const;

	u32       GetTotalTagSets(FragmentID fragmentID) const
	{
		u32k numFragmentTypes = m_fragmentList.size();
		if (fragmentID < numFragmentTypes)
		{
			const TOptFragmentTagSetList* pList = m_fragmentList[fragmentID]->compiledList;
			return pList ? pList->Size() : 0;
		}
		else
		{
			return 0;
		}
	}
	u32 GetTagSetInfo(FragmentID fragmentID, u32 tagSetID, SFragTagState& fragTagState) const
	{
		u32k numFragmentTypes = m_fragmentList.size();
		if (fragmentID < numFragmentTypes)
		{
			const SFragmentEntry& fragmentEntry = *m_fragmentList[fragmentID];

			const TOptFragmentTagSetList* pList = fragmentEntry.compiledList;
			if (pList && (tagSetID < pList->Size()))
			{
				pList->GetKey(fragTagState, tagSetID);
				return pList->Get(tagSetID)->size();
			}
		}

		return 0;
	}

	const CFragment* GetEntry(FragmentID fragmentID, const SFragTagState& tags, u32 optionIdx) const
	{
		u32k numFragmentTypes = m_fragmentList.size();
		if (fragmentID < numFragmentTypes)
		{
			const SFragmentEntry& fragmentEntry = *m_fragmentList[fragmentID];
			const TOptFragmentTagSetList* pList = fragmentEntry.compiledList;
			const TFragmentOptionList* pOptionList = pList ? pList->Find(tags) : NULL;
			if (pOptionList)
			{
				if (optionIdx >= (u32)pOptionList->size())
				{
					DRX_ASSERT(false);
					return NULL;
				}

				return (*pOptionList)[optionIdx].fragment;
			}
		}

		return NULL;
	}

	const CFragment* GetEntry(FragmentID fragmentID, u32 fragSetIdx, u32 optionIdx) const
	{
		u32k numFragmentTypes = m_fragmentList.size();
		if (fragmentID < numFragmentTypes)
		{
			const SFragmentEntry& fragmentEntry = *m_fragmentList[fragmentID];
			const TOptFragmentTagSetList* pList = fragmentEntry.compiledList;
			const TFragmentOptionList* pOptionList = pList ? pList->Get(fragSetIdx) : NULL;

			if (pOptionList)
			{
				if (optionIdx >= (u32)pOptionList->size())
				{
					DRX_ASSERT(false);
					return NULL;
				}

				return (*pOptionList)[optionIdx].fragment;
			}
		}

		return NULL;
	}

	virtual const CFragment* GetBestEntry(const SFragmentQuery& fragQuery, SFragmentSelection* fragSelection = NULL) const
	{
		u32k numFragmentTypes = m_fragmentList.size();
		SFragTagState* pSelectedFragTags = NULL;
		if (fragSelection)
		{
			fragSelection->tagState.globalTags = TAG_STATE_EMPTY;
			fragSelection->tagState.fragmentTags = TAG_STATE_EMPTY;
			fragSelection->tagSetIdx = TAG_SET_IDX_INVALID;
			fragSelection->optionIdx = 0;
			pSelectedFragTags = &fragSelection->tagState;
		}

		if (fragQuery.fragID < numFragmentTypes)
		{
			const CTagDefinition* fragTagDef = m_pFragDef->GetSubTagDefinition(fragQuery.fragID);
			const SFragmentEntry& fragmentEntry = *m_fragmentList[fragQuery.fragID];
			u32 tagSetIdx = TAG_SET_IDX_INVALID;
			const TFragmentOptionList* pOptionList = fragmentEntry.compiledList ? fragmentEntry.compiledList->GetBestMatch(fragQuery.tagState, fragQuery.requiredTags, pSelectedFragTags, &tagSetIdx) : NULL;

			if (pOptionList)
			{
				u32k numOptions = pOptionList->size();

				CFragment* ret = NULL;
				if (numOptions > 0)
				{
					u32 option = fragQuery.optionIdx % numOptions;
					ret = (*pOptionList)[option].fragment;

					if (fragSelection)
					{
						fragSelection->tagSetIdx = tagSetIdx;
						fragSelection->optionIdx = option;
					}
				}

				return ret;
			}
		}
		else if (fragQuery.fragID != FRAGMENT_ID_INVALID)
		{
			DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "AnimDatabase: Invalid fragment idx: %d passed to %s", fragQuery.fragID, m_filename.c_str());
		}

		return NULL;
	}

	virtual bool Query(SFragmentData& outFragmentData, const SFragmentQuery& inFragQuery, SFragmentSelection* outFragSelection = NULL) const
	{
		const CFragment* fragment = GetBestEntry(inFragQuery, outFragSelection);

		outFragmentData.animLayers.clear();
		outFragmentData.procLayers.clear();
		if (fragment)
		{
			outFragmentData.animLayers = fragment->m_animLayers;
			outFragmentData.procLayers = fragment->m_procLayers;
			return true;
		}
		return false;
	}

	virtual u32 Query(SFragmentData& outFragmentData, const SBlendQuery& inBlendQuery, u32 inOptionIdx, const IAnimationSet* inAnimSet, SFragmentSelection* outFragSelection = NULL) const;

	virtual u32 FindBestMatchingTag(const SFragmentQuery& inFragQuery, SFragTagState* matchedTagState /* = NULL*/, u32* tagSetIdx /* = NULL*/) const
	{
		if (m_pFragDef->IsValidTagID(inFragQuery.fragID))
		{
			SFragmentEntry& fragmentEntry = *m_fragmentList[inFragQuery.fragID];
			const CTagDefinition* fragTagDef = m_pFragDef->GetSubTagDefinition(inFragQuery.fragID);
			const TFragmentOptionList* pOptionList = fragmentEntry.compiledList ? fragmentEntry.compiledList->GetBestMatch(inFragQuery.tagState, inFragQuery.requiredTags, matchedTagState, tagSetIdx) : NULL;

			if (pOptionList)
			{
				return pOptionList->size();
			}
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, "AnimDatabase: Invalid fragment idx: %d passed to %s", inFragQuery.fragID, m_filename.c_str());
		}

		if (matchedTagState)
		{
			*matchedTagState = SFragTagState();
		}

		if (tagSetIdx)
		{
			*tagSetIdx = TAG_SET_IDX_INVALID;
		}

		return 0;
	}

	virtual const CTagDefinition& GetTagDefs() const
	{
		return *m_pTagDef;
	}

	virtual const CTagDefinition& GetFragmentDefs() const
	{
		return *m_pFragDef;
	}

	virtual FragmentID GetFragmentID(tukk szFragmentName) const
	{
		return m_pFragDef->Find(szFragmentName);
	}

	//--- Transition queries

	virtual u32                GetNumBlends(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo) const;
	virtual const SFragmentBlend* GetBlend(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, u32 blendNum) const;
	virtual const SFragmentBlend* GetBlend(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid uid) const;
	virtual void                  FindBestBlends(const SBlendQuery& blendQuery, SBlendQueryResult& result1, SBlendQueryResult& result2) const;

	//--- Editor entries

	void   SetEntry(FragmentID fragmentID, const SFragTagState& tags, u32 optionIdx, const CFragment& fragment);
	bool   DeleteEntry(FragmentID fragmentID, const SFragTagState& tags, u32 optionIdx);
	u32 AddEntry(FragmentID fragmentID, const SFragTagState& tags, const CFragment& fragment);

	void   SetAutoSort(bool autoSort)
	{
		m_autoSort = autoSort;
	}
	void                Sort();
	void                Compress();

	virtual tukk FindSubADBFilenameForID(FragmentID fragmentID) const;
	virtual bool        RemoveSubADBFragmentFilter(FragmentID fragmentID);
	virtual bool        AddSubADBFragmentFilter(const string& sADBFileName, FragmentID fragmentID);
	virtual void        GetSubADBFragmentFilters(SMiniSubADB::TSubADBArray& outList) const;

	virtual bool        AddSubADBTagFilter(const string& sParentFilename, const string& sADBFileName, const TagState& tag);
	virtual bool        MoveSubADBFilter(const string& sADBFileName, const bool bMoveUp);
	virtual bool        DeleteSubADBFilter(const string& sADBFileName);
	virtual bool        ClearSubADBFilter(const string& sADBFileName);

	virtual tukk GetFilename() const
	{
		return m_filename.c_str();
	}

	virtual void QueryUsedTags(const FragmentID fragmentID, const SFragTagState& filter, SFragTagState& usedTags) const;

	void         DeleteFragmentID(FragmentID fragmentID);

	static void  RegisterCVars();

private:

	void              EnumerateFragmentAnimAssets(const CFragment* pFragment, const IAnimationSet* animSet, SAnimAssetReport& assetReport, MannAssetCallback assetCallback, uk callbackContext) const;
	bool              ValidateFragment(const CFragment* pFragment, const IAnimationSet* animSet, SMannequinErrorReport& errorReport, MannErrorCallback errorCallback, uk errorCallbackContext) const;
	bool              ValidateAnimations(FragmentID fragID, u32 tagSetID, u32 numOptions, const SFragTagState& tagState, const IAnimationSet* animSet, MannErrorCallback errorCallback, uk errorCallbackContext) const;
	bool              IsDuplicate(const CFragment* pFragmentA, const CFragment* pFragmentB) const;

	void              CompressFragmentID(FragmentID fragID);

	SFragmentBlendUid AddBlendInternal(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlend& fragBlend);
	SFragmentBlendUid AddBlend(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, const SFragmentBlend& fragBlend);
	void              SetBlend(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid, const SFragmentBlend& fragmentBlend);
	void              DeleteBlend(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo, SFragmentBlendUid blendUid);

	struct SExplicitBlend
	{
		TagState   tag;
		SAnimBlend blend;
	};

	struct SExplicitBlendList
	{
		std::vector<SExplicitBlend> blendList;
	};

	struct SFragmentOption
	{
		SFragmentOption(CFragment* _fragment = NULL, u32 _chance = 100)
			:
			fragment(_fragment),
			chance(_chance)
		{
		}

		CFragment* fragment;
		u32     chance;
	};
	typedef std::vector<SFragmentOption> TFragmentOptionList;

	//--- A group of fragments with a specific tag state
	typedef TTagSortedList<TFragmentOptionList>          TFragmentTagSetList;

	typedef TOptimisedTagSortedList<TFragmentOptionList> TOptFragmentTagSetList;

	//--- The root fragment structure, holds all fragments for a specific FragmentID
	struct SFragmentEntry
	{
		SFragmentEntry()
			:
			compiledList(NULL)
		{
		}
		TFragmentTagSetList     tagSetList;
		TOptFragmentTagSetList* compiledList;
	};
	typedef std::vector<SFragmentEntry*> TFragmentList;

	//--- Fragment blends
	typedef std::vector<SFragmentBlend> TFragmentBlendList;
	struct SFragmentBlendVariant
	{
		SFragTagState      tagsFrom;
		SFragTagState      tagsTo;

		TFragmentBlendList blendList;

		u32             FindBlendIndexForUid(SFragmentBlendUid uid) const
		{
			for (u32 i = 0; i < blendList.size(); ++i)
			{
				if (blendList[i].uid == uid)
					return i;
			}
			return u32(-1);
		}

		const SFragmentBlend* FindBlend(SFragmentBlendUid uid) const
		{
			u32 idx = FindBlendIndexForUid(uid);
			return (idx < blendList.size()) ? &blendList[idx] : NULL;
		}

		SFragmentBlend* FindBlend(SFragmentBlendUid uid)
		{
			u32 idx = FindBlendIndexForUid(uid);
			return (idx < blendList.size()) ? &blendList[idx] : NULL;
		}
	};

	typedef std::vector<SFragmentBlendVariant> TFragmentVariantList;
	struct SFragmentBlendEntry
	{
		TFragmentVariantList variantList;
	};
	struct SFragmentBlendID
	{
		FragmentID fragFrom;
		FragmentID fragTo;

		bool operator<(const SFragmentBlendID& blend) const
		{
			return (fragFrom < blend.fragFrom) || ((fragFrom == blend.fragFrom) && (fragTo < blend.fragTo));
		}
	};
	typedef std::map<SFragmentBlendID, SFragmentBlendEntry> TFragmentBlendDatabase;
	TFragmentBlendDatabase m_fragmentBlendDB;

	struct SSubADB
	{
		TagState              tags;
		TagState              comparisonMask;
		string                filename;
		const CTagDefinition* pFragDef;
		const CTagDefinition* pTagDef;
		TagState              knownTags;

		typedef std::vector<FragmentID> TFragIDList;
		TFragIDList vFragIDs;

		typedef std::vector<SSubADB> TSubADBList;
		TSubADBList subADBs;

		bool        IsSubADB(tukk szSubADBFilename) const
		{
			return (filename.compareNoCase(szSubADBFilename) == 0);
		}

		bool ContainsSubADB(tukk szSubADBFilename) const
		{
			for (TSubADBList::const_iterator cit = subADBs.begin(); cit != subADBs.end(); ++cit)
			{
				const SSubADB& subADB = *cit;
				if (subADB.IsSubADB(szSubADBFilename))
				{
					return true;
				}

				if (subADB.ContainsSubADB(szSubADBFilename))
				{
					return true;
				}
			}

			return false;
		}
	};
	typedef SSubADB::TSubADBList TSubADBList;

	SSubADB*              FindSubADB(tukk szSubADBFilename, bool recursive);
	const SSubADB*        FindSubADB(tukk szSubADBFilename, bool recursive) const;

	static const SSubADB* FindSubADB(const TSubADBList& subAdbList, tukk szSubADBFilename, bool recursive);

	struct SCompareBlendVariantFunctor : public std::binary_function<const SFragmentBlendVariant&, const SFragmentBlendVariant&, bool>
	{
		SCompareBlendVariantFunctor(const CTagDefinition& tagDefs, const CTagDefinition* pFragTagDefsFrom, const CTagDefinition* pFragTagDefsTo)
			:
			m_tagDefs(tagDefs),
			m_pFragTagDefsFrom(pFragTagDefsFrom),
			m_pFragTagDefsTo(pFragTagDefsTo)
		{
		}

		bool operator()(const SFragmentBlendVariant& lhs, const SFragmentBlendVariant& rhs);

		const CTagDefinition& m_tagDefs;
		const CTagDefinition* m_pFragTagDefsFrom;
		const CTagDefinition* m_pFragTagDefsTo;
	};

	SFragmentBlendVariant*       GetVariant(FragmentID fragmentIDFrom, FragmentID fragmentIDTo, const SFragTagState& tagFrom, const SFragTagState& tagTo) const;
	const SFragmentBlendVariant* FindBestVariant(const SFragmentBlendID& fragmentBlendID, const SFragTagState& tagFrom, const SFragTagState& tagTo, const TagState& requiredTags, FragmentID& outFragmentFrom, FragmentID& outFragmentTo) const;
	void                         FindBestBlendInVariant(const SFragmentBlendVariant& variant, const SBlendQuery& blendQuery, SBlendQueryResult& result) const;

	tukk                  FindSubADBFilenameForIDInternal(FragmentID fragmentID, const SSubADB& pSubADB) const;
	bool                         RemoveSubADBFragmentFilterInternal(FragmentID fragmentID, SSubADB& subADB);
	bool                         AddSubADBFragmentFilterInternal(const string& sADBFileName, FragmentID fragmentID, SSubADB& subADB);
	void                         FillMiniSubADB(SMiniSubADB& outMiniSub, const SSubADB& inSub) const;
	bool                         MoveSubADBFilterInternal(const string& sADBFileName, SSubADB& subADB, const bool bMoveUp);
	bool                         DeleteSubADBFilterInternal(const string& sADBFileName, SSubADB& subADB);
	bool                         ClearSubADBFilterInternal(const string& sADBFileName, SSubADB& subADB);
	bool                         AddSubADBTagFilterInternal(const string& sParentFilename, const string& sADBFileName, const TagState& tag, SSubADB& subADB);

	static void                  AdjustSubADBListAfterFragmentIDDeletion(SSubADB::TSubADBList& subADBs, const FragmentID fragmentID);

	bool                         ValidateSet(const TFragmentTagSetList& tagSetList);

	string                m_filename;

	const CTagDefinition* m_pFragDef;
	const CTagDefinition* m_pTagDef;

	TFragmentList         m_fragmentList;

	SSubADB::TSubADBList  m_subADBs;

	bool                  m_autoSort;

	static i32            s_mnAllowEditableDatabasesInPureGame;
	static bool           s_registeredCVars;
};

#endif //__ANIMATIONDATABASE_H__
