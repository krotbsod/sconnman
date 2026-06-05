#ifndef SCONNMAN_INTERFACE_MANAGER_HPP
#define SCONNMAN_INTERFACE_MANAGER_HPP

#include <vector>

#include <linux/netlink.h>

#include "components/interface.hpp"
#include "core/netlink.hpp"

namespace sconnman {
class InterfaceManager {
  private:
  	int parse_interface(const struct nlmsghdr *nlh, Interface &iface);

	int get_interfaces(NetlinkCtx *ctx, std::vector<Interface> &ifaces);
	int get_interface_stats(NetlinkCtx *ctx, Interface &iface);

	int set_link_state(NetlinkCtx *ctx, const Interface &iface, bool up);

  public:
	InterfaceManager();
	~InterfaceManager();

	std::vector<Interface> getNetworkInterfaces();

	int setLinkState(Interface &iface, bool up);
};
} // namespace sconnman

#endif // SCONNMAN_INTERFACE_MANAGER_HPP
