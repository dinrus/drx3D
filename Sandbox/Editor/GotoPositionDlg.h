// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "Controls/EditorDialog.h"

class QString;
class QLineEdit;

class CPositionSpinBox;
class CRotationSpinBox;
class CSegmentSpinBox;

class CGotoPositionDialog
	: public CEditorDialog
{
public:
	explicit CGotoPositionDialog();
	~CGotoPositionDialog();

protected:
	void OnPositionStringChanged();
	void OnValueChanged();

	void GotoPosition();

private:
	QLineEdit*        m_pPositionString;

	CPositionSpinBox* m_pPositionBoxes[3];
	CRotationSpinBox* m_pRotationBoxes[3];
	CSegmentSpinBox*  m_pSegmentBoxes[2];

	Vec3              m_position;
	Ang3              m_rotation;
	Vec2              m_segment;
};

