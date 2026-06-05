#ifndef SCONNMAN_NETCTL_HPP
#define SCONNMAN_NETCTL_HPP

// TODO: core for implement netlink or ioctl, sysfs, procfs
// now only netlink

#include <memory>
#include <type_traits>
#include <utility>

namespace sconnman {
class NetCtx {
  public:
	NetCtx() = default;
	virtual ~NetCtx() = default;

	int id = {0};
};

class NetCtl {
  public:
	template <typename T, typename... Args> static typename std::enable_if<std::is_base_of_v<NetCtx, T>, void>::type init(Args &&...args) {
		_ctx = std::make_unique<T>(std::forward<Args>(args)...);
	};

	template <typename T> static typename std::enable_if<std::is_base_of_v<NetCtx, T>, T *>::type context() {
		return dynamic_cast<T *>(_ctx.get());
	}

  private:
	NetCtl() = delete;
	virtual ~NetCtl() = delete;

	inline static std::unique_ptr<NetCtx> _ctx;
};
} // namespace sconnman

#endif // SCONNMAN_NETCTL_HPP
