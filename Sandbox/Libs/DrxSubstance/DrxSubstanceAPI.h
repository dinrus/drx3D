// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifndef DRX_SUBSTANCE_STATIC
#if defined(DrxSubstance_EXPORTS)
#define DRX_SUBSTANCE_API __declspec(dllexport)
#else
#define DRX_SUBSTANCE_API __declspec(dllimport)
#endif
#else
#define DRX_SUBSTANCE_API
#endif

