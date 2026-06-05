#include "utils/command_utils.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "handlers/logger.hpp"

using namespace sconnman;

int sconnman::execute_command(const char *command) {
	Logger::log(INFO, "command: %s", command);

	std::string cmd(command);
	cmd.append(" 2>&1");

	FILE *fp = popen(cmd.c_str(), "r");
	if (!fp) {
		Logger::log(CRITICAL, "popen: ", strerror(errno));
		return EXIT_FAILURE;
	}

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Logger::log(DEBUG, buffer);
		// Logger::flush();
	}

	int status = pclose(fp);
	if (status == -1) {
		Logger::log(CRITICAL, "pclose: ", strerror(errno));
		return EXIT_FAILURE;
	}
	// Logger::flush();

	return WEXITSTATUS(status);
}

execute_return sconnman::execute_command_ret(const char *command) {
	Logger::log(INFO, "command: %s", command);

	std::string cmd(command);
	cmd.append(" 2>&1");

	execute_return ret;
	FILE *fp = popen(cmd.c_str(), "r");
	if (!fp) {
		Logger::log(CRITICAL, "popen: ", strerror(errno));
		ret.code = EXIT_FAILURE;
		return ret;
	}

	char buffer[1024] = {0};
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		Logger::log(DEBUG, buffer);
		// Logger::flush();
		ret.echo.push_back(buffer);
	}

	int status = pclose(fp);
	if (status == -1) {
		Logger::log(CRITICAL, "pclose: ", strerror(errno));
		ret.code = EXIT_FAILURE;
		return ret;
	}
	// Logger::flush();

	ret.code = WEXITSTATUS(status);
	return ret;
}
