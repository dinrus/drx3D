
//tinyRendererPlugin implements the TinyRenderer as a plugin
//it is statically linked when using preprocessor #define STATIC_LINK_VR_PLUGIN
//otherwise you can dynamically load it using pybullet.loadPlugin

#include "collisionFilterPlugin.h"
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Plugins/PluginContext.h>
#include <stdio.h>
#include <drx3D/Common/b3HashMap.h>

#include <drx3D/Plugins/b3PluginCollisionInterface.h>

struct b3CustomCollisionFilter
{
	i32 m_objectUniqueIdA;
	i32 m_linkIndexA;
	i32 m_objectUniqueIdB;
	i32 m_linkIndexB;
	bool m_enableCollision;

	D3_FORCE_INLINE u32 getHash() const
	{
		i32 obA = (m_objectUniqueIdA & 0xff);
		i32 obB = ((m_objectUniqueIdB & 0xf) << 8);
		i32 linkA = ((m_linkIndexA & 0xff) << 16);
		i32 linkB = ((m_linkIndexB & 0xff) << 24);
		z64 key = obA + obB + linkA + linkB;
		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return (i32) key;
	}
	bool equals(const b3CustomCollisionFilter& other) const
	{
		return m_objectUniqueIdA == other.m_objectUniqueIdA &&
			   m_objectUniqueIdB == other.m_objectUniqueIdB &&
			   m_linkIndexA == other.m_linkIndexA &&
			   m_linkIndexB == other.m_linkIndexB;
	}
};

struct DefaultPluginCollisionInterface : public b3PluginCollisionInterface
{
	b3HashMap<b3CustomCollisionFilter, b3CustomCollisionFilter> m_customCollisionFilters;

	virtual void setBroadphaseCollisionFilter(
		i32 objectUniqueIdA, i32 objectUniqueIdB,
		i32 linkIndexA, i32 linkIndexB,
		bool enableCollision)
	{
		b3CustomCollisionFilter keyValue;
		keyValue.m_objectUniqueIdA = objectUniqueIdA;
		keyValue.m_linkIndexA = linkIndexA;
		keyValue.m_objectUniqueIdB = objectUniqueIdB;
		keyValue.m_linkIndexB = linkIndexB;
		keyValue.m_enableCollision = enableCollision;

		if (objectUniqueIdA > objectUniqueIdB)
		{
			b3Swap(keyValue.m_objectUniqueIdA, keyValue.m_objectUniqueIdB);
			b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
		}
		if (objectUniqueIdA == objectUniqueIdB)
		{
			if (keyValue.m_linkIndexA > keyValue.m_linkIndexB)
			{
				b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
			}
		}

		m_customCollisionFilters.insert(keyValue, keyValue);
	}

	virtual void removeBroadphaseCollisionFilter(
		i32 objectUniqueIdA, i32 objectUniqueIdB,
		i32 linkIndexA, i32 linkIndexB)
	{
		b3CustomCollisionFilter keyValue;
		keyValue.m_objectUniqueIdA = objectUniqueIdA;
		keyValue.m_linkIndexA = linkIndexA;
		keyValue.m_objectUniqueIdB = objectUniqueIdB;
		keyValue.m_linkIndexB = linkIndexB;

		if (objectUniqueIdA > objectUniqueIdB)
		{
			b3Swap(keyValue.m_objectUniqueIdA, keyValue.m_objectUniqueIdB);
			b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
		}
		if (objectUniqueIdA == objectUniqueIdB)
		{
			if (keyValue.m_linkIndexA > keyValue.m_linkIndexB)
			{
				b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
			}
		}

		m_customCollisionFilters.remove(keyValue);
	}

	virtual i32 getNumRules() const
	{
		return m_customCollisionFilters.size();
	}

	virtual void resetAll()
	{
		m_customCollisionFilters.clear();
	}

	virtual i32 needsBroadphaseCollision(i32 objectUniqueIdA, i32 linkIndexA,
										 i32 collisionFilterGroupA, i32 collisionFilterMaskA,
										 i32 objectUniqueIdB, i32 linkIndexB,
										 i32 collisionFilterGroupB, i32 collisionFilterMaskB,
										 i32 filterMode)
	{
		//check and apply any custom rules for those objects/links
		b3CustomCollisionFilter keyValue;
		keyValue.m_objectUniqueIdA = objectUniqueIdA;
		keyValue.m_linkIndexA = linkIndexA;
		keyValue.m_objectUniqueIdB = objectUniqueIdB;
		keyValue.m_linkIndexB = linkIndexB;

		if (objectUniqueIdA > objectUniqueIdB)
		{
			b3Swap(keyValue.m_objectUniqueIdA, keyValue.m_objectUniqueIdB);
			b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
		}
		if (objectUniqueIdA == objectUniqueIdB)
		{
			if (keyValue.m_linkIndexA > keyValue.m_linkIndexB)
			{
				b3Swap(keyValue.m_linkIndexA, keyValue.m_linkIndexB);
			}
		}

		b3CustomCollisionFilter* filter = m_customCollisionFilters.find(keyValue);
		if (filter)
		{
			return filter->m_enableCollision;
		}

		//otherwise use the default fallback

		if (filterMode == D3_FILTER_GROUPAMASKB_AND_GROUPBMASKA)
		{
			bool collides = (collisionFilterGroupA & collisionFilterMaskB) != 0;
			collides = collides && (collisionFilterGroupB & collisionFilterMaskA);
			return collides;
		}

		if (filterMode == D3_FILTER_GROUPAMASKB_OR_GROUPBMASKA)
		{
			bool collides = (collisionFilterGroupA & collisionFilterMaskB) != 0;
			collides = collides || (collisionFilterGroupB & collisionFilterMaskA);
			return collides;
		}
		return false;
	}
};

struct CollisionFilterMyClass
{
	i32 m_testData;

	DefaultPluginCollisionInterface m_collisionFilter;

	CollisionFilterMyClass()
		: m_testData(42)
	{
	}
	virtual ~CollisionFilterMyClass()
	{
	}
};

DRX3D_SHARED_API i32 initPlugin_collisionFilterPlugin(struct PluginContext* context)
{
	CollisionFilterMyClass* obj = new CollisionFilterMyClass();
	context->m_userPointer = obj;
	return SHARED_MEMORY_MAGIC_NUMBER;
}

DRX3D_SHARED_API struct b3PluginCollisionInterface* getCollisionInterface_collisionFilterPlugin(struct PluginContext* context)
{
	CollisionFilterMyClass* obj = (CollisionFilterMyClass*)context->m_userPointer;
	return &obj->m_collisionFilter;
}

DRX3D_SHARED_API i32 executePluginCommand_collisionFilterPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments)
{
	return 0;
}

DRX3D_SHARED_API void exitPlugin_collisionFilterPlugin(struct PluginContext* context)
{
	CollisionFilterMyClass* obj = (CollisionFilterMyClass*)context->m_userPointer;
	delete obj;
	context->m_userPointer = 0;
}
