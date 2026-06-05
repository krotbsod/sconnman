#ifndef SCONNMAN_ROUTE_HPP
#define SCONNMAN_ROUTE_HPP
#include <cstdint>
#include <netinet/in.h>
#include <string>
namespace sconnman {
class Route {
  private:
	/* struct rtmsg */
	unsigned char family{0};
	unsigned char dst_len{0};
	unsigned char src_len{0};
	unsigned char tos{0};

	unsigned char table{0};	   /* Routing table id */
	unsigned char protocol{0}; /* Routing protocol; see below	*/
	unsigned char scope{0};	   /* See below */
	unsigned char type{0};	   /* See below	*/

	unsigned flags{0};

	/* rtattr */
	/* RTA_DST */
	union {
		in_addr in;
		in6_addr in6;
	} dst{};

	/* RTA_SRC */
	union {
		in_addr in;
		in6_addr in6;
	} src{};

	/* RTA_IIF */
	int iif = 0;

	/* RTA_OIF */
	int oif = 0;

	/* RTA_GATEWAY */
	union {
		in_addr in;
		in6_addr in6;
	} gateway{};

	/* RTA_PRIORITY */
	int metric = 0;

  public:
	Route();
	virtual ~Route() = default;

	int set(uint8_t family, const std::string &dst, int dst_len, const std::string &gw = "0.0.0.0", int metric = 0);

	const std::string getDst() const;
	const int getDstLen() const;
	const std::string getGw() const;
	const int getMetric() const;
	const std::string getIfname() const; // TODO: move to RouteManager

	void printInfo() const;

	friend class RouteManager;
};
} // namespace sconnman

#endif // SCONNMAN_ROUTE_HPP
