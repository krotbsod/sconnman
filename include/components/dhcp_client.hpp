#ifndef SCONNMAN_DHCP_CLIENT_HPP
#define SCONNMAN_DHCP_CLIENT_HPP

#include "components/interface.hpp"
namespace sconnman {
class DHCPClient {
  private:
  public:
	DHCPClient() = default;
	virtual ~DHCPClient() = default;
	virtual int acquire(const Interface &iface) = 0;
};
} // namespace sconnman

#endif // SCONNMAN_DHCP_CLIENT_HPP
