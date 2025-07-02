// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeItem.h>
#include <NodeGraph/AbstractPinItem.h>

#include <QString>
#include <QVariant>

class CDataTypeItem;

namespace DrxGraphEditor {

class CPinWidget;
class CNodeWidget;
class CNodeGraphView;

}

namespace DrxSchematycEditor {

enum EPinFlag : u32
{
	None   = 0,
	Input  = 1 << 0,
	Output = 1 << 1,
};

class CNodeItem;

// Note: This is part of a workaround because pin IDs in Schematc are not unique atm.
class CPinId
{
public:
	enum class EType : u8
	{
		Unset  = 0,
		Input  = EPinFlag::Input,
		Output = EPinFlag::Output
	};

	CPinId()
		: m_type(EType::Unset)
	{}

	CPinId(Schematyc::CUniqueId portId, EType type)
		: m_portId(portId)
		, m_type(type)
	{}

	const Schematyc::CUniqueId& GetPortId()  const { return m_portId; }
	bool                        IsInput() const    { return (m_type == EType::Input); }
	bool                        IsOutput() const   { return (m_type == EType::Output); }

	//
	bool operator==(const CPinId& other) const
	{
		return (m_type == other.m_type && m_portId == other.m_portId);
	}

	bool operator!=(const CPinId& other) const
	{
		return (m_type != other.m_type || m_portId != other.m_portId);
	}

private:
	Schematyc::CUniqueId m_portId;
	EType                m_type;
};
// ~Note

enum class EPinType : int8
{
	Unset = 0,
	Execution,
	Data,
	Signal
};

class CPinItem : public DrxGraphEditor::CAbstractPinItem
{
public:
	CPinItem(u32 index, u32 flags, CNodeItem& nodeItem, DrxGraphEditor::CNodeGraphViewModel& model);
	virtual ~CPinItem();

	// DrxGraphEditor::CAbstractPinItem
	virtual DrxGraphEditor::CPinWidget*        CreateWidget(DrxGraphEditor::CNodeWidget& nodeWidget, DrxGraphEditor::CNodeGraphView& view) override;
	virtual tukk                        GetStyleId() const { return m_styleId.c_str(); }
	virtual DrxGraphEditor::CAbstractNodeItem& GetNodeItem() const override;

	virtual QString                            GetName() const override        { return m_name; };
	virtual QString                            GetDescription() const override { return QString(); }
	virtual QString                            GetTypeName() const override;

	virtual QVariant                           GetId() const override;
	virtual bool                               HasId(QVariant id) const override;

	virtual bool                               IsInputPin() const  { return m_flags & EPinFlag::Input; }
	virtual bool                               IsOutputPin() const { return m_flags & EPinFlag::Output; }

	virtual bool                               CanConnect(const DrxGraphEditor::CAbstractPinItem* pOtherPin) const;
	// ~DrxGraphEditor::CAbstractPinItem

	const CDataTypeItem& GetDataTypeItem() const { return *m_pDataTypeItem; }
	u32               GetPinIndex() const     { return m_index; }
	void                 UpdateWithNewIndex(u32 index);

	Schematyc::CUniqueId GetPortId() const;
	EPinType             GetPinType() const { return m_pinType; }

	CPinId               GetPinId() const;

protected:
	CNodeItem&           m_nodeItem;
	const CDataTypeItem* m_pDataTypeItem;
	QString              m_name;
	string               m_styleId;

	CPinId               m_id;
	u32               m_flags;
	u16               m_index;
	EPinType             m_pinType;

};

inline CPinId CPinItem::GetPinId() const
{
	return CPinId(GetPortId(), IsInputPin() ? CPinId::EType::Input : CPinId::EType::Output);
}

}

Q_DECLARE_METATYPE(DrxSchematycEditor::CPinId);

