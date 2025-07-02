// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GRIDUTILS_H__
#define __GRIDUTILS_H__

namespace GridUtils
{
template<typename F> inline void IterateGrid(F& f, const float minPixelsPerTick, float zoomX, float originX, float fps, i32 left, i32 right)
{
	float pixelsPerSecond = zoomX;
	float pixelsPerFrame = pixelsPerSecond / fps;
	float framesPerTick = ceil(minPixelsPerTick / pixelsPerFrame);
	float scale = 1;
	bool foundScale = false;
	i32 numIters = 0;
	for (float OOM = 1; !foundScale; OOM *= 10)
	{
		float scales[] = { 1, 2, 5 };
		for (i32 scaleIndex = 0; !foundScale && scaleIndex < DRX_ARRAY_COUNT(scales); ++scaleIndex)
		{
			scale = scales[scaleIndex] * OOM;
			if (framesPerTick <= scale + 0.1f)
			{
				framesPerTick = scale;
				foundScale = true;
				if (numIters++ > 1000) break;
			}
		}
		if (numIters++ > 1000) break;
	}
	float pixelsPerTick = pixelsPerFrame * framesPerTick;
	float timeAtLeft = -left / zoomX + originX;
	float frameAtLeft = ceil(timeAtLeft * fps / framesPerTick) * framesPerTick;
	float firstTick = floor((frameAtLeft / fps - originX) * zoomX + 0.5f) + left;
	i32 frame = i32(frameAtLeft);
	i32 lockupPreventionCounter = 10000;
	for (float tickX = firstTick; tickX < right && lockupPreventionCounter >= 0; tickX += pixelsPerTick, --lockupPreventionCounter)
	{
		f(frame, tickX);

		frame += i32(framesPerTick);
	}
}
}

#endif //__GRIDUTILS_H__

