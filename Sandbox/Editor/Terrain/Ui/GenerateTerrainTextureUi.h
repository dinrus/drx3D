// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QObject>

class QFormLayout;
class CTerrainTextureDimensionsUi;
class QNumericBox;
class QCheckBox;

class CGenerateTerrainTextureUi
	: public QObject
{
	Q_OBJECT
public:
	struct Result
	{
		Result();
		i32   resolution;
		float colorMultiplier;

		bool  isHighQualityCompression;
	};

public:
	explicit CGenerateTerrainTextureUi(const Result& initial, QFormLayout* pFormLayout, QObject* parent = nullptr);

	Result GetResult() const;

	void   SetEnabled(bool enabled);

private:
	i32   GetResolution() const;
	float GetColorMultiplier() const;
	bool  IsHighQualityCompression() const;

private:
	struct Ui
	{
		CTerrainTextureDimensionsUi* m_pTerrainTextureDimensionsUi;
		QNumericBox*           m_pColorMultiplierSpinBox;
		QCheckBox*                   m_pHighQualityCompressionCheckBox;
	};
	Ui m_ui;
};

