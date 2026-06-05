#include "components/route.hpp"

#include "handlers/logger.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <openssl/ssl.h>

using namespace sconnman;

Route::Route() {
	memset(&this->dst, 0, sizeof(struct in6_addr));
	memset(&this->src, 0, sizeof(struct in6_addr));
	memset(&this->gateway, 0, sizeof(struct in6_addr));
}

int Route::set(uint8_t family, const std::string &dst, int dst_len, const std::string &gw, int metric) {
	int res = -1;
	if (dst == "default") {
		res = inet_pton(family, "0.0.0.0", &this->dst);
		dst_len = 0;
	} else {
		res = inet_pton(family, dst.c_str(), &this->dst);
	}
	if (res != 1) {
		Logger::log(ERR, "inet_pton: %s(%d)", strerror(errno), errno);
		return -1;
	}

	res = inet_pton(family, gw.c_str(), &this->gateway);
	if (res != 1) {
		Logger::log(ERR, "inet_pton: %s(%d)", strerror(errno), errno);
		return -1;
	}

	this->family = family;
	this->dst_len = dst_len;
	this->src_len = 0;
	this->tos = 0;
	this->table = RT_TABLE_MAIN;
	this->protocol = RTPROT_BOOT;
	this->scope = RT_SCOPE_UNIVERSE;
	this->type = RTN_UNICAST;

	this->metric = metric;

	return res == 1 ? 0 : -1;
}

const std::string Route::getDst() const {
	char dst_str[INET6_ADDRSTRLEN] = "default";
	if (dst_len > 0) {
		inet_ntop(family, &dst, dst_str, sizeof(dst_str));
	}
	return dst_str;
}

const int Route::getDstLen() const {
	return dst_len;
}

const std::string Route::getGw() const {
	char gateway_str[INET6_ADDRSTRLEN] = {};
	inet_ntop(family, &gateway, gateway_str, sizeof(gateway_str));
	return gateway_str;
}

const int Route::getMetric() const {
	return metric;
}

const std::string Route::getIfname() const {
	char oif_str[IFNAMSIZ] = "";
	if_indextoname(oif, oif_str);
	return oif_str;
}

void Route::printInfo() const {
	char src_str[INET6_ADDRSTRLEN] = {};
	char mask_str[INET6_ADDRSTRLEN] = {};

	if (src_len > 0) {
		inet_ntop(family, &src, src_str, sizeof(src_str));
	}

	char table_str[32] = {};
	snprintf(table_str,
		sizeof(table_str),
		"table(%d): %s",
		table,
		table == RT_TABLE_COMPAT	? "COMPAT"
		: table == RT_TABLE_DEFAULT ? "DEFAULT"
		: table == RT_TABLE_MAIN	? "MAIN"
		: table == RT_TABLE_LOCAL	? "LOCAL"
									: "");

	char protocol_str[32] = {};
	snprintf(protocol_str,
		sizeof(protocol_str),
		"proto(%d): %s",
		protocol,
		protocol == RTPROT_REDIRECT ? "REDIRECT"
		: protocol == RTPROT_KERNEL ? "KERNEL"
		: protocol == RTPROT_BOOT	? "BOOT"
		: protocol == RTPROT_STATIC ? "STATIC"
		: protocol == RTPROT_DHCP	? "DHCP"
									: "");

	char scope_str[32] = {};
	snprintf(scope_str,
		sizeof(scope_str),
		"scope(%d): %s",
		scope,
		scope == RT_SCOPE_UNIVERSE	? "UNIVERSE"
		: scope == RT_SCOPE_SITE	? "SITE"
		: scope == RT_SCOPE_LINK	? "LINK"
		: scope == RT_SCOPE_HOST	? "HOST"
		: scope == RT_SCOPE_NOWHERE ? "NOWHERE"
									: "");

	char type_str[32] = {};
	snprintf(type_str,
		sizeof(type_str),
		"type(%d): %s",
		type,
		type == RTN_UNICAST		  ? "UNICAST"
		: type == RTN_LOCAL		  ? "LOCAL"
		: type == RTN_BROADCAST	  ? "BROADCAST"
		: type == RTN_ANYCAST	  ? "ANYCAST"
		: type == RTN_MULTICAST	  ? "MULTICAST"
		: type == RTN_BLACKHOLE	  ? "BLACKHOLE"
		: type == RTN_UNREACHABLE ? "UNREACHABLE"
		: type == RTN_PROHIBIT	  ? "PROHIBIT"
		: type == RTN_THROW		  ? "THROW"
		: type == RTN_NAT		  ? "NAT"
		: type == RTN_XRESOLVE	  ? "XRESOLVE"
								  : "");

	char flags_str[32] = {};
	snprintf(flags_str,
		sizeof(flags_str),
		"flags(0x%04x): %s%s%s%s%s%s%s%s",
		flags,
		(flags & RTM_F_NOTIFY) ? "NOTIFY " : "",
		(flags & RTM_F_CLONED) ? "CLONED " : "",
		(flags & RTM_F_EQUALIZE) ? "EQUALIZE " : "",
		(flags & RTM_F_PREFIX) ? "PREFIX " : "",
		(flags & RTM_F_LOOKUP_TABLE) ? "LOOKUP_TABLE " : "",
		(flags & RTM_F_FIB_MATCH) ? "FIB_MATCH " : "",
		(flags & RTM_F_OFFLOAD) ? "OFFLOAD " : "",
		(flags & RTM_F_TRAP) ? "TRAP " : "");

	Logger::log(DEBUG,
		"%-24s /%-4d %-24s %-16s %-8d %-20s %-20s %-20s %-20s %s",
		getDst().c_str(),
		dst_len,
		getGw().c_str(),
		getIfname().c_str(),
		metric,
		protocol_str,
		scope_str,
		type_str,
		table_str,
		flags_str);
}
