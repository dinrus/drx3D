// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QObject>

class QMenuComboBox;

class QFormLayout;

/**
 * @brief Manages form elements to select a proper terrain texture size
 */
class CTerrainTextureDimensionsUi
	: public QObject
{
	Q_OBJECT

public:
	struct Result
	{
		Result();
		i32 resolution;
	};

public:
	explicit CTerrainTextureDimensionsUi(const Result& initial, QFormLayout* pFormLayout, QObject* pParent = nullptr);

	Result GetResult() const;

	void   SetEnabled(bool enabled);

private:
	i32 GetResolution() const;

private:
	void SetupResolutions(i32);

private:
	struct Ui
	{
		QMenuComboBox* m_pResolutionComboBox;
	};
	Ui m_ui;
};

