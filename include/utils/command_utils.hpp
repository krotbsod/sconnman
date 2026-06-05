#ifndef SCONNMAN_COMMAND_UTILS_HPP
#define SCONNMAN_COMMAND_UTILS_HPP

#include <cstdlib>
#include <stdio.h>
#include <string>
#include <vector>

namespace sconnman {
int execute_command(const char *command);

template <typename... Args> int execute_command(const char *command, Args &&...args) {
	char buffer[1024] = {'\0'};
	int result = -1;
	if constexpr (sizeof...(Args) == 0) {
		result = snprintf(buffer, sizeof(buffer), "%s", command);
	} else {
		result = snprintf(buffer, sizeof(buffer), command, std::forward<Args>(args)...);
	}

	if (result > 0 && static_cast<size_t>(result) < sizeof(buffer)) {
		return execute_command(buffer);
	}
	return -1;
}

struct execute_return {
	std::vector<std::string> echo;
	int code = EXIT_FAILURE;
};

execute_return execute_command_ret(const char *command);

template <typename... Args> execute_return execute_command_ret(const char *command, Args &&...args) {
	char buffer[1024] = {'\0'};
	int result = -1;
	if constexpr (sizeof...(Args) == 0) {
		result = snprintf(buffer, sizeof(buffer), "%s", command);
	} else {
		result = snprintf(buffer, sizeof(buffer), command, std::forward<Args>(args)...);
	}

	if (result > 0 && static_cast<size_t>(result) < sizeof(buffer)) {
		return execute_command_ret(buffer);
	}
	return {
		.echo = {},
		.code = EXIT_FAILURE,
	};
}
} // namespace sconnman

#endif /* SCONNMAN_COMMAND_UTILS_HPP */
