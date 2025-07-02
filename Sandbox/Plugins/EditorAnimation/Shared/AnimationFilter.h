// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/smartptr.h>

#include <drx3D/CoreX/Serialization/Forward.h>

struct SAnimationFilterItem
{
	string              path;
	std::vector<string> tags;
	string              skeletonAlias;
	i32                 selectedRule;

	SAnimationFilterItem()
		: selectedRule(-1)
	{
	}

	bool operator==(const SAnimationFilterItem& rhs) const
	{
		if (path != rhs.path)
			return false;
		if (tags.size() != rhs.tags.size())
			return false;
		for (size_t i = 0; i < tags.size(); ++i)
			if (tags[i] != rhs.tags[i])
				return false;
		return true;
	}
};

typedef std::vector<SAnimationFilterItem> SAnimationFilterItems;

struct IAnimationFilterCondition : public _reference_target_t
{
	bool not;
	virtual bool Check(const SAnimationFilterItem& item) const = 0;
	virtual void FindTags(std::vector<string>* tags) const {}
	virtual void Serialize(Serialization::IArchive& ar);
	IAnimationFilterCondition() : not (false) {}
};

struct SAnimationFilter
{
	_smart_ptr<IAnimationFilterCondition> condition;

	bool MatchesFilter(const SAnimationFilterItem& item) const;
	void Filter(std::vector<SAnimationFilterItem>* matchingItems, const std::vector<SAnimationFilterItem>& items) const;
	void Serialize(Serialization::IArchive& ar);

	void FindTags(std::vector<string>* tags) const;
	void SetInFolderCondition(tukk folder);
};

