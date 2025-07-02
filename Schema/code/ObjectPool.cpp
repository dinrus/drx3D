// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ObjectPool.h>

#include <drx3D/Schema/IRuntimeClass.h>

#include <drx3D/Schema/Core.h>
#include <drx3D/Schema/Object.h>
#include <drx3D/Schema/RuntimeRegistry.h>

namespace sxema
{
bool CObjectPool::CreateObject(const sxema::SObjectParams& params, IObject*& pObjectOut)
{
	CRuntimeClassConstPtr pClass = CCore::GetInstance().GetRuntimeRegistryImpl().GetClassImpl(params.classGUID);
	if (pClass)
	{
		if (m_freeSlots.empty() && !AllocateSlots(m_slots.size() + 1))
		{
			return false;
		}

		u32k slodIdx = m_freeSlots.back();
		m_freeSlots.pop_back();
		SSlot& slot = m_slots[slodIdx];

		CObjectPtr pObject = std::make_shared<CObject>(*params.pEntity, slot.objectId, params.pCustomData);
		if (pObject->Init(params.classGUID, params.pProperties))
		{
			slot.pObject = pObject;
			pObjectOut = pObject.get();
			return true;
		}
		else
		{
			m_freeSlots.push_back(slodIdx);
		}
	}
	return false;
}

IObject* CObjectPool::GetObject(ObjectId objectId)
{
	u32 slotIdx;
	u32 salt;
	if (ExpandObjectId(objectId, slotIdx, salt))
	{
		return m_slots[slotIdx].pObject.get();
	}
	return nullptr;
}

void CObjectPool::DestroyObject(ObjectId objectId)
{
	u32 slotIdx;
	u32 salt;
	if (ExpandObjectId(objectId, slotIdx, salt))
	{
		SSlot& slot = m_slots[slotIdx];
		slot.pObject.reset();
		slot.objectId = CreateObjectId(slotIdx, IncrementSalt(salt));
		m_freeSlots.push_back(slotIdx);
	}
}

void CObjectPool::SendSignal(ObjectId objectId, const SObjectSignal& signal)
{
	IObject* pObject = GetObject(objectId);
	if (pObject)
	{
		pObject->ProcessSignal(signal);
	}
}

void CObjectPool::BroadcastSignal(const SObjectSignal& signal)
{
	for (SSlot& slot : m_slots)
	{
		if (slot.pObject)
		{
			slot.pObject->ProcessSignal(signal);
		}
	}
}

bool CObjectPool::AllocateSlots(u32 slotCount)
{
	u32k prevSlotCount = m_slots.size();
	if (slotCount > prevSlotCount)
	{
		u32k maxSlotCount = 0xffff;
		if (slotCount > maxSlotCount)
		{
			return false;
		}

		u32k minSlotCount = 100;
		slotCount = max(max(slotCount, min(prevSlotCount * 2, maxSlotCount)), minSlotCount);

		m_slots.resize(slotCount);
		m_freeSlots.reserve(slotCount);

		for (u32 slotIdx = prevSlotCount; slotIdx < slotCount; ++slotIdx)
		{
			m_slots[slotIdx].objectId = CreateObjectId(slotIdx, 0);
			m_freeSlots.push_back(slotIdx);
		}
	}
	return true;
}

ObjectId CObjectPool::CreateObjectId(u32 slotIdx, u32 salt) const
{
	return static_cast<ObjectId>((slotIdx << 16) | salt);
}

bool CObjectPool::ExpandObjectId(ObjectId objectId, u32& slotIdx, u32& salt) const
{
	u32k value = static_cast<u32>(objectId);
	slotIdx = value >> 16;
	salt = value & 0xffff;
	return (slotIdx < m_slots.size()) && (m_slots[slotIdx].objectId == objectId);
}

u32 CObjectPool::IncrementSalt(u32 salt) const
{
	u32k maxSalt = 0x7fff;
	if (++salt > maxSalt)
	{
		salt = 0;
	}
	return salt;
}
} // sxema
