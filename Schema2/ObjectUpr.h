// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

#include <drx3D/Schema2/IObjectUpr.h>

#include <drx3D/Schema2/Object.h>
#include <drx3D/Schema2/Lib.h>

// #SchematycTODO : Finish implementing object pool and look-up system!!!

namespace sxema2
{
	// Object manager.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CObjectUpr : public IObjectUpr
	{
	public:

		CObjectUpr();

		~CObjectUpr();

		// IObjectUpr
		virtual IObject* CreateObject(const SObjectParams& params) override;
		virtual void DestroyObject(IObject* pObject) override;
		virtual IObject* GetObjectById(const ObjectId& objectId) override;
		//virtual IObject* GetObjectByEntityId(const ExplicitEntityId& entityId) override;
		virtual void VisitObjects(const IObjectVisitor& visitor) override;
		virtual void VisitObjects(const IObjectConstVisitor& visitor) const override;
		virtual void SendSignal(const ObjectId& objectId, const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) override;
		virtual void SendSignal(const ExplicitEntityId& entityId, const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) override;
		virtual void BroadcastSignal(const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) override;
		// ~IObjectUpr

	private:

		static u32k POOL_SIZE						= 256;
		static u32k MAX_POOL_COUNT			= 256;
		static u32k POOL_INDEX_SHIFT		= 0;
		static u32k POOL_INDEX_MASK			= 0x000000ff;
		static u32k BUCKET_INDEX_SHIFT	= 8;
		static u32k BUCKET_INDEX_MASK		= 0x0000ff00;
		static u32k SALT_SHIFT					= 16;
		static u32k SALT_MASK						= 0x00ff0000;

		class CPool
		{
		public:

			CPool();

			bool Allocate(uk & ptr, u32& iBucket, u32& salt);
			void Free(u32 iBucket);
			uk Get(u32 iBucket, u32 salt);

		private:

			struct SBucket
			{
				union
				{
					u32	iNextFreeBucket;
					u8		storage[sizeof(CObject)];
				};

				u32	salt;
			};

			SBucket	m_buckets[POOL_SIZE];
			u32	m_iFirstFreeBucket;
		};

		DECLARE_SHARED_POINTERS(CPool)

		typedef stl::TPoolAllocator<CObject>	TObjectAllocator;
		typedef VectorMap<ObjectId, CObject*>	TObjectMap;
		typedef std::vector<CPoolPtr>					TPoolVector;

		bool CreateObject(const SObjectParams& params, CObject*& pObject, ObjectId& objectId);
		void DestroyObject(const ObjectId& objectId);
		CObject* GetObjectImpl(const ObjectId& objectId);

		ObjectId            m_nextObjectId;
		TObjectAllocator    m_objectAllocator;
		TObjectMap          m_objects;
		TPoolVector         m_pools;
		bool                m_bDestroying;
	};
}
