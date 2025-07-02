#include <drx3D/Common/b3Logging.h>

#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif  //_WIN32

void drx3DPrintfFuncDefault(tukk msg)
{
#ifdef _WIN32
	OutputDebugStringA(msg);
#endif
	printf("%s", msg);
	//is this portable?
	fflush(stdout);
}

void drx3DWarningMessageFuncDefault(tukk msg)
{
#ifdef _WIN32
	OutputDebugStringA(msg);
#endif
	printf("%s", msg);
	//is this portable?
	fflush(stdout);
}

void drx3DErrorMessageFuncDefault(tukk msg)
{
#ifdef _WIN32
	OutputDebugStringA(msg);
#endif
	printf("%s", msg);

	//is this portable?
	fflush(stdout);
}

static drx3DPrintfFunc* b3s_printfFunc = drx3DPrintfFuncDefault;
static drx3DWarningMessageFunc* b3s_warningMessageFunc = drx3DWarningMessageFuncDefault;
static drx3DErrorMessageFunc* b3s_errorMessageFunc = drx3DErrorMessageFuncDefault;

///The developer can route drx3DPrintf output using their own implementation
void b3SetCustomPrintfFunc(drx3DPrintfFunc* printfFunc)
{
	b3s_printfFunc = printfFunc;
}
void b3SetCustomWarningMessageFunc(drx3DPrintfFunc* warningMessageFunc)
{
	b3s_warningMessageFunc = warningMessageFunc;
}
void b3SetCustomErrorMessageFunc(drx3DPrintfFunc* errorMessageFunc)
{
	b3s_errorMessageFunc = errorMessageFunc;
}

//#define D3_MAX_DEBUG_STRING_LENGTH 2048
#define D3_MAX_DEBUG_STRING_LENGTH 32768

void b3OutputPrintfVarArgsInternal(tukk str, ...)
{
	char strDebug[D3_MAX_DEBUG_STRING_LENGTH] = {0};
	va_list argList;
	va_start(argList, str);
#ifdef _MSC_VER
	vsprintf_s(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#else
	vsnprintf(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#endif
	(b3s_printfFunc)(strDebug);
	va_end(argList);
}
void b3OutputWarningMessageVarArgsInternal(tukk str, ...)
{
	char strDebug[D3_MAX_DEBUG_STRING_LENGTH] = {0};
	va_list argList;
	va_start(argList, str);
#ifdef _MSC_VER
	vsprintf_s(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#else
	vsnprintf(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#endif
	(b3s_warningMessageFunc)(strDebug);
	va_end(argList);
}
void b3OutputErrorMessageVarArgsInternal(tukk str, ...)
{
	char strDebug[D3_MAX_DEBUG_STRING_LENGTH] = {0};
	va_list argList;
	va_start(argList, str);
#ifdef _MSC_VER
	vsprintf_s(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#else
	vsnprintf(strDebug, D3_MAX_DEBUG_STRING_LENGTH, str, argList);
#endif
	(b3s_errorMessageFunc)(strDebug);
	va_end(argList);
}

void b3EnterProfileZoneDefault(tukk name)
{
}
void b3LeaveProfileZoneDefault()
{
}
static b3EnterProfileZoneFunc* b3s_enterFunc = b3EnterProfileZoneDefault;
static b3LeaveProfileZoneFunc* b3s_leaveFunc = b3LeaveProfileZoneDefault;
void b3EnterProfileZone(tukk name)
{
	(b3s_enterFunc)(name);
}
void b3LeaveProfileZone()
{
	(b3s_leaveFunc)();
}

void b3SetCustomEnterProfileZoneFunc(b3EnterProfileZoneFunc* enterFunc)
{
	b3s_enterFunc = enterFunc;
}
void b3SetCustomLeaveProfileZoneFunc(b3LeaveProfileZoneFunc* leaveFunc)
{
	b3s_leaveFunc = leaveFunc;
}

#ifndef _MSC_VER
#undef vsprintf_s
#endif
