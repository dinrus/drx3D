// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../DrxIcon.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QImageReader>
#include <QPalette>
#include <QPixmapCache>
#include <QString>
#include <QStringBuilder>
#include <QStyle>
#include <QStyleOption>
#include "QBitmap"

class QWindow;
#if QT_VERSION >= 0x050000
#include <QWindow>
#endif

#ifdef WIN32
#include <windows.h>
#endif

// This class is a modified version of QPixmapIconEngine from the Qt source code.

DrxPixmapIconEngine::DrxPixmapIconEngine(DrxIconColorMap colorMap) : m_colormap(colorMap)
{
}

DrxPixmapIconEngine::DrxPixmapIconEngine(const DrxPixmapIconEngine& other) : DrxQtIconEngine(other), m_colormap(other.m_colormap), pixmaps(other.pixmaps)
{
}

DrxPixmapIconEngine::~DrxPixmapIconEngine()
{
}

static qreal qt_effective_device_pixel_ratio(QWindow* window = 0)
{
#if QT_VERSION >= 0x050000
	if (!qApp->testAttribute(Qt::AA_UseHighDpiPixmaps))
		return qreal(1.0);

	if (window)
		return window->devicePixelRatio();

	return qApp->devicePixelRatio(); // Don't know which window to target.
#else
	return qreal(1.0);
#endif
}

static inline i32                area(const QSize& s) { return s.width() * s.height(); }

static DrxPixmapIconEngineEntry* bestSizeMatch(const QSize& size, DrxPixmapIconEngineEntry* pa, DrxPixmapIconEngineEntry* pb)
{
	i32 s = area(size);
	if (pa->size == QSize() && pa->pixmap.isNull())
	{
		pa->pixmap = QPixmap(pa->fileName);
		pa->size = pa->pixmap.size();
	}
	i32 a = area(pa->size);
	if (pb->size == QSize() && pb->pixmap.isNull())
	{
		pb->pixmap = QPixmap(pb->fileName);
		pb->size = pb->pixmap.size();
	}
	i32 b = area(pb->size);
	i32 res = a;
	if (qMin(a, b) >= s)
		res = qMin(a, b);
	else
		res = qMax(a, b);
	if (res == a)
		return pa;
	return pb;
}

template<typename T>
struct HexString
{
	inline HexString(const T t)
		: val(t)
	{}

	inline void write(QChar*& dest) const
	{
		const ushort hexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		tukk c = reinterpret_cast<tukk >(&val);
		for (uint i = 0; i < sizeof(T); ++i)
		{
			* dest++ = hexChars[*c & 0xf];
			* dest++ = hexChars[(*c & 0xf0) >> 4];
			++c;
		}
	}
	const T val;
};

template<typename T>
struct QConcatenable<HexString<T>>
{
	typedef HexString<T> type;
	enum { ExactSize = true };
	static i32         size(const HexString<T>&)                      { return sizeof(T) * 2; }
	static inline void appendTo(const HexString<T>& str, QChar*& out) { str.write(out); }
	typedef QString ConvertTo;
};

void DrxPixmapIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
	QSize pixmapSize = rect.size() * qt_effective_device_pixel_ratio(0);
	QPixmap px = pixmap(pixmapSize, mode, state);
	painter->drawPixmap(rect, px);
}

QPixmap applyQIconStyleHelper(QIcon::Mode mode, const QPixmap& base)
{
	QStyleOption opt(0);
	opt.palette = QGuiApplication::palette();
	return QApplication::style()->generatedIconPixmap(mode, base, &opt);
}

