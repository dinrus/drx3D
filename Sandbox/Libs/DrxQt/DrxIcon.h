// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QMap>
#include <QIcon>
#include <QString>
#include <QPainter>

#if QT_VERSION < 0x050000
#include <QIconEngineV2>
typedef QIconEngineV2 DrxQtIconEngine;
#else
#include <QIconEngine>
typedef QIconEngine DrxQtIconEngine;
#endif

#include "DrxQtAPI.h"
#include "DrxQtCompatibility.h"

struct DrxPixmapIconEngineEntry
{
	DrxPixmapIconEngineEntry() : mode(QIcon::Normal), state(QIcon::Off){}
	DrxPixmapIconEngineEntry(const QPixmap& pm, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
		: pixmap(pm), size(pm.size()), mode(m), state(s){}
	DrxPixmapIconEngineEntry(const QString& file, const QSize& sz = QSize(), QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
		: fileName(file), size(sz), mode(m), state(s){}
	DrxPixmapIconEngineEntry(const QString& file, const QImage& image, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off);
	QPixmap      pixmap;
	QString      fileName;
	QSize        size;
	QIcon::Mode  mode;
	QIcon::State state;
	bool         isNull() const { return (fileName.isEmpty() && pixmap.isNull()); }
};

inline DrxPixmapIconEngineEntry::DrxPixmapIconEngineEntry(const QString& file, const QImage& image, QIcon::Mode m, QIcon::State s)
	: fileName(file), size(image.size()), mode(m), state(s)
{
	pixmap.convertFromImage(image);
#if QT_VERSION >= 0x0500000
	// Reset the devicePixelRatio. The pixmap may be loaded from a @2x file,
	// but be used as a 1x pixmap by QIcon.
	pixmap.setDevicePixelRatio(1.0);
#endif
}

typedef QMap<QIcon::Mode, QBrush> DrxIconColorMap;

class DRXQT_API DrxPixmapIconEngine : public DrxQtIconEngine
{
public:
	DrxPixmapIconEngine(DrxIconColorMap colorMap = DrxIconColorMap());
	DrxPixmapIconEngine(const DrxPixmapIconEngine&);
	~DrxPixmapIconEngine();
	void                      paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) Q_DECL_OVERRIDE;
	QPixmap                   pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) Q_DECL_OVERRIDE;
	DrxPixmapIconEngineEntry* bestMatch(const QSize& size, QIcon::Mode mode, QIcon::State state, bool sizeOnly);
	QSize                     actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) Q_DECL_OVERRIDE;
	void                      addPixmap(const QPixmap& pixmap, QIcon::Mode mode, QIcon::State state) Q_DECL_OVERRIDE;
	void                      addFile(const QString& fileName, const QSize& size, QIcon::Mode mode, QIcon::State state) Q_DECL_OVERRIDE;

	QString                   key() const Q_DECL_OVERRIDE;
	DrxQtIconEngine* clone() const Q_DECL_OVERRIDE;
	bool                      read(QDataStream& in) Q_DECL_OVERRIDE;
	bool                      write(QDataStream& out) const Q_DECL_OVERRIDE;
	void                      virtual_hook(i32 id, uk data) Q_DECL_OVERRIDE;

private:
	DrxPixmapIconEngineEntry* tryMatch(const QSize& size, QIcon::Mode mode, QIcon::State state);
	QBrush                    getBrush(QIcon::Mode mode);
	QVector<DrxPixmapIconEngineEntry> pixmaps;
	DrxIconColorMap                   m_colormap;
	friend class DrxIcon;
};

class DRXQT_API DrxIcon : public QIcon
{
public:
	explicit DrxIcon(DrxIconColorMap colorMap = DrxIconColorMap());
	DrxIcon(const QIcon& icon);
	explicit DrxIcon(const QString& filename, DrxIconColorMap colorMap = DrxIconColorMap());
	explicit DrxIcon(const QPixmap& pixmap, DrxIconColorMap colorMap = DrxIconColorMap());
	static void SetDefaultTint(QIcon::Mode mode, QBrush brush);
};

