// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>

namespace Serialization
{

class CContextList
{
public:
	template<class T>
	void Update(T* contextObject)
	{
		for (size_t i = 0; i < links_.size(); ++i)
			if (links_[i]->type == TypeID::get<T>())
			{
				links_[i]->object = (uk )contextObject;
				return;
			}

		SContextLink* newLink = new SContextLink;
		newLink->type = TypeID::get<T>();
		newLink->previousContext = links_.empty() ? connectedList_ : links_.back();
		newLink->object = (uk )contextObject;
		tail_.previousContext = newLink;
		links_.push_back(newLink);
	}

	CContextList()
	{
		tail_.previousContext = 0;
		tail_.object = 0;
		connectedList_ = 0;
	}

	explicit CContextList(SContextLink* connectedList)
	{
		tail_.previousContext = 0;
		tail_.object = 0;
		connectedList_ = connectedList;
	}

	~CContextList()
	{
		for (size_t i = 0; i < links_.size(); ++i)
			delete links_[i];
		links_.clear();
	}

	SContextLink* Tail() { return &tail_; }
private:
	SContextLink               tail_;
	std::vector<SContextLink*> links_;
	SContextLink*              connectedList_;
};

}

