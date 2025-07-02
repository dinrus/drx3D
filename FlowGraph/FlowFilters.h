// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowFilters.h
//  Version:     v1.00
//  Created:     27/03/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Some Inspector filters
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __FLOWFILTERS_H__
#define __FLOWFILTERS_H__

#include <drx3D/FlowGraph/IFlowSystem.h>

typedef enum
{
	eFF_AIAction,
	eFF_Entity,
	eFF_Node
} TFlowInspectorFilter;

class CFlowFilterBase : public IFlowGraphInspector::IFilter
{
protected:
	CFlowFilterBase(TFlowInspectorFilter type) : m_type(type), m_refs(0) {}

public:
	virtual void AddRef()
	{
		++m_refs;
	}

	virtual void Release()
	{
		if (--m_refs <= 0)
			delete this;
	}

	virtual TFlowInspectorFilter GetTypeId() const
	{
		return m_type;
	}

	virtual void AddFilter(IFlowGraphInspector::IFilterPtr pFilter)
	{
		stl::push_back_unique(m_filters, pFilter);
	}

	virtual void RemoveFilter(IFlowGraphInspector::IFilterPtr pFilter)
	{
		stl::find_and_erase(m_filters, pFilter);
	}

	virtual const IFlowGraphInspector::IFilter_AutoArray& GetFilters() const
	{
		return m_filters;
	}

protected:
	TFlowInspectorFilter                   m_type;
	i32                                    m_refs;
	IFlowGraphInspector::IFilter_AutoArray m_filters;
public:
	// quick/dirty factory by name
	// but actual CFlowFilter instances can also be instantiated explicetly
	static CFlowFilterBase* CreateFilter(tukk filterType);
};

class CFlowFilterEntity : public CFlowFilterBase
{
public:
	static tukk GetClassName() { return "Entity"; }

	// filter lets only pass flows which happen in this entity's graph
	CFlowFilterEntity();
	void                  SetEntity(EntityId entityId) { m_entityId = entityId; }
	EntityId              GetEntity() const            { return m_entityId; }

	virtual EFilterResult Apply(IFlowGraph* pGraph, const SFlowAddress from, const SFlowAddress to);

protected:
	EntityId m_entityId;
};

class CFlowFilterAIAction : public CFlowFilterBase
{
public:
	static tukk GetClassName() { return "AIAction"; }

	// filter lets only pass AIAction flows for a specific AIAction
	// if ActionName is empty works on all AIActions
	// optionally works on specific user class, object class, user ID, objectID
	// all can be mixed
	CFlowFilterAIAction();
	void                  SetAction(const string& actionName) { m_actionName = actionName; }
	const string&         GetAction() const                   { return m_actionName; }

	void                  SetParams(const string& userClass, const string& objectClass, EntityId userId = 0, EntityId objectId = 0);
	void                  GetParams(string& userClass, EntityId& userId, string& objectClass, EntityId& objectId) const;

	virtual EFilterResult Apply(IFlowGraph* pGraph, const SFlowAddress from, const SFlowAddress to);

protected:
	string   m_actionName;
	string   m_userClass;
	string   m_objectClass;
	EntityId m_userId;    // will be 0 if we run on all entity classes
	EntityId m_objectId;  // will be 0 if we run on all entity classes
};

class CFlowFilterNode : public CFlowFilterBase
{
public:
	static tukk GetClassName() { return "Node"; }

	// filter lets only pass flows for nodes of a specific type and node id
	CFlowFilterNode();
	void                  SetNodeType(const string& nodeTypeName) { m_nodeType = nodeTypeName; }
	const string&         GetNodeType() const                     { return m_nodeType; }

	void                  SetParams(const TFlowNodeId id)         { m_nodeId = id; }
	void                  GetParams(TFlowNodeId& id) const        { id = m_nodeId; }

	virtual EFilterResult Apply(IFlowGraph* pGraph, const SFlowAddress from, const SFlowAddress to);

protected:
	string      m_nodeType;
	TFlowNodeId m_nodeId;
};

#endif
