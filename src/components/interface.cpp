#include "components/interface.hpp"
#include "handlers/logger.hpp"

#include <linux/if_arp.h>

using namespace sconnman;

const std::string Interface::name() const {
	return ifname;
}

void Interface::printInfo() const {
	char mac_str[18]{};
	snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	char brd_str[18]{};
	snprintf(brd_str, sizeof(brd_str), "%02x:%02x:%02x:%02x:%02x:%02x", brd[0], brd[1], brd[2], brd[3], brd[4], brd[5]);

	Logger::log(INFO, "%s:", ifname);
	Logger::log(INFO, "\tmac: %s", mac_str);
	Logger::log(INFO, "\tbrd: %s", brd_str);
	Logger::log(INFO, "\tmtu: %d", mtu);
	Logger::log(INFO, "\tindex: %d", index);
	Logger::log(INFO, "\tlink: %d", link);

	Logger::log(INFO,
		"\tflags(0x%04x): %s%s%s%s%s%s",
		flags,
		(flags & IFF_UP) ? "UP " : "",
		(flags & IFF_BROADCAST) ? "BROADCAST " : "",
		(flags & IFF_LOOPBACK) ? "LOOPBACK " : "",
		(flags & IFF_RUNNING) ? "RUNNING " : "",
		(flags & IFF_NOARP) ? "NOARP " : "",
		(flags & IFF_MULTICAST) ? "MULTICAST " : "");

	Logger::log(INFO,
		"\ttype(%d): %s",
		type,
		type == ARPHRD_ETHER	  ? "(Ethernet)"
		: type == ARPHRD_LOOPBACK ? "(Loopback)"
		: type == ARPHRD_PPP	  ? "(PPP)"
		: type == 512			  ? "(WWAN)"
								  : "");
}

void Interface::printStats() const {
	Logger::log(INFO, "Stats:");
	Logger::log(INFO,
		"\trx_bytes"
		"\trx_packets %d"
		"\trx_errors %d"
		"\trx_dropped %d"
		"\ttx_bytes %d"
		"\ttx_packets %d"
		"\ttx_errors %d"
		"\ttx_dropped %d",
		stats.rx_bytes,
		stats.rx_packets,
		stats.rx_errors,
		stats.rx_dropped,
		stats.tx_bytes,
		stats.tx_packets,
		stats.tx_errors,
		stats.tx_dropped);
}
