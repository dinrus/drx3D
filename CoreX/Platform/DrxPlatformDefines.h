// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// Заметка: используются '#define xxx 1', так как в этом случае работают
// при проверке как '#if xxx', так и '#if defined(xxx)'.

//! Это определение добавлено для проверки уже произошедшего включения
//! DrxPlatformDefines.h программистом (и уже доступно DRX_PLATFORM_xxx).
#define DRX_PLATFORM 1

// Обнаружение ЦПБ (Центрального Процессорного Блока). Смотрите:
// http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491c/BABJFEFG.html
#if defined(__x86_64__) || defined(_M_X64)
	#define DRX_PLATFORM_X64       1
	#define DRX_PLATFORM_64BIT     1
	#define DRX_PLATFORM_SSE2      1
#elif defined(__i386) || defined(_M_IX86)
	#define DRX_PLATFORM_X86       1
	#define DRX_PLATFORM_32BIT     1
#elif defined(__arm__)
	#define DRX_PLATFORM_ARM    1
	#define DRX_PLATFORM_32BIT  1
	#if defined(__ARM_NEON__)
		#define DRX_PLATFORM_NEON 1
	#endif
#elif defined(__aarch64__)
	#define DRX_PLATFORM_ARM    1
	#define DRX_PLATFORM_64BIT  1
#else
	#define DRX_PLATFORM_UNKNOWNCPU 1
#endif

#if defined(__APPLE__)

	#define DRX_PLATFORM_APPLE 1
	#define DRX_PLATFORM_POSIX 1
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		#define DRX_PLATFORM_MOBILE 1
		#define DRX_PLATFORM_IOS    1
		#if !DRX_PLATFORM_UNKNOWNCPU
			#error iOS: Неподдерживаемый ЦПБ.
		#endif
		#define DRX_PLATFORM_64BIT   1
	#elif TARGET_OS_MAC
		#define DRX_PLATFORM_DESKTOP 1
		#define DRX_PLATFORM_MAC     1
		#if !DRX_PLATFORM_X64
			#error Неподдерживаемый ЦПБ Mac (поддерживается только x86-64).
		#endif
	#else
		#error Неизвестная платформа Apple.
	#endif

#elif defined(_DURANGO) || defined(_XBOX_ONE)

	#define DRX_PLATFORM_CONSOLE 1
	#define DRX_PLATFORM_DURANGO 1
	#define DRX_PLATFORM_WINAPI  1
	#if !DRX_PLATFORM_X64
		#error Неподдерживаемый ЦПБ Durango (поддерживается только x86-64).
	#endif
	#define DRX_PLATFORM_SSE4    1
	#define DRX_PLATFORM_F16C    1
	#define DRX_PLATFORM_BMI1    1
	#define DRX_PLATFORM_TBM     1

#elif defined(_ORBIS) || defined(__ORBIS__)

	#define DRX_PLATFORM_CONSOLE 1
	#define DRX_PLATFORM_ORBIS   1
	#define DRX_PLATFORM_POSIX   1
	#if !DRX_PLATFORM_X64
		#error Неподдерживаемый ЦПБ Orbis (поддерживается только x86-64).
	#endif
	#define DRX_PLATFORM_SSE4    1
	#define DRX_PLATFORM_F16C    1
	#define DRX_PLATFORM_BMI1    1
	#define DRX_PLATFORM_TBM     0

#elif defined(ANDROID) || defined(__ANDROID__)

	#define DRX_PLATFORM_MOBILE  1
	#define DRX_PLATFORM_ANDROID 1
	#define DRX_PLATFORM_POSIX   1
	#if DRX_PLATFORM_ARM && !(DRX_PLATFORM_64BIT || DRX_PLATFORM_32BIT)
		#error Неподдерживаемый ЦПБ Android (поддерживаются только 32-битный и 64-битный ARM).
	#endif

