// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_NOISE_H__D47F4AD3_03B3_4E25_957A_697628BCDDEA__INCLUDED_)
#define AFX_NOISE_H__D47F4AD3_03B3_4E25_957A_697628BCDDEA__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

class CDynamicArray2D;

// Parameter structures
struct SNoiseParams
{
	u32 iWidth;
	u32 iHeight;
	u32 iCover;
	u32 iPasses;
	float        fFrequencyStep;
	float        fFrequency;
	u32 iSmoothness;
	u32 iRandom;
	float        fFade;
	float        iSharpness;
	bool         bBlueSky;
	bool         bValid; // Used internally to verify serialized data, no
	                     // need to set it from outside the class
};

// Basis matrix for spline interpolation
#define CR00 -0.5f
#define CR01 1.5f
#define CR02 -1.5f
#define CR03 0.5f
#define CR10 1.0f
#define CR11 -2.5f
#define CR12 2.0f
#define CR13 -0.5f
#define CR20 -0.5f
#define CR21 0.0f
#define CR22 0.5f
#define CR23 0.0f
#define CR30 0.0f
#define CR31 1.0f
#define CR32 0.0f
#define CR33 0.0f

class CNoise
{
public:
	CNoise();
	virtual ~CNoise();

	void  FracSynthPass(CDynamicArray2D* hBuf, float freq, float zscale, i32 xres, i32 zres, BOOL bLoop);
	float Spline(float x, /*i32 nknots,*/ float* knot);

};

#endif // !defined(AFX_NOISE_H__D47F4AD3_03B3_4E25_957A_697628BCDDEA__INCLUDED_)

