#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include "Buffer.hpp"

const int MAX_MSG_LEN = (32 << 14);

namespace Util
{

struct Conn{
  int fd = 0;
  bool want_read = false;
  bool want_write = false;
  bool want_close = false;
  Buffer buff_in, buff_out;
};

void setNoBlockFd(int fd);
void handle_request(Util::Conn *conn);
void handle_write(Util::Conn *conn);
void read_full(Util::Conn *conn);

}

