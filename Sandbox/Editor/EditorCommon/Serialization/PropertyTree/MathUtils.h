/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once

inline i32 xround(float v)
{
	return i32(v + 0.5f);
}

inline i32 min(i32 a, i32 b)
{
	return a < b ? a : b;
}

inline i32 max(i32 a, i32 b)
{
	return a > b ? a : b;
}

inline float min(float a, float b)
{
	return a < b ? a : b;
}

inline float max(float a, float b)
{
	return a > b ? a : b;
}

inline float clamp(float value, float min, float max)
{
	return ::min(::max(min, value), max);
}

inline i32 clamp(i32 value, i32 min, i32 max)
{
	return ::min(::max(min, value), max);
}


