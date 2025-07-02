// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AbstractObjectItem.h"

#include <DrxIcon.h>

#include <QVariant>
#include <QString>

namespace Schematyc {

struct IScriptElement;
struct IScriptFunction;
struct IScriptConstructor;
struct IScriptSignalReceiver;

}

namespace DrxSchematycEditor {

class CStateItem;
class CNodeGraphViewModel;

class CGraphItem : public CAbstractObjectStructureModelItem
{
public:
	enum EGraphType
	{
		eGraphType_Function,
		eGraphType_Construction,
		eGraphType_Destruction,
		eGraphType_SignalReceiver,
	};

public:
	CGraphItem(Schematyc::IScriptFunction& scriptFunction, CAbstractObjectStructureModel& model);
	CGraphItem(Schematyc::IScriptConstructor& scriptConstructor, CAbstractObjectStructureModel& model);
	CGraphItem(Schematyc::IScriptSignalReceiver& scriptSignalReceiver, CAbstractObjectStructureModel& model);
	virtual ~CGraphItem();

	// CAbstractObjectStructureModelItem
	virtual void                               SetName(QString name) override;
	virtual i32                              GetType() const override { return eObjectItemType_Graph; }
	virtual const DrxIcon*                     GetIcon() const override;
	virtual CAbstractObjectStructureModelItem* GetParentItem() const override;

	virtual u32                             GetIndex() const;
	virtual void                               Serialize(Serialization::IArchive& archive) override;

	virtual bool                               AllowsRenaming() const override;
	// ~CAbstractObjectStructureModelItem

	void                 SetParentItem(CAbstractObjectStructureModelItem* pParentItem) { m_pParentItem = pParentItem; }

	DrxGUID     GetGUID() const;
	EGraphType           GetGraphType() const  { return m_graphType; }
	CNodeGraphViewModel* GetGraphModel() const { return m_pGraphModel; }

protected:
	void LoadFromScriptElement();

private:
	CAbstractObjectStructureModelItem* m_pParentItem;
	CNodeGraphViewModel*               m_pGraphModel;

	union
	{
		Schematyc::IScriptElement*        m_pScriptElement;
		Schematyc::IScriptFunction*       m_pScriptFunction;
		Schematyc::IScriptConstructor*    m_pScriptConstructor;
		Schematyc::IScriptSignalReceiver* m_pScriptSignalReceiver;
	};

	EGraphType m_graphType;
};

}

