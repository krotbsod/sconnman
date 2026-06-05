#ifndef SCONNMAN_UDHCPC_DHCP_CLIENT_HPP
#define SCONNMAN_UDHCPC_DHCP_CLIENT_HPP

#include <list>

#include "components/dhcp_client.hpp"

namespace sconnman {
class UdhcpcDHCPClient : public DHCPClient {
  private:
	const int udhcp_pause_between_packets = 1;
	const int udhcp_wait_of_lease = 3;

	std::list<std::string> _dns_servers;

  public:
	UdhcpcDHCPClient() = default;
	~UdhcpcDHCPClient() = default;

	int acquire(const Interface &iface) override final;
	const std::list<std::string> &getNameservers();
};
} // namespace sconnman

#endif
