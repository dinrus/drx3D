// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace ScreenResolution
{
  struct SScreenResolution
  {
    i32 iWidth;
    i32 iHeight;
    u32 nDephtPerPixel;
    string sResolution;

    SScreenResolution(u32 _iWidth, u32 _iHeight, u32 _nDepthPerPixel, tukk _sResolution):
        iWidth(_iWidth)
      , iHeight(_iHeight)
      , nDephtPerPixel(_nDepthPerPixel)
      , sResolution(_sResolution)
    {}

    SScreenResolution()
    {}
  };

  void InitialiseScreenResolutions();
  void ReleaseScreenResolutions();

  bool GetScreenResolutionAtIndex(u32 nIndex, SScreenResolution& resolution);
	bool HasResolution(i32k width, i32k height);
	i32 GetNearestResolution(i32k width, i32k height);
  u32 GetNumScreenResolutions();
}