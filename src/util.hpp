#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
//#include <errno.h>
//#include <stdio.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>
//#include <iostream>
#include <unistd.h>

const int MAX_MSG_LEN = 4*1024;

namespace Util
{

struct Conn{
  int fd = 0;
  bool want_read = false;
  bool want_write = false;
  bool want_close = false;
  std::vector<uint8_t> buff_in, buff_out;
};

void setNoBlockFd(int fd);
void handle_request(Util::Conn *conn);
void handle_write(Util::Conn *conn);
void read_full(Util::Conn *conn);

}

