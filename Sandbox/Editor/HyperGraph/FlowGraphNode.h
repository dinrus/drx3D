// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxEntitySystem/IEntitySystem.h>
#include <DrxFlowGraph/IFlowSystem.h>

#include "HyperGraphNode.h"

class CFlowNode;
class CEntityObject;

//////////////////////////////////////////////////////////////////////////
class CFlowNode : public CHyperNode
{
	friend class CFlowGraphManager;

public:
	CFlowNode();
	virtual ~CFlowNode();

	//////////////////////////////////////////////////////////////////////////
	// CHyperNode implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void           Init() override;
	virtual void           Done() override;
	virtual CHyperNode*    Clone() override;
	virtual tukk    GetDescription() const override;
	virtual void           Serialize(XmlNodeRef& node, bool bLoading, CObjectArchive* ar) override;
	virtual void           SetName(tukk sName) override;
	virtual void           OnInputsChanged() override;
	virtual void           OnEnteringGameMode() override;
	virtual void           Unlinked(bool bInput) override;
	virtual CString        GetPortName(const CHyperNodePort& port) override;
	virtual void           PostClone(CBaseObject* pFromObject, CObjectCloneContext& ctx) override;
	virtual Gdiplus::Color GetCategoryColor() const override;
	virtual void           DebugPortActivation(TFlowPortId port, tukk value, bool bIsInitializationStep) override;
	virtual bool           IsPortActivationModified(const CHyperNodePort* port = nullptr) const override;
	virtual void           ClearDebugPortActivation() override;
	virtual CString        GetDebugPortValue(const CHyperNodePort& pp) override;
	virtual void           ResetDebugPortActivation(CHyperNodePort* port) override;
	virtual bool           GetAdditionalDebugPortInformation(const CHyperNodePort& pp, bool& bOutIsInitialization) override;
	virtual bool           IsDebugPortActivated(CHyperNodePort* port) const override;
	virtual bool           IsObsolete() const override { return m_flowSystemNodeFlags & EFLN_OBSOLETE; }
	virtual TFlowNodeId    GetTypeId() const override;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// FlowSystem Flags. We don't want to replicate those again....
	//////////////////////////////////////////////////////////////////////////
	u32         GetCoreFlags() const  { return m_flowSystemNodeFlags & EFLN_CORE_MASK; }
	u32         GetCategory() const   { return m_flowSystemNodeFlags & EFLN_CATEGORY_MASK; }
	u32         GetUsageFlags() const { return m_flowSystemNodeFlags & EFLN_USAGE_MASK; }
	tukk    GetCategoryName() const;
	tukk    GetUIClassName() const;

	void           SetFromNodeId(TFlowNodeId flowNodeId);

	void           SetEntity(CEntityObject* pEntity);
	CEntityObject* GetEntity() const;

	// Takes selected entity as target entity.
	void           SetSelectedEntity();
	// Takes graph default entity as target entity.
	void           SetDefaultEntity();
	CEntityObject* GetDefaultEntity() const;

	// Returns IFlowNode.
	IFlowGraph* GetIFlowGraph() const;

	// Return ID of the flow node.
	TFlowNodeId          GetFlowNodeId() const { return m_flowNodeId; }

	void                 SetInputs(bool bActivate, bool bForceResetEntities = false);
	virtual CVarBlock*   GetInputsVarBlock();

	virtual CString      GetTitle() const;

	virtual IUndoObject* CreateUndo();
protected:
	virtual bool         IsEntityValid() const;
	virtual CString      GetEntityTitle() const;

protected:
	DrxGUID                       m_entityGuid;
	_smart_ptr<CEntityObject>     m_pEntity;
	u32                        m_flowSystemNodeFlags;
	TFlowNodeId                   m_flowNodeId;
	tukk                   m_szUIClassName;
	tukk                   m_szDescription;
	std::map<TFlowPortId, string> m_portActivationMap;
	std::map<TFlowPortId, bool>   m_portActivationAdditionalDebugInformationMap;
	std::vector<TFlowPortId>      m_debugPortActivations;
};

