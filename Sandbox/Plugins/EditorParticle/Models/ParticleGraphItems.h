// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxParticleSystem/IParticlesPfx2.h>

#include <NodeGraph/IDrxGraphEditor.h>
#include <NodeGraph/AbstractNodeItem.h>
#include <NodeGraph/AbstractPinItem.h>
#include <NodeGraph/AbstractConnectionItem.h>

#include <NodeGraph/PinWidget.h>

#include <drx3D/CoreX/Extension/DrxGUID.h>

namespace DrxGraphEditor {

class CNodeWidget;
class CNodeGraphViewModel;
class CNodeGraphView;

}

namespace DrxParticleEditor {

class CParentPinItem;
class CChildPinItem;
class CFeaturePinItem;
class CFeatureItem;

typedef std::vector<CFeatureItem*> FeatureItemArray;

class CNodeItem : public DrxGraphEditor::CAbstractNodeItem
{
	friend class CFlowGraphViewModel;

public:
	CNodeItem(pfx2::IParticleComponent& component, DrxGraphEditor::CNodeGraphViewModel& viewModel);
	virtual ~CNodeItem();

	// DrxGraphEditor::CAbstractNodeItem
	virtual void                                SetPosition(QPointF position) override;

	virtual DrxGraphEditor::CNodeWidget*        CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;

	virtual QVariant                            GetId() const override;
	virtual bool                                HasId(QVariant id) const override;
	virtual QVariant                            GetTypeId() const override;

	virtual const DrxGraphEditor::PinItemArray& GetPinItems() const override { return m_pins; };
	virtual QString                             GetName() const;
	virtual void                                SetName(const QString& name);

	virtual bool                                IsDeactivated() const override;
	virtual void                                SetDeactivated(bool isDeactivated) override;

	virtual void                                Serialize(Serialization::IArchive& archive) override;
	// ~DrxGraphEditor::CAbstractNodeItem

	tukk               GetIdentifier() const         { return m_component.GetName(); }
	pfx2::IParticleComponent& GetComponentInterface() const { return m_component; }

	u32                    GetIndex() const;
	CParentPinItem*           GetParentPinItem();
	CChildPinItem*            GetChildPinItem();

	const FeatureItemArray& GetFeatureItems() const { return m_features; }
	const size_t            GetNumFeatures() const  { return m_features.size(); }

	CFeatureItem*           AddFeature(u32 index, const pfx2::SParticleFeatureParams& featureParams);
	CFeatureItem*           AddFeature(const pfx2::SParticleFeatureParams& featureParams);
	void                    RemoveFeature(u32 index);

	bool                    MoveFeatureAtIndex(u32 featureIndex, u32 destIndex);

	void                    SetVisible(bool isVisible);
	bool                    IsVisible();

public:
	CDrxSignal<void(CFeatureItem&)>  SignalFeatureAdded;
	CDrxSignal<void(CFeatureItem&)>  SignalFeatureRemoved;
	CDrxSignal<void(CFeatureItem&)>  SignalFeatureMoved;

	CDrxSignal<void(bool isVisible)> SignalVisibleChanged;

private:
	pfx2::IParticleComponent&    m_component;
	DrxGraphEditor::PinItemArray m_pins;
	FeatureItemArray             m_features;
};

inline CFeatureItem* CNodeItem::AddFeature(const pfx2::SParticleFeatureParams& featureParams)
{
	return AddFeature(m_component.GetNumFeatures(), featureParams);
}

class CConnectionItem;

enum class EPinType
{
	Unset,
	Parent,
	Child,
	Feature
};

enum ECustomItemType : i32
{
	eCustomItemType_Feature = DrxGraphEditor::eItemType_UserType,
};

class CBasePinItem : public DrxGraphEditor::CAbstractPinItem
{
public:
	typedef DrxGraphEditor::CIconArray<DrxGraphEditor::CPinWidget::Icon_Count> PinIconMap;

public:
	CBasePinItem(CNodeItem& node)
		: DrxGraphEditor::CAbstractPinItem(node.GetViewModel())
		, m_nodeItem(node)
	{}

	virtual EPinType GetPinType() const { return EPinType::Unset; }

	// DrxGraphEditor::CAbstractPinItem
	virtual DrxGraphEditor::CAbstractNodeItem& GetNodeItem() const override { return static_cast<DrxGraphEditor::CAbstractNodeItem&>(m_nodeItem); };
	// ~DrxGraphEditor::CAbstractPinItem

protected:
	CNodeItem& m_nodeItem;
};

class CParentPinItem : public CBasePinItem
{
public:
	CParentPinItem(CNodeItem& node): CBasePinItem(node) {}

	// CBasePinItem
	virtual EPinType GetPinType() const { return EPinType::Parent; }

	virtual QString  GetName() const override { return QString("Parent"); }
	virtual QString  GetDescription() const override { return QString("Parent effect."); }
	virtual QString  GetTypeName() const override { return QString("Effect"); }

	virtual QVariant GetId() const override { return QVariant::fromValue(QString("Parent")); }
	virtual bool     HasId(QVariant id) const override { return QString("Parent") == id.value<QString>(); }

