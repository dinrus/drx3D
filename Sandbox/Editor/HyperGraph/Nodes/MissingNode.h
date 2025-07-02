// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HyperGraph/HyperGraphNode.h"

#define MISSING_NODE_CLASS ("MissingNode")

class CMissingNode : public CHyperNode
{
public:
	CMissingNode(tukk sMissingClassName);

	// CHyperNode
	virtual void            Init() override {}
	virtual CHyperNode*     Clone() override;
	virtual void            Serialize(XmlNodeRef& node, bool bLoading, CObjectArchive* ar = 0) override;

	virtual bool            IsEditorSpecialNode() const override { return true; }
	virtual bool            IsFlowNode() const override          { return false; }
	virtual tukk     GetClassName() const override        { return MISSING_NODE_CLASS; };
	virtual tukk     GetDescription() const override      { return "This node class no longer exists in the code base"; }
	virtual tukk     GetInfoAsString() const override;

	virtual CHyperNodePort* FindPort(tukk portname, bool bInput);
	// ~CHyperNode

private:
	CString m_sMissingClassName;
	CString m_sMissingName;
	CString m_sGraphEntity;
	DrxGUID m_entityGuid;
	i32     m_iOrgFlags;
};

