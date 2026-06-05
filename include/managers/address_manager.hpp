#ifndef SCONNMAN_ADDRESS_MANAGER_HPP
#define SCONNMAN_ADDRESS_MANAGER_HPP

#include <cstring>
#include <vector>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "components/address.hpp"
#include "components/interface.hpp"
#include "core/netlink.hpp"

namespace sconnman {
class AddressManager {
  private:
	int parse_address(const struct nlmsghdr *nlh, Address &addr);

	int add_address(NetlinkCtx *ctx, const Interface &iface, const Address &addr);
	int del_address(NetlinkCtx *ctx, const Interface &iface, const Address &addr);

	int get_addresses(NetlinkCtx *ctx, const Interface &iface, std::vector<Address> &addr);

  public:
	AddressManager();
	~AddressManager();

	int addAddress(const Interface &iface, const Address &addr);
	int delAddress(const Interface &iface, const Address &addr);

	std::vector<Address> getAddresses(const Interface &iface);
};
} // namespace sconnman

#endif // SCONNMAN_ADDRESS_MANAGER_HPP
