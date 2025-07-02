// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "CustomResolutionDlg.h"

#include "Controls/QMenuComboBox.h"

#include <QGridLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

QCustomResolutionDialog::QCustomResolutionDialog(i32 xresolution, i32 yresolution, bool use_aspect, QWidget* parent)
	: CEditorDialog("Custom Resolution", parent)
	, aspectCombo(nullptr)
{
	QGridLayout* localLayout = new QGridLayout();

	m_aspectConstraint = eConstraintAspect::eConstraintNone;
	i32 d = gcd(xresolution, yresolution);
	m_aspectNumerator = xresolution / d;
	m_aspectDenominator = yresolution / d;

	widthField = new QSpinBox();
	heightField = new QSpinBox();

	widthField->setRange(128, 10000);
	widthField->setVal(xresolution);

	heightField->setRange(128, 10000);
	heightField->setVal(yresolution);

	okButton = new QPushButton("OK");
	cancelButton = new QPushButton("Cancel");

	localLayout->addWidget(widthField, 0, 0);
	localLayout->addWidget(heightField, 0, 1);

	if (use_aspect)
	{
		QSpinBox* aspectx = new QSpinBox;
		QSpinBox* aspecty = new QSpinBox;

		aspectx->setRange(1, 10000);
		aspectx->setVal(m_aspectNumerator);

		aspecty->setRange(1, 10000);
		aspecty->setVal(m_aspectDenominator);

		aspectCombo = new QMenuComboBox();
		aspectCombo->AddItem(tr("No Constraint"), QVariant((i32)eConstraintAspect::eConstraintNone));
		aspectCombo->AddItem(tr("Fit To Width"), QVariant((i32)eConstraintAspect::eConstraintWidth));
		aspectCombo->AddItem(tr("Fit To Height"), QVariant((i32)eConstraintAspect::eConstraintHeight));

		localLayout->addWidget(new QLabel("Aspect Ratio:"), 1, 0);
		localLayout->addWidget(aspectCombo, 1, 1);

		localLayout->addWidget(aspectx, 2, 0);
		localLayout->addWidget(aspecty, 2, 1);

		QObject::connect(aspectx, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged), this, &QCustomResolutionDialog::AspectChangedX);
		QObject::connect(aspecty, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged), this, &QCustomResolutionDialog::AspectChangedY);
		aspectCombo->signalCurrentIndexChanged.Connect(this, &QCustomResolutionDialog::AspectOptionChanged);
	}

	localLayout->addWidget(okButton, 3, 0, Qt::AlignBottom);
	localLayout->addWidget(cancelButton, 3, 1, Qt::AlignBottom);

	QObject::connect(okButton, &QPushButton::clicked, this, &QCustomResolutionDialog::accept);
	QObject::connect(cancelButton, &QPushButton::clicked, this, &QCustomResolutionDialog::reject);

	QObject::connect(widthField, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged), this, &QCustomResolutionDialog::ResolutionChangedX);
	QObject::connect(heightField, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged), this, &QCustomResolutionDialog::ResolutionChangedY);

	setLayout(localLayout);

	m_xres = xresolution;
	m_yres = yresolution;

	setWindowTitle("Set Resolution");
}

void QCustomResolutionDialog::GetResolution(i32& xresolution, i32& yresolution)
{
	xresolution = m_xres;
	yresolution = m_yres;
}

i32 QCustomResolutionDialog::ClampValue(i32 value)
{
	value = min(value, 10000);
	return max(value, 128);
}

void QCustomResolutionDialog::ResolutionChangedX(i32 value)
{
	switch (m_aspectConstraint)
	{
	case eConstraintAspect::eConstraintNone:
		m_xres = value;
		break;
	case eConstraintAspect::eConstraintWidth:
		{
			float aspect = m_aspectNumerator / (float)m_aspectDenominator;
			m_xres = value;
			m_yres = ClampValue(value / aspect);
			heightField->setVal(m_yres);
			break;
		}
	case eConstraintAspect::eConstraintHeight:
		break;
	}
}

void QCustomResolutionDialog::ResolutionChangedY(i32 value)
{
	switch (m_aspectConstraint)
	{
	case eConstraintAspect::eConstraintNone:
		m_yres = value;
		break;
	case eConstraintAspect::eConstraintWidth:
		break;
	case eConstraintAspect::eConstraintHeight:
		{
			float aspect = m_aspectNumerator / (float)m_aspectDenominator;
			m_yres = value;
			m_xres = ClampValue(value * aspect);
			widthField->setVal(m_xres);
			break;
		}
	}
}

void QCustomResolutionDialog::AspectChanged()
{
	float aspect = m_aspectNumerator / (float)m_aspectDenominator;

	switch (m_aspectConstraint)
	{
	case eConstraintAspect::eConstraintNone:
		break;
	case eConstraintAspect::eConstraintWidth:
		m_yres = ClampValue(m_xres / aspect);
		heightField->setVal(m_yres);
		break;
	case eConstraintAspect::eConstraintHeight:
		m_xres = ClampValue(m_yres * aspect);
		widthField->setVal(m_xres);
		break;
	}
}

void QCustomResolutionDialog::AspectChangedX(i32 value)
{
	m_aspectNumerator = value;

	AspectChanged();
}

void QCustomResolutionDialog::AspectChangedY(i32 value)
{
	m_aspectDenominator = value;

	AspectChanged();
}

void QCustomResolutionDialog::AspectOptionChanged(i32 index)
{
	if (!aspectCombo)
	{
		return;
	}

	m_aspectConstraint = (eConstraintAspect)aspectCombo->GetData(index).toInt();

	switch (m_aspectConstraint)
	{
	case eConstraintAspect::eConstraintNone:
		widthField->setEnabled(true);
		heightField->setEnabled(true);
		widthField->setReadOnly(false);
		heightField->setReadOnly(false);
		break;
	case eConstraintAspect::eConstraintWidth:
		widthField->setEnabled(true);
		heightField->setEnabled(false);
		widthField->setReadOnly(false);
		heightField->setReadOnly(true);
		break;
	case eConstraintAspect::eConstraintHeight:
		widthField->setEnabled(false);
		heightField->setEnabled(true);
		widthField->setReadOnly(true);
		heightField->setReadOnly(false);
		break;
	}
}

