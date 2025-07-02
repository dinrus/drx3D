#ifndef TIME_SERIES_CANVAS_H
#define TIME_SERIES_CANVAS_H

#include <drxtypes.h>

class TimeSeriesCanvas
{
protected:
	struct TimeSeriesInternalData* m_internalData;
	void shift1PixelToLeft();

public:
	TimeSeriesCanvas(struct Common2dCanvasInterface* canvasInterface, i32 width, i32 height, tukk windowTitle);
	virtual ~TimeSeriesCanvas();

	void setupTimeSeries(float yScale, i32 ticksPerSecond, i32 startTime, bool clearCanvas = true);
	void addDataSource(tukk dataSourceLabel, u8 red, u8 green, u8 blue);
	void insertDataAtCurrentTime(float value, i32 dataSourceIndex, bool connectToPrevious);
	float getCurrentTime() const;
	void grapicalPrintf(tukk str, uk fontData, i32 rasterposx, i32 rasterposy, u8 red, u8 green, u8 blue, u8 alpha);

	virtual void nextTick();
};

#endif  //TIME_SERIES_CANVAS_H