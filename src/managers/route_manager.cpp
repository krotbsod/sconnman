#include "managers/route_manager.hpp"

#include "core/netlink.hpp"
#include "handlers/logger.hpp"

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/in.h>
#include <vector>

using namespace sconnman;

int RouteManager::parse_route(struct nlmsghdr *nlh, Route &route) {
	struct rtmsg *rtm = (struct rtmsg *)NLMSG_DATA(nlh);

	route.family = rtm->rtm_family;
	route.dst_len = rtm->rtm_dst_len;
	route.src_len = rtm->rtm_src_len;
	route.tos = rtm->rtm_tos;

	route.table = rtm->rtm_table;
	route.protocol = rtm->rtm_protocol;
	route.scope = rtm->rtm_scope;
	route.type = rtm->rtm_type;

	route.flags = rtm->rtm_flags;

	// TODO: need filter or another solution
	// see: `ip route show table all` or `ip route`
	if (route.type != RTN_UNICAST) {
		return -1;
	}

	struct rtattr *rta;
	int rta_len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*rtm));
	for (rta = RTM_RTA(rtm); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
		switch (rta->rta_type) {
		case RTA_DST:
			if (rtm->rtm_family == AF_INET) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in_addr)) {
					memcpy(&route.dst, RTA_DATA(rta), sizeof(struct in_addr));
				}
			} else if (rtm->rtm_family == AF_INET6) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
					memcpy(&route.dst, RTA_DATA(rta), sizeof(struct in6_addr));
				}
			} else {
				Logger::log(ERR, "Unknown address family");
				return -1;
			}

			break;
		case RTA_GATEWAY:
			if (rtm->rtm_family == AF_INET) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in_addr)) {
					memcpy(&route.gateway, RTA_DATA(rta), sizeof(struct in_addr));
				}
			} else if (rtm->rtm_family == AF_INET6) {
				if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
					memcpy(&route.gateway, RTA_DATA(rta), sizeof(struct in6_addr));
				}
			} else {
				Logger::log(ERR, "Unknown address family");
				return -1;
			}
			break;
		case RTA_OIF:
			route.oif = *(int *)RTA_DATA(rta);
			break;
		case RTA_IIF:
			route.iif = *(int *)RTA_DATA(rta);
			break;
		case RTA_PRIORITY:
			route.metric = *(int *)RTA_DATA(rta);
			break;
		}
	}
	return 0;
}

int RouteManager::add_route(NetlinkCtx *ctx, const Interface &iface, const Route &route) {
	struct rtmsg rtm{};
	rtm.rtm_family = route.family;
	rtm.rtm_dst_len = route.dst_len;
	rtm.rtm_src_len = route.src_len;
	rtm.rtm_tos = route.tos;
	rtm.rtm_table = route.table;
	rtm.rtm_protocol = route.protocol;
	rtm.rtm_scope = route.scope;
	rtm.rtm_type = route.type;
	rtm.rtm_flags = route.flags;

	std::vector<uint8_t> dst_data;
	std::vector<uint8_t> gateway_data;
	if (route.family == AF_INET) {
		dst_data = NetlinkCtx::to_data_vec(&route.dst, sizeof(struct in_addr));
		gateway_data = NetlinkCtx::to_data_vec(&route.gateway, sizeof(struct in_addr));
	} else if (route.family == AF_INET6) {
		dst_data = NetlinkCtx::to_data_vec(&route.dst, sizeof(struct in6_addr));
		gateway_data = NetlinkCtx::to_data_vec(&route.gateway, sizeof(struct in6_addr));
	} else {
		Logger::log(ERR, "Unknown address family");
		return -1;
	}

	int oif = if_nametoindex(iface.name().c_str());

	NetlinkCtx::data_vec metric_data = NetlinkCtx::to_data_vec(&route.metric, sizeof(route.metric));
	NetlinkCtx::data_vec oif_data = NetlinkCtx::to_data_vec(&oif, sizeof(oif));

	NetlinkCtx::ifa_rta_attrs attrs = {
		{RTA_DST, dst_data},
		{RTA_GATEWAY, gateway_data},
		{RTA_PRIORITY, metric_data},
		{RTA_OIF, oif_data},
	};

	if (ctx->request(&rtm, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL, attrs) < 0) {
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
					Logger::log(ERR, "RTM_NEWROUTE error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
		}
		return 1;
	});

	return ctx->receive();
}

int RouteManager::del_route(NetlinkCtx *ctx, const Interface &iface, const Route &route) {
	struct rtmsg rtm{};
	rtm.rtm_family = route.family;
	rtm.rtm_dst_len = route.dst_len;
	rtm.rtm_src_len = route.src_len;
	rtm.rtm_tos = route.tos;
	rtm.rtm_table = route.table;
	rtm.rtm_protocol = route.protocol;
	rtm.rtm_scope = route.scope;
	rtm.rtm_type = route.type;
	rtm.rtm_flags = route.flags;

	std::vector<uint8_t> dst_data;
	std::vector<uint8_t> gateway_data;
	if (route.family == AF_INET) {
		dst_data = NetlinkCtx::to_data_vec(&route.dst, sizeof(struct in_addr));
		gateway_data = NetlinkCtx::to_data_vec(&route.gateway, sizeof(struct in_addr));
	} else if (route.family == AF_INET6) {
		dst_data = NetlinkCtx::to_data_vec(&route.dst, sizeof(struct in6_addr));
		gateway_data = NetlinkCtx::to_data_vec(&route.gateway, sizeof(struct in6_addr));
	} else {
		Logger::log(ERR, "Unknown address family");
		return -1;
	}

	int oif = if_nametoindex(iface.name().c_str());

	NetlinkCtx::data_vec metric_data = NetlinkCtx::to_data_vec(&route.metric, sizeof(route.metric));
	NetlinkCtx::data_vec oif_data = NetlinkCtx::to_data_vec(&oif, sizeof(oif));

	NetlinkCtx::ifa_rta_attrs attrs = {
		{RTA_DST, dst_data},
		{RTA_GATEWAY, gateway_data},
		{RTA_PRIORITY, metric_data},
		{RTA_OIF, oif_data},
	};

	if (ctx->request(&rtm, RTM_DELROUTE, NLM_F_REQUEST | NLM_F_ACK, attrs) < 0) {
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
					Logger::log(ERR, "RTM_DELROUTE error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
		}
		return 1;
	});

	return ctx->receive();
}

int RouteManager::get_routes(NetlinkCtx *ctx, std::vector<Route> &routes) {
	struct rtmsg rtm{};
	rtm.rtm_family = AF_UNSPEC;

	if (ctx->request(&rtm, RTM_GETROUTE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP) < 0) {
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
					Logger::log(ERR, "RTM_GETROUTE error: %s(%d)", strerror(-err->error), err->error);
					return -1;
				}
				return 0;
			}
			if (nlh->nlmsg_type == RTM_NEWROUTE) {
				Route route;
				if (parse_route(nlh, route) == 0) {
					routes.push_back(route);
				}
			}
		}
		return 1;
	});

	return ctx->receive();
}

int RouteManager::addRoute(const Interface &iface, const Route &route) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	return add_route(ctx, iface, route);
}

int RouteManager::delRoute(const Interface &iface, const Route &route) {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	return del_route(ctx, iface, route);
}

std::vector<Route> RouteManager::getRoutes() {
	NetlinkCtx *ctx = NetCtl::context<NetlinkCtx>();
	std::vector<Route> routes;
	get_routes(ctx, routes);
	return routes;
}
