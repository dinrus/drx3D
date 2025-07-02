// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once
#ifndef QTWM_DLL
#define QTWM_DLL
#endif // ! QTWM_DLL
#if defined(_WIN32) && defined(QTWM_DLL)
#ifdef DrxQt_EXPORTS
#define DRXQT_API __declspec(dllexport)
#else
#define DRXQT_API __declspec(dllimport)
#endif
#else
#define DRXQT_API
#endif

