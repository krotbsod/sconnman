#include "managers/interface_manager.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "components/interface.hpp"
#include "core/netctl.hpp"
#include "core/netlink.hpp"
#include "handlers/logger.hpp"

using namespace sconnman;

InterfaceManager::InterfaceManager() {
}

InterfaceManager::~InterfaceManager() {
}

int InterfaceManager::get_interface_stats(NetlinkCtx *ctx, Interface &iface) {
	Interface::Stats &stats = iface.stats;
	int ifindex = iface.index;

	memset(&iface.stats, 0, sizeof(iface.stats));

	struct if_stats_msg ifsm{};
	ifsm.family = AF_UNSPEC;
	ifsm.ifindex = ifindex;
	ifsm.filter_mask = IFLA_STATS_FILTER_BIT(IFLA_STATS_LINK_64);

	if (ctx->request(&ifsm, RTM_GETSTATS, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP) < 0) {
		Logger::log(ERR, "netlink_stats_request: %s(%i)", strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([&](void *buf, size_t size) {
		for (nlmsghdr *nlh = (struct nlmsghdr *)buf; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {

			if (nlh->nlmsg_type == NLMSG_DONE)
				return 0;

			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "RTM_GETSTATS error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}

			if (nlh->nlmsg_type == RTM_NEWSTATS) {
				struct if_stats_msg *ifsm = (struct if_stats_msg *)NLMSG_DATA(nlh);

				if (ifsm->ifindex != ifindex) {
					continue;
				}

				struct rtattr *rta;
				int rta_len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifsm));
				for (rta = IFLA_RTA(ifsm); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
					switch (rta->rta_type) {
					case IFLA_STATS_LINK_64: {
						struct rtnl_link_stats64 *st = (struct rtnl_link_stats64 *)RTA_DATA(rta);
						stats.rx_bytes = st->rx_bytes;
						stats.rx_packets = st->rx_packets;
						stats.rx_errors = st->rx_errors;
						stats.rx_dropped = st->rx_dropped;
						stats.tx_bytes = st->tx_bytes;
						stats.tx_packets = st->tx_packets;
						stats.tx_errors = st->tx_errors;
						stats.tx_dropped = st->tx_dropped;
						break;
					}
					}
				}
			}
		}
		return 1;
	});

	return ctx->receive();
}

int InterfaceManager::parse_interface(const struct nlmsghdr *nlh, Interface &iface) {
	struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(nlh);

	/* fill ifinfomsg */
	iface.family = ifi->ifi_family;
	iface.type = ifi->ifi_type;
	iface.index = ifi->ifi_index;
	iface.flags = ifi->ifi_flags;
	iface.change = ifi->ifi_change;

	struct rtattr *rta;
	int rta_len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
	for (rta = IFLA_RTA(ifi); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
		switch (rta->rta_type) {
		case IFLA_ADDRESS:
			if (RTA_PAYLOAD(rta) >= 6) {
				memcpy(iface.mac, (unsigned char *)RTA_DATA(rta), sizeof(iface.mac));
			}
			break;
		case IFLA_BROADCAST:
			if (RTA_PAYLOAD(rta) >= 6) {
				memcpy(iface.brd, (unsigned char *)RTA_DATA(rta), sizeof(iface.brd));
			}
			break;
		case IFLA_IFNAME:
			strncpy(iface.ifname, (const char *)RTA_DATA(rta), IFNAMSIZ - 1);
			break;
		case IFLA_MTU:
			iface.mtu = *(unsigned *)RTA_DATA(rta);
			break;
		case IFLA_LINK:
			iface.link = *(int *)RTA_DATA(rta);
			break;
		}
	}
	return 0;
}

int InterfaceManager::get_interfaces(NetlinkCtx *ctx, std::vector<Interface> &ifaces) {
	struct ifinfomsg ifi{};
	ifi.ifi_family = AF_UNSPEC;

	if (ctx->request(&ifi, RTM_GETLINK, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP) < 0) {
		Logger::log(ERR, "netlink_dump_request: %s(%i)", strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([&](void *data, size_t size) {
		for (nlmsghdr *nlh = (struct nlmsghdr *)data; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {

			if (nlh->nlmsg_type == NLMSG_DONE)
				return 0;

			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "RTM_GETLINK error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}

			if (nlh->nlmsg_type == RTM_NEWLINK) {
				Interface iface;
				if (parse_interface(nlh, iface) == 0) {
					ifaces.push_back(iface);
				}
			}
		}
		return 1;
	});

	return ctx->receive();
}

int InterfaceManager::set_link_state(NetlinkCtx *ctx, const Interface &iface, bool up) {
	struct ifinfomsg ifi{};

	ifi.ifi_family = AF_UNSPEC;
	ifi.ifi_change = IFF_UP;
	ifi.ifi_flags = up ? IFF_UP : 0;

	NetlinkCtx::ifa_rta_attrs attrs = {
		{IFLA_IFNAME, NetlinkCtx::to_data_vec(iface.name().c_str(), iface.name().size() + 1)},
	};

	if (ctx->request(&ifi, RTM_SETLINK, NLM_F_REQUEST | NLM_F_ACK, attrs) < 0) {
		Logger::log(ERR, "netlink_setlink_request failed for %s: %s(%i)", iface.name().c_str(), strerror(errno), errno);
		return -1;
	}

	ctx->onReceiveCallback([&](void *data, size_t size) {
		for (nlmsghdr *nlh = (struct nlmsghdr *)data; NLMSG_OK(nlh, size); nlh = NLMSG_NEXT(nlh, size)) {

			if (nlh->nlmsg_type == NLMSG_DONE) {
				return 0;
			}

			if (nlh->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
				if (err->error) {
					Logger::log(ERR, "RTM_SETLINK error for %s: %s(%d)", iface.name().c_str(), strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
		}
		return 1;
	});

	return ctx->receive();
}

std::vector<Interface> InterfaceManager::getNetworkInterfaces() {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	std::vector<Interface> ifaces;
	get_interfaces(ctx, ifaces);
	for (auto &iface : ifaces) {
		get_interface_stats(ctx, iface);
	}
	return ifaces;
}

int InterfaceManager::setLinkState(Interface &iface, bool up) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	return set_link_state(ctx, iface, up);
}