QPixmap DrxPixmapIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	QPixmap pm;
	DrxPixmapIconEngineEntry* pe = bestMatch(size, mode, state, false);
	if (pe)
		pm = pe->pixmap;

	if (pm.isNull())
	{
		i32 idx = pixmaps.count();
		while (--idx >= 0)
		{
			if (pe == &pixmaps[idx])
			{
				pixmaps.remove(idx);
				break;
			}
		}
		if (pixmaps.isEmpty())
			return pm;
		else
			return pixmap(size, mode, state);
	}

	QSize actualSize = pm.size();
	if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
		actualSize.scale(size, Qt::KeepAspectRatio);

	QString key = QLatin1String("drx_")
	              % HexString<quint64>(pm.cacheKey())
	              % HexString<uint>(pe->mode)
	              % HexString<quint64>(QGuiApplication::palette().cacheKey())
	              % HexString<uint>(actualSize.width())
	              % HexString<uint>(actualSize.height());

	if (QPixmapCache::find(key % HexString<uint>(mode) % HexString<uint>(state), pm))
		return pm;

	if (pm.size() != actualSize)
		pm = pm.scaled(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QPixmap normal = QPixmap(pm.size());
	normal.fill(Qt::transparent); // Enable alpha channel in the output
	QPainter p(&normal);
	p.drawPixmap(QPoint(0, 0), pm);


	//Go through pixels and change alpha channel to 0 if RGB values are equal --> colors from white over gray to black
	QImage imageColorOnly = pm.toImage();
	for (i32 i = 0; i < imageColorOnly.width(); i++)
	{
		for (i32 j = 0; j < imageColorOnly.height(); j++)
		{
			QRgb pixel = imageColorOnly.pixel(i, j);
			i32 red = qRed(pixel);
			i32 green = qGreen(pixel);
			i32 blue = qBlue(pixel);
			if (red == green && red == blue)
			{
				imageColorOnly.setPixelColor(i, j, QColor(red, green, blue, 0));
			}
		}
	}
	//This pixelmap now only contains the colored part of the image
	QPixmap pmColor = pmColor.fromImage(imageColorOnly);

	// Tint full image
	p.setCompositionMode(QPainter::CompositionMode_Multiply);
	QIcon::Mode brushMode = mode;
	if (state == QIcon::On && mode != QIcon::Disabled)
	{
		brushMode = QIcon::Selected;
	}
	p.fillRect(normal.rect(), getBrush(brushMode));

	//After tinting, overdraw with colored part of the image
	p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	p.drawPixmap(QPoint(0, 0), pmColor);

	// Use original alpha channel to crop image
	p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
	p.drawPixmap(QPoint(0, 0), pm);

	QPixmapCache::insert(key % HexString<uint>(mode) % HexString<uint>(state), normal);
	return normal;
}

DrxPixmapIconEngineEntry* DrxPixmapIconEngine::bestMatch(const QSize& size, QIcon::Mode mode, QIcon::State state, bool sizeOnly)
{
	DrxPixmapIconEngineEntry* pe = tryMatch(size, mode, state);
	while (!pe)
	{
		QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
		if (mode == QIcon::Disabled || mode == QIcon::Selected)
		{
			QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
			if ((pe = tryMatch(size, QIcon::Normal, state)))
				break;
			if ((pe = tryMatch(size, QIcon::Active, state)))
				break;
			if ((pe = tryMatch(size, mode, oppositeState)))
				break;
			if ((pe = tryMatch(size, QIcon::Normal, oppositeState)))
				break;
			if ((pe = tryMatch(size, QIcon::Active, oppositeState)))
				break;
			if ((pe = tryMatch(size, oppositeMode, state)))
				break;
			if ((pe = tryMatch(size, oppositeMode, oppositeState)))
				break;
		}
		else
		{
			QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
			if ((pe = tryMatch(size, oppositeMode, state)))
				break;
			if ((pe = tryMatch(size, mode, oppositeState)))
				break;
			if ((pe = tryMatch(size, oppositeMode, oppositeState)))
				break;
			if ((pe = tryMatch(size, QIcon::Disabled, state)))
				break;
			if ((pe = tryMatch(size, QIcon::Selected, state)))
				break;
			if ((pe = tryMatch(size, QIcon::Disabled, oppositeState)))
				break;
			if ((pe = tryMatch(size, QIcon::Selected, oppositeState)))
				break;
		}

		if (!pe)
			return pe;
	}

	if (sizeOnly ? (pe->size.isNull() || !pe->size.isValid()) : pe->pixmap.isNull())
	{
		pe->pixmap = QPixmap(pe->fileName);
		if (!pe->pixmap.isNull())
			pe->size = pe->pixmap.size();
	}

	return pe;
}

QSize DrxPixmapIconEngine::actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	QSize actualSize;
	if (DrxPixmapIconEngineEntry* pe = bestMatch(size, mode, state, true))
		actualSize = pe->size;

	if (actualSize.isNull())
		return actualSize;

	if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
		actualSize.scale(size, Qt::KeepAspectRatio);
	return actualSize;
}

