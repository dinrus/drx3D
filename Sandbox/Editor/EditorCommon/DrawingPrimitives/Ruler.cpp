// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "Ruler.h"
#include <drx3D/CoreX/Math/Drx_Math.h>
#include "QtUtil.h"
#include "EditorStyleHelper.h"

#include <QPainter>

namespace DrawingPrimitives
{
enum
{
	RULER_MIN_PIXELS_PER_TICK = 3,
};

void CalculateTicks(uint size, Range visibleRange, Range rulerRange, i32* pRulerPrecision, Range* pScreenRulerRange, Ticks& ticksOut, const Range* innerRange)
{
	ticksOut.clear();

	if (size == 0)
	{
		if (pRulerPrecision)
		{
			*pRulerPrecision = 0;
		}

		return;
	}

	const float pixelsPerUnit = visibleRange.Length() > 0.0f ? (float)size / visibleRange.Length() : 1.0f;

	const float startTime = rulerRange.start;
	const float endTime = rulerRange.end;
	const float totalDuration = endTime - startTime;

	const float ticksMinPower = log10f(RULER_MIN_PIXELS_PER_TICK);
	const float ticksPowerDelta = ticksMinPower - log10f(pixelsPerUnit);

	i32k digitsAfterPoint = max(-i32(ceil(ticksPowerDelta)) - 1, 0);
	if (pRulerPrecision)
	{
		*pRulerPrecision = digitsAfterPoint;
	}

	const float scaleStep = powf(10.0f, ceil(ticksPowerDelta));
	const float scaleStepPixels = scaleStep * pixelsPerUnit;
	i32k numMarkers = i32(totalDuration / scaleStep) + 1;

	const float startTimeRound = i32(startTime / scaleStep) * scaleStep;
	i32k startOffsetMod = i32(startTime / scaleStep) % 10;
	i32k scaleOffsetPixels = (startTime - startTimeRound) * pixelsPerUnit;

	i32k startX = (float)(rulerRange.start - visibleRange.start) * pixelsPerUnit;
	i32k endX = startX + (numMarkers - 1) * scaleStepPixels - scaleOffsetPixels;

	if (pScreenRulerRange)
	{
		*pScreenRulerRange = Range(startX, endX);
	}

	i32k startLoop = std::max((i32)((scaleOffsetPixels - startX) / scaleStepPixels) - 1, 0);
	i32k endLoop = std::min((i32)((size + scaleOffsetPixels - startX) / scaleStepPixels) + 1, numMarkers);

	i32k innerNumMarkers = innerRange ? i32((innerRange->end - innerRange->start) / scaleStep) : 0;
	i32k innerBegX = innerRange ? (innerRange->start - visibleRange.start) * pixelsPerUnit - 1 : startX;
	i32k innerEndX = innerRange ? innerBegX + innerNumMarkers * scaleStepPixels + 1 : endX;

	for (i32 i = startLoop; i < endLoop; ++i)
	{
		STick tick;

		i32k xi = startX + i;
		i32k x = xi * scaleStepPixels - scaleOffsetPixels;
		const float value = startTimeRound + i * scaleStep;

		tick.m_bTenth = (startOffsetMod + i) % 10 != 0;
		tick.m_position = x;
		tick.m_value = value;
		tick.m_bIsOuterTick = x<innerBegX || x> innerEndX;

		ticksOut.push_back(tick);
	}
}

void DrawTicks(const std::vector<STick>& ticks, QPainter& painter, const QPalette& palette, const STickOptions& options)
{
	QVector<QLine> innerTicks;
	innerTicks.reserve(ticks.size());
	QVector<QLine> outerTicks;
	outerTicks.reserve(ticks.size());

	i32k height = options.m_rect.height();
	i32k top = options.m_rect.top();
	i32k lowY = top + height - options.m_markHeight;
	i32k highY = top + height - options.m_markHeight / 2;

	for (const STick& tick : ticks)
	{
		i32k x = tick.m_position + options.m_rect.left();
		const QLine line = QLine(QPoint(x, !tick.m_bTenth ? lowY : highY), QPoint(x, top + height));
		if (!tick.m_bIsOuterTick)
			innerTicks.append(line);
		else
			outerTicks.append(line);
	}

	painter.setPen(QPen(GetStyleHelper()->rulerInnerTicks()));
	painter.drawLines(innerTicks);

	painter.setPen(QPen(GetStyleHelper()->rulerOuterTicks()));
	painter.drawLines(outerTicks);
}

void DrawTicks(QPainter& painter, const QPalette& palette, const SRulerOptions& options)
{
	std::vector<STick> ticks;
	CalculateTicks(options.m_rect.width(), options.m_visibleRange, options.m_rulerRange, nullptr, nullptr, ticks);
	DrawTicks(ticks, painter, palette, options);
}

void DrawRuler(QPainter& painter, const SRulerOptions& options, i32* pRulerPrecision)
{
	i32 rulerPrecision;
	Range screenRulerRange;
	std::vector<STick> ticks;
	CalculateTicks(options.m_rect.width(), options.m_visibleRange, options.m_rulerRange, &rulerPrecision, &screenRulerRange, ticks, options.m_pInnerRange);

	if (pRulerPrecision)
	{
		*pRulerPrecision = rulerPrecision;
	}

	// We don't want shadows in the new flat design.
	/*if (options.m_shadowSize > 0)
	   {
	   QRect shadowRect = QRect(options.m_rect.left(), options.m_rect.height(), options.m_rect.width(), options.m_shadowSize);
	   QLinearGradient upperGradient(shadowRect.left(), shadowRect.top(), shadowRect.left(), shadowRect.bottom());
	   upperGradient.setColorAt(0.0f, QColor(0, 0, 0, 128));
	   upperGradient.setColorAt(1.0f, QColor(0, 0, 0, 0));
	   QBrush upperBrush(upperGradient);
	   painter.fillRect(shadowRect, upperBrush);
	   }*/

	painter.fillRect(options.m_rect, GetStyleHelper()->rulerBackground());
	if (options.m_drawBackgroundCallback)
		options.m_drawBackgroundCallback();

	const QColor innerTickColor = GetStyleHelper()->rulerInnerTicks();
	const QColor outerTickColor = GetStyleHelper()->rulerOuterTicks();

	QFont font;
	font.setPixelSize(10);
	painter.setFont(font);

	char format[16];
	drx_sprintf(format, "%%.%df", rulerPrecision);

	i32k height = options.m_rect.height();
	i32k top = options.m_rect.top();

	i32k rulerYBase = top + height - options.m_ticksYOffset - 1;

	const QColor innerTickTextColor = GetStyleHelper()->rulerInnerTickText();
	const QColor outerTickTextColor = GetStyleHelper()->rulerOuterTickText();

	QString str;
	for (const STick& tick : ticks)
	{
		i32k x = tick.m_position + options.m_rect.left();
		const float value = tick.m_value;

		if (!tick.m_bIsOuterTick)
			painter.setPen(innerTickColor);
		else
			painter.setPen(outerTickColor);

		if (tick.m_bTenth)
		{
			painter.drawLine(QPoint(x, rulerYBase - options.m_markHeight / 2), QPoint(x, rulerYBase));
		}
		else
		{
			painter.drawLine(QPoint(x, rulerYBase - options.m_markHeight), QPoint(x, rulerYBase));

			if (!tick.m_bIsOuterTick)
				painter.setPen(innerTickTextColor);
			else
				painter.setPen(outerTickTextColor);

			str.sprintf(format, value);
			painter.drawText(QPoint(x + 2, rulerYBase - options.m_markHeight + 1), str);
		}
	}

	// Disabled. We don't want a fake 3D timeline in the new flat design.
	//painter.setPen(QPen(palette.color(QPalette::Dark)));
	//painter.drawLine(QPoint(options.m_rect.left(), 0), QPoint(options.m_rect.left(), options.m_rect.top() + height));
	//painter.drawLine(QPoint(options.m_rect.right(), 0), QPoint(options.m_rect.right(), options.m_rect.top() + height));
}

float InUnits(const SAnimTime& animTime, i32 unit)
{
	return static_cast<float>(animTime.GetTicks()) / unit;
}

void CRuler::CalculateMarkers(TRange<i32>* pScreenRulerRange)
{
	// TODO: Refactor this function!

	if (m_options.rect.width() == 0)
	{
		return;
	}

	m_ticks.clear();

	const bool bIsFramesOrTimecode = m_options.timeUnit == SAnimTime::EDisplayMode::Frames /* || m_options.timeUnit == SAnimTime::EDisplayMode::Timecode*/;
	i32k unit = !bIsFramesOrTimecode ? SAnimTime::numTicksPerSecond : m_options.ticksPerFrame;
	i32k fps = SAnimTime::numTicksPerSecond / m_options.ticksPerFrame;

	const float startTick = InUnits(m_options.rulerRange.start, unit);
	const float endTick = InUnits(m_options.rulerRange.end, unit);
	const float totalDuration = endTick - startTick;

	const float pixelsPerUnit = m_options.visibleRange.Length().GetTicks() > 0 ? m_options.rect.width() / InUnits(m_options.visibleRange.Length(), unit) : 1.0f;

	const float ticksMinPower = log10f(7);
	const float ticksPowerDelta = ticksMinPower - log10f(pixelsPerUnit);

	m_decimalDigits = max(-i32(ceil(ticksPowerDelta)) - 1, 0);

	const float scaleStep = powf(10.f, ceil(ticksPowerDelta));
	const float scaleStepPixels = scaleStep * pixelsPerUnit;
	i32k numMarkers = i32(totalDuration / scaleStep) + 1;

	const float startTimeRound = i32(startTick / scaleStep) * scaleStep;
	i32k startOffsetMod = i32(startTick / scaleStep) % 10;
	i32k scaleOffsetPixels = (startTick - startTimeRound) * pixelsPerUnit;

	i32k startX = static_cast<i32>(InUnits(m_options.rulerRange.start - m_options.visibleRange.start, unit) * pixelsPerUnit);
	i32k endX = startX + (numMarkers - 1) * scaleStepPixels - scaleOffsetPixels;

	if (pScreenRulerRange)
	{
		*pScreenRulerRange = TRange<i32>(startX, endX);
	}

	i32k startLoop = std::max((i32)((scaleOffsetPixels - startX) / scaleStepPixels) - 1, 0);
	i32k endLoop = std::min((i32)((m_options.rect.width() + scaleOffsetPixels - startX) / scaleStepPixels) + 1, numMarkers);

	i32k innerNumMarkers = !m_options.innerRange.IsEmpty() ? i32(InUnits(m_options.innerRange.end - m_options.innerRange.start, unit) / scaleStep) : 0;
	i32k innerBegX = !m_options.innerRange.IsEmpty() ? i32(InUnits(m_options.innerRange.start - m_options.visibleRange.start, unit) * pixelsPerUnit - 1) : startX;
	i32k innerEndX = !m_options.innerRange.IsEmpty() ? innerBegX + innerNumMarkers * scaleStepPixels + 1 : endX;

	const float startFrameRound = startTimeRound * float(unit);
	const float frameScaleStep = scaleStep * float(unit);

	for (i32 i = startLoop; i < endLoop; ++i)
	{
		STick tick;

		i32k xi = startX + i;
		i32k x = xi * scaleStepPixels - scaleOffsetPixels;

		tick.position = x;
		if (!bIsFramesOrTimecode)
		{
			tick.bTenth = ((startOffsetMod + i) % 10) == 0;
			tick.value.SetTime(startTimeRound + i * scaleStep);
		}
		else
		{
			tick.bTenth = endLoop <= fps || ((startOffsetMod + i) % 10) == 0;
			if (/*m_options.timeUnit == SAnimTime::EDisplayMode::Frames && */ !tick.bTenth)
				continue;

			tick.value.SetTicks(startFrameRound + i * frameScaleStep);
		}
		tick.bIsOuterTick = x<innerBegX || x> innerEndX;

		m_ticks.push_back(tick);
	}
}

void CRuler::Draw(QPainter& painter, const QPalette& palette)
{
	if (m_options.rect.width() == 0)
	{
		return;
	}

	painter.fillRect(m_options.rect, QtUtil::InterpolateColor(palette.color(QPalette::Button), palette.color(QPalette::Midlight), 0.25f));

	const QColor innerTickColor = GetStyleHelper()->rulerInnerTicks();
	const QColor outerTickColor = GetStyleHelper()->rulerOuterTicks();

	QFont font;
	font.setPixelSize(10);
	painter.setFont(font);

	char format[16];
	GenerateFormat(format);

	i32k height = m_options.rect.height();
	i32k top = m_options.rect.top();

	i32k rulerYBase = top + height - m_options.ticksYOffset - 1;

	const QColor innerTickTextColor = GetStyleHelper()->rulerInnerTickText();
	const QColor outerTickTextColor = GetStyleHelper()->rulerOuterTickText();

	i32k lowMarker = m_options.markHeight;
	i32k highMarker = m_options.markHeight / 2;

	QString str;
	for (const STick& tick : m_ticks)
	{
		i32k x = tick.position + m_options.rect.left();

		if (!tick.bIsOuterTick)
			painter.setPen(innerTickColor);
		else
			painter.setPen(outerTickColor);

		if (!tick.bTenth)
		{
			painter.drawLine(QPoint(x, rulerYBase - highMarker), QPoint(x, rulerYBase));
		}
		else
		{
			painter.drawLine(QPoint(x, rulerYBase - lowMarker), QPoint(x, rulerYBase));

			if (!tick.bIsOuterTick)
				painter.setPen(innerTickTextColor);
			else
				painter.setPen(outerTickTextColor);

			painter.drawText(QPoint(x + 2, rulerYBase - m_options.markHeight + 1), ToString(tick, str, format));
		}
	}
}

void CRuler::GenerateFormat(tuk szFormatOut)
{
	switch (m_options.timeUnit)
	{
	default:
	case SAnimTime::EDisplayMode::Time:
		sprintf(szFormatOut, "%%.%df", m_decimalDigits);
		return;
	case SAnimTime::EDisplayMode::Frames:
	case SAnimTime::EDisplayMode::Ticks:
		sprintf(szFormatOut, "%%d");
		return;
	case SAnimTime::EDisplayMode::Timecode:
		{
			i32k fps = SAnimTime::numTicksPerSecond / m_options.ticksPerFrame;
			if (fps < 100)
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.2d");
			else if (fps < 1000)
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.3d");
			else
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.4d");
		}
		return;
	}
}

QString& CRuler::ToString(const STick& tick, QString& strOut, tukk szFormat)
{
	DrxFixedStringT<16> str;
	switch (m_options.timeUnit)
	{
	default:
	case SAnimTime::EDisplayMode::Time:
		{
			str.Format(szFormat, tick.value.ToFloat());
		}
		break;
	case SAnimTime::EDisplayMode::Frames:
		{
			i32 frame = tick.value.GetTicks() / m_options.ticksPerFrame;
			str.Format(szFormat, frame);
		}
		break;
	case SAnimTime::EDisplayMode::Ticks:
		{
			str.Format(szFormat, tick.value.GetTicks());
		}
		break;
	case SAnimTime::EDisplayMode::Timecode:
		{
			i32 seconds = static_cast<i32>(tick.value.ToFloat());
			i32 frames = (tick.value.GetTicks() - SAnimTime::numTicksPerSecond * seconds) / m_options.ticksPerFrame;
			i32 minutes = seconds / 60;
			seconds -= minutes * 60;

			str.Format(szFormat, minutes, seconds, frames);
		}
		break;
	}

	strOut = str.c_str();
	return strOut;
}
}

