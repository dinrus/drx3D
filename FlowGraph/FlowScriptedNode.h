// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWSCRIPTEDNODE_H__
#define __FLOWSCRIPTEDNODE_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>

class CFlowScriptedNodeFactory;
TYPEDEF_AUTOPTR(CFlowScriptedNodeFactory);
typedef CFlowScriptedNodeFactory_AutoPtr CFlowScriptedNodeFactoryPtr;
class CFlowSimpleScriptedNodeFactory;
TYPEDEF_AUTOPTR(CFlowSimpleScriptedNodeFactory);
typedef CFlowSimpleScriptedNodeFactory_AutoPtr CFlowSimpleScriptedNodeFactoryPtr;

class CFlowScriptedNode : public IFlowNode
{
public:
	CFlowScriptedNode(const SActivationInfo*, CFlowScriptedNodeFactoryPtr, SmartScriptTable);
	~CFlowScriptedNode();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo);
	virtual void         GetConfiguration(SFlowNodeConfig&);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo*);
	virtual bool         SerializeXML(SActivationInfo*, const XmlNodeRef& root, bool reading);
	virtual void         Serialize(SActivationInfo*, TSerialize ser);
	virtual void         PostSerialize(SActivationInfo*) {}
	virtual void         GetMemoryUsage(IDrxSizer* s) const;
	// ~IFlowNode

	i32 ActivatePort(IFunctionHandler* pH, size_t nOutput, const TFlowInputData& data);

private:
	SActivationInfo             m_info;
	SmartScriptTable            m_table;
	CFlowScriptedNodeFactoryPtr m_pFactory;
};

class CFlowScriptedNodeFactory : public IFlowNodeFactory
{
public:
	CFlowScriptedNodeFactory();
	~CFlowScriptedNodeFactory();

	bool                 Init(tukk path, tukk name);

	virtual IFlowNodePtr Create(IFlowNode::SActivationInfo*);

	ILINE size_t         NumInputs() const      { return m_inputs.size() - 1; }
	ILINE tukk    InputName(i32 n) const { return m_inputs[n].name; }
	void                 GetConfiguration(SFlowNodeConfig&);

	virtual void         GetMemoryUsage(IDrxSizer* s) const;

	void                 Reset() {}

private:
	SmartScriptTable               m_table;

	std::set<string>               m_stringTable;
	std::vector<SInputPortConfig>  m_inputs;
	std::vector<SOutputPortConfig> m_outputs;
	u32                         m_category;

	const string& AddString(tukk str);

	static i32    ActivateFunction(IFunctionHandler* pH, uk pBuffer, i32 nSize);
};

class CFlowSimpleScriptedNode : public IFlowNode
{
public:
	CFlowSimpleScriptedNode(const SActivationInfo*, CFlowSimpleScriptedNodeFactoryPtr);
	~CFlowSimpleScriptedNode();

	// IFlowNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo);
	virtual void         GetConfiguration(SFlowNodeConfig&);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo*);
	virtual bool         SerializeXML(SActivationInfo*, const XmlNodeRef& root, bool reading);
	virtual void         Serialize(SActivationInfo*, TSerialize ser);
	virtual void         PostSerialize(SActivationInfo*) {};
	// ~IFlowNode

	i32          ActivatePort(IFunctionHandler* pH, size_t nOutput, const TFlowInputData& data);

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

private:
	CFlowSimpleScriptedNodeFactoryPtr m_pFactory;
};

class CFlowSimpleScriptedNodeFactory : public IFlowNodeFactory
{
public:
	CFlowSimpleScriptedNodeFactory();
	~CFlowSimpleScriptedNodeFactory();

	bool                 Init(tukk path, tukk name);

	virtual IFlowNodePtr Create(IFlowNode::SActivationInfo*);

	ILINE size_t         NumInputs() const      { return m_inputs.size() - 1; }
	ILINE tukk    InputName(i32 n) const { return m_inputs[n].name; }
	ILINE i32            GetActivateFlags()     { return activateFlags; }
	void                 GetConfiguration(SFlowNodeConfig&);

	bool                 CallFunction(IFlowNode::SActivationInfo* pInputData);
	HSCRIPTFUNCTION      GetFunction() { return m_func; }

	virtual void         GetMemoryUsage(IDrxSizer* s) const;

	void                 Reset() {}

private:
	HSCRIPTFUNCTION                m_func;

	std::set<string>               m_stringTable;
	std::vector<SInputPortConfig>  m_inputs;
	std::vector<SOutputPortConfig> m_outputs;
	std::vector<ScriptAnyValue>    m_outputValues;
	u32                         m_category;

	const string&                  AddString(tukk str);
	i32                            activateFlags; // one bit per input port; true means a value change will activate the node
};

#endif
