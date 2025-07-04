// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "SplashScreen.h"
#include <QPainter>
#include <QApplication>

SplashScreen* SplashScreen::s_pLogoWindow = 0;

SplashScreen::SplashScreen(const QPixmap& pixmap /* = QPixmap() */, Qt::WindowFlags f /* = 0 */)
	: QSplashScreen(pixmap, f)
{
	init();
}

SplashScreen::SplashScreen(QWidget* parent, const QPixmap& pixmap /* = QPixmap() */, Qt::WindowFlags f /* = 0 */)
	: QSplashScreen(parent, pixmap, f)
{
	init();
}

SplashScreen::~SplashScreen()
{
	s_pLogoWindow = 0;
}

void SplashScreen::SetVersion(const Version& v)
{
	char pVersionText[256];
	drx_sprintf(pVersionText, "Version %d.%d.%d - Build %d", v[3], v[2], v[1], v[0]);
	s_pLogoWindow->version = pVersionText;
	s_pLogoWindow->repaint();
	qApp->processEvents();
}

void SplashScreen::SetInfo(tukk text)
{
	m_text = text;
	update();
	qApp->processEvents();
}

void SplashScreen::SetText(tukk text)
{
	if (s_pLogoWindow)
		s_pLogoWindow->SetInfo(text);
}

void SplashScreen::drawContents(QPainter* painter)
{
	QSplashScreen::drawContents(painter);
	painter->setPen(Qt::white);
	painter->drawText(rect().adjusted(30, 5, -5, -42), Qt::AlignLeft | Qt::AlignBottom, version);
	painter->drawText(rect().adjusted(30, 5, -5, -25), Qt::AlignLeft | Qt::AlignBottom, m_text);
}

void SplashScreen::init()
{
	s_pLogoWindow = this;
	setWindowTitle("Starting DinrusPro 3D Sandbox Editor");
}

