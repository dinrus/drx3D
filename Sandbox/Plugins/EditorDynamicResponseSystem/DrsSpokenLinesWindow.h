// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include <drx3D/CoreX/Serialization/Forward.h>

class QPropertyTree;
class QPushButton;

class CSpokenLinesWidget : public QWidget
{
public:
	CSpokenLinesWidget();
	void SetAutoUpdateActive(bool bValue);

	void Serialize(Serialization::IArchive& ar);

protected:
	QPropertyTree* m_pPropertyTree;
	QPushButton*   m_pUpdateButton;

	i32            m_SerializationFilter;
};

