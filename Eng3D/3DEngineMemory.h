// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   3DEngineMemory.h
//  Version:     v1.00
//  Created:     23/04/2010 by Chris Raine.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////

#ifndef Engine3DMemory_H
#define Engine3DMemory_H

// The type of pool responsible for temporary allocations within the 3dengine
//
// Note: The header is included here for reasons of devirtualization. If
// included directly from the precompiled header in 3dEngine, the gamedll will
// fail to compile!
#include <drx3D/CoreX/Memory/Pool/PoolAlloc.h>
#include <drx3D/Sys/InplaceFactory.h>
#include <drx3D/CoreX/StlUtils.h>

using NDrxPoolAlloc::CFirstFit;        // speed of allocations are crucial, so simply use the first fitting free allocation
using NDrxPoolAlloc::CInPlace;         //
using NDrxPoolAlloc::CMemoryDynamic;   // the pool itself will be dynamically allocated
using NDrxPoolAlloc::CListItemInPlace; // use inplace items

// Tempororary Pool Holder
class CTemporaryPool
{
private:
	// Access granted for 3dEngine to create, destroy and maintain the temporary
	// pool for the 3d engine
	friend class C3DEngine;

	// The static pool instance - one pool to rule them all (temp allocations at least)
	static CTemporaryPool* s_Instance;

	// The type of the backing temporary pool
	typedef CFirstFit<CInPlace<CMemoryDynamic>, CListItemInPlace> TTemporaryPool;
	TTemporaryPool Pool;

	// A non-recursive critical section guards the pool against concurrent access
	typedef DrxCriticalSectionNonRecursive TTemporaryPoolLock;
	TTemporaryPoolLock Lock;

	// Initialize the pool manager.
	//
	// Allocates the backing storage and initializes the temporary pool
	// itself. The backing storage is aligned to 16 bytes to reduce the amount of
	// cachelines crossed by the temporary pool
	static bool Initialize(size_t poolSize)
	{
		// Create the object instance
		s_Instance = new CTemporaryPool();
		if (!s_Instance)
		{
			DrxFatalError("CTemporaryPool::Init(): could not create an instance of CTemporaryPool");
			return false;
		}

		// Allocate the backing storage
		u8* tempPool = reinterpret_cast<u8*>(DrxModuleMemalign(poolSize, 16));
		if (!tempPool)
		{
			DrxFatalError("CTemporaryPool::Init(): could not allocate %" PRISIZE_T " bytes for temportary pool", poolSize);
			return false;
		}

		// Initialize the actual pool
		s_Instance->Pool.InitMem(poolSize, tempPool);
		return true;
	}

	// Shutdown the temporary pool manager.
	//
	// Frees the temporary pool
	static bool Shutdown()
	{
		if (s_Instance == NULL)
		{
			DrxFatalError("CTemporaryPool::Shutdown(): no temporary pool instance present");
			return false;
		}

		bool error = false;
		CTemporaryPool& instance = *s_Instance;
		if (instance.Pool.Data())
			DrxModuleMemalignFree(instance.Pool.Data());
		else
			error = true;

		delete s_Instance;
		s_Instance = NULL;
		return !error;
	}

	// Templated construct helper member function using an inplace factory
	//
	// Called from the templated New<T, Expr> function below. Returns a typed
	// pointer to the inplace constructed object.
	template<typename T, typename InPlaceFactory>
	T* Construct(const InPlaceFactory& factory, uk storage)
	{
		return reinterpret_cast<T*>(factory.template apply<T>(storage));
	}

	// Templated destruct helper member function.
	//
	// Calls the object's destructor and returns a void pointer to the storage
	template<typename T>
	uk Destruct(T* obj)
	{
		obj->~T();
		return reinterpret_cast<uk>(obj);
	}

	// Empty private constructor/destructors to prevent clients from creating and
	// destroying instances of CTemporaryPool (there should only be one instance
	// in the 3DEngine).
	CTemporaryPool() {};
	~CTemporaryPool() {};

public:

