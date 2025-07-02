// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/EditorDialog.h"

class QMenuComboBox;

class QSpinBox;
class QPushButton;

class SANDBOX_API QCustomResolutionDialog : public CEditorDialog
{
	Q_OBJECT

public:
	QCustomResolutionDialog(i32 xresolution, i32 yresolution, bool use_aspect = true, QWidget* parent = nullptr);

	void GetResolution(i32& xresolution, i32& yresolution);

public slots:
	void ResolutionChangedX(i32 value);
	void ResolutionChangedY(i32 value);
	void AspectOptionChanged(i32 value);
	void AspectChangedX(i32 value);
	void AspectChangedY(i32 value);

private:
	i32  ClampValue(i32 value);
	void AspectChanged();

	enum class eConstraintAspect
	{
		eConstraintNone   = 0,
		eConstraintWidth  = 1,
		eConstraintHeight = 2,
	};
	QSpinBox*         widthField;
	QSpinBox*         heightField;
	QPushButton*      okButton;
	QPushButton*      cancelButton;
	QMenuComboBox*    aspectCombo;
	i32               m_xres;
	i32               m_yres;
	eConstraintAspect m_aspectConstraint;
	i32               m_aspectNumerator;
	i32               m_aspectDenominator;
};

