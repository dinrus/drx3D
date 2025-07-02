// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowBaseXmlNode.h
//  Описание: Flowgraph nodes to read/write Xml files
// -------------------------------------------------------------------------
//  История:
//    - 8/16/08 : File created - Kevin Kirst
//    - 09/06/2011: Added to SDK - Sascha Hoba
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <drx3D/FlowGraph/IFlowBaseNode.h>

#define ADD_BASE_INPUTS() \
  InputPortConfig_Void("Execute", _HELP("Execute Xml instruction"))

#define ADD_BASE_OUTPUTS()                                                                        \
  OutputPortConfig<bool>("Success", _HELP("Called if Xml instruction is executed successfully")), \
  OutputPortConfig<bool>("Fail", _HELP("Called if Xml instruction fails")),                       \
  OutputPortConfig<bool>("Done", _HELP("Called when Xml instruction is carried out"))

////////////////////////////////////////////////////
class CFlowXmlNode_Base : public CFlowBaseNode<eNCT_Instanced>
{
public:
	////////////////////////////////////////////////////
	CFlowXmlNode_Base(SActivationInfo* pActInfo);
	virtual ~CFlowXmlNode_Base(void);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);

protected:
	enum EInputs
	{
		EIP_Execute,
		EIP_CustomStart,
	};

	enum EOutputs
	{
		EOP_Success,
		EOP_Fail,
		EOP_Done,
		EOP_CustomStart,
	};

	//! Overload to handle Xml execution
	virtual bool Execute(SActivationInfo* pActInfo) = 0;

private:
	SActivationInfo m_actInfo;
	bool            m_initialized;
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
struct SXmlDocument
{
	XmlNodeRef root;
	XmlNodeRef active;
	size_t     refCount;
};

class CGraphDocUpr
{
private:
	CGraphDocUpr();
	static CGraphDocUpr* m_instance;
public:
	virtual ~CGraphDocUpr();
	static void              Create();
	static CGraphDocUpr* Get();

	void                     MakeXmlDocument(IFlowGraph* pGraph);
	void                     DeleteXmlDocument(IFlowGraph* pGraph);
	bool                     GetXmlDocument(IFlowGraph* pGraph, SXmlDocument** document);

private:
	typedef std::map<IFlowGraph*, SXmlDocument> GraphDocMap;
	GraphDocMap m_GraphDocMap;
};
extern CGraphDocUpr* GDM;
