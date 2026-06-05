#ifndef SCONNMAN_ADDRESS_HPP
#define SCONNMAN_ADDRESS_HPP

#include "netinet/in.h"
#include <cstdint>
#include <linux/rtnetlink.h>
#include <string>

namespace sconnman {
class Address {
  private:
	/* ifaddrmsg */
	uint8_t family{0};
	uint8_t prefixlen{0}; /* The prefix length		*/
	// uint8_t flags{0};	  /* Flags			*/
	uint8_t scope{0};	  /* Address scope		*/
	uint32_t index{0};	  /* Link index			*/

	/* rtattr */
	/* IFA_FLAGS */
	union {
		uint8_t u8;
		uint32_t u32;
	} flags{};
	
	/* IFA_LOCAL */
	union {
		in_addr in;
		in6_addr in6;
	} local{};

	/* IFA_ADDRESS */
	union {
		in_addr in;
		in6_addr in6;
	} address{};

  public:
	Address();
	virtual ~Address() = default;

	int set(uint8_t family, const std::string &addr_str, uint8_t prefixlen, uint8_t scope = RT_SCOPE_UNIVERSE, uint32_t flags = IFA_F_PERMANENT);

	void printInfo() const;

	friend class AddressManager;
};
} // namespace sconnman

#endif // SCONNMAN_ADDRESS_HPP
