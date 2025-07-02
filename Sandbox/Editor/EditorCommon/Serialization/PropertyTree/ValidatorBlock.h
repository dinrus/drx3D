// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>
#include <vector>
#include <algorithm>

enum ValidatorEntryType
{
	VALIDATOR_ENTRY_WARNING,
	VALIDATOR_ENTRY_ERROR
};

struct ValidatorEntry
{
	ukk handle;
	yasli::TypeID typeId;

	ValidatorEntryType type;
	yasli::string message;

	bool operator<(const ValidatorEntry& rhs) const {
		if (handle != rhs.handle)
			return handle < rhs.handle; 
		return typeId < rhs.typeId;
	}

	ValidatorEntry(ValidatorEntryType type, ukk handle, const yasli::TypeID& typeId, tukk message)
	: type(type)
	, handle(handle)
	, message(message)
	, typeId(typeId)
	{
	}

	ValidatorEntry()
	: handle()
	, type(VALIDATOR_ENTRY_WARNING)
	{
	}
};
typedef std::vector<ValidatorEntry> ValidatorEntries;

class ValidatorBlock
{
public:
	ValidatorBlock()
	: enabled_(false)
	{
	}

	void clear()
	{
		entries_.clear();
		used_.clear();
	}

	void addEntry(const ValidatorEntry& entry)
	{
		ValidatorEntries::iterator it = std::upper_bound(entries_.begin(), entries_.end(), entry);
		used_.insert(used_.begin() + (it - entries_.begin()), false);
		entries_.insert(it, entry);
		enabled_ = true;
	}

	bool isEnabled() const { return enabled_; }

	const ValidatorEntry* getEntry(i32 index, i32 count) const {
		if (size_t(index) >= entries_.size())
			return 0;
		if (size_t(index + count) > entries_.size())
			return 0;
		return &entries_[index];
	}

	bool findHandleEntries(i32* outIndex, i32* outCount, ukk handle, const yasli::TypeID& typeId)
	{
		if (handle == 0)
			return false;
		ValidatorEntry e;
		e.handle = handle;
		e.typeId = typeId;
		ValidatorEntries::iterator begin = std::lower_bound(entries_.begin(), entries_.end(), e);
		ValidatorEntries::iterator end = std::upper_bound(entries_.begin(), entries_.end(), e);
		if (begin != end)
		{
			*outIndex = i32(begin - entries_.begin());
			*outCount = i32(end - begin);
			return true;
		}
		return false;
	}

	void markAsUsed(i32 start, i32 count)
	{
		if (start < 0)
			return;
		if (start + count > entries_.size())
			return;
		for(i32 i = start; i < start + count; ++i) {
			used_[i] = true;
		}
	}

	void mergeUnusedItemsWithRootItems(i32* firstUnusedItem, i32* count, ukk newHandle, const yasli::TypeID& typeId) 
	{
		size_t numItems = used_.size();
		for (size_t i = 0; i < numItems; ++i) {
			if (entries_[i].handle == newHandle) {
				entries_.push_back(entries_[i]);
				used_.push_back(true);
				entries_[i].typeId = yasli::TypeID();
			}
			if (!used_[i]) {
				entries_.push_back(entries_[i]);
				used_.push_back(true);
				entries_.back().handle = newHandle;
				entries_.back().typeId = typeId;
			}
		}
		*firstUnusedItem = (i32)numItems;
		*count = i32(entries_.size() - numItems);
	}

	bool containsWarnings() const
	{
		for (size_t i = 0; i < entries_.size(); ++i)
		{
			if (entries_[i].type == VALIDATOR_ENTRY_WARNING)
				return true;
		}
		return false;
	}

	bool containsErrors() const
	{
		for (size_t i = 0; i < entries_.size(); ++i)
		{
			if (entries_[i].type == VALIDATOR_ENTRY_ERROR)
				return true;
		}
		return false;
	}

private:
	ValidatorEntries entries_;
	std::vector<bool> used_;
	bool enabled_;
};

