// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "IPropertyTreeWidget.h"

#include <QWidget>

struct SStaticResourceSelectorEntry;

class QHBoxLayout;
class QLineEdit;

namespace Serialization
{
struct IResourceSelector;
}

class CResourcePicker : public QWidget, public IPropertyTreeWidget
{
	Q_OBJECT
	                             Q_INTERFACES(IPropertyTreeWidget)
public:
	CResourcePicker();

	virtual void SetValue(uk valuePtr, const yasli::TypeID& type, const yasli::Archive& ar) final;
	virtual void GetValue(uk valuePtr, const yasli::TypeID& type) const final;
	virtual void Serialize(Serialization::IArchive& ar) final;
	virtual bool SupportsMultiEdit() const final { return true; }
	virtual void SetMultiEditValue() final;

	void OnValueContinuousChange(tukk szFilename);

protected:
	bool event(QEvent* pEvent) override;

private:
	void Init(Serialization::IResourceSelector* pSelector, const yasli::Archive& ar);
	void AddButton(tukk szIconPath, tukk szToolTip, void (CResourcePicker::*pCallback)());

	void OnValueChanged(bool isContinuous = false);
	void OnEdit();
	void OnPick();
	void OnPickLegacy();

private:
	SResourceSelectorContext            m_context;
	QString                             m_previousValue;
	const SStaticResourceSelectorEntry* m_pSelector;
	string                              m_type;

	QHBoxLayout*                        m_pLayout;
	QLineEdit*                          m_pLineEdit;
};

