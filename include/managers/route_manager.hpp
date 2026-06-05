#ifndef SCONNMAN_ROUTE_MANAGER_HPP
#define SCONNMAN_ROUTE_MANAGER_HPP

#include <vector>

#include "components/interface.hpp"
#include "components/route.hpp"
#include "core/netlink.hpp"

namespace sconnman {
class RouteManager {
  private:
	int parse_route(struct nlmsghdr *nlh, Route &route);

	int add_route(NetlinkCtx *ctx, const Interface &iface, const Route &route);
	int del_route(NetlinkCtx *ctx, const Interface &iface, const Route &route);

	int get_routes(NetlinkCtx *ctx, std::vector<Route> &routes);

  public:
	RouteManager() = default;
	~RouteManager() = default;

	int addRoute(const Interface &iface, const Route &route);
	int delRoute(const Interface &iface, const Route &route);

	std::vector<Route> getRoutes();
};
} // namespace sconnman

#endif // SCONNMAN_ROUTE_MANAGER_HPP
