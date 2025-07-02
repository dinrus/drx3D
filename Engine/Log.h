#pragma once

#include <cassert>
#include <sstream>
#include <mutex>
#include <filesystem>
#include <string_view>
#include <iostream>
#include <fstream>

#include <drx3D/Maths/Time.h>

namespace drx3d {
/**
 * @brief A logging class used in drx3D, will write output to the standard stream and into a file.
 */
class DRX3D_EXPORT Log {
public:
	class Styles {
	public:
		constexpr static STxtview Default = "\033[0m";
		constexpr static STxtview Bold = "\033[1m";
		constexpr static STxtview Dim = "\033[2m";
		constexpr static STxtview Underlined = "\033[4m";
		constexpr static STxtview Blink = "\033[5m";
		constexpr static STxtview Reverse = "\033[7m";
		constexpr static STxtview Hidden = "\033[8m";
	};

	class Colors {
	public:
		constexpr static STxtview Default = "\033[39m";
		constexpr static STxtview Black = "\033[30m";
		constexpr static STxtview Red = "\033[31m";
		constexpr static STxtview Green = "\033[32m";
		constexpr static STxtview Yellow = "\033[33m";
		constexpr static STxtview Blue = "\033[34m";
		constexpr static STxtview Magenta = "\033[35m";
		constexpr static STxtview Cyan = "\033[36m";
		constexpr static STxtview LightGrey = "\033[37m";
		constexpr static STxtview DarkGrey = "\033[90m";
		constexpr static STxtview LightRed = "\033[91m";
		constexpr static STxtview LightGreen = "\033[92m";
		constexpr static STxtview LightYellow = "\033[93m";
		constexpr static STxtview LightBlue = "\033[94m";
		constexpr static STxtview LightMagenta = "\033[95m";
		constexpr static STxtview LightCyan = "\033[96m";
		constexpr static STxtview White = "\033[97m";
	};

	constexpr static STxtview TimestampFormat = "%H:%M:%S";

	/**
	 * Outputs a message into the console.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Out(Args ... args) {
		Write(args...);
	}

	/**
	 * Outputs a message into the console.
	 * @tparam Args The value types to write.
	 * @param style The style to output as.
	 * @param color The color to output as.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Out(const STxtview &style, const STxtview &color, Args ... args) {
		Write(style, color, args..., Styles::Default);
	}

	/**
	 * Outputs a debug message into the console.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Debug(Args ... args) {
#ifdef DRX3D_DEBUG
		Out(Styles::Default, Colors::LightBlue, args...);
#endif
	}

	/**
	 * Outputs a info message into the console.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Info(Args ... args) {
		Out(Styles::Default, Colors::Green, args...);
	}

	/**
	 * Outputs a warning message into the console.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Warning(Args ... args) {
		Out(Styles::Default, Colors::Yellow, args...);
	}

	/**
	 * Outputs a error message into the console.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Error(Args ... args) {
		Out(Styles::Default, Colors::Red, args...);
	}

	/**
	 * Outputs a assert message into the console.
	 * @tparam Args The value types to write.
	 * @param expr The expression to assertion check.
	 * @param args The values to write.
	 */
	 /*
	template<typename ... Args>
	static void Assert(bool expr, Args ... args) {
		if (expr) {
			Out(Styles::Default, Colors::Magenta, args...);
			assert(false);
		}
	}
*/
	static void OpenLog(const std::filesystem::path &filepath);
	static void CloseLog();

private:
	// TODO: Only use mutex in synced writes (where output order must be the same).
	static std::mutex WriteMutex;
	static std::ofstream FileStream;

	/**
	 * A internal method used to write values to the out stream and to a file.
	 * @tparam Args The value types to write.
	 * @param args The values to write.
	 */
	template<typename ... Args>
	static void Write(Args ... args) {
		std::unique_lock<std::mutex> lock(WriteMutex);

		((std::cout << std::forward<Args>(args)), ...);
		if (FileStream.is_open()) {
			((FileStream << std::forward<Args>(args)), ...);
		}
	}
};

template<typename T = std::nullptr_t>
class Loggable {
public:
	explicit Loggable(STxt &&className) :
		className(std::move(className)) {
	}
	template<typename = std::enable_if_t<!std::is_same_v<T, std::nullptr_t>>>
	Loggable() :
		Loggable(typeid(T).name()) {
	}

	virtual ~Loggable() = default;

protected:
	template<typename ... Args>
	void WriteOut(Args ... args) const {
		auto logPrefix = GetLogPrefix();
		Log::Out(logPrefix.rdbuf(), args...);
	}

	template<typename ... Args>
	void WriteInfo(Args ... args) const {
		auto logPrefix = GetLogPrefix();
		Log::Info("INFO: ", logPrefix.rdbuf(), args...);
	}

	template<typename ... Args>
	void WriteWarning(Args ... args) const {
		auto logPrefix = GetLogPrefix();
		Log::Warning("WARN: ", logPrefix.rdbuf(), args...);
	}

	template<typename ... Args>
	void WriteError(Args ... args) const {
		auto logPrefix = GetLogPrefix();
		Log::Error("ERROR: ", logPrefix.rdbuf(), args...);
	}

private:
	std::stringstream GetLogPrefix() const {
		std::stringstream logPrefix;
		logPrefix << Time::GetDateTime(Log::TimestampFormat) << " [" << className << "]" <<
			"(0x" << std::hex << std::uppercase << reinterpret_cast<long>(this) << ") " << std::dec;
		return logPrefix;
	}

	STxt className;
};
}
