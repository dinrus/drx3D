// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

namespace DrxMT
{

//--------------------------------------------------------------------------
// Pipe
//
// This class is a templated, dynamically-resizing, lock-free queue class
// that can be used to convey data from a single writer thread to a single
// reader thread. It does *NOT* handle the case where there are multiple
// readers or writers. It is designed for maximum efficiency in its
// intended use-case.
//
// It differs from standard lock-free queue implementations in that it
// uses a circular buffer approach, rather than a linked-list approach.
// This is more memory efficient, and also should be more efficient in
// time in all cases.
//
// In addition there is zero contention overhead. A normal lock-free
// implementation makes use of atomic primitives such as Compare and Swap
// (CAS). The drawback of this is that if there is contention, the code
// must loop until it successfully updates the variable, whereas the time
// taken here is basically constant (in the equlibrium case where the
// buffer has grown to a size that is large enough to handle all the data,
// which should happen very quickly).
//
//
// Usage Notes:
//
// The class implements the following members:
//
// Push(item) -          MUST ONLY BE CALLED FROM WRITER THREAD. Push a
//                       single value onto the queue. Constant complexity
//                       in equilibrium case.
//
// bool Pop(item) -      MUST ONLY BE CALLED FROM READER THREAD. Retrieve
//                       the next value from the queue. The return value
//                       indicates whether there was anything to retrieve.
//                       Constant complexity in equilibrium case.
//
// Clear() -             MUST ONLY BE CALLED FROM WRITER THREAD. Remove all
//                       items from the queue. Constant complexity in
//                       equilibrium case.
//
// Replace(item, repl) - MUST ONLY BE CALLED FROM WRITER THREAD. Replace
//                       all occurrences of item with repl. Linear
//                       complexity with respect to the size of the queue.
//                       NOTE that this is *only* supported for data types
//                       that can be written atomically - ints, pointers,
//                       etc.
//
// Swap(other) -         MUST ONLY BE CALLED FROM WRITER THREAD. Replace
//                       the contents of the queue with those of another
//                       instance. This is *only* safe if the other queue
//                       is not referred to by other threads!
//
//
// Implementation Overview:
//
// The class basically contains a queue of queues. This is required to
// support dynamic resizing. Each sub-queue is called a page. Values are
// only ever pushed onto the last page - the earlier pages are simply kept
// around until all items on them have been read, at which point they are
// deleted.
//
// When the last page is full, a new page is allocated that is twice the
// size of the last page. This means that eventually we will reach the
// situation where we have a single page that is large enough to handle
// the traffic in the current situation - in this situation performance is
// optimal.
//
//--------------------------------------------------------------------------

#if defined(PROFILE_PIPE_SYS)
	#define PIPE_FUNCTION_PROFILER(sys) DRX_PROFILE_FUNCTION(sys)
#else //defined(PROFILE_PIPE_SYS)
	#define PROFILE_PIPE_SYS
	#define PIPE_FUNCTION_PROFILER(sys)
#endif //defined(PROFILE_PIPE_SYS)

#if defined(PIPE_TESTING)
namespace
{
FILE* f = 0;
tukk indentation = "                                          ";
}
#endif //defined(PIPE_TESTING)

//! Metafunction to decide whether replacing should be supported - replacing can only
//! be performed if the data type can be written atomically with no special
//! synching/primitives, ie datatype is one word or less and has no copy constructor/
//! assignment operator.
template<typename _T> struct Pipe_ReplaceSupported
{
	enum
	{
		Value = false
	};
};

// Replace is supported for these simple types.
#define PIPE_REPLACE_SUPPORTED_FOR(type) template<> struct Pipe_ReplaceSupported<type> {enum {Value = true}; }
PIPE_REPLACE_SUPPORTED_FOR(i32);
PIPE_REPLACE_SUPPORTED_FOR(u32);
PIPE_REPLACE_SUPPORTED_FOR(char);
PIPE_REPLACE_SUPPORTED_FOR(u8);
PIPE_REPLACE_SUPPORTED_FOR(float);
#undef REPLACE_SUPPORTED_FOR

//! Replace is supported for pointer types.
template<typename _V> struct Pipe_ReplaceSupported<_V*> {enum {Value = true}; };

template<typename T> class Pipe
{
public:
	typedef T Item;

	Pipe();    //!< Not thread-safe.
	~Pipe();   //!< Not thread-safe.

	// Operations that can be called from the writer thread.
	void Push(const Item& item);
	void Replace(const Item& item, const Item& replacement);
	void Swap(Pipe& other);
	void Clear();

	// Operations that can be called from the reader thread.
	bool Pop(Item& item);

#if defined(PIPE_TESTING)
	i32 lastHead, lastTail, lastSize, lastPageCount, lastPagesExpired;
#endif //defined(PIPE_TESTING)

private:
	enum {DEFAULT_INITIAL_SIZE = 512};
	enum {SIZE_INCREASE_FACTOR = 2};
	enum {MAX_PAGES = 32};

	struct Page
	{
		i32                   size;
		i32           head;
		i32           tail;
		u32  skipSend;
		u32          skipReceive;
		i32                   lastSkipHeadSend;
		i32                   lastSkipHeadReceive;

#if defined(PIPE_TESTING)
		i32 dbgSkipDistance, dbgClearSize, dbgClearTail;
#endif //defined(PIPE_TESTING)

		Item  elements[1];
	};

	struct Data
	{
		i32            expiredHead;
		i32    head;
		i32    tail;
		Page*  pages[MAX_PAGES];
	};

	Data*  m_data;
};

template<typename T> inline Pipe<T>::Pipe()
	: m_data(0)
{
}

template<typename T> inline Pipe<T>::~Pipe()
{
	// Loop through all the pages and delete them.
	if (Data* data = m_data)
	{
		i32 head = data->head;
		i32 tail = data->tail;
		for (i32 index = head; index != tail; index = (index < MAX_PAGES - 1 ? index + 1 : 0))
			delete[] reinterpret_cast<tuk>(data->pages[index]);
	}
}

template<typename T> inline void Pipe<T >::Push(const Item& item)
{
	PIPE_FUNCTION_PROFILER(PROFILE_PIPE_SYS);

	// First we need to check whether the class is empty - this means we have to initialize the object.
	Data* data = m_data;
	if (!data)
	{
		data = new Data;
		data->expiredHead = 0;
		data->head = 0;
		data->tail = 0;
		m_data = data;
	}

	// Now we should clean up any pages in the expired queue. When a page is emptied, and there is a larger page
	// to use, the reader thread increments the head pointer to point to the next page. The expiredHead member
	// lags behind, so we can use it to see if there are any pages that *both* the reader and writer threads are
	// done with. These can then be safely deleted.
	i32 expiredHead = data->expiredHead;
	i32 pageHead = data->head;
	i32 pageTail = data->tail;
	for (; expiredHead != pageHead; expiredHead = (expiredHead < MAX_PAGES - 1 ? expiredHead + 1 : 0))
	{
#if defined(PIPE_TESTING)
		i32 pageCount = pageTail - expiredHead;
		if (pageCount < 0)
			pageCount += MAX_PAGES;
		if (f)
			fprintf(f, "Deleting page %d of size %d, %d pages remaining\n", expiredHead, data->pages[expiredHead]->size, pageCount - 1);
#endif //defined(PIPE_TESTING)

		delete[] reinterpret_cast<tuk>(data->pages[expiredHead]);
	}
	data->expiredHead = expiredHead;

	// In order to guarantee delivery order, we can only add to the last page. If there is no last page or there
	// is no space in the last page, then we must allocate a new page.
	Page* page = 0;
	i32 head = 0;
	i32 tail = -1; // This makes the calculations think the queue is full if there is no page.
	i32 size = 0;
	if (pageHead != pageTail)
	{
		i32 lastPageIndex = pageTail - 1;
		if (lastPageIndex < 0)
			lastPageIndex += MAX_PAGES;
		page = data->pages[lastPageIndex];
		head = page->head;
		tail = page->tail;
		size = page->size;
	}

	// Figure out how many free elements are in this page.
	i32 freeCount = head - tail;
	if (freeCount < 0)
		freeCount += size;

	// If there is no free page, create a new one now.
	if (!page || freeCount == 1)
	{
		// Calculate the size of the new page - we start at 16 elements and double the size each time after that.
		i32 newSize = size * SIZE_INCREASE_FACTOR;
		if (newSize == 0)
			newSize = DEFAULT_INITIAL_SIZE;
		i32 newSizeInBytes = sizeof(Page) + (newSize - 1) * sizeof(Item); // The -1 is because the Page structure has a 1-element array member.

		// Create and initialize the page.
		page = reinterpret_cast<Page*>(new char[newSizeInBytes]);
		page->size = size = newSize;
		page->head = head = 0;
		page->tail = tail = 0;

		page->skipSend = 0;
		page->skipReceive = 0;
		page->lastSkipHeadSend = 0;
		page->lastSkipHeadReceive = 0;

		// Add the page to the page queue. We have made the page queue big enough that it is inconceivable that it
		// could ever be full (if the page queue became full, it would mean that the writer thread added more than
		// 68bn items without any being read by the reader thread).
		data->pages[pageTail] = page;
		++pageTail;
		if (pageTail >= MAX_PAGES)
			pageTail = 0;
		data->tail = pageTail;

#if defined(PIPE_TESTING)
		i32 pageCount = pageTail - expiredHead;
		if (pageCount < 0)
			pageCount += MAX_PAGES;
		if (f)
			fprintf(f, "Creating page of size %d, %d pages exist\n", newSize, pageCount);
#endif //defined(PIPE_TESTING)
	}

	// Add the item to the selected page. We can now safely assume that the page is not full.
	page->elements[tail] = item;
	++tail;
	if (tail >= size)
		tail = 0;
	page->tail = tail;
}

template<typename T> inline void Pipe<T >::Swap(Pipe& other)
{
	// Basically this operation simply exchanges the m_data pointers of both instances. Note that
	// it is not safe to destruct either of the instances immediately, as the reader thread may
	// still be referring to the m_data object. Both instances must be destructed when only one thread
	// is in operation. This means that code like Pipe().Swap(myPipe) is a very bad idea (this is
	// an stl idiom that some people might be tempted to try).

	// NB that it is NOT safe to do a swap if BOTH instances are currently being read by other threads.
	// There is no way to guarantee that the right items go to the right threads in this case. It is
	// only safe to call Swap() on the instance that is being actively read, passing it the inactive
	// instance as the parameter.
	Data* data = m_data;
	m_data = other.m_data; // At this point both instances refer to the same data.
	other.m_data = data;
}

template<typename T> inline void Pipe<T >::Replace(const Item& item, const Item& replacement)
{
	PIPE_FUNCTION_PROFILER(PROFILE_PIPE_SYS);

	Data* data = m_data;
	if (!data)
		return;

	// In the code following, it is possible that we may end up searching through elements of the pages that
	// the reader thread has already read. However, they should still be in a valid state, since the reader thread
	// does not alter the items in the queue, and overwriting them should have no impact. Note that no new items will
	// be added to any pages, since we are the only writer thread.

	// Loop through all the pages that are still active, searching for any items to replace.
	i32 iterations = 0;
	for (i32 pageIndex = data->head, pageTail = data->tail; pageIndex != pageTail; pageIndex = (pageIndex < MAX_PAGES - 1 ? pageIndex + 1 : 0))
	{
		Page* page = data->pages[pageIndex];
		i32 head = page->head;
		i32 tail = page->tail;
		i32 size = page->size;

		for (i32 index = head; index != tail; index = (index < size - 1 ? index + 1 : 0))
		{
			++iterations;
			if (page->elements[index] == item)
				page->elements[index] = replacement;
		}
	}
}

template<typename T> inline void Pipe<T >::Clear()
{
	PIPE_FUNCTION_PROFILER(PROFILE_PIPE_SYS);

	Data* data = m_data;
	if (!data)
		return;

	// It is not safe to clear the data directly, and in particular it's not safe to delete any pages. A simple
	// way to clear the queue would be to move the head index down to the tail. However, this cannot be done
	// safely directly. Therefore we need another variable which contains the desired head, which the reader
	// thread can copy into the head. Yet this too is unsafe, since the reader thread wont know whether a skip
	// value is new or whether it is the same one it read last time. Therefore we use a monotonically increasing variable
	// to convey the desired skip position to the reader thread.
	for (i32 pageIndex = data->head, pageTail = data->tail; pageIndex != pageTail; pageIndex = (pageIndex < MAX_PAGES - 1 ? pageIndex + 1 : 0))
	{
		Page* page = data->pages[pageIndex];
		//i32 head = page->head;
		i32 tail = page->tail;
		i32 size = page->size;

		// Work out how far to increment the skip position. We want to move the head pointer to the tail pointer.
		// We cannot safely refer directly to the head variable, since it is , so we refer to the last head
		// value we conveyed to the reader thread.
		i32 skipDistance = tail - page->lastSkipHeadSend;
		page->lastSkipHeadSend = tail;
		if (skipDistance < 0)
			skipDistance += size;

#if defined(PIPE_TESTING)
		i32 skipReceive = page->skipReceive;
		i32 oldDistance = page->skipSend - skipReceive;
		if (oldDistance + skipDistance >= size)
		{
			static i32 s_size;
			static i32 s_skipDistance;
			static i32 s_tail;
			static i32 s_pageIndex;
			static i32 s_lastSkipHeadSend;
			static i32 s_oldDistance;
			static i32 s_skipReceive;
			s_size = size;
			s_skipDistance = skipDistance;
			s_tail = tail;
			s_pageIndex = pageIndex;
			s_lastSkipHeadSend = page->lastSkipHeadSend;
			s_oldDistance = oldDistance;
			s_skipReceive = skipReceive;

			//DrxDebugBreak();
		}
#endif //defined(PIPE_TESTING)

		// Convey this new skip value to the reader thread. We do this by adding the skip distance to the send variable
		// - the reader thread then adds this difference to its own copy of the forced head, and can then update the
		// variable accordingly. This is safe to do multiple times before the reader thread reacts, because the send
		// variable is monotonic, and so does not throw away information. Even if the variable wraps, the mechanism
		// works with differences, so it should be fine unless we want to skip more than 2bn elements before the reader
		// reacts.
		page->skipSend += skipDistance;

#if defined(PIPE_TESTING)
		page->dbgSkipDistance = skipDistance;
		page->dbgClearSize = size;
		page->dbgClearTail = tail;
#endif //defined(PIPE_TESTING)
	}
}

template<typename T> inline bool Pipe<T >::Pop(Item& item)
{
	PIPE_FUNCTION_PROFILER(PROFILE_PIPE_SYS);

	// Check whether we have been initialized.
	Data* data = m_data;
	if (!data)
		return false;

	// Find out how many pages exist.
	i32 pageHead = data->head;
	i32 pageTail = data->tail;
	i32 pageCount = pageTail - pageHead;
	if (pageCount < 0)
		pageCount += MAX_PAGES;

	// Loop through the pages until we find a non-empty one. If there are any empty ones,
	// move the head index past them. This will indicate to the writer thread that it can
	// safely delete those pages.
	Page* page = 0;
	i32 head = -1;
	i32 tail = -1;
	i32 size = 0;
	if (pageHead != pageTail)
	{
		for (;; )
		{
			Page* p = data->pages[pageHead];
			head = p->head;
			tail = p->tail;
			size = p->size;

			// It's possible that the writer thread wants us to skip some elements - this happens
			// when the queue is cleared, for example. The writer thread conveys the desired head
			// position to us by increasing the value of a variable - the difference is the amount
			// the head variable should be moved (based on the last time we were told to skip forward).
			u32 skipSend = p->skipSend;
			u32 skipDistance = skipSend - p->skipReceive;
			if (skipDistance)
			{
				p->skipReceive = skipSend;
				p->lastSkipHeadReceive = (p->lastSkipHeadReceive + skipDistance) % size;

				if (p->lastSkipHeadReceive >= size)
				{
					static u32 s_skipSend;
					static u32 s_skipDistance;
					static u32 s_head;
					static u32 s_size;
					static u32 s_tail;
					s_skipSend = skipSend;
					s_skipDistance = skipDistance;
					s_head = head;
					s_size = size;
					s_tail = tail;

					//DrxDebugBreak();
				}

				p->head = head = p->lastSkipHeadReceive; // I'm pretty sure it's impossible for our head to be ahead of this value...
			}

			if (head != tail)
			{
				page = p; // We have found a non-empty page.
				break;
			}

			// Don't skip the last page, otherwise it will be deleted and we will have to reallocate
			// it next push.
			i32 nextPageHead = (pageHead < MAX_PAGES - 1 ? pageHead + 1 : 0);
			if (nextPageHead == pageTail)
				break;
			pageHead = nextPageHead;
		}
	}
#if defined(PIPE_TESTING)
	lastPagesExpired = pageHead - data->head;
#endif //defined(PIPE_TESTING)
	data->head = pageHead;

	// If we found no non-empty page, then the queue is empty and we return nothing.
	if (!page)
		return false;

#if defined(PIPE_TESTING)
	lastHead = head;
	lastTail = tail;
	lastSize = size;
	lastPageCount = pageCount;
#endif //defined(PIPE_TESTING)

	// Get the first item of the selected page. It is now safe to assume that the page is not empty.
	item = page->elements[head];
	++head;
	if (head >= size)
		head = 0;
	page->head = head;
	return true;
}

#if defined(PIPE_TESTING)

namespace PipeTest
{
inline void TestPipeSingleThread()
{
	static i32k ITERATION_COUNT = 100;
	static i32k PUSH_COUNT = 100000;

	Pipe<i32> pipe;
	f = fopen("c:\\pipetestsinglethread.txt", "w");
	i32 currentValue = 0;
	for (i32 iteration = 0; iteration < ITERATION_COUNT; ++iteration)
	{
		fprintf(f, "\n******************************   ITERATION %d   **************************************\n", iteration);
		i32 pushCount = drx_random(0, PUSH_COUNT - 1);
		i32 popCount = drx_random(0, PUSH_COUNT - 1);

		for (i32 i = 0; i < pushCount; ++i)
		{
			fprintf(f, "Pushing: %d\n", currentValue);
			pipe.Push(currentValue++);
		}

		for (i32 i = 0; i < popCount; ++i)
		{
			i32 value;
			if (pipe.Pop(value))
				fprintf(f, "%sPopping: %d\n", indentation, value);
			else
				fprintf(f, "%sPopping: -\n", indentation);
		}

		fprintf(f, "%sClearing:\n", indentation);
		pipe.Clear();
		for (i32 i = 0; i < 2; ++i)
		{
			i32 value;
			if (pipe.Pop(value))
				fprintf(f, "%sPopping: %d\n", indentation, value);
			else
				fprintf(f, "%sPopping: -\n", indentation);
		}
	}
	fclose(f);
	f = 0;
}

struct TestPipeThreadData
{
	Pipe<i32>* pipe;
};

inline DWORD WINAPI TestPipeWriterThread(uk userData)
{
	static i32k MAX_VALUES = 100000000;

	if (f)
		fprintf(f, "Writer thread beginning\n");

	TestPipeThreadData* threadData = static_cast<TestPipeThreadData*>(userData);
	Pipe<i32>& pipe = *threadData->pipe;
	for (i32 i = 0; i < MAX_VALUES; ++i)
	{
		if (i % 100000 == 0)
		{
			if (f)
				fprintf(f, "Clearing queue\n");
			pipe.Clear();
		}
		//if (i % 100000 == 10)
		//{
		//	i32 replaceIndex = i - 9;
		//	if (f)
		//		fprintf(f, "Replacing %d\n", replaceIndex);
		//	pipe.Replace(replaceIndex, 0);
		//}

		pipe.Push(i);
	}

	if (f)
		fprintf(f, "Pushing -1\n");
	pipe.Push(-1);

	if (f)
		fprintf(f, "Writer thread exitting\n");

	return 0;
}

inline DWORD WINAPI TestPipeReaderThread(uk userData)
{
	TestPipeThreadData* threadData = static_cast<TestPipeThreadData*>(userData);
	Pipe<i32>& pipe = *threadData->pipe;

	if (f)
		fprintf(f, "%sReader thread beginning\n", indentation);

	static Pipe<i32>* TEST;
	TEST = &pipe;

	i32 lastReceivedValue = -1;
	for (;; )
	{
		i32 value;
		if (pipe.Pop(value))
		{
			if (value == -1)
			{
				if (f)
					fprintf(f, "%sReceived -1\n", indentation);
				break;
			}

			if (value - lastReceivedValue != 1)
			{
				if (pipe.lastPagesExpired)
				{
					pipe.lastPagesExpired = pipe.lastPagesExpired;
				}

				if (f)
					fprintf(f, "%sMissed %d items starting with %d (lastHead = %d, lastTail = %d, lastSize = %d, lastPageCount = %d, lastPagesExpired = %d)\n",
					        indentation, value - lastReceivedValue - 1, lastReceivedValue + 1, pipe.lastHead, pipe.lastTail, pipe.lastSize, pipe.lastPageCount, pipe.lastPagesExpired);
			}

			lastReceivedValue = value;
		}
	}

	if (f)
		fprintf(f, "%sReader thread exitting\n", indentation);

	return 0;
}

inline void TestPipeMultiThread()
{
	TestPipeThreadData threadData;
	Pipe<i32> pipe;
	threadData.pipe = &pipe;

	f = fopen("c:\\pipetestmultithread.txt", "w");

	HANDLE hWriterThread = CreateThread(0, 0, TestPipeWriterThread, &threadData, 0, 0);
	HANDLE hReaderThread = CreateThread(0, 0, TestPipeReaderThread, &threadData, 0, 0);

	HANDLE threads[] = { hWriterThread, hReaderThread };
	WaitForMultipleObjects(DRX_ARRAY_COUNT(threads), threads, TRUE, INFINITE);

	fclose(f);
	f = 0;
}

inline void TestPipe()
{
	//TestPipeSingleThread();
	TestPipeMultiThread();
}
}

#endif //defined(PIPE_TESTING)

}

//! \endcond