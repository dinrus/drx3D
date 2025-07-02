/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#include "stdafx.h"
#include "QtFlowLayout.h"

#include <QWidget>

QFlowLayout::QFlowLayout(QWidget* parent, i32 margin, i32 hSpacing, i32 vSpacing)
	: QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
	setContentsMargins(margin, margin, margin, margin);
}

QFlowLayout::QFlowLayout(i32 margin, i32 hSpacing, i32 vSpacing)
	: m_hSpace(hSpacing), m_vSpace(vSpacing)
{
	setContentsMargins(margin, margin, margin, margin);
}

QFlowLayout::~QFlowLayout()
{
	QLayoutItem* item;
	while ((item = takeAt(0)))
		delete item;
}

void QFlowLayout::addItem(QLayoutItem* item)
{
	itemList.append(item);
}

i32 QFlowLayout::horizontalSpacing() const
{
	if (m_hSpace >= 0)
	{
		return m_hSpace;
	}
	else
	{
		return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
	}
}

i32 QFlowLayout::verticalSpacing() const
{
	if (m_vSpace >= 0)
	{
		return m_vSpace;
	}
	else
	{
		return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
	}
}

i32 QFlowLayout::count() const
{
	return itemList.size();
}

QLayoutItem* QFlowLayout::itemAt(i32 index) const
{
	return itemList.value(index);
}

QLayoutItem* QFlowLayout::takeAt(i32 index)
{
	if (index >= 0 && index < itemList.size())
		return itemList.takeAt(index);
	else
		return 0;
}

Qt::Orientations QFlowLayout::expandingDirections() const
{
	return 0;
}

bool QFlowLayout::hasHeightForWidth() const
{
	return true;
}

i32 QFlowLayout::heightForWidth(i32 width) const
{
	i32 height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}

void QFlowLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}

QSize QFlowLayout::sizeHint() const
{
	return minimumSize();
}

QSize QFlowLayout::minimumSize() const
{
	QSize size;
	QLayoutItem* item;
	foreach(item, itemList)
	size = size.expandedTo(item->minimumSize());

	size += QSize(2 * margin(), 2 * margin());
	return size;
}

i32 QFlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
	i32 left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
	i32 x = effectiveRect.x();
	i32 y = effectiveRect.y();
	i32 lineHeight = 0;

	QLayoutItem* item;
	foreach(item, itemList)
	{
		QWidget* wid = item->widget();
		i32 spaceX = horizontalSpacing();
		if (spaceX == -1)
			spaceX = wid->style()->layoutSpacing(
			  QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
		i32 spaceY = verticalSpacing();
		if (spaceY == -1)
			spaceY = wid->style()->layoutSpacing(
			  QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
		i32 nextX = x + item->sizeHint().width() + spaceX;
		if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
		{
			x = effectiveRect.x();
			y = y + lineHeight + spaceY;
			nextX = x + item->sizeHint().width() + spaceX;
			lineHeight = 0;
		}

		if (!testOnly)
			item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

		x = nextX;
		lineHeight = qMax(lineHeight, item->sizeHint().height());
	}
	return y + lineHeight - rect.y() + bottom;
}
i32 QFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
	QObject* parent = this->parent();
	if (!parent)
	{
		return -1;
	}
	else if (parent->isWidgetType())
	{
		QWidget* pw = static_cast<QWidget*>(parent);
		return pw->style()->pixelMetric(pm, 0, pw);
	}
	else
	{
		return static_cast<QLayout*>(parent)->spacing();
	}
}

