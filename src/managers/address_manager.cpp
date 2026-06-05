#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ifaddrs.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "components/address.hpp"
#include "components/interface.hpp"
#include "core/netctl.hpp"
#include "core/netlink.hpp"
#include "handlers/logger.hpp"
#include "managers/address_manager.hpp"

using namespace sconnman;

AddressManager::AddressManager() {
}

AddressManager::~AddressManager() {
}

int AddressManager::parse_address(const struct nlmsghdr *nlh, Address &addr) {
	struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);

	addr.family = ifa->ifa_family;
	addr.prefixlen = ifa->ifa_prefixlen;
	addr.flags.u8 = ifa->ifa_flags;
	addr.scope = ifa->ifa_scope;
	addr.index = ifa->ifa_index;

	struct rtattr *rta;
	int rta_len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa));
	for (rta = IFA_RTA(ifa); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
		// Logger::log(DEBUG, "rta type: %d", rta->rta_type);
		if (rta->rta_type == IFA_LOCAL) {
			if (ifa->ifa_family == AF_INET) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in_addr)) {
					memcpy(&addr.local, RTA_DATA(rta), sizeof(struct in_addr));
				}
			} else if (ifa->ifa_family == AF_INET6) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
					memcpy(&addr.local, RTA_DATA(rta), sizeof(struct in6_addr));
				}
			} else {
				Logger::log(ERR, "Unknown address family");
				return -1;
			}
		}
		if (rta->rta_type == IFA_ADDRESS) {
			if (ifa->ifa_family == AF_INET) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in_addr)) {
					memcpy(&addr.address, RTA_DATA(rta), sizeof(struct in_addr));
				}
			} else if (ifa->ifa_family == AF_INET6) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
					memcpy(&addr.address, RTA_DATA(rta), sizeof(struct in6_addr));
				}
			} else {
				Logger::log(ERR, "Unknown address family");
				return -1;
			}
		}
		if (rta->rta_type == IFA_FLAGS) {
			if (RTA_PAYLOAD(rta) >= sizeof(uint32_t)) {
				addr.flags.u32 |= *(uint32_t *)RTA_DATA(rta);
			}
		}
	}
	return 0;
}

int AddressManager::add_address(NetlinkCtx *ctx, const Interface &iface, const Address &addr) {
	struct ifaddrmsg ifa{};
	ifa.ifa_family = addr.family;
	ifa.ifa_prefixlen = addr.prefixlen;
	ifa.ifa_flags = addr.flags.u8;
	ifa.ifa_scope = addr.scope;
	ifa.ifa_index = iface.index;

	std::vector<uint8_t> local_data;
	std::vector<uint8_t> address_data;
	if (addr.family == AF_INET) {
		local_data = NetlinkCtx::to_data_vec(&addr.local, sizeof(struct in_addr));
		address_data = NetlinkCtx::to_data_vec(&addr.address, sizeof(struct in_addr));
	} else if (addr.family == AF_INET6) {
		local_data = NetlinkCtx::to_data_vec(&addr.local, sizeof(struct in6_addr));
		address_data = NetlinkCtx::to_data_vec(&addr.address, sizeof(struct in6_addr));
	} else {
		Logger::log(ERR, "Unknown address family");
		return -1;
	}

	std::vector<uint8_t> flags_data = NetlinkCtx::to_data_vec(&addr.flags, sizeof(addr.flags));

	NetlinkCtx::ifa_rta_attrs attrs = {
		{IFA_LOCAL, local_data},
		{IFA_ADDRESS, address_data},
		{IFA_FLAGS, flags_data},
	};

	if (ctx->request(&ifa, RTM_NEWADDR, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, attrs) < 0) {
		Logger::log(ERR, "netlink_request: %s(%d)", strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([](void *data, size_t size) {
		for (struct nlmsghdr *nlh = (struct nlmsghdr *)data; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {
			if (nlh->nlmsg_type == NLMSG_DONE) {
				return 0;
			}
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "Netlink error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
		}
		return 1;
	});

	return ctx->receive();
}

int AddressManager::del_address(NetlinkCtx *ctx, const Interface &iface, const Address &addr) {
	struct ifaddrmsg ifa{};
	ifa.ifa_family = addr.family;
	ifa.ifa_prefixlen = addr.prefixlen;
	ifa.ifa_flags = addr.flags.u8;
	ifa.ifa_scope = addr.scope;
	ifa.ifa_index = iface.index;

	std::vector<uint8_t> local_data;
	std::vector<uint8_t> address_data;
	if (addr.family == AF_INET) {
		local_data = NetlinkCtx::to_data_vec(&addr.local, sizeof(struct in_addr));
		address_data = NetlinkCtx::to_data_vec(&addr.address, sizeof(struct in_addr));
	} else if (addr.family == AF_INET6) {
		local_data = NetlinkCtx::to_data_vec(&addr.local, sizeof(struct in6_addr));
		address_data = NetlinkCtx::to_data_vec(&addr.address, sizeof(struct in6_addr));
	} else {
		Logger::log(ERR, "Unknown address family");
		return -1;
	}

	NetlinkCtx::ifa_rta_attrs attrs = {
		{IFA_LOCAL, local_data},
		{IFA_ADDRESS, address_data},
	};

	if (ctx->request(&ifa, RTM_DELADDR, NLM_F_REQUEST | NLM_F_ACK, attrs) < 0) {
		Logger::log(ERR, "netlink_request: %s(%d)", strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([](void *data, size_t size) {
		for (struct nlmsghdr *nlh = (struct nlmsghdr *)data; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {
			if (nlh->nlmsg_type == NLMSG_DONE) {
				return 0;
			}
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "Netlink error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
		}
		return 1;
	});

	return ctx->receive();
}

int AddressManager::get_addresses(NetlinkCtx *ctx, const Interface &iface, std::vector<Address> &addrs) {

	struct ifaddrmsg ifa{};
	ifa.ifa_family = AF_UNSPEC;
	ifa.ifa_index = 0; // iface.index

	if (ctx->request(&ifa, RTM_GETADDR, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP) < 0) {
		Logger::log(ERR, "netlink_request: %s(%i)", strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([&](void *data, size_t size) {
		for (struct nlmsghdr *nlh = (struct nlmsghdr *)data; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {

			if (nlh->nlmsg_type == NLMSG_DONE)
				return 0;
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "RTM_GETADDR error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
			if (nlh->nlmsg_type == RTM_NEWADDR) {
				struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);

				if (ifa->ifa_index != iface.index) {
					continue;
				}

				Address addr;
				if (parse_address(nlh, addr) == 0) {
					addrs.push_back(addr);
				}
			}
		}
		return 1;
	});

	return ctx->receive();
}

int AddressManager::addAddress(const Interface &iface, const Address &addr) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	return add_address(ctx, iface, addr);
}

int AddressManager::delAddress(const Interface &iface, const Address &addr) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	return del_address(ctx, iface, addr);
}

std::vector<Address> AddressManager::getAddresses(const Interface &iface) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	std::vector<Address> addrs;
	get_addresses(ctx, iface, addrs);
	return addrs;
}
