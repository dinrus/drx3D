// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Range.h>
#include <DrxMovie/AnimTime.h>

#include <functional>
#include <vector>
#include <QRect>

#include <DrxMovie/AnimTime.h>

class QPainter;
class QPalette;

namespace DrawingPrimitives
{
struct SRulerOptions;
typedef std::function<void ()> TDrawCallback;

struct SRulerOptions
{
	QRect         m_rect;
	Range         m_visibleRange;
	Range         m_rulerRange;
	const Range*  m_pInnerRange;
	i32           m_markHeight;
	i32           m_shadowSize;
	i32           m_ticksYOffset;

	TDrawCallback m_drawBackgroundCallback;

	SRulerOptions()
		: m_pInnerRange(nullptr)
		, m_markHeight(0)
		, m_shadowSize(0)
		, m_ticksYOffset(0)
	{}
};

struct STick
{
	i32   m_position;
	float m_value;
	bool  m_bTenth;
	bool  m_bIsOuterTick;

	STick()
		: m_position(0)
		, m_value(.0f)
		, m_bTenth(false)
		, m_bIsOuterTick(false)
	{}
};

typedef SRulerOptions      STickOptions;

typedef std::vector<STick> Ticks;

void CalculateTicks(uint size, Range visibleRange, Range rulerRange, i32* pRulerPrecision, Range* pScreenRulerRange, Ticks& ticksOut, const Range* innerRange = nullptr);
void DrawTicks(const std::vector<STick>& ticks, QPainter& painter, const QPalette& palette, const STickOptions& options);
void DrawTicks(QPainter& painter, const QPalette& palette, const STickOptions& options);
void DrawRuler(QPainter& painter, const SRulerOptions& options, i32* pRulerPrecision);

class CRuler
{
public:
	struct SOptions
	{
		TRange<SAnimTime>       innerRange;
		TRange<SAnimTime>       visibleRange;
		TRange<SAnimTime>       rulerRange;

		QRect                   rect;
		i32                   markHeight;
		i32                   ticksYOffset;
		i32                   ticksPerFrame;
		SAnimTime::EDisplayMode timeUnit;

		SOptions()
			: markHeight(0)
			, ticksYOffset(0)
			, ticksPerFrame(0)
			, timeUnit(SAnimTime::EDisplayMode::Time)
		{}
	};

	struct STick
	{
		SAnimTime value;
		i32     position;
		bool      bTenth;
		bool      bIsOuterTick;

		STick()
			: position(0)
			, bTenth(false)
			, bIsOuterTick(false)
		{}
	};

	CRuler()
		: m_decimalDigits(0)
	{}

	SOptions&                 GetOptions() { return m_options; }

	void                      CalculateMarkers(TRange<i32>* pScreenRulerRange = nullptr);
	void                      Draw(QPainter& painter, const QPalette& palette);

	const std::vector<STick>& GetTicks() const { return m_ticks; }

protected:
	void     GenerateFormat(tuk szFormatOut);
	QString& ToString(const STick& tick, QString& strOut, tukk szFormat);

private:
	SOptions           m_options;
	std::vector<STick> m_ticks;
	i32              m_decimalDigits;
};
}

