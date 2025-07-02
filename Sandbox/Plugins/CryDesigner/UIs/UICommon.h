// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ToolFactory.h"
#include "Serialization/QPropertyTree/QPropertyTree.h"
#include "Controls/QMenuComboBox.h"

#include <QWidget>
#include <QBoxLayout>

class CBaseObject;
class CMaterial;
class QFrame;

namespace Designer
{
class BaseTool;

class QPropertyTreeWidget : public QWidget
{
	Q_OBJECT
public:
	template<class ChangeCallback, class SerializationClass>
	void CreatePropertyTree(SerializationClass* serialization, ChangeCallback changeCallback)
	{
		CreatePropertyTree(serialization);
		QObject::connect(m_pPropertyTree, &QPropertyTree::signalChanged, this, [ = ] { changeCallback(false); });
		QObject::connect(m_pPropertyTree, &QPropertyTree::signalContinuousChange, this, [ = ] { changeCallback(true); });
	}

	template<class SerializationClass>
	void CreatePropertyTree(SerializationClass* serialization)
	{
		QBoxLayout* pBoxLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		pBoxLayout->setContentsMargins(0, 0, 0, 0);
		m_pPropertyTree = new QPropertyTree(this);
		m_pPropertyTree->setExpandLevels(1);
		m_pPropertyTree->setCompact(false);
		m_pPropertyTree->setSizeToContent(true);
		auto style = QPropertyTree::defaultTreeStyle();
		style.propertySplitter = false;
		m_pPropertyTree->setTreeStyle(style);
		m_pPropertyTree->attach(Serialization::SStruct(*serialization));
		pBoxLayout->addWidget(m_pPropertyTree);
		setLayout(pBoxLayout);
	}

	virtual void Serialize(Serialization::IArchive& ar) {}

	QSize        contentSize() const                    { return m_pPropertyTree->contentSize(); }

protected:
	QPropertyTree* m_pPropertyTree;
};

class QMaterialComboBox : public QWidget
{
public:
	explicit QMaterialComboBox(QWidget* pParent = nullptr);

	void FillWithSubMaterials();
	i32  GetSubMatID();
	void SetSubMatID(CBaseObject* pObj, i32 nID);
	bool SelectMaterial(CBaseObject* pObj, CMaterial* pMaterial);

	void SetCurrentIndex(i32 index);

	i32  GetCurrentIndex() const;
	i32  GetItemCount() const;

private:
	QMenuComboBox* m_pComboBox;

public:
	// binds to signalCurrentIndexChanged from QMenuComboBox. must be declared after it
	CDrxSignal<void(i32)>& signalCurrentIndexChanged;
};

QString GetDefaultButtonStyle();
QString GetSelectButtonStyle(QWidget* pWidget);

QFrame* CreateHorizontalLine();
}

