#ifndef SCONNMAN_CONNECTION_HPP
#define SCONNMAN_CONNECTION_HPP

namespace sconnman {
class Connection {
  private:
  public:
	Connection() = default;
	virtual ~Connection() = default;

	virtual int connect() = 0;
	virtual int disconnect() = 0;
};

} // namespace sconnman

#endif // SCONNMAN_CONNECTION_HPP
