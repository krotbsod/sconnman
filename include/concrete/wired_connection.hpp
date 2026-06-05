#ifndef SCONNMAN_WIRED_CONNECTION_HPP
#define SCONNMAN_WIRED_CONNECTION_HPP

#include "components/connection.hpp"

namespace sconnman {
class WiredConnection : public Connection {
  private:
  public:
	WiredConnection() = default;
	~WiredConnection() = default;

	int connect() override {
		return -1;
	};
	int disconnect() override {
		return -1;
	};
};
} // namespace sconnman

#endif // SCONNMAN_WIRED_CONNECTION_HPP
