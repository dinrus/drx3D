// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "ResourcePicker.h"

#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceSelector.h>

#include "IResourceSelectorHost.h"
#include "DrxIcon.h"

#include <IEditor.h>
#include <IUndoManager.h>

namespace Private_ResourcePickers
{

class ResourceSelectionCallback : public IResourceSelectionCallback
{
public:
	virtual void SetValue(tukk newValue) override
	{
		if (m_pResourcePicker)
		{
			(m_pResourcePicker->*m_pCallback)(newValue);
		}
	}

	void SetValueChangedCallback(CResourcePicker* pResourcePicker, void (CResourcePicker::* pCallback)(tukk))
	{
		m_pResourcePicker = pResourcePicker;
		m_pCallback = pCallback;
	}

private:
	CResourcePicker* m_pResourcePicker;
	void             (CResourcePicker::* m_pCallback)(tukk);
};
}

CResourcePicker::CResourcePicker()
	: m_pSelector(nullptr)
{
	m_pLineEdit = new QLineEdit();

	m_pLayout = new QHBoxLayout();
	m_pLayout->addWidget(m_pLineEdit);
	m_pLayout->setMargin(0);

	setLayout(m_pLayout);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	// needs to be lambda because of default param
	connect(m_pLineEdit, &QLineEdit::editingFinished, [this]() { OnValueChanged(); });
}

bool CResourcePicker::event(QEvent* pEvent)
{
	if (pEvent->type() == QEvent::ToolTip)
	{
		if (m_pSelector->ShowTooltip(m_context, m_pLineEdit->text().toStdString().c_str()))
		{
			return true;
		}
	}

	return QWidget::event(pEvent);
}

void CResourcePicker::OnValueChanged(bool isContinuous /*= false*/)
{
	DRX_ASSERT(m_pSelector);   // must have valid selector

	QString newValue = m_pLineEdit->text();
	if (m_previousValue == newValue)
		return;

	// Run validation check
	dll_string validatedPath = m_pSelector->ValidateValue(m_context, newValue.toStdString().c_str(), m_previousValue.toStdString().c_str());

	if (m_previousValue == validatedPath.c_str())
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Invalid %s: %s", m_context.typeName, newValue.toStdString().c_str());
		m_pLineEdit->setText(m_previousValue);
		if (m_pLineEdit->hasFocus())
		{
			m_pLineEdit->selectAll();
		}
		return;
	}
	
	if (isContinuous)
	{
		OnContinuousChanged();
	}
	else
	{
		OnChanged();
	}

	m_previousValue = m_pLineEdit->text();

	if (m_pLineEdit->hasFocus())
	{
		m_pLineEdit->selectAll();
	}
}

void CResourcePicker::OnValueContinuousChange(tukk szFilename)
{
	m_pLineEdit->setText(szFilename);
	OnValueChanged(true);
}

void CResourcePicker::OnPick()
{
	DRX_ASSERT(m_pSelector);

	Private_ResourcePickers::ResourceSelectionCallback callback;
	callback.SetValueChangedCallback(this, &CResourcePicker::OnValueContinuousChange);
	m_context.callback = &callback;

	const QString value = m_pLineEdit->text();
	dll_string filename = m_pSelector->SelectResource(m_context, m_previousValue.toStdString().c_str());

	if (value != filename.c_str())
	{
		m_pLineEdit->setText(filename.c_str());
		OnValueChanged();
	}
}

void CResourcePicker::OnEdit()
{
	m_pSelector->EditResource(m_context, m_pLineEdit->text().toStdString().c_str());
}

void CResourcePicker::OnPickLegacy()
{
	m_context.useLegacyPicker = true;
	OnPick();
	m_context.useLegacyPicker = false;
}

void CResourcePicker::Init(Serialization::IResourceSelector* pSelector, const yasli::Archive& ar)
{
	// If we have a valid selector and the type hasn't changed for this row, then return
	if (m_pSelector && strcmp(pSelector->resourceType, m_context.typeName) == 0)
		return;

	m_type = pSelector->resourceType;
	m_pSelector = GetIEditor()->GetResourceSelectorHost()->GetSelector(m_type.c_str());

	m_context.resourceSelectorEntry = m_pSelector;
	m_context.typeName = m_type.c_str();
	m_context.parentWidget = QWidget::window();     // TODO: This used to be the root widget for the property tree
	m_context.pCustomParams = pSelector->pCustomParams;

	Serialization::TypeID contextTypeID = m_pSelector->GetContextType();
	if (contextTypeID)
	{
		m_context.contextObject = ar.contextByType(contextTypeID);
		m_context.contextObjectType = contextTypeID;
	}

	// Remove all buttons if we're reusing this row and create on demand based on the requirements of the selector
	while (m_pLayout->count() > 1)
	{
		QLayoutItem* pItem = m_pLayout->itemAt(1);
		m_pLayout->removeItem(pItem);
		delete pItem;
	}

	if (m_pSelector->HasLegacyPicker())
	{
		AddButton("icons:General/LegacyResourceSelectorPicker.ico", "Legacy Open", &CResourcePicker::OnPickLegacy);
	}

	if (m_pSelector->CanEdit())
	{
		AddButton("icons:General/Edit_Asset.ico", "Edit", &CResourcePicker::OnEdit);
	}

	AddButton("icons:General/Folder.ico", "Open", &CResourcePicker::OnPick);
}

void CResourcePicker::AddButton(tukk szIconPath, tukk szToolTip, void (CResourcePicker::* pCallback)())
{
	QToolButton* pButton = new QToolButton();
	pButton->setIcon(DrxIcon(szIconPath));
	pButton->setToolTip(szToolTip);
	m_pLayout->addWidget(pButton);
	connect(pButton, &QToolButton::clicked, this, pCallback);
}

void CResourcePicker::SetValue(uk valuePtr, const yasli::TypeID& type, const yasli::Archive& ar)
{
	if (type != yasli::TypeID::get<Serialization::IResourceSelector>())
		return;

	Serialization::IResourceSelector* pSelector = (Serialization::IResourceSelector*)valuePtr;

	Init(pSelector, ar);

	m_previousValue = pSelector->GetValue();
	m_pLineEdit->setText(m_previousValue);
}

void CResourcePicker::GetValue(uk valuePtr, const yasli::TypeID& type) const
{
	if (type != yasli::TypeID::get<Serialization::IResourceSelector>())
		return;

	Serialization::IResourceSelector* pSelector = (Serialization::IResourceSelector*)valuePtr;
	pSelector->SetValue(m_pLineEdit->text().toStdString().c_str());
}

void CResourcePicker::Serialize(Serialization::IArchive& ar)
{
	string str = m_pLineEdit->text().toStdString().c_str();
	ar(str, "text", "Text");
	QString newValue = str;
	
	if (newValue != m_previousValue && ar.isInput())
	{
		m_pLineEdit->setText(QString(str));
		OnValueChanged();
	}
}

void CResourcePicker::SetMultiEditValue()
{
	m_pLineEdit->setText("");
	m_pLineEdit->setPlaceholderText("Multiple Values");
}

namespace Serialization
{
REGISTER_PROPERTY_WIDGET(IResourceSelector, CResourcePicker);
}

