/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

//#include "stdafx.h"
#include <memory>
#include "IconXPMCache.h"
#include "QPropertyTree.h"
#include <drx3D/CoreX/Serialization/yasli/decorators/IconXPM.h>
#include "Serialization/PropertyTree/Unicode.h"
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QBitmap>

#ifndef _MSC_VER
# define _stricmp strcasecmp
#endif

// ---------------------------------------------------------------------------

namespace property_tree {

IconXPMCache::~IconXPMCache()
{
	flush();
}


void IconXPMCache::flush()
{
	for (XPMToBitmap::iterator it = xpmToImageMap_.begin(); it != xpmToImageMap_.end(); ++it)
		delete it->second.bitmap;
	xpmToImageMap_.clear();

	for (FilenameToBitmap::iterator it = filenameToImageMap_.begin(); it != filenameToImageMap_.end(); ++it)
		delete it->second.bitmap;
	filenameToImageMap_.clear();
}

struct RGBAImage
{
	i32 width_;
	i32 height_;
	std::vector<Color> pixels_;

	RGBAImage() : width_(0), height_(0) {}
};

bool IconXPMCache::parseXPM(RGBAImage* out, const yasli::IconXPM& icon) 
{
	if (icon.lineCount < 3) {
		return false;
	}

	// parse values
	std::vector<Color> pixels;
	i32 width = 0;
	i32 height = 0;
	i32 charsPerPixel = 0;
	i32 colorCount = 0;
	i32 hotSpotX = -1;
	i32 hotSpotY = -1;

	i32 scanResult = sscanf(icon.source[0], "%d %d %d %d %d %d", &width, &height, &colorCount, &charsPerPixel, &hotSpotX, &hotSpotY);
	if (scanResult != 4 && scanResult != 6)
		return false;

	if (charsPerPixel > 4)
		return false;

	if(icon.lineCount != 1 + colorCount + height) {
		YASLI_ASSERT(0 && "Wrong line count");
		return false;
	}

	// parse colors
	std::vector<std::pair<i32, Color> > colors;
	colors.resize(colorCount);

	for (i32 colorIndex = 0; colorIndex < colorCount; ++colorIndex) {
		tukk p = icon.source[colorIndex + 1];
		i32 code = 0;
		for (i32 charIndex = 0; charIndex < charsPerPixel; ++charIndex) {
			if (*p == '\0')
				return false;
			code = (code << 8) | *p;
			++p;
		}
		colors[colorIndex].first = code;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p != 'c' && *p != 'g')
			return false;
		++p;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p == '#') {
			++p;
			if (strlen(p) == 6) {
				i32 colorCode;
				if(sscanf(p, "%x", &colorCode) != 1)
					return false;
				Color color((colorCode & 0xff0000) >> 16,
							(colorCode & 0xff00) >> 8,
							(colorCode & 0xff),
							255);
				colors[colorIndex].second = color;
			}
		}
		else {
			if(_stricmp(p, "None") == 0)
				colors[colorIndex].second = Color(0, 0, 0, 0);
			else if (_stricmp(p, "Black") == 0)
				colors[colorIndex].second = Color(0, 0, 0, 255)/*GetSysColor(COLOR_BTNTEXT)*/;
			else {
				// unknown color
				colors[colorIndex].second = Color(255, 0, 0, 255);
			}
		}
	}

	// parse pixels
	pixels.resize(width * height);
	i32 pi = 0;
	for (i32 y = 0; y < height; ++y) {
		tukk p = icon.source[1 + colorCount + y];
		if (strlen(p) != width * charsPerPixel)
			return false;

		for (i32 x = 0; x < width; ++x) {
			i32 code = 0;
			for (i32 i = 0; i < charsPerPixel; ++i) {
				code = (code << 8) | *p;
				++p;
			}

			for (size_t i = 0; i < size_t(colorCount); ++i)
				if (colors[i].first == code)
					pixels[pi] = colors[i].second;
			++pi;
		}
	}

	out->pixels_.swap(pixels);
	out->width_ = width;
	out->height_ = height;
	return true;
}


QImage* IconXPMCache::getImageForIcon(const Icon& icon)
{
	if (icon.type == icon.TYPE_XPM)
	{
		XPMToBitmap::iterator it = xpmToImageMap_.find(icon.xpm.source);
		if (it != xpmToImageMap_.end())
			return it->second.bitmap;
		RGBAImage image;
		if (!parseXPM(&image, icon.xpm))
			return 0;

		BitmapCache& cache = xpmToImageMap_[icon.xpm.source];
		cache.pixels.swap(image.pixels_);
		cache.bitmap = new QImage((u8*)&cache.pixels[0], image.width_, image.height_, QImage::Format_ARGB32);
		return cache.bitmap;
	}
	else if (icon.type == icon.TYPE_FILE)
	{
		FilenameToBitmap::iterator it = filenameToImageMap_.find(icon.filename);
		if (it != filenameToImageMap_.end())
			return it->second.bitmap;
		;

		BitmapCache& cache = filenameToImageMap_[icon.filename];
		cache.bitmap = new QImage(property_tree::PropertyIcon(QString::fromUtf8(icon.filename.c_str())).pixmap(16, 16).toImage());
		return cache.bitmap;
	}

	static QImage emptyImage;
	return &emptyImage;
}

// ---------------------------------------------------------------------------


}