	virtual bool     IsInputPin() const override  { return true; }
	virtual bool     IsOutputPin() const override { return false; }

	virtual bool     CanConnect(const DrxGraphEditor::CAbstractPinItem* pOtherPin) const override { return pOtherPin && pOtherPin->IsOutputPin() && !IsConnected(); }
	virtual bool     IsConnected() const override { return GetConnectionItems().size() > 0; }
	// ~CBasePinItem
};

class CChildPinItem : public CBasePinItem
{
public:
	CChildPinItem(CNodeItem& node): CBasePinItem(node) {}

	// CBasePinItem
	virtual EPinType GetPinType() const { return EPinType::Child; }

	virtual QString  GetName() const override { return QString("Children"); }
	virtual QString  GetDescription() const override { return QString("Child effects."); }
	virtual QString  GetTypeName() const override { return QString("Effect"); }

	virtual QVariant GetId() const override { return QVariant::fromValue(QString("Children")); }
	virtual bool     HasId(QVariant id) const override { return QString("Children") == id.value<QString>(); }

	virtual bool     IsInputPin() const override  { return false; }
	virtual bool     IsOutputPin() const override { return true; }

	virtual bool     CanConnect(const DrxGraphEditor::CAbstractPinItem* pOtherPin) const override { return !pOtherPin || pOtherPin->IsInputPin(); }
	virtual bool     IsConnected() const override { return GetConnectionItems().size() > 0; }
	// ~CBasePinItem
};

class CFeaturePinItem : public CBasePinItem
{
public:
	CFeaturePinItem(CFeatureItem& feature);
	virtual ~CFeaturePinItem() {}

	// DrxGraphEditor::CAbstractPinItem
	virtual EPinType GetPinType() const { return EPinType::Feature; }

	virtual QString  GetName() const override;
	virtual QString  GetDescription() const override { return GetName(); }
	virtual QString  GetTypeName() const override { return QString(); }

	virtual QVariant GetId() const override;
	virtual bool     HasId(QVariant id) const override;

	virtual bool     IsInputPin() const override  { return false; }
	virtual bool     IsOutputPin() const override { return true; }

	virtual bool     CanConnect(const DrxGraphEditor::CAbstractPinItem* pOtherPin) const override;
	virtual bool     IsConnected() const override { return m_connections.size() > 0; }
	// ~DrxGraphEditor::CAbstractPinItem

	CFeatureItem& GetFeatureItem() const { return m_featureItem; }

private:
	CFeatureItem& m_featureItem;
};

typedef std::vector<CFeaturePinItem*> FeaturePinItemArray;

class CFeatureItem : public DrxGraphEditor::CAbstractNodeGraphViewModelItem
{
public:
	enum : i32 { Type = eCustomItemType_Feature };

public:
	CFeatureItem(pfx2::IParticleFeature& feature, CNodeItem& node, DrxGraphEditor::CNodeGraphViewModel& viewModel);
	~CFeatureItem();

	// DrxGraphEditor::CAbstractNodeGraphViewModelItem
	virtual i32 GetType() const override { return Type; }

	virtual bool  IsDeactivated() const override;
	virtual void  SetDeactivated(bool isDeactivated) override;

	virtual void  Serialize(Serialization::IArchive& archive) override;
	// ~DrxGraphEditor::CAbstractNodeGraphViewModelItem

	QString                 GetGroupName() const;
	QString                 GetName() const;
	u32                  GetIndex() const;

	pfx2::IParticleFeature& GetFeatureInterface() const { return m_featureInterface; }
	CNodeItem&              GetNodeItem() const         { return m_node; }
	CFeaturePinItem*        GetPinItem() const          { return m_pPin; }

	bool                    HasComponentConnector() const;
	QColor                  GetColor() const;

public:
	CDrxSignal<void(bool)> SignalItemDeactivatedChanged;

private:
	CNodeItem&              m_node;
	pfx2::IParticleFeature& m_featureInterface;
	CFeaturePinItem*        m_pPin;
};

class CConnectionItem : public DrxGraphEditor::CAbstractConnectionItem
{
public:
	CConnectionItem(CBasePinItem& sourcePin, CBasePinItem& targetPin, DrxGraphEditor::CNodeGraphViewModel& viewModel);
	virtual ~CConnectionItem();

	// DrxGraphEditor::CAbstractConnectionItem
	virtual DrxGraphEditor::CConnectionWidget* CreateWidget(DrxGraphEditor::CNodeGraphView& view) override;

	virtual DrxGraphEditor::CAbstractPinItem&  GetSourcePinItem() const override { return m_sourcePin; }
	virtual DrxGraphEditor::CAbstractPinItem&  GetTargetPinItem() const override { return m_targetPin; }

	virtual QVariant                           GetId() const override;
	virtual bool                               HasId(QVariant id) const override;
	// ~DrxGraphEditor::CAbstractConnectionItem

	void OnConnectionRemoved();

private:
	CBasePinItem& m_sourcePin;
	CBasePinItem& m_targetPin;

	u32        m_id;
};

}

