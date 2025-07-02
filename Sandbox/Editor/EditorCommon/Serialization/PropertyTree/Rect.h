// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace property_tree {

struct Point
{
	Point() {}
	Point(i32 x, i32 y) : x_(x), y_(y) {}

	Point operator+(const Point& p) const { return Point(x_ + p.x_, y_ + p.y_); }
	Point operator-(const Point& p) const { return Point(x_ - p.x_, y_ - p.y_); }
	Point& operator+=(const Point& p) { *this = *this + p; return *this; }
	bool operator!=(const Point& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

	i32 manhattanLength() const { return abs(x_) + abs(y_); }

	i32 x_;
	i32 y_;

	// weird accessors to mimic QPoint
	i32 x() const { return x_; }
	i32 y() const { return y_; }
	void setX(i32 x) { x_ = x; }
	void setY(i32 y) { y_ = y; }
};

struct Rect
{
	i32 x;
	i32 y;
	i32 w;
	i32 h;

	i32 left() const { return x; }
	i32 top() const { return y; }
	void setTop(i32 top) { y = top; }
	i32 right() const { return x + w; }
	i32 bottom() const { return y + h; }

	Point topLeft() const { return Point(x, y); }
	Point bottomRight() const { return Point(x+w, y+h); }

	i32 width() const { return w; }
	i32 height() const { return h; }

	Rect() : x(0), y(0), w(0), h(0) {}
	Rect(i32 x, i32 y, i32 w, i32 h) : x(x), y(y), w(w), h(h) {}

	bool isValid() const { return w >= 0 && h >= 0; }

	template<class TPoint>
	bool contains(const TPoint& p) const {
		if (p.x() < x || p.x() >= x + w)
			return false;
		if (p.y() < y || p.y() >= y + h)
			return false;
		return true;
	}

	Point center() const { return Point(x + w / 2, y + h / 2); }

	Rect adjusted(i32 l, i32 t, i32 r, i32 b) const {
		return Rect(x + l, y + t,
			x + w + r - (x + l),
			y + h + b - (y + t));
	}

	Rect translated(i32 x, i32 y) const	{
		Rect r = *this;
		r.x += x;
		r.y += y;
		return r;
	}

	Rect united(const Rect& rhs) const {
		i32 newLeft = x < rhs.x ? x :rhs.x;
		i32 newTop = y < rhs.y ? y : rhs.y;
		i32 newRight = x + w > rhs.x + rhs.w ? x + w : rhs.x + rhs.w;
		i32 newBottom = y + h > rhs.y + rhs.h ? y + h : rhs.y + rhs.h;
		return Rect(newLeft, newTop, newRight - newLeft, newBottom - newTop);
	}
};

}

using property_tree::Rect; // temporary
using property_tree::Point; // temporary

