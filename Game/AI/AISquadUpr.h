// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef AISquadUpr_h
#define AISquadUpr_h

#include <drx3D/AI/IClusterDetector.h>

typedef ClusterId SquadId;
#define UnknownSquadId SquadId(~0)

class SquadScopeID
{
public:
	SquadScopeID()
		: m_nameCRC(0)
	{
	}

	SquadScopeID(tukk name)
	{
#ifdef DRXAISYSTEM_DEBUG
		m_debugName = name;
#endif
		m_nameCRC = CCrc32::ComputeLowercase(name);
	}

	bool operator == (const SquadScopeID& rhs) const
	{
		return m_nameCRC == rhs.m_nameCRC;
	}

private:
#ifdef DRXAISYSTEM_DEBUG
	stack_string m_debugName;
#endif
	u32 m_nameCRC;
};

struct SquadAgent
{
	SquadAgent(const EntityId _entityId)
	:entityId(_entityId)
	,squadID(~0)
	{
	}

	bool IsInScope(const SquadScopeID& scopeId) const
	{
		return std::find(enteredScopes.begin(),enteredScopes.end(),scopeId) != enteredScopes.end();
	}

	EntityId entityId;
	typedef std::vector<SquadScopeID> EnteredScopes;
	EnteredScopes enteredScopes;
	SquadId squadID;
};

class AISquadUpr
{
public:
	AISquadUpr();
	~AISquadUpr();

	void RegisterAgent( const EntityId entityId );
	void UnregisterAgent( const EntityId entityId );

	bool EnterSquadScope( const SquadScopeID& scopeId, const EntityId entityId, u32k concurrentUserAllowed );
	void LeaveSquadScope( const SquadScopeID& scopeId, const EntityId entityId );

	void SendSquadEvent( const EntityId sourcEntityId, tukk eventName);

	typedef std::vector<EntityId> MemberIDArray;
	void GetSquadMembers( const SquadId squadId, MemberIDArray& members ) const;
	void GetSquadMembersInScope( const SquadId squadId, const SquadScopeID& squadScopeID, MemberIDArray& members ) const;
	u32 GetSquadMemberCount( const SquadId squadId ) const;

	SquadId GetSquadIdForAgent(const EntityId entityId) const;

	void Reset();
	void RequestSquadsUpdate();
	
	void Init();	
	void Update(float frameDeltaTime);

	// Utility functions for a specific game purpose
	IEntity* GetFormationLeaderForMySquad(const EntityId requesterId);

private:

	void ClusterDetectorCallback(IClusterRequest* result);

	u32 GetCurrentSquadMatesInScope(const SquadId squadId, const SquadScopeID& squadScopeId) const;

	void DebugDraw() const;
	ColorB GetColorForCluster(SquadId squadID) const;
	typedef std::map<EntityId, SquadAgent> SquadAgentMap;
	SquadAgentMap m_registeredAgents;
	float m_timeAccumulator; // Needs to be change to avoid hit-store penalty
	size_t m_totalSquads;
};

#endif //AISquadUpr_h