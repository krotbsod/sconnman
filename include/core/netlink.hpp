#ifndef SCONNMAN_NETLINK_HPP
#define SCONNMAN_NETLINK_HPP

#include <cstdint>
#include <cstring>
#include <functional>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/types.h>
#include <vector>

#include "netctl.hpp"

namespace sconnman {
class NetlinkCtx : public NetCtx {
  public:
	NetlinkCtx(int buf_size = 8192);
	virtual ~NetlinkCtx();

	using data_vec = std::vector<uint8_t>;
	using ifa_rta_attrs = std::vector<std::pair<uint16_t, data_vec>>;
	using ReceiveHandler = std::function<int(void *data, ssize_t size)>;

	static data_vec to_data_vec(const void *data, size_t size) {
		data_vec vec = {};
		vec.resize(size);
		memcpy(vec.data(), data, size);
		return vec;
	}

	template <typename T> int request(T *msg, int rtm_type, uint16_t nlm_flags) {
		struct {
			struct nlmsghdr nlh;
			T msg;
		} req{};

		req.nlh.nlmsg_len = sizeof(req);
		req.nlh.nlmsg_type = rtm_type;
		req.nlh.nlmsg_flags = nlm_flags;
		req.nlh.nlmsg_seq = ++this->seq;
		req.nlh.nlmsg_pid = this->pid;
		memcpy(&req.msg, msg, sizeof(T));

		return send(this->sock, &req, sizeof(req), 0);
	}

	template <typename T> int request(T *msg, int rtm_type, uint16_t nlm_flags, const ifa_rta_attrs &attrs) {
		size_t total_len = NLMSG_SPACE(sizeof(T));
		for (const auto &[f, s] : attrs)
			total_len += RTA_SPACE(s.size());

		std::vector<uint8_t> buf(total_len);
		struct nlmsghdr *nlh = reinterpret_cast<struct nlmsghdr *>(buf.data());

		nlh->nlmsg_len = NLMSG_LENGTH(sizeof(T));
		nlh->nlmsg_type = rtm_type;
		nlh->nlmsg_flags = nlm_flags;
		nlh->nlmsg_seq = ++this->seq;
		nlh->nlmsg_pid = this->pid;
		memcpy(NLMSG_DATA(nlh), msg, sizeof(T));

		size_t offset = NLMSG_ALIGN(nlh->nlmsg_len);
		for (const auto &[f, s] : attrs) {
			struct rtattr *rta = reinterpret_cast<struct rtattr *>(buf.data() + offset);
			rta->rta_type = f;
			rta->rta_len = RTA_LENGTH(s.size());
			memcpy(RTA_DATA(rta), s.data(), s.size());
			offset += RTA_ALIGN(rta->rta_len);
		}
		nlh->nlmsg_len = offset;

		return send(this->sock, buf.data(), nlh->nlmsg_len, 0);
	}

	int receive();

	void onReceiveCallback(ReceiveHandler f);

  private:
	int sock = {-1};
	char *buf = nullptr;
	size_t buf_size = {0};
	uint32_t seq = {0};
	uint32_t pid = {0};

	ReceiveHandler _on_recv_callback = nullptr;
};
} // namespace sconnman

#endif // SCONNMAN_NETLINK_HPP
