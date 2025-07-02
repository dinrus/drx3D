// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "NodeGraphStyleUtils.h"

namespace DrxGraphEditor {

namespace StyleUtils {

QIcon CreateIcon(tukk szIcon, QColor color)
{
	return DrxIcon(szIcon, {
				{ QIcon::Mode::Normal, color }
	    });
}

QPixmap CreateIconPixmap(tukk szIcon, QColor color, QSize size)
{
	return CreateIconPixmap(CreateIcon(szIcon, color), size);
}

QPixmap CreateIconPixmap(const DrxIcon& icon, QSize size)
{
	return icon.pixmap(size.width(), size.height(), QIcon::Normal);
}

QPixmap ColorizePixmap(QPixmap pixmap, QColor color)
{
	const QSize size = pixmap.size();

	return DrxIcon(pixmap, {
				{ QIcon::Mode::Normal, color }
	    }).pixmap(size, QIcon::Normal);
}

QColor ColorMuliply(QColor a, QColor b)
{
	QColor r;
	r.setRedF(1.0f - (1.0f - a.redF()) * (1.0f - b.redF()));
	r.setGreenF(1.0f - (1.0f - a.greenF()) * (1.0f - b.greenF()));
	r.setBlueF(1.0f - (1.0f - a.blueF()) * (1.0f - b.blueF()));
	r.setAlphaF(1.0f - (1.0f - a.alphaF()) * (1.0f - b.alphaF()));
	return r;
}

}

}