#elif defined(_WIN32)

	#define DRX_PLATFORM_DESKTOP 1
	#define DRX_PLATFORM_WINDOWS 1
	#define DRX_PLATFORM_WINAPI  1
	#if defined(_WIN64)
		#if !DRX_PLATFORM_X64
			#error Неподдерживаемый ЦПБ Windows 64 (поддерживается только x86-64).
		#endif
	#else
		#if !DRX_PLATFORM_X86
			#error Неподдерживаемый ЦПБ Windows 32 (поддерживается только x86).
		#endif
	#endif

#elif defined(__linux__) || defined(__linux)

	#define DRX_PLATFORM_DESKTOP 1
	#define DRX_PLATFORM_LINUX   1
	#define DRX_PLATFORM_POSIX   1
	#if !DRX_PLATFORM_X64 && !DRX_PLATFORM_X86
		#error Неподдерживаемый ЦПБ Linux (поддерживаются только x86 и x86-64).
	#endif

#else

	#error Неизвестная целевая платформа.

#endif

#if DRX_PLATFORM_AVX
#define DRX_PLATFORM_ALIGNMENT 32
#elif DRX_PLATFORM_SSE2 || DRX_PLATFORM_SSE4 || DRX_PLATFORM_NEON
#define DRX_PLATFORM_ALIGNMENT 16U
#else
#define DRX_PLATFORM_ALIGNMENT 1U
#endif

// Валидация

#if defined(DRX_PLATFORM_X64) && DRX_PLATFORM_X64 != 1
	#error Неверное значение DRX_PLATFORM_X64.
#endif

#if defined(DRX_PLATFORM_X86) && DRX_PLATFORM_X86 != 1
	#error Неверное значение DRX_PLATFORM_X86.
#endif

#if defined(DRX_PLATFORM_ARM) && DRX_PLATFORM_ARM != 1
	#error Неверное значение DRX_PLATFORM_ARM.
#endif

#if defined(DRX_PLATFORM_UNKNOWN_CPU) && DRX_PLATFORM_UNKNOWN_CPU != 1
	#error Неверное значение DRX_PLATFORM_UNKNOWN_CPU.
#endif

#if DRX_PLATFORM_X64 + DRX_PLATFORM_X86 + DRX_PLATFORM_ARM + DRX_PLATFORM_UNKNOWNCPU != 1
	#error Invalid CPU type.
#endif

#if defined(DRX_PLATFORM_64BIT) && DRX_PLATFORM_64BIT != 1
	#error Неверное значение DRX_PLATFORM_64BIT.
#endif

#if defined(DRX_PLATFORM_32BIT) && DRX_PLATFORM_32BIT != 1
	#error Неверное значение DRX_PLATFORM_32BIT.
#endif

#if DRX_PLATFORM_64BIT + DRX_PLATFORM_32BIT != 1
	#error Неполноценный DRX_PLATFORM_xxBIT.
#endif

#if defined(DRX_PLATFORM_DESKTOP) && DRX_PLATFORM_DESKTOP != 1
	#error Неверное значение DRX_PLATFORM_DESKTOP.
#endif

#if defined(DRX_PLATFORM_MOBILE) && DRX_PLATFORM_MOBILE != 1
	#error Неверное значение DRX_PLATFORM_MOBILE.
#endif

#if defined(DRX_PLATFORM_CONSOLE) && DRX_PLATFORM_CONSOLE != 1
	#error Неверное значение DRX_PLATFORM_CONSOLE.
#endif

#if DRX_PLATFORM_DESKTOP + DRX_PLATFORM_CONSOLE + DRX_PLATFORM_MOBILE != 1
	#error Неполноценный DRX_PLATFORM_(DESKTOP/CONSOLE/MOBILE)
#endif

#if defined(__clang__)
	#include <drx3D/CoreX/Compiler/Clangspecific.h>
#elif defined(__GNUC__)
	#include <drx3D/CoreX/Compiler/GCCspecific.h>
#elif defined(_MSC_VER)
	#include <drx3D/CoreX/Compiler/MSVCspecific.h>
#else
	#error Используется неподдерживаемый компилятор.
#endif
