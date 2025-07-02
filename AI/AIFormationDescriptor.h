// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AIFormationDescriptor.h
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 9:2:2005   15:41 : Created by Kirill Bulatsev

 *********************************************************************/

#ifndef __AIFormationDescriptor_H__
#define __AIFormationDescriptor_H__

// Unit classes for group behavior.
#define UNIT_CLASS_UNDEFINED    1
#define UNIT_CLASS_LEADER       (1 << 1)
#define UNIT_CLASS_INFANTRY     (1 << 2)
#define UNIT_CLASS_SCOUT        (1 << 3)
#define UNIT_CLASS_ENGINEER     (1 << 4)
#define UNIT_CLASS_MEDIC        (1 << 5)
#define UNIT_CLASS_CIVILIAN     (1 << 6)
#define UNIT_CLASS_COMPANION    (1 << 7)
#define SHOOTING_SPOT_POINT     (1 << 15)
#define SPECIAL_FORMATION_POINT (1 << 16)
#define UNIT_ALL                0xffffffff

typedef struct FormationNode
{
	FormationNode() : vOffset(0, 0, 0), vSightDirection(0, 0, 0), fFollowDistance(0),
		fFollowOffset(0), fFollowDistanceAlternate(0), fFollowOffsetAlternate(0),
		eClass(UNIT_CLASS_UNDEFINED), fFollowHeightOffset(0){}

	Vec3  vOffset;
	Vec3  vSightDirection;

	float fFollowDistance;
	float fFollowOffset;
	float fFollowDistanceAlternate;
	float fFollowOffsetAlternate;
	float fFollowHeightOffset;

	i32   eClass;
} FormationNode;

class CFormationDescriptor
{
public:
	typedef std::vector<FormationNode> TVectorOfNodes;
	string m_sName;
	// Francesco TODO: the best would be to always use the crc32 and use
	// the string member only to expose the formation names to Sandbox.
	u32   m_nNameCRC32;
	TVectorOfNodes m_Nodes;
public:
	CFormationDescriptor() {};
	void AddNode(const FormationNode& nodeDescriptor);
	i32  GetNodeClass(i32 i) const
	{
		if (i < (i32)m_Nodes.size() && i >= 0)
			return m_Nodes[i].eClass;
		return -1;
	}
	void  Clear() { m_Nodes.resize(0); };
	float GetNodeDistanceToOwner(const FormationNode& nodeDescriptor) const;

	template<typename Sizer>
	void GetMemoryUsage(Sizer* pSizer) const
	{
		pSizer->AddObject(m_sName);
		pSizer->AddContainer(m_Nodes);
	}
};

#endif // __AIFormationDescriptor_H__
