// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ScreenResolution.h>
#include <vector>

namespace
{
  std::vector<ScreenResolution::SScreenResolution> s_ScreenResolutions;
}

namespace ScreenResolution
{
  //////////////////////////////////////////////////////////////////////////
  void InitialiseScreenResolutions()
  {
    #if DRX_PLATFORM_DESKTOP
		
		DrxFixedStringT<16> format;

		SDispFormat *formats = NULL;
		i32 numFormats = gEnv->pRenderer->EnumDisplayFormats(NULL);
		if(numFormats)
		{
			formats = new SDispFormat[numFormats];
			gEnv->pRenderer->EnumDisplayFormats(formats);
		}

		i32 lastWidth, lastHeight;
		lastHeight = lastWidth = -1;

		for(i32 i = 0; i < numFormats; ++i)
		{

			if(HasResolution(formats[i].m_Width, formats[i].m_Height))
			{
				continue;
			}

			if(formats[i].m_Width < 800)
				continue;

			format.Format("%i X %i", formats[i].m_Width, formats[i].m_Height);

			SScreenResolution resolution(formats[i].m_Width, formats[i].m_Height, formats[i].m_BPP, format.c_str());
			s_ScreenResolutions.push_back(resolution);

			lastHeight = formats[i].m_Height;
			lastWidth = formats[i].m_Width;
		}

		if(formats)
			delete[] formats;

    #endif
  }
  //////////////////////////////////////////////////////////////////////////
  void ReleaseScreenResolutions()
  {
    s_ScreenResolutions.clear();
  }
	//////////////////////////////////////////////////////////////////////////
	bool GetScreenResolutionAtIndex(u32 nIndex, SScreenResolution& resolution)
	{
		assert(!(nIndex >= s_ScreenResolutions.size()));

		if (nIndex < s_ScreenResolutions.size())
		{
			resolution = s_ScreenResolutions.at(nIndex);
			return true;
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	bool HasResolution(i32k width, i32k height)
	{
		u32k size = GetNumScreenResolutions();
		for(u32 i=0; i<size; ++i)
		{
			if(s_ScreenResolutions[i].iWidth == width && s_ScreenResolutions[i].iHeight == height)
			{
				return true;
			}
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	i32 GetNearestResolution(i32k width, i32k height)
	{
		u32k size = GetNumScreenResolutions();
		float minDifference = 1.0f;
		i32 nearestIndex = -1;
		for(u32 i=0; i<size; ++i)
		{

			if(s_ScreenResolutions[i].iWidth == width && s_ScreenResolutions[i].iHeight == height)
			{
				return (i32)i;
			}

			float difference = 0.0f;

			difference += abs(1.0f - (float)s_ScreenResolutions[i].iWidth / (float)width);
			difference += abs(1.0f - (float)s_ScreenResolutions[i].iHeight / (float)height);

			if(difference < minDifference)
			{
				nearestIndex = (i32)i;
				minDifference = difference;
			}
		}

		return nearestIndex;
	}
  //////////////////////////////////////////////////////////////////////////
  u32 GetNumScreenResolutions()
  {
    return s_ScreenResolutions.size();
  }
}
