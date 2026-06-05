#include <algorithm>
#include <cstdarg>
#include <cstring>

#include "components/address.hpp"
#include "components/dhcp_client.hpp"
#include "components/interface.hpp"
#include "components/route.hpp"
#include "concrete/udhcpc_dhcp_client.hpp"
#include "core/netctl.hpp"
#include "core/netlink.hpp"
#include "handlers/logger.hpp"
#include "managers/address_manager.hpp"
#include "managers/interface_manager.hpp"
#include "managers/route_manager.hpp"

void loggerHandler(int level, const char *format, ...) {
	static char buffer[1024] = {};
	static char msg[16] = {};
	
	switch (level) {
	case sconnman::level::TRACE:
		strncpy(msg, "TRACE", sizeof(msg));
		break;
	case sconnman::level::DEBUG:
		strncpy(msg, "DEBUG", sizeof(msg));
		break;
	case sconnman::level::INFO:
		strncpy(msg, "INFO", sizeof(msg));
		break;
	case sconnman::level::WARN:
		strncpy(msg, "WARN", sizeof(msg));
		break;
	case sconnman::level::ERR:
		strncpy(msg, "ERR", sizeof(msg));
		break;
	case sconnman::level::CRITICAL:
		strncpy(msg, "CRITICAL", sizeof(msg));
		break;
	default:
		return;
	}

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fprintf(stdout, "[%s]: %s\n", msg, buffer);
}

int initLogger() {
	sconnman::Logger::setHandler(loggerHandler);
	return 0;
}

void interface_addresses(const sconnman::Interface &iface, sconnman::AddressManager &aman) {
	auto addrs = aman.getAddresses(iface);
	sconnman::Logger::log(sconnman::INFO, "Interface: %s", iface.name().c_str());
	for (const auto &addr : addrs) {
		addr.printInfo();
		sconnman::Logger::log(sconnman::INFO, "---");
	}
	sconnman::Logger::log(sconnman::INFO, "====\n");
};

int interface_manager_test(sconnman::InterfaceManager &iman) {
	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "Interfaces list");

	auto ifaces = iman.getNetworkInterfaces();
	if (ifaces.empty()) {
		sconnman::Logger::log(sconnman::ERR, "%s", "Interfaces list empty");
		return -1;
	}
	for (const auto &iface : ifaces) {
		iface.printInfo();
		iface.printStats();
		sconnman::Logger::log(sconnman::INFO, "====\n");
	}

	return 0;
}

int address_manager_test(sconnman::InterfaceManager &iman, sconnman::AddressManager &aman) {
	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "Interfaces addresses");

	auto ifaces = iman.getNetworkInterfaces();
	for (const auto &iface : ifaces) {
		interface_addresses(iface, aman);
	}

	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "Add addresses");
	auto it = std::find_if(ifaces.begin(), ifaces.end(), [](const sconnman::Interface &iface) {
		return iface.name() == "enp4s0" || iface.name() == "eth0";
	});
	if (it != ifaces.end()) {
		sconnman::Address addr_0;
		addr_0.set(AF_INET, "192.168.144.80", 24);
		aman.addAddress(*it, addr_0);

		sconnman::Address addr_1;
		addr_1.set(AF_INET, "192.168.144.90", 30);
		aman.addAddress(*it, addr_1);

		interface_addresses(*it, aman);
	}

	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "Del addresses");
	it = std::find_if(ifaces.begin(), ifaces.end(), [](const sconnman::Interface &iface) {
		return iface.name() == "enp4s0" || iface.name() == "eth0";
	});
	if (it != ifaces.end()) {
		sconnman::Address addr_0;
		addr_0.set(AF_INET, "192.168.144.80", 24);
		aman.delAddress(*it, addr_0);

		sconnman::Address addr_1;
		addr_1.set(AF_INET, "192.168.144.90", 30);
		aman.delAddress(*it, addr_1);

		interface_addresses(*it, aman);
	}

	return 0;
}

int route_manager_test(sconnman::InterfaceManager &iman, sconnman::RouteManager &rman) {
	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "Interfaces routes");

	auto routes = rman.getRoutes();
	sconnman::Logger::log(sconnman::INFO, "%-24s %-4s %-24s %-16s %-8s", "Destination", "Mask", "Gateway", "Iface", "Metric");
	sconnman::Logger::log(sconnman::INFO, "------------------------------------------------------------");
	for (const auto &route : routes) {
		route.printInfo();
	}

	auto ifaces = iman.getNetworkInterfaces();
	auto it = std::find_if(ifaces.begin(), ifaces.end(), [](const sconnman::Interface &iface) {
		return iface.name() == "enp4s0" || iface.name() == "eth0";
	});

	if (it != ifaces.end()) {
		sconnman::Route route_0;
		route_0.set(AF_INET, "192.168.144.0", 24, "0.0.0.0", 600);
		rman.addRoute(*it, route_0);

		sconnman::Route route_1;
		route_1.set(AF_INET, "default", 0, "192.168.144.1", 500);
		rman.addRoute(*it, route_1);
	}

	routes = rman.getRoutes();
	sconnman::Logger::log(sconnman::INFO, "%-24s %-4s %-24s %-16s %-8s", "Destination", "Mask", "Gateway", "Iface", "Metric");
	sconnman::Logger::log(sconnman::INFO, "------------------------------------------------------------");
	for (const auto &route : routes) {
		route.printInfo();
	}

	if (it != ifaces.end()) {
		sconnman::Route route_0;
		route_0.set(AF_INET, "192.168.144.0", 24, "0.0.0.0", 600);
		rman.delRoute(*it, route_0);

		sconnman::Route route_1;
		route_1.set(AF_INET, "default", 0, "192.168.144.1", 500);
		rman.delRoute(*it, route_1);
	}

	routes = rman.getRoutes();
	sconnman::Logger::log(sconnman::INFO, "%-24s %-4s %-24s %-16s %-8s", "Destination", "Mask", "Gateway", "Iface", "Metric");
	sconnman::Logger::log(sconnman::INFO, "------------------------------------------------------------");
	for (const auto &route : routes) {
		route.printInfo();
	}
	return 0;
}

int dhcp_client_test(sconnman::DHCPClient &dhcp_client, sconnman::InterfaceManager &iman, sconnman::AddressManager &aman) {
	sconnman::Logger::log(sconnman::DEBUG, "Testing: %s", "DHCP client");

	auto ifaces = iman.getNetworkInterfaces();
	auto it = std::find_if(ifaces.begin(), ifaces.end(), [](const sconnman::Interface &iface) {
		return iface.name() == "eth0";
	});
	if (it != ifaces.end()) {
		dhcp_client.acquire(*it);

		interface_addresses(*it, aman);
	}
	return 0;
}

auto main(int argc, char **argv) -> int {
	initLogger();
	sconnman::NetCtl::init<sconnman::NetlinkCtx>();

	sconnman::InterfaceManager iman;
	interface_manager_test(iman);

	sconnman::AddressManager aman;
	address_manager_test(iman, aman);

	sconnman::RouteManager rman;
	route_manager_test(iman, rman);

	sconnman::UdhcpcDHCPClient dhcp_client;
	dhcp_client_test(dhcp_client, iman, aman);

	return 0;
};
