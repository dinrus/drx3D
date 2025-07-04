// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   OpenList.h
//  Описание: This is a helper class to implement a open list
////////////////////////////////////////////////////////////////////////////

#ifndef OPENLIST_H
#define OPENLIST_H

#pragma once

template<typename ElementNode, class BestNodePredicate>
class OpenList
{
public:
	typedef std::vector<ElementNode> ElementList;

	OpenList(const size_t maxExpectedSize)
	{
		SetupOpenList(maxExpectedSize);
	}

	ILINE void SetupOpenList(const size_t maxExpectedSize)
	{
		openElements.reserve(maxExpectedSize);
	}

	ElementNode PopBestElement()
	{
		//DRX_PROFILE_FUNCTION(PROFILE_AI);

		DRX_ASSERT_MESSAGE(!openElements.empty(), "PopBestElement has been requested for an empty ElementNode open list.");
		BestNodePredicate predicate;
		typename ElementList::iterator bestElementIt = std::min_element(openElements.begin(), openElements.end(), predicate);
		ElementNode bestElement = *bestElementIt;
		*bestElementIt = openElements.back();
		openElements.pop_back();

		return bestElement;
	}

	ILINE void Reset()
	{
		openElements.clear();
	}

	ILINE bool IsEmpty() const
	{
		return openElements.empty();
	}

	ILINE void InsertElement(const ElementNode& newElement)
	{
		//DRX_PROFILE_FUNCTION(PROFILE_AI);

		assert(!stl::find(openElements, newElement));
		stl::push_back_unique(openElements, newElement);
	}

private:
	ElementList openElements;
};

#endif // OPENLIST_H
