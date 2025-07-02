// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Serialization/QPropertyTree2/IPropertyTreeWidget.h"

#include <QWidget>

class QNumericBox;

class EDITOR_COMMON_API CVec3Widget : public QWidget, public IPropertyTreeWidget
{
	Q_OBJECT
	Q_INTERFACES(IPropertyTreeWidget)
public:
	CVec3Widget();

	virtual void SetValue(uk valuePtr, const yasli::TypeID& type, const yasli::Archive& ar) final;
	virtual void GetValue(uk valuePtr, const yasli::TypeID& type) const final;
	virtual void Serialize(Serialization::IArchive& ar) final;
	virtual bool SupportsMultiEdit() const final { return true; }
	virtual void SetMultiEditValue() final;

private:
	void UpdateRow(yasli::TypeID type);
	void OnValueChanged(QWidget* pField = nullptr, bool isContinuous = false);
	void SetValue(const Vec3& value);

private:
	QNumericBox*  m_pXField;
	QNumericBox*  m_pYField;
	QNumericBox*  m_pZField;

	Vec3          m_previousValue;

	yasli::TypeID m_type;
	bool          m_uniformScale;
	bool          m_ignoreSignals;
};

