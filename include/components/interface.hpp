#ifndef SCONNMAN_INTERFACE_HPP
#define SCONNMAN_INTERFACE_HPP

#include <cstdint>
#include <net/if.h>
#include <string>

namespace sconnman {
class Interface {
  public:
	struct Stats {
		uint64_t rx_bytes;
		uint64_t rx_packets;
		uint64_t rx_errors;
		uint64_t rx_dropped;
		uint64_t tx_bytes;
		uint64_t tx_packets;
		uint64_t tx_errors;
		uint64_t tx_dropped;
	};

  private:
	/* ifinfomsg */
	unsigned char family{0};
	unsigned char __ifi_pad{0};
	unsigned short type{0}; /* ARPHRD_* */
	int index{0};			/* Link index	*/
	unsigned flags{0};		/* IFF_* flags	*/
	unsigned change{0};		/* IFF_* change mask */

	/* rtattr */
	/* IFLA_ADDRESS */
	uint8_t mac[6]{};
	/* IFLA_BROADCAST */
	uint8_t brd[6]{};
	/* IFLA_IFNAME */
	char ifname[IFNAMSIZ]{};
	/* IFLA_MTU */
	unsigned mtu{0};
	/* IFLA_LINK */
	int link{0};

	/* IFLA_STATS_LINK_64 */
	Stats stats{};

	/* user */
	bool blocked = true;

  public:
	Interface() = default;
	virtual ~Interface() = default;

	const std::string name() const;

	void printInfo() const;
	void printStats() const;

	friend class InterfaceManager;
	friend class AddressManager;
};
} // namespace sconnman

#endif // SCONNMAN_INTERFACE_HPP