bool DrxPixmapIconEngine::read(QDataStream& in)
{
	i32 num_entries;
	QPixmap pm;
	QString fileName;
	QSize sz;
	uint mode;
	uint state;

	in >> num_entries;
	for (i32 i = 0; i < num_entries; ++i)
	{
		if (in.atEnd())
		{
			pixmaps.clear();
			return false;
		}
		in >> pm;
		in >> fileName;
		in >> sz;
		in >> mode;
		in >> state;
		if (pm.isNull())
		{
			addFile(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
		}
		else
		{
			DrxPixmapIconEngineEntry pe(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
			pe.pixmap = pm;
			pixmaps += pe;
		}
	}
	return true;
}

void DrxPixmapIconEngine::addPixmap(const QPixmap& pixmap, QIcon::Mode mode, QIcon::State state)
{
	if (!pixmap.isNull())
	{
		DrxPixmapIconEngineEntry* pe = tryMatch(pixmap.size(), mode, state);
		if (pe && pe->size == pixmap.size())
		{
			pe->pixmap = pixmap;
			pe->fileName.clear();
		}
		else
		{
			pixmaps += DrxPixmapIconEngineEntry(pixmap, mode, state);
		}
	}
}

static inline i32 origIcoDepth(const QImage& image)
{
	const QString s = image.text(QStringLiteral("_q_icoOrigDepth"));
	return s.isEmpty() ? 32 : s.toInt();
}

static inline i32 findBySize(const QList<QImage>& images, const QSize& size)
{
	for (i32 i = 0; i < images.size(); ++i)
	{
		if (images.at(i).size() == size)
			return i;
	}
	return -1;
}

namespace {
class ImageReader
{
public:
	ImageReader(const QString& fileName) : m_reader(fileName), m_atEnd(false) {}

	QByteArray format() const { return m_reader.format(); }

	bool       read(QImage* image)
	{
		if (m_atEnd)
			return false;
		*image = m_reader.read();
		if (!image->size().isValid())
		{
			m_atEnd = true;
			return false;
		}
		m_atEnd = !m_reader.jumpToNextImage();
		return true;
	}

private:
	QImageReader m_reader;
	bool         m_atEnd;
};
} //endns

void DrxPixmapIconEngine::addFile(const QString& fileName, const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	if (fileName.isEmpty())
		return;
	QString abs = fileName.startsWith(QLatin1Char(':')) ? fileName : QFileInfo(fileName).absoluteFilePath();
	bool ignoreSize = !size.isValid();
	ImageReader imageReaderTry(abs);
	QByteArray format = imageReaderTry.format();
	if (format.isEmpty()) // Device failed to open or unsupported format.
	{
		qWarning() << "Could not load icon at " << fileName;
		//Try the default Icon
		abs = QFileInfo("icons:common/general_icon_missing.ico").absoluteFilePath();
		ImageReader imageReaderNew(abs);
		format = imageReaderNew.format();
	}
	ImageReader imageReader(abs);
	QImage image;
	if (format != "ico")
	{
		if (ignoreSize)   // No size specified: Add all images.
		{
			while (imageReader.read(&image))
				pixmaps += DrxPixmapIconEngineEntry(abs, image, mode, state);
		}
		else
		{
			// Try to match size. If that fails, add a placeholder with the filename and empty pixmap for the size.
			while (imageReader.read(&image) && image.size() != size)
			{
			}
			pixmaps += image.size() == size ?
			           DrxPixmapIconEngineEntry(abs, image, mode, state) : DrxPixmapIconEngineEntry(abs, size, mode, state);
		}
		return;
	}
	// Special case for reading Windows ".ico" files. Historically (QTBUG-39287),
	// these files may contain low-resolution images. As this information is lost,
	// ICOReader sets the original format as an image text key value. Read all matching
	// images into a list trying to find the highest quality per size.
	QList<QImage> icoImages;
	while (imageReader.read(&image))
	{
		if (ignoreSize || image.size() == size)
		{
			i32k position = findBySize(icoImages, image.size());
			if (position >= 0)   // Higher quality available? -> replace.
			{
				if (origIcoDepth(image) > origIcoDepth(icoImages.at(position)))
					icoImages[position] = image;
			}
			else
			{
				icoImages.append(image);
			}
		}
	}
	foreach(const QImage &i, icoImages)
	pixmaps += DrxPixmapIconEngineEntry(abs, i, mode, state);
	if (icoImages.isEmpty() && !ignoreSize) // Add placeholder with the filename and empty pixmap for the size.
		pixmaps += DrxPixmapIconEngineEntry(abs, size, mode, state);
}

QString DrxPixmapIconEngine::key() const
{
	return QLatin1String("DrxPixmapIconEngine");
}

DrxQtIconEngine* DrxPixmapIconEngine::clone() const
{
	return new DrxPixmapIconEngine(*this);
}

bool DrxPixmapIconEngine::write(QDataStream& out) const
{
	i32 num_entries = pixmaps.size();
	out << num_entries;
	for (i32 i = 0; i < num_entries; ++i)
	{
		if (pixmaps.at(i).pixmap.isNull())
			out << QPixmap(pixmaps.at(i).fileName);
		else
			out << pixmaps.at(i).pixmap;
		out << pixmaps.at(i).fileName;
		out << pixmaps.at(i).size;
		out << (uint) pixmaps.at(i).mode;
		out << (uint) pixmaps.at(i).state;
	}
	return true;
}

void DrxPixmapIconEngine::virtual_hook(i32 id, uk data)
{
	switch (id)
	{
	case DrxQtIconEngine::AvailableSizesHook:
		{
		DrxQtIconEngine::AvailableSizesArgument& arg =
			  *reinterpret_cast<DrxQtIconEngine::AvailableSizesArgument*>(data);
			arg.sizes.clear();
			for (i32 i = 0; i < pixmaps.size(); ++i)
			{
				DrxPixmapIconEngineEntry& pe = pixmaps[i];
				if (pe.size == QSize() && pe.pixmap.isNull())
				{
					pe.pixmap = QPixmap(pe.fileName);
					pe.size = pe.pixmap.size();
				}
				if (pe.mode == arg.mode && pe.state == arg.state && !pe.size.isEmpty())
					arg.sizes.push_back(pe.size);
			}
			break;
		}
	default:
		DrxQtIconEngine::virtual_hook(id, data);
	}
}

DrxPixmapIconEngineEntry* DrxPixmapIconEngine::tryMatch(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
	DrxPixmapIconEngineEntry* pe = 0;
	for (i32 i = 0; i < pixmaps.count(); ++i)
		if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state)
		{
			if (pe)
				pe = bestSizeMatch(size, &pixmaps[i], pe);
			else
				pe = &pixmaps[i];
		}
	return pe;
}

namespace DefaultTints
{
	QBrush Normal = QBrush(QColor(255, 255, 255));
	QBrush Disabled = QBrush(QColor(255, 255, 255));
	QBrush Active = QBrush(QColor(255, 255, 255));
	QBrush Selected = QBrush(QColor(255, 255, 255));
}

QBrush DrxPixmapIconEngine::getBrush(QIcon::Mode mode)
{
	if (m_colormap.contains(mode))
	{
		return m_colormap[mode];
	}
	switch (mode)
	{
	case QIcon::Normal:
		return DefaultTints::Normal;
	case QIcon::Disabled:
		return DefaultTints::Disabled;
	case QIcon::Active:
		return DefaultTints::Active;
	case QIcon::Selected:
		return DefaultTints::Selected;
	default:
		return DefaultTints::Normal;
	}
}

DrxIcon::DrxIcon(DrxIconColorMap colorMap /*= DrxIconColorMap()*/) : QIcon(new DrxPixmapIconEngine(colorMap))
{
}

DrxIcon::DrxIcon(const QString& filename, DrxIconColorMap colorMap /*= DrxIconColorMap()*/) : QIcon(new DrxPixmapIconEngine(colorMap))
{
	addFile(filename);
}

DrxIcon::DrxIcon(const QPixmap& pixmap, DrxIconColorMap colorMap /*= DrxIconColorMap()*/) : QIcon(new DrxPixmapIconEngine(colorMap))
{
	addPixmap(pixmap);
}

DrxIcon::DrxIcon(const QIcon& icon) : QIcon(icon)
{

}

void DrxIcon::SetDefaultTint(QIcon::Mode mode, QBrush brush)
{
	switch (mode)
	{
	case QIcon::Normal:
		DefaultTints::Normal = brush;
		break;
	case QIcon::Disabled:
		DefaultTints::Disabled = brush;
		break;
	case QIcon::Active:
		DefaultTints::Active = brush;
		break;
	case QIcon::Selected:
		DefaultTints::Selected = brush;
		break;
	default:
		break;
	}
}

