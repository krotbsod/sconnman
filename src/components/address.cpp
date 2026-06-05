#include "components/address.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>

#include "handlers/logger.hpp"

using namespace sconnman;

Address::Address() {
	memset(&this->flags, 0, sizeof(uint32_t));
	memset(&this->local, 0, sizeof(struct in6_addr));
	memset(&this->address, 0, sizeof(struct in6_addr));
}

int Address::set(uint8_t family, const std::string &addr_str, uint8_t prefixlen, uint8_t scope, uint32_t flags) {
	int res = inet_pton(family, addr_str.c_str(), &local);
	if (res != 1) {
		Logger::log(ERR, "inet_pton: %s(%d)", strerror(errno), errno);
		return -1;
	}

	res = inet_pton(family, addr_str.c_str(), &address); // TODO: think about peer addresses
	if (res != 1) {
		Logger::log(ERR, "inet_pton: %s(%d)", strerror(errno), errno);
		return -1;
	}

	this->family = family;
	this->prefixlen = prefixlen;
	this->scope = scope;
	this->flags.u32 = flags;
	// memcpy(&this->flags, &flags, sizeof(uint32_t));

	return res == 1 ? 0 : -1;
}

void Address::printInfo() const {

	char local_str[INET6_ADDRSTRLEN] = {};
	char address_str[INET6_ADDRSTRLEN] = {};
	char mask_str[INET6_ADDRSTRLEN] = {};

	inet_ntop(family, &local, local_str, sizeof(local_str));
	inet_ntop(family, &address, address_str, sizeof(address_str));
	snprintf(mask_str, sizeof(mask_str), "/%d", prefixlen);

	if (local_str[0]) {
		Logger::log(INFO, "\t%s%s", local_str, mask_str);
	}

	if (address_str[0]) {
		Logger::log(INFO, "\t%s", address_str);
	}

	Logger::log(INFO,
		"\tscope(%d): %s",
		scope,
		scope == RT_SCOPE_UNIVERSE	? "UNIVERSE"
		: scope == RT_SCOPE_SITE	? "SITE"
		: scope == RT_SCOPE_LINK	? "LINK"
		: scope == RT_SCOPE_HOST	? "HOST"
		: scope == RT_SCOPE_NOWHERE ? "NOWHERE"
									: "");

	Logger::log(INFO,
		"\tflags(0x%04x): %s%s%s%s%s%s%s%s%s%s%s%s",
		flags.u32,
		(flags.u32 & IFA_F_SECONDARY) ? "SECONDARY " : "",
		(flags.u32 & IFA_F_NODAD) ? "NODAD " : "",
		(flags.u32 & IFA_F_OPTIMISTIC) ? "OPTIMISTIC " : "",
		(flags.u32 & IFA_F_DADFAILED) ? "DADFAILED " : "",
		(flags.u32 & IFA_F_HOMEADDRESS) ? "HOMEADDRESS " : "",
		(flags.u32 & IFA_F_DEPRECATED) ? "DEPRECATED " : "",
		(flags.u32 & IFA_F_TENTATIVE) ? "TENTATIVE " : "",
		(flags.u32 & IFA_F_PERMANENT) ? "PERMANENT " : "",
		(flags.u32 & IFA_F_MANAGETEMPADDR) ? "MANAGETEMPADDR " : "",
		(flags.u32 & IFA_F_NOPREFIXROUTE) ? "NOPREFIXROUTE " : "",
		(flags.u32 & IFA_F_MCAUTOJOIN) ? "MCAUTOJOIN " : "",
		(flags.u32 & IFA_F_STABLE_PRIVACY) ? "STABLE_PRIVACY " : "");

	Logger::log(INFO, "\tindex: %d", index);
}
