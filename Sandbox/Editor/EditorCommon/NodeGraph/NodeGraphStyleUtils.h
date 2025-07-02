// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QColor>
#include <QSize>
#include <QPixmap>
#include <QRectF>

namespace DrxGraphEditor {

namespace StyleUtils {

QIcon   CreateIcon(tukk szIcon, QColor color);
QPixmap CreateIconPixmap(tukk szIcon, QColor color, QSize size);
QPixmap CreateIconPixmap(const DrxIcon& icon, QSize size);

QPixmap ColorizePixmap(QPixmap pixmap, QColor color);
QColor  ColorMuliply(QColor a, QColor b);

}

}

inline QRectF& operator+=(QRectF& a, const QRectF& b)
{
	a.adjust(b.left(), b.top(), b.right(), b.bottom());
	return a;
}

inline QRectF operator+(const QRectF& a, const QRectF& b)
{
	return a.adjusted(b.left(), b.top(), b.right(), b.bottom());
}

inline QRectF& operator+=(QRectF& a, const QRect& b)
{
	a.adjust(b.left(), b.top(), b.right(), b.bottom());
	return a;
}

inline QRectF operator+(const QRectF& a, const QRect& b)
{
	return a.adjusted(b.left(), b.top(), b.right(), b.bottom());
}

