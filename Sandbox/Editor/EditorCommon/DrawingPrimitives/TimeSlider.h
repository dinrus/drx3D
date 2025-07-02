// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QRect>

class QPainter;
class QPalette;

namespace DrawingPrimitives
{
struct STimeSliderOptions
{
	QRect m_rect;
	i32   m_precision;
	i32   m_position;
	float m_time;
	bool  m_bHasFocus;
};

void DrawTimeSlider(QPainter& painter, const STimeSliderOptions& options);
}

