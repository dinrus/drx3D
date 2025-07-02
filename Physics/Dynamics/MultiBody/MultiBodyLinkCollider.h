#ifndef DRX3D_FEATHERSTONE_LINK_COLLIDER_H
#define DRX3D_FEATHERSTONE_LINK_COLLIDER_H

#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Maths/Linear/Serializer.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define MultiBodyLinkColliderData MultiBodyLinkColliderDoubleData
#define MultiBodyLinkColliderDataName "MultiBodyLinkColliderDoubleData"
#else
#define MultiBodyLinkColliderData MultiBodyLinkColliderFloatData
#define MultiBodyLinkColliderDataName "MultiBodyLinkColliderFloatData"
#endif

class MultiBodyLinkCollider : public CollisionObject2
{
	//protected:
public:
	MultiBody* m_multiBody;
	i32 m_link;

	virtual ~MultiBodyLinkCollider()
	{

	}
	MultiBodyLinkCollider(MultiBody* multiBody, i32 link)
		: m_multiBody(multiBody),
		  m_link(link)
	{
		m_checkCollideWith = true;
		//we need to remove the 'CF_STATIC_OBJECT' flag, otherwise links/base doesn't merge islands
		//this means that some constraints might point to bodies that are not in the islands, causing crashes
		//if (link>=0 || (multiBody && !multiBody->hasFixedBase()))
		{
			m_collisionFlags &= (~CollisionObject2::CF_STATIC_OBJECT);
		}
		// else
		//{
		//	m_collisionFlags |= (CollisionObject2::CF_STATIC_OBJECT);
		//}

		m_internalType = CO_FEATHERSTONE_LINK;
	}
	static MultiBodyLinkCollider* upcast(CollisionObject2* colObj)
	{
		if (colObj->getInternalType() & CollisionObject2::CO_FEATHERSTONE_LINK)
			return (MultiBodyLinkCollider*)colObj;
		return 0;
	}
	static const MultiBodyLinkCollider* upcast(const CollisionObject2* colObj)
	{
		if (colObj->getInternalType() & CollisionObject2::CO_FEATHERSTONE_LINK)
			return (MultiBodyLinkCollider*)colObj;
		return 0;
	}

	virtual bool checkCollideWithOverride(const CollisionObject2* co) const
	{
		const MultiBodyLinkCollider* other = MultiBodyLinkCollider::upcast(co);
		if (!other)
			return true;
		if (other->m_multiBody != this->m_multiBody)
			return true;
		if (!m_multiBody->hasSelfCollision())
			return false;

		//check if 'link' has collision disabled
		if (m_link >= 0)
		{
			const MultibodyLink& link = m_multiBody->getLink(this->m_link);
			if (link.m_flags & DRX3D_MULTIBODYLINKFLAGS_DISABLE_ALL_PARENT_COLLISION)
			{
				i32 parent_of_this = m_link;
				while (1)
				{
					if (parent_of_this == -1)
						break;
					parent_of_this = m_multiBody->getLink(parent_of_this).m_parent;
					if (parent_of_this == other->m_link)
					{
						return false;
					}
				}
			}
			else if (link.m_flags & DRX3D_MULTIBODYLINKFLAGS_DISABLE_PARENT_COLLISION)
			{
				if (link.m_parent == other->m_link)
					return false;
			}
		}

		if (other->m_link >= 0)
		{
			const MultibodyLink& otherLink = other->m_multiBody->getLink(other->m_link);
			if (otherLink.m_flags & DRX3D_MULTIBODYLINKFLAGS_DISABLE_ALL_PARENT_COLLISION)
			{
				i32 parent_of_other = other->m_link;
				while (1)
				{
					if (parent_of_other == -1)
						break;
					parent_of_other = m_multiBody->getLink(parent_of_other).m_parent;
					if (parent_of_other == this->m_link)
						return false;
				}
			}
			else if (otherLink.m_flags & DRX3D_MULTIBODYLINKFLAGS_DISABLE_PARENT_COLLISION)
			{
				if (otherLink.m_parent == this->m_link)
					return false;
			}
		}
		return true;
	}

	bool isStaticOrKinematic() const
	{
		return isStaticOrKinematicObject();
	}

	bool isKinematic() const
	{
		return isKinematicObject();
	}

	void setDynamicType(i32 dynamicType)
	{
		i32 oldFlags = getCollisionFlags();
		oldFlags &= ~(CollisionObject2::CF_STATIC_OBJECT | CollisionObject2::CF_KINEMATIC_OBJECT);
		setCollisionFlags(oldFlags | dynamicType);
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, class Serializer* serializer) const;
};

// clang-format off

struct	MultiBodyLinkColliderFloatData
{
	CollisionObject2FloatData m_colObjData;
	MultiBodyFloatData	*m_multiBody;
	i32			m_link;
	char		m_padding[4];
};

struct	MultiBodyLinkColliderDoubleData
{
	CollisionObject2DoubleData m_colObjData;
	MultiBodyDoubleData		*m_multiBody;
	i32			m_link;
	char		m_padding[4];
};

// clang-format on

SIMD_FORCE_INLINE i32 MultiBodyLinkCollider::calculateSerializeBufferSize() const
{
	return sizeof(MultiBodyLinkColliderData);
}

SIMD_FORCE_INLINE tukk MultiBodyLinkCollider::serialize(uk dataBuffer, class Serializer* serializer) const
{
	MultiBodyLinkColliderData* dataOut = (MultiBodyLinkColliderData*)dataBuffer;
	CollisionObject2::serialize(&dataOut->m_colObjData, serializer);

	dataOut->m_link = this->m_link;
	dataOut->m_multiBody = (MultiBodyData*)serializer->getUniquePointer(m_multiBody);

	// Fill padding with zeros to appease msan.
	memset(dataOut->m_padding, 0, sizeof(dataOut->m_padding));

	return MultiBodyLinkColliderDataName;
}

#endif  //DRX3D_FEATHERSTONE_LINK_COLLIDER_H
