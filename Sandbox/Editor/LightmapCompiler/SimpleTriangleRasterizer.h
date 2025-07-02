// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// Helper class
//
// clipping is done in integer
//

class CSimpleTriangleRasterizer
{
public:

	class IRasterizeSink
	{
	public:

		//! is called once per triangel for the first possible visible line
		//! /param iniStartY
		virtual void Triangle(i32k iniStartY)
		{
		}

		//! callback function
		//! /param infXLeft included - not clipped against left and reight border
		//! /param infXRight excluded - not clipped against left and reight border
		//! /param iniXLeft included
		//! /param iniXRight excluded
		//! /param iniY
		virtual void Line(const float infXLeft, const float infXRight,
		                  i32k iniXLeft, i32k iniXRight, i32k iniY) = 0;
	};

	typedef u64 DWORD;

	// -----------------------------------------------------

	//! implementation sink sample
	class CDWORDFlatFill : public IRasterizeSink
	{
	public:

		//! constructor
		CDWORDFlatFill(DWORD* inpBuffer, const DWORD indwPitchInPixels, DWORD indwValue)
		{
			m_dwValue = indwValue;
			m_pBuffer = inpBuffer;
			m_dwPitchInPixels = indwPitchInPixels;
		}

		virtual void Triangle(i32k iniY)
		{
			m_pBufferLine = &m_pBuffer[iniY * m_dwPitchInPixels];
		}

		virtual void Line(const float infXLeft, const float infXRight,
		                  i32k iniLeft, i32k iniRight, i32k iniY)
		{
			DWORD* mem = &m_pBufferLine[iniLeft];

			for (i32 x = iniLeft; x < iniRight; x++)
				*mem++ = m_dwValue;

			m_pBufferLine += m_dwPitchInPixels;
		}

	private:
		DWORD  m_dwValue;         //!< fill value
		DWORD* m_pBufferLine;     //!< to get rid of the multiplication per line

		DWORD  m_dwPitchInPixels; //!< in DWORDS, not in Bytes
		DWORD* m_pBuffer;         //!< pointer to the buffer
	};

	// -----------------------------------------------------

	//! constructor
	//! /param iniWidth excluded
	//! /param iniHeight excluded
	CSimpleTriangleRasterizer(i32k iniWidth, i32k iniHeight)
	{
		m_iMinX = 0;
		m_iMinY = 0;
		m_iMaxX = iniWidth - 1;
		m_iMaxY = iniHeight - 1;
	}
	/*
	   //! constructor
	   //! /param iniMinX included
	   //! /param iniMinY included
	   //! /param iniMaxX included
	   //! /param iniMaxY included
	   CSimpleTriangleRasterizer( i32k iniMinX, i32k iniMinY, i32k iniMaxX, i32k iniMaxY )
	   {
	    m_iMinX=iniMinX;
	    m_iMinY=iniMinY;
	    m_iMaxX=iniMaxX;
	    m_iMaxY=iniMaxY;
	   }
	 */
	//! simple triangle filler with clipping (optimizable), not subpixel correct
	//! /param pBuffer pointer o the color buffer
	//! /param indwWidth width of the color buffer
	//! /param indwHeight height of the color buffer
	//! /param x array of the x coordiantes of the three vertices
	//! /param y array of the x coordiantes of the three vertices
	//! /param indwValue value of the triangle
	void DWORDFlatFill(DWORD* inpBuffer, const DWORD indwPitchInPixels, float x[3], float y[3], DWORD indwValue, bool inbConservative)
	{
		CDWORDFlatFill pix(inpBuffer, indwPitchInPixels, indwValue);

		if (inbConservative)
			CallbackFillConservative(x, y, &pix);
		else
			CallbackFillSubpixelCorrect(x, y, &pix);
	}

	// Rectangle around triangle - more stable - use for debugging purpose
	void CallbackFillRectConservative(float x[3], float y[3], IRasterizeSink* inpSink);

	//! subpixel correct triangle filler (conservative or not conservative)
	//! \param pBuffer pointe to the DWORD
	//! \param indwWidth width of the buffer pBuffer pointes to
	//! \param indwHeight height of the buffer pBuffer pointes to
	//! \param x array of the x coordiantes of the three vertices
	//! \param y array of the x coordiantes of the three vertices
	//! \param inpSink pointer to the sink interface (is called per triangle and per triangle line)
	void CallbackFillConservative(float x[3], float y[3], IRasterizeSink* inpSink);

	//! subpixel correct triangle filler (conservative or not conservative)
	//! \param pBuffer pointe to the DWORD
	//! \param indwWidth width of the buffer pBuffer pointes to
	//! \param indwHeight height of the buffer pBuffer pointes to
	//! \param x array of the x coordiantes of the three vertices
	//! \param y array of the x coordiantes of the three vertices
	//! \param inpSink pointer to the sink interface (is called per triangle and per triangle line)
	void CallbackFillSubpixelCorrect(float x[3], float y[3], IRasterizeSink* inpSink);

	//!
	//! /param inoutfX
	//! /param inoutfY
	//! /param infAmount could be positive or negative
	static void ShrinkTriangle(float inoutfX[3], float inoutfY[3], float infAmount);

private:

	// Clipping Rect;

	i32 m_iMinX;      //!< minimum x value included
	i32 m_iMinY;      //!< minimum y value included
	i32 m_iMaxX;      //!< maximum x value included
	i32 m_iMaxY;      //!< maximum x value included

	void lambertHorizlineConservative(float fx1, float fx2, i32 y, IRasterizeSink* inpSink);
	void lambertHorizlineSubpixelCorrect(float fx1, float fx2, i32 y, IRasterizeSink* inpSink);
	void CopyAndSortY(const float infX[3], const float infY[3], float outfX[3], float outfY[3]);
};

// extension ideas:
// * callback with coverage mask (possible non ordered sampling)
// * z-buffer behaviour
// * gouraud shading
// * texture mapping with nearest/bicubic/bilinear filter
// * further primitives: thick line, ellipse
// * build a template version
// *

