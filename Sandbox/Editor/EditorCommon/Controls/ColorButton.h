// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class EDITOR_COMMON_API CColorButton : public QPushButton
{
	Q_OBJECT
public:
	CColorButton();

	void SetColor(const QColor& color);
	void SetColor(const ColorB& color);
	void SetColor(const ColorF& color);

	QColor GetQColor() const { return m_color; }
	ColorB GetColorB() const;
	ColorF GetColorF() const;

	void SetHasAlpha(bool hasAlpha) { m_hasAlpha = hasAlpha; }

	//!Sent when the color has changed and editing is finished
	CDrxSignal<void(const QColor&)> signalColorChanged;

	//!Sent when the user is still editing the color
	CDrxSignal<void(const QColor&)> signalColorContinuousChanged;

protected:
	void paintEvent(QPaintEvent* paintEvent) override;

private:
	void OnClick();

	QColor m_color;
	bool m_hasAlpha;
};
