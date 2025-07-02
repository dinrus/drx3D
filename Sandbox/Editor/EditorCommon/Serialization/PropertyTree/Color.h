/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Config.h>

namespace yasli { class Archive; }

namespace property_tree {

struct Color
{
    u8 b, g, r, a;
	
	Color() : r(255), g(255), b(255), a(255) { }
	Color(u8 _r, u8 _g, u8 _b, u8 _a = 255) { r=_r; g=_g; b=_b; a=_a; }
	explicit Color(u64 _argb) { argb() = _argb; }
	void set(i32 rc,i32 gc,i32 bc,i32 ac = 255) { r=rc; g=gc; b=bc; a=ac; }
	
	Color& setGDI(u64 color) { 
        b = (u8)(color >> 16);
        g = (u8)(color >> 8);
        r = (u8)(color);
		a = 255;
		return *this;
	}

	void setHSV(float h,float s,float v, u8 alpha = 255);
	void toHSV(float& h,float& s, float& v);

	Color& operator*= (float f) { r=i32(r*f); g=i32(g*f); b=i32(b*f); a=i32(a*f); return *this; }
	Color& operator+= (Color &p) { r+=p.r; g+=p.g; b+=p.b; a+=p.a; return *this; }
	Color& operator-= (Color &p) { r-=p.r; g-=p.g; b-=p.b; a-=p.a; return *this; }
	Color operator+ (Color &p) { return Color(r+p.r,g+p.g,b+p.b,a+p.a); }
	Color operator- (Color &p) { return Color(r-p.r,g-p.g,b-p.b,a-p.a); }
	Color operator* (float f) const { return Color(i32(r*f), i32(g*f), i32(b*f), i32(a*f)); }
	Color operator* (i32 f) const { return Color(r*f,g*f,b*f,a*f); }
	Color operator/ (i32 f) const { if(f!=0) f=(1<<16)/f; else f=1<<16; return Color((r*f)>>16,(g*f)>>16,(b*f)>>16,(a*f)>>16); }
	
	bool operator==(const Color& rhs) const { return argb() == rhs.argb(); }
	bool operator!=(const Color& rhs) const { return argb() != rhs.argb(); }
	
	u64 argb() const { return *reinterpret_cast<const u64*>(this); }
	u64& argb() { return *reinterpret_cast<u64*>(this); }
	u64 rgb() const { return r | g << 8 | b << 16; }
	u64 rgba() const { return r | g << 8 | b << 16 | a << 24; }
	u8& operator[](i32 i) { return ((u8*)this)[i];}
	Color interpolate(const Color &v, float f) const
	{
		return Color(i32(r+i32(v.r-r)*f),
					 i32(g+i32(v.g-g)*f),
					 i32(b+i32(v.b-b)*f),
					 i32(a+(v.a-a)*f));
	}
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar);
};

}

