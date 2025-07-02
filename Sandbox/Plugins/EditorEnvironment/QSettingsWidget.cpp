// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include "stdafx.h"

#include "QSettingsWidget.h"
#include <Mission.h>
#include <DrxEditDoc.h>
#include <GameEngine.h>
#include <IEditorImpl.h>

#include <QSizePolicy>
#include <QLayout>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>

#include <IEditor.h>
#include <DrxSystem/TimeValue.h>
#include <Util/Variable.h>
#include <Util/EditorUtils.h>
#include <LightingSettings.h>

#include <Drx3DEngine/I3DEngine.h>
#include <Drx3DEngine/ITimeOfDay.h>

namespace
{
ITimeOfDay* GetTimeOfDay()
{
	ITimeOfDay* pTimeOfDay = gEnv->p3DEngine->GetTimeOfDay();
	assert(pTimeOfDay);
	return pTimeOfDay;
}
}

QSunSettingsWidget::QSunSettingsWidget()
	: m_latitudeSlider(nullptr)
	, m_latitudeSpin(nullptr)
	, m_longitudeSlider(nullptr)
	, m_longitudeSpin(nullptr)
{
	CreateUi();
}

QSunSettingsWidget::~QSunSettingsWidget()
{

}

void QSunSettingsWidget::CreateUi()
{
	setMinimumWidth(100);
	setMaximumWidth(400);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	QBoxLayout* rootVerticalLayput = new QBoxLayout(QBoxLayout::TopToBottom);
	rootVerticalLayput->setContentsMargins(1, 1, 1, 1);
	rootVerticalLayput->setSizeConstraint(QLayout::SetMinimumSize);
	setLayout(rootVerticalLayput);

	ITimeOfDay* pTimeOfDay = GetTimeOfDay();
	float sunLatitude = 0.0f;
	float sunLongitude = 0.0f;
	if (pTimeOfDay)
	{
		sunLatitude = pTimeOfDay->GetSunLatitude();
		sunLongitude = pTimeOfDay->GetSunLongitude();
	}

	{
		QLabel* pSunDirectionLablel = new QLabel("Sun direction:");
		rootVerticalLayput->addWidget(pSunDirectionLablel, Qt::AlignTop);

		QBoxLayout* pHLayout = new QHBoxLayout;
		pHLayout->setContentsMargins(QMargins(0, 0, 0, 0));

		QSlider* pSunDirection = new QSlider(Qt::Horizontal);
		pSunDirection->setMinimum(0);
		pSunDirection->setMaximum(360);
		pSunDirection->setVal(i32(sunLatitude));

		pHLayout->addWidget(pSunDirection);

		QSpinBox* pSpinBox = new QSpinBox;
		pSpinBox->setFixedWidth(42);
		pSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pSpinBox->setMinimum(0);
		pSpinBox->setMaximum(360);
		pSpinBox->setVal(i32(sunLatitude));

		pHLayout->addWidget(pSpinBox);

		m_latitudeSlider = pSunDirection;
		m_latitudeSpin = pSpinBox;

		connect(m_latitudeSlider, &QSlider::valueChanged,
		        [pSpinBox, this](i32 value)
		{
			pSpinBox->blockSignals(true);
			pSpinBox->setVal(value);
			pSpinBox->blockSignals(false);
			UpdateLightingSettings();
		}
		        );

		connect(m_latitudeSpin, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged),
		        [pSunDirection, this](i32 value)
		{
			pSunDirection->blockSignals(true);
			pSunDirection->setVal(value);
			pSunDirection->blockSignals(false);
			UpdateLightingSettings();
		}
		        );

		rootVerticalLayput->addLayout(pHLayout, Qt::AlignTop);
	}

	{
		QLabel* pSunLongitudeLabel = new QLabel("NorthPole..Equator..SouthPole:");
		rootVerticalLayput->addWidget(pSunLongitudeLabel, Qt::AlignTop);

		QBoxLayout* pHLayout = new QHBoxLayout;
		pHLayout->setContentsMargins(QMargins(0, 0, 0, 0));

		QSlider* pSunLongitude = new QSlider(Qt::Horizontal);
		pSunLongitude->setMinimum(0);
		pSunLongitude->setMaximum(180);
		pSunLongitude->setVal(i32(sunLongitude));

		pHLayout->addWidget(pSunLongitude);
		QSpinBox* pSpinBox = new QSpinBox;
		pSpinBox->setFixedWidth(42);
		pSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pSpinBox->setMinimum(0);
		pSpinBox->setMaximum(180);
		pSpinBox->setVal(i32(sunLongitude));

		pHLayout->addWidget(pSpinBox);

		m_longitudeSlider = pSunLongitude;
		m_longitudeSpin = pSpinBox;

		connect(m_longitudeSlider, &QSlider::valueChanged,
		        [pSpinBox, this](i32 value)
		{
			pSpinBox->blockSignals(true);
			pSpinBox->setVal(value);
			pSpinBox->blockSignals(false);
			UpdateLightingSettings();
		}
		        );

		connect(m_longitudeSpin, static_cast<void (QSpinBox::*)(i32)>(&QSpinBox::valueChanged),
		        [pSunLongitude, this](i32 value)
		{
			pSunLongitude->blockSignals(true);
			pSunLongitude->setVal(value);
			pSunLongitude->blockSignals(false);
			UpdateLightingSettings();
		}
		        );

		rootVerticalLayput->addLayout(pHLayout, Qt::AlignTop);
	}

	rootVerticalLayput->addStretch(1);
}

void QSunSettingsWidget::OnNewScene()
{
	ITimeOfDay* pTimeOfDay = GetTimeOfDay();
	if (!pTimeOfDay)
		return;

	const float sunLatitude = pTimeOfDay->GetSunLatitude();
	const float sunLongitude = pTimeOfDay->GetSunLongitude();

	m_latitudeSlider->blockSignals(true);
	m_latitudeSlider->setVal(i32(sunLatitude));
	m_latitudeSlider->blockSignals(false);

	m_latitudeSpin->blockSignals(true);
	m_latitudeSpin->setVal(i32(sunLatitude));
	m_latitudeSpin->blockSignals(false);

	m_longitudeSlider->blockSignals(true);
	m_longitudeSlider->setVal(i32(sunLongitude));
	m_longitudeSlider->blockSignals(false);

	m_longitudeSpin->blockSignals(true);
	m_longitudeSpin->setVal(i32(sunLongitude));
	m_longitudeSpin->blockSignals(false);
}

void QSunSettingsWidget::UpdateLightingSettings()
{
	CMission* pMission = GetIEditor()->GetDocument()->GetCurrentMission();
	LightingSettings* pLightingSettings = pMission->GetLighting();
	pLightingSettings->iSunRotation = m_latitudeSlider->value();
	pLightingSettings->iLongitude = m_longitudeSlider->value();

	GetIEditor()->SetModifiedFlag();

	GetIEditor()->GetGameEngine()->ReloadEnvironment();
	GetIEditor()->UpdateViews(eRedrawViewports);
	GetIEditor()->Get3DEngine()->SetRecomputeCachedShadows();

	ITimeOfDay* pTimeOfDay = GetTimeOfDay();
	pTimeOfDay->Update(false, true);
}

