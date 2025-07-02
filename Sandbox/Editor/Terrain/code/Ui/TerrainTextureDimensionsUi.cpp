// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "TerrainTextureDimensionsUi.h"

#include "Controls/QMenuComboBox.h"

#include <QFormLayout>

namespace Private_GenerateTerrainTextureUi
{
namespace
{

static i32k kResolutionStart = 1 << 9;
static i32k kResolutionMax = 1 << 14;
static i32k kResolutionDefault = 1 << 12;

} //endns
} //endns Private_TerrainTextureDimensions

CTerrainTextureDimensionsUi::Result::Result()
	: resolution(Private_GenerateTerrainTextureUi::kResolutionDefault)
{
}

CTerrainTextureDimensionsUi::CTerrainTextureDimensionsUi(const CTerrainTextureDimensionsUi::Result& initial, QFormLayout* pFormLayout, QObject* pParent)
	: QObject(pParent)
{
	auto pTextureResolutionComboBox = new QMenuComboBox();
	pFormLayout->addRow(tr("Texture &Dimensions"), pTextureResolutionComboBox);

	m_ui.m_pResolutionComboBox = pTextureResolutionComboBox;

	SetupResolutions(initial.resolution);
}

CTerrainTextureDimensionsUi::Result CTerrainTextureDimensionsUi::GetResult() const
{
	Result result;
	result.resolution = GetResolution();
	return result;
}

void CTerrainTextureDimensionsUi::SetEnabled(bool enabled)
{
	m_ui.m_pResolutionComboBox->setEnabled(enabled);
}

i32 CTerrainTextureDimensionsUi::GetResolution() const
{
	auto combobox = m_ui.m_pResolutionComboBox;
	return combobox->GetCurrentData().toInt();
}

void CTerrainTextureDimensionsUi::SetupResolutions(i32 initialResolution)
{
	auto combobox = m_ui.m_pResolutionComboBox;
	combobox->Clear();
	for (auto resolution = Private_GenerateTerrainTextureUi::kResolutionStart;
	     resolution <= Private_GenerateTerrainTextureUi::kResolutionMax;
	     resolution *= 2)
	{
		auto label = QStringLiteral("%1 x %1").arg(resolution);
		combobox->AddItem(label, resolution);
		if (resolution == initialResolution)
		{
			combobox->SetChecked(combobox->GetItemCount() - 1);
		}
	}
}

