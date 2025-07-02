// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QObject>

class QMenuComboBox;

class QLabel;
class QFormLayout;

class CGenerateHeightmapUi
	: public QObject
{
	Q_OBJECT

public:
	struct Result
	{
		Result();
		bool isTerrain;
		i32  resolution;
		float unitSize;
	};
	struct Config
	{
		Config();
		bool   isOptional;
		Result initial;
	};

public:
	CGenerateHeightmapUi(const Config& config, QFormLayout* pFormLayout, QObject* pParent);

	Result GetResult() const;

signals:
	void IsTerrainChanged(bool);

private:
	bool IsTerrain() const;
	i32  GetResolution() const;
	float GetUnitSize() const;

	void SetupResolution(i32 initialResolution);
	void SetupUnitSize(float initialUnitSize);
	void SetupTerrainSize();

private:
	const bool m_isTerrainOptional;

	struct Ui
	{
		QMenuComboBox* m_pResolutionComboBox;
		QMenuComboBox* m_pUnitSizeComboBox;
		QLabel*        m_pSizeValueLabel;
	};
	Ui m_ui;
};

