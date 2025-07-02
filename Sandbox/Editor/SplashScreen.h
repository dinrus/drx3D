// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <QSplashScreen>

class SplashScreen : public QSplashScreen
{

public:
	SplashScreen(const QPixmap& pixmap = QPixmap(), Qt::WindowFlags f = 0);
	SplashScreen(QWidget* parent, const QPixmap& pixmap = QPixmap(), Qt::WindowFlags f = 0);
	~SplashScreen();

	static void  SetVersion(const Version& v);
	static void  SetText(tukk text);

protected:
	virtual void drawContents(QPainter* painter) override;

private:
	void         SetInfo(tukk text);
	void         init();

	QString            version;
	QString            m_text;

	static SplashScreen* s_pLogoWindow;
};

