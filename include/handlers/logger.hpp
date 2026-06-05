#ifndef SCONNMAN_LOGGER_HPP
#define SCONNMAN_LOGGER_HPP

#include <cstdio>
#include <utility>

namespace sconnman {

enum level : int {
	TRACE = 0x00,
	DEBUG = 0x01,
	INFO = 0x02,
	WARN = 0x03,
	ERR = 0x04,
	CRITICAL = 0x05,
	OFF = 0x06,
	N_LEVELS,
};

class Logger {
  private:
	using LoggerHandler = void (*)(int, const char *, ...);
	static inline LoggerHandler _handler = nullptr;

  public:
	static void setHandler(LoggerHandler &&handler) {
		_handler = std::move(handler);
	}

	template <typename... Args> static void fallback_log(int level, const char *format, Args &&...args);
	template <typename... Args> static void log(int level, const char *format, Args &&...args);
};

template <typename... Args> void Logger::fallback_log(int level, const char *format, Args &&...args) {
	char buffer[4096] = {'\0'};
	int result = -1;
	if constexpr (sizeof...(Args) == 0) {
		result = snprintf(buffer, sizeof(buffer), "%s", format);
	} else {
		result = snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
	}

	if (result > 0 && static_cast<size_t>(result) < sizeof(buffer)) {
		fprintf(stderr, "[FALLBACK] %s\n", buffer);
		return;
	}
	fprintf(stderr, "[FALLBACK] Log message formatting error\n");
}

template <typename... Args> void Logger::log(int level, const char *format, Args &&...args) {
	if (_handler) {
		_handler(level, format, std::forward<Args>(args)...);
		return;
	}
	fallback_log(level, format, std::forward<Args>(args)...);
}

} // namespace sconnman

#endif // SCONNMAN_LOGGER_HPP
