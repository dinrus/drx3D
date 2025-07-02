// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ObjectUpr.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CObjectUpr::CObjectUpr()
		: m_nextObjectId(1)
		, m_bDestroying(false)
	{
		m_objects.reserve(256);
	}

	//////////////////////////////////////////////////////////////////////////
	CObjectUpr::~CObjectUpr()
	{
		m_bDestroying = true;

		TObjectMap objects;
		objects.swap(m_objects);

		for(TObjectMap::iterator iObject = objects.begin(), iEndObject = objects.end(); iObject != iEndObject; ++ iObject)
		{
			static_cast<CObject*>(iObject->second)->~CObject();
		}
		m_objectAllocator.FreeMemory();
	}

	//////////////////////////////////////////////////////////////////////////
	IObject* CObjectUpr::CreateObject(const SObjectParams& params)
	{
		DRX_ASSERT(params.pLibClass);
		DRX_ASSERT(!m_bDestroying);
		if(params.pLibClass && !m_bDestroying)
		{
			const ObjectId	objectId = m_nextObjectId ++;
			CObject*				pObject = new (m_objectAllocator.Allocate()) CObject(objectId, params);
			m_objects.insert(TObjectMap::value_type(objectId, pObject));
			return pObject;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::DestroyObject(IObject* pObject)
	{
		DRX_ASSERT(pObject != NULL);
		if(pObject != NULL)
		{
			TObjectMap::iterator it = m_objects.find(pObject->GetObjectId());
			if (it != m_objects.end())
			{
				m_objects.erase(it);
				pObject->~IObject();
				m_objectAllocator.Deallocate(static_cast<CObject*>(pObject));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IObject* CObjectUpr::GetObjectById(const ObjectId& objectId)
	{
		TObjectMap::iterator	iObject = m_objects.find(objectId);
		return iObject != m_objects.end() ? iObject->second : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::VisitObjects(const IObjectVisitor& visitor)
	{
		DRX_ASSERT(visitor);
		if(visitor)
		{
			for(TObjectMap::iterator iObject = m_objects.begin(), iEndObject = m_objects.end(); iObject != iEndObject; ++ iObject)
			{
				if(visitor(*iObject->second) != EVisitStatus::Continue)
				{
					return;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::VisitObjects(const IObjectConstVisitor& visitor) const
	{
		DRX_ASSERT(visitor);
		if(visitor)
		{
			for(TObjectMap::const_iterator iObject = m_objects.begin(), iEndObject = m_objects.end(); iObject != iEndObject; ++ iObject)
			{
				if(visitor(*iObject->second) != EVisitStatus::Continue)
				{
					return;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::SendSignal(const ObjectId& objectId, const SGUID& signalGUID, const TVariantConstArray& inputs)
	{
		IObject*	pObject = GetObjectById(objectId);
		if(pObject != NULL)
		{
			pObject->ProcessSignal(signalGUID, inputs);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::SendSignal(const ExplicitEntityId& entityId, const SGUID& signalGUID, const TVariantConstArray& inputs)
	{
		for(TObjectMap::iterator iObject = m_objects.begin(), iEndObject = m_objects.end(); iObject != iEndObject; ++ iObject)
		{
			CObject&	object = *iObject->second;
			if(object.GetEntityId() == entityId)
			{
				object.ProcessSignal(signalGUID, inputs);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::BroadcastSignal(const SGUID& signalGUID, const TVariantConstArray& inputs)
	{
		for(TObjectMap::iterator iObject = m_objects.begin(), iEndObject = m_objects.end(); iObject != iEndObject; ++ iObject)
		{
			iObject->second->ProcessSignal(signalGUID, inputs);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CObjectUpr::CPool::CPool()
		: m_iFirstFreeBucket(0)
	{
		for(u32 iBucket = 0; iBucket < POOL_SIZE; ++ iBucket)
		{
			SBucket&	bucket = m_buckets[iBucket];
			bucket.iNextFreeBucket	= iBucket + 1;
			bucket.salt							= 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool CObjectUpr::CPool::Allocate(uk & ptr, u32& iBucket, u32& salt)
	{
		if(m_iFirstFreeBucket < POOL_SIZE)
		{
			SBucket&	bucket = m_buckets[m_iFirstFreeBucket];
			ptr									= bucket.storage;
			iBucket							= m_iFirstFreeBucket;
			salt								= bucket.salt;
			m_iFirstFreeBucket	= bucket.iNextFreeBucket;
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::CPool::Free(u32 iBucket)
	{
		DRX_ASSERT(iBucket < POOL_SIZE);
		if(iBucket < POOL_SIZE)
		{
			SBucket&	bucket = m_buckets[iBucket];
			bucket.iNextFreeBucket	= m_iFirstFreeBucket;
			m_iFirstFreeBucket			= iBucket;
			++ bucket.salt;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	uk CObjectUpr::CPool::Get(u32 iBucket, u32 salt)
	{
		DRX_ASSERT(iBucket < POOL_SIZE);
		if(iBucket < POOL_SIZE)
		{
			SBucket&	bucket = m_buckets[iBucket];
			if(bucket.salt == salt)
			{
				return bucket.storage;
			}
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CObjectUpr::CreateObject(const SObjectParams& params, CObject*& pObject, ObjectId& objectId)
	{
		uk 		ptr = NULL;
		u32	iPool = 0;
		u32	iBucket = POOL_SIZE;
		u32	salt = 0;
		for(size_t poolCount = m_pools.size(); iPool < poolCount; ++ iPool)
		{
			if(m_pools[iPool]->Allocate(ptr, iBucket, salt))
			{
				break;
			}
		}
		if((ptr == NULL) && (iPool < MAX_POOL_COUNT))
		{
			CPoolPtr	pNewPool(new CPool());
			m_pools.push_back(pNewPool);
			pNewPool->Allocate(ptr, iBucket, salt);
		}
		DRX_ASSERT(ptr != NULL);
		if(ptr != NULL)
		{
			iPool		= (iPool << POOL_INDEX_SHIFT) & POOL_INDEX_MASK;
			iBucket	= (iBucket << BUCKET_INDEX_SHIFT) & BUCKET_INDEX_MASK;
			salt		= (salt << SALT_SHIFT) & SALT_MASK;
			u32k	rawObjectId = iPool | iBucket | salt;
			objectId.SetValue(rawObjectId);
			pObject = new (ptr) CObject(objectId, params);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void CObjectUpr::DestroyObject(const ObjectId& objectId)
	{
		u32k	rawObjectId = objectId.GetValue();
		u32k	iPool = (rawObjectId & POOL_INDEX_MASK) >> POOL_INDEX_SHIFT;
		u32k	iBucket = (rawObjectId & BUCKET_INDEX_MASK) >> BUCKET_INDEX_SHIFT;
		u32k	salt = (rawObjectId & SALT_MASK) >> SALT_SHIFT;
		DRX_ASSERT(iPool < m_pools.size());
		if(iPool < m_pools.size())
		{
			CPool&		pool = *m_pools[iPool];
			CObject*	pObject = static_cast<CObject*>(pool.Get(iBucket, salt));
			pObject->~CObject();
			pool.Free(iBucket);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CObject* CObjectUpr::GetObjectImpl(const ObjectId& objectId)
	{
		u32k	rawObjectId = objectId.GetValue();
		u32k	iPool = (rawObjectId & POOL_INDEX_MASK) >> POOL_INDEX_SHIFT;
		u32k	iBucket = (rawObjectId & BUCKET_INDEX_MASK) >> BUCKET_INDEX_SHIFT;
		u32k	salt = (rawObjectId & SALT_MASK) >> SALT_SHIFT;
		DRX_ASSERT(iPool < m_pools.size());
		if(iPool < m_pools.size())
		{
			return static_cast<CObject*>(m_pools[iPool]->Get(iBucket, salt));
		}
		return NULL;
	}
}
