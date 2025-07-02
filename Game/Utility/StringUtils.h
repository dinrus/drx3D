// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
StringUtils.h

TODO: Move contents of this file into DrxEngine/DrxCommon/StringUtils.h
and contents of StringUtils.cpp into DrxEngine/DrxCommon/StringUtils.cpp
*************************************************************************/

#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

//--------------------------------------------------------------------------------
// drx_copyStringUntilFindChar: Parameter order is the same as strncpy;
// additional 'until' parameter defines which additional character should
// stop the copying. Return value is number of bytes (including NULL)
// written into 'destination', or 0 if 'until' character not found in
// first 'bufferLength' bytes of 'source'.
size_t drx_copyStringUntilFindChar(char * destination, tukk  source, size_t bufferLength, char until);

#if !defined(_DEBUG)
#define drx_displayMemInHexAndAscii(...)			(void)(0)
#else

class ITextOutputHandler
{
public:
	virtual void DoOutput(tukk  text) = 0;
};

class CDrxWatchOutputHandler : public ITextOutputHandler
{
	virtual void DoOutput(tukk  text);
};

class CDrxLogOutputHandler : public ITextOutputHandler
{
	virtual void DoOutput(tukk  text);
};

class CDrxLogAlwaysOutputHandler : public ITextOutputHandler
{
	virtual void DoOutput(tukk  text);
};

//--------------------------------------------------------------------------------
// drx_displayMemInHexAndAscii outputs (using an ITextOutputHandler subclass) the
// contents of the first 'size' bytes starting at memory location 'data'.
void drx_displayMemInHexAndAscii(tukk  startEachLineWith, ukk  data, i32 size, ITextOutputHandler & output, i32k bytesPerLine = 32);
#endif

//--------------------------------------------------------------------------------
// Generates a string in the format X days X hrs X mins X secs, or if useShortForm is set 00:00:00.
tukk  GetTimeString(i32 secs, bool useShortForm=false, bool includeSeconds=true, bool useSingleLetters=false);
tukk  GetTimeString(float secs, bool useShortForm=false, bool includeSeconds=true, bool useSingleLetters=false);


#endif // __STRING_UTILS_H__
