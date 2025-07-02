// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
StringUtils.cpp
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/StringUtils.h>
#include <drx3D/Game/DrxWatch.h>
#include <drx3D/Game/UI/HUD/HUDUtils.h>

static bool s_stringUtils_assertEnabled = true;

//--------------------------------------------------------------------------------
size_t drx_copyStringUntilFindChar(char * destination, tukk  source, size_t bufferLength, char until)
{
	size_t reply = 0;

	DRX_ASSERT(destination);
	DRX_ASSERT(source);

	if (bufferLength)
	{
		size_t i;
		for (i = 0; source[i] && source[i] != until && (i + 1) < bufferLength; ++ i)
		{
			destination[i] = source[i];
		}
		destination[i] = '\0';
		reply = (source[i] == until) ? (i + 1) : 0;
	}

	return reply;
}

//--------------------------------------------------------------------------------

#ifndef drx_displayMemInHexAndAscii
void drx_displayMemInHexAndAscii(tukk  startEachLineWith, ukk  data, i32 size, ITextOutputHandler & output, i32k bytesPerLine)
{
	if (size > 0 && data)
	{
		u8k * charData = (u8k*) data;
		string hexLine, asciiLine;
		i32 bytesOnThisLineSoFar = 0;
		i32 padTo = 0;

		while (size)
		{
			if (++ bytesOnThisLineSoFar > bytesPerLine)
			{
				output.DoOutput(string().Format("%s%s  %s", startEachLineWith, hexLine.c_str(), asciiLine.c_str()));
				padTo = hexLine.length();
				bytesOnThisLineSoFar -= bytesPerLine;
				asciiLine = "";
				hexLine = "";
			}

			hexLine = hexLine + string().Format("%02x", (i32) * charData);

			if ((bytesOnThisLineSoFar & 7) == 0 && (bytesOnThisLineSoFar != bytesPerLine))
			{
				hexLine += " ";
			}

			asciiLine = asciiLine + string().Format("%c", (*charData >= 32 && *charData != 127) ? *charData : '.');

			++ charData;
			-- size;
		}

		output.DoOutput(string().Format("%s%s%s  %s", startEachLineWith, hexLine.c_str(), padTo ? string(padTo - hexLine.length(), ' ').c_str() : "", asciiLine.c_str()));
	}
	else
	{
		output.DoOutput(string().Format("%sPTR=%p SIZE=%d", startEachLineWith, size));
	}
}

//---------------------------------------------------------------------
void CDrxWatchOutputHandler::DoOutput(tukk  text)
{
	DrxWatch ("%s", text);
}

//---------------------------------------------------------------------
void CDrxLogOutputHandler::DoOutput(tukk  text)
{
	DrxLog ("%s", text);
}

//---------------------------------------------------------------------
void CDrxLogAlwaysOutputHandler::DoOutput(tukk  text)
{
	DrxLogAlways ("%s", text);
}
#endif

//---------------------------------------------------------------------
tukk  GetTimeString( i32 secs, bool useShortForm/*=false*/, bool includeSeconds/*=true*/, bool useSingleLetters/*=false*/ )
{
	i32 d, h, m, s;

	CGame::ExpandTimeSeconds(secs, d, h, m, s);

	static DrxFixedStringT<64> result;
	result.clear();

	if (useShortForm)
	{
		if(includeSeconds)
		{
			if (d>0)
				result.Format("%.2d:%.2d:%.2d:%.2d", d, h, m, s);
			else if(h>0)
				result.Format("%.2d:%.2d:%.2d", h, m, s);
			else
				result.Format("%.2d:%.2d", m, s);
		}
		else
		{
			if (d>0)
				result.Format("%.2d:%.2d:%.2d", d, h, m);
			else
				result.Format("%.2d:%.2d", h, m);
		}
	}
	else
	{
    if ( useSingleLetters == false )
    {
		  if (d==1)
			  result.Format("%d %s",d, CHUDUtils::LocalizeString("@ui_day"));
		  else if (d>1)
			  result.Format("%d %s",d, CHUDUtils::LocalizeString("@ui_days"));

		  if (h==1)
			  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", h, CHUDUtils::LocalizeString("@ui_hr"));
		  else if (h>1 || d>0)
			  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", h, CHUDUtils::LocalizeString("@ui_hrs"));

		  if (m==1)
			  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", m, CHUDUtils::LocalizeString("@ui_min"));
		  else if (m>1 || h>0 || d>0)
			  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", m, CHUDUtils::LocalizeString("@ui_mins"));

		  if(includeSeconds)
		  {
			  if (s==1)
				  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", s, CHUDUtils::LocalizeString("@ui_sec"));
			  else
				  result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", s, CHUDUtils::LocalizeString("@ui_secs"));
		  }
    }
    else
    {
      if (d>0)
        result.Format("%d %s",d, CHUDUtils::LocalizeString("@ui_mp_days"));

      if (h>0 || d>0)
        result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", h, CHUDUtils::LocalizeString("@ui_mp_hrs"));

      if (m>0 || h>0 || d>0)
        result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", m, CHUDUtils::LocalizeString("@ui_mp_mins"));

      if(includeSeconds)
      {
        if (s>0 || (h==0 && d==0 && m==0))
          result.Format("%s%s%d %s",result.c_str(), result.empty()?"":" ", s, CHUDUtils::LocalizeString("@ui_mp_sec"));
      }
    }
	}

	return result.c_str();
}

//---------------------------------------------------------------------
tukk  GetTimeString( float secs, bool useShortForm/*=false*/, bool includeSeconds/*=true*/, bool useSingleLetters/*=false*/ )
{
	return GetTimeString(int_round(secs), useShortForm, includeSeconds, useSingleLetters);
}
