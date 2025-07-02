// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Thread/DrxThread.h>
#include <drx3D/Plugins/concqueue/concqueue.hpp>

#include <list>
#include <functional>

template<class T>
struct SElementPool
{
	using AllocFunctionType = std::function<T*()>;
	using FreeFunctionType = std::function<void(T*)>;

	DrxCriticalSection         availableFreeElementsLock;
	std::list<T*>              allocatedElements;
	ConcQueue<UnboundMPSC, T*> availableFreeElements;

	AllocFunctionType        allocElementFunction = nullptr;
	FreeFunctionType         freeElementFunction = nullptr;

	SElementPool() = default;

	template <typename AllocFuncType, typename FreeFuncType>
	SElementPool(AllocFuncType&& allocFunc, FreeFuncType&& freeFunc) :
		allocElementFunction(std::forward<AllocFuncType>(allocFunc)), freeElementFunction(std::forward<FreeFuncType>(freeFunc)) {}

	T* GetOrCreateOneElement()
	{
		T* pElement = nullptr;

		AUTO_LOCK(availableFreeElementsLock);
		if (!availableFreeElements.dequeue(pElement))
		{
			DRX_ASSERT_MESSAGE(allocElementFunction, "Allocate element function is not provided.");
			pElement = allocElementFunction();
			allocatedElements.emplace_back(pElement);
		}

		return pElement;
	}

	void ReturnToPool(T* pElement)
	{
		if (pElement)
		{
			AUTO_LOCK(availableFreeElementsLock);
			DRX_ASSERT_MESSAGE(freeElementFunction, "Free element function is not provided.");
			if (freeElementFunction != nullptr)
			{
				freeElementFunction(pElement);
			}
			availableFreeElements.enqueue(pElement);
		}
	}

	void ShutDown()
	{
		DRX_ASSERT_MESSAGE(freeElementFunction, "Free element function is not provided.");

		// Release all elements
		T* pElement;
		while (availableFreeElements.dequeue(pElement));

		while (allocatedElements.size())
		{
			if (freeElementFunction != nullptr)
			{
				freeElementFunction(allocatedElements.back());
			}
			delete allocatedElements.back();
			allocatedElements.pop_back();
		}
	}
};