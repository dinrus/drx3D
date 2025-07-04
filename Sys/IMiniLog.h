// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct IMiniLog
{
	enum ELogType
	{
		eMessage,
		eWarning,
		eError,
		eAlways,
		eWarningAlways,
		eErrorAlways,
		eInput,           //!< e.g. "e_CaptureFolder ?" or all printouts from history or auto completion.
		eInputResponse,   //!< e.g. "Set output folder for video capturing" in response to "e_CaptureFolder ?".
		eComment,
		eAssert
	};

	// <interfuscator:shuffle>
	virtual void LogV(const ELogType nType, tukk szFormat, va_list args) PRINTF_PARAMS(3, 0) = 0;
	virtual void LogV(const ELogType nType, i32 flags, tukk szFormat, va_list args) PRINTF_PARAMS(4, 0) = 0;

	//! Logs using type.
	inline void LogWithType(ELogType nType, tukk szFormat, ...) PRINTF_PARAMS(3, 4);
	inline void LogWithType(ELogType nType, i32 flags, tukk szFormat, ...) PRINTF_PARAMS(4, 5);

	virtual ~IMiniLog() {}

	//! This is the simplest log function for messages with the default implementation.
	virtual inline void Log(tukk szFormat, ...) PRINTF_PARAMS(2, 3);

	//! This is the simplest log function for warnings with the default implementation.
	virtual inline void LogWarning(tukk szFormat, ...) PRINTF_PARAMS(2, 3);

	//! This is the simplest log function for errors with the default implementation.
	virtual inline void LogError(tukk szFormat, ...) PRINTF_PARAMS(2, 3);
	// </interfuscator:shuffle>
};

inline void IMiniLog::LogWithType(ELogType nType, i32 flags, tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogV(nType, flags, szFormat, args);
	va_end(args);
}

inline void IMiniLog::LogWithType(ELogType nType, tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogV(nType, szFormat, args);
	va_end(args);
}

inline void IMiniLog::Log(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogV(eMessage, szFormat, args);
	va_end(args);
}

inline void IMiniLog::LogWarning(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogV(eWarning, szFormat, args);
	va_end(args);
}

inline void IMiniLog::LogError(tukk szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	LogV(eError, szFormat, args);
	va_end(args);
}

//! By default, to make it possible not to implement the log at the beginning at all, empty implementations of the two functions are given.
struct CNullMiniLog : public IMiniLog
{
	//! The default implementation just won't do anything.
	//! ##@{.
	void LogV(tukk szFormat, va_list args)                                  {}
	void LogV(const ELogType nType, tukk szFormat, va_list args)            {}
	void LogV(const ELogType nType, i32 flags, tukk szFormat, va_list args) {}
	//! ##@}.
};

//! \endcond