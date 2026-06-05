#include "concrete/udhcpc_dhcp_client.hpp"

#include "utils/command_utils.hpp"
#include "handlers/logger.hpp"
#include <regex>

using namespace sconnman;

int UdhcpcDHCPClient::acquire(const Interface &iface) {
	execute_return result = execute_command_ret("udhcpc -i %s -T %i -A %i -n -q -f", iface.name().c_str(), udhcp_pause_between_packets, udhcp_wait_of_lease);
	if (result.code == 0) {
		Logger::log(DEBUG, "acquired dhcp successful");
	} else {
		Logger::log(ERR, "acquired dhcp failed");
	}

	_dns_servers.clear();
	std::regex pattern(R"(adding dns (\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
	for (const auto &line : result.echo) {
		std::smatch match;
		if (std::regex_search(line, match, pattern)) {
			Logger::log(DEBUG, "DNS nameserver found: '%s'", match[1].str().c_str());
			_dns_servers.push_back(match[1]);
		}
	}

	return result.code;
}

const std::list<std::string> &UdhcpcDHCPClient::getNameservers() {
	return _dns_servers;
}