	// Allocate a block of memory with the given size and alignment
	uk Allocate(size_t size, size_t align)
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, Lock);
		uk pData = Pool.Allocate<uk>(size, align);
		if (pData == NULL)
		{
			DrxFatalError("**** could not allocate %" PRISIZE_T " bytes from temporary pool", size);
		}
		return Pool.Resolve<uk>(pData);
	};

	// Allocates memory and constructs object of type 'T'
	//
	// Note: This method is respects the alignment of 'T' via C99 alignof()
	template<typename T, typename Expr>
	T* New(const Expr& expr)
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, Lock);
		uk pObjStorage = Pool.Allocate<uk>(sizeof(T), alignof(T));
		if (pObjStorage == NULL)
		{
			DrxFatalError("**** could not allocate %d bytes from temporary pool",
			              (i32)sizeof(T));
		}
		return Construct<T>(expr, pObjStorage);
	};

	// Allocates memory and constructs object of type 'T'
	//
	// Note: This method is respects the alignment of 'T' via C99 alignof()
	template<typename T>
	T* New()
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, Lock);
		uk pObjStorage = Pool.Allocate<uk>(sizeof(T), alignof(T));
		if (pObjStorage == NULL)
		{
			DrxFatalError("**** could not allocate %d bytes from temporary pool",
			              (i32)sizeof(T));
		}
		return Construct<T>(InplaceFactory(), pObjStorage);
	};

	// Frees a block of memory from the temporary pool
	//
	void Free(uk ptr)
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, Lock);
		Pool.Free(ptr);
	}

	// Destroys an object of type 'T' and frees the underlying block of memory
	template<typename T>
	void Delete(T* ptr)
	{
		AUTO_LOCK_T(DrxCriticalSectionNonRecursive, Lock);
		Pool.Free(Destruct<T>(ptr));
	}

	// Static function to retrieve the static instance of CTemporaryPool
	static CTemporaryPool* Get() { return s_Instance; };

	void                   GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(Pool.Data(), Pool.MemSize());
	}
};

// A stl compliant scratch allocator that uses the given temporary pool.
template<class Type>
class scratch_allocator : public stl::SAllocatorConstruct
{
public:
	typedef Type              value_type;
	typedef value_type*       pointer;
	typedef const value_type* const_pointer;
	typedef value_type&       reference;
	typedef const value_type& const_reference;
	typedef size_t            size_type;
	typedef ptrdiff_t         difference_type;

	template<class value_type1> struct rebind
	{
		typedef scratch_allocator<value_type1> other;
	};

	scratch_allocator() {}
	template<class value_type1> scratch_allocator(const scratch_allocator<value_type1>&) {}
	scratch_allocator(const scratch_allocator<value_type>&) {}
	~scratch_allocator() {}

	pointer       address(reference x) const       { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	// Note: size can be zero - return value will be null in that case
	value_type* allocate(size_type n, ukk = 0)
	{
		if (n != 0)
		{
			size_type buf_size = n * sizeof(value_type);

			uk ret = CTemporaryPool::Get()->Allocate(
			  buf_size,
			  alignof(value_type));

			return reinterpret_cast<value_type*>(ret);
		}
		return 0;
	}

	// Note: size can be zero.
	void deallocate(pointer p, size_type n)
	{
		if (p != NULL)
		{
			CTemporaryPool::Get()->Free(p);
		}
	}

	size_type max_size() const           { return size_t(-1) / sizeof(value_type); }

	void      destroy(pointer p)         { p->~value_type(); }

	void      cleanup()                  {}

	size_t    get_heap_size()            { return 0; }

	size_t    get_wasted_in_allocation() { return 0; }

	size_t    get_wasted_in_blocks()     { return 0; }
};

// A scratch vector type to use the stl vector
template<typename Type>
class scratch_vector : public std::vector<Type, scratch_allocator<Type>>
{
};

namespace util
{
extern uk pool_allocate(size_t nSize);
extern void  pool_free(uk ptr);
}

#endif
