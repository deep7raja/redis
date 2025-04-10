#include <string>
#include <iostream>
#include "util.hpp"

void Util::setNoBlockFd(int fd)
{
  int rv = fcntl(fd, F_GETFL, 0);
  assert(rv > -1);
  rv = fcntl(fd, F_SETFL, rv | O_NONBLOCK);
  assert(rv != -1);
}

std::string process_msg(std::string msg)
{
  std::cout << "processing msg=[" << msg << "]\n";
  for(char &c: msg){
    if(c >= 'a' && c <= 'z') c += 'A'-'a';
  }
  return msg;
}

void Util::read_full(Util::Conn *conn)
{
  while(true)
  {
    uint8_t rbuf[4 + MAX_MSG_LEN];
    errno = 0;
    int rv = read(conn->fd, rbuf, 4 + MAX_MSG_LEN);
    if(rv < 0 && EAGAIN == errno)
    {
      return;
    }
    else if(rv <= 0)
    {
        perror("read returned negative closing the connection");
        std::cout << "[CRITICAL] rv=" << rv << ", errno=" << errno << "\n";
        conn->want_close = true;
        return;
    }
    //conn->buff_in.insert(conn->buff_in.end(), rbuf, rbuf+rv);
    conn->buff_in.append_back(rbuf, rv);
  }
}

void Util::handle_request(Util::Conn *conn)
{
  read_full(conn);
  while(true)
  {
    if(conn->buff_in.size() < 5) return;  // need atleast 4+1 data to process the request
                                          //
    uint32_t msg_len = *(reinterpret_cast<const uint32_t*>(conn->buff_in.data()));
    std::cout << "received msg with length=" << msg_len << "\n";

    assert(msg_len <= MAX_MSG_LEN);
    if(conn->buff_in.size() < 4 + msg_len) return;

    std::string rsp = process_msg(std::string((const char*)conn->buff_in.data() + 4, msg_len));
    conn->buff_in.erase_front(4 + msg_len);

    uint32_t rsp_len = rsp.length();
    uint8_t* len_arr = reinterpret_cast<uint8_t*>(&rsp_len);
    assert(rsp_len <= MAX_MSG_LEN);

    conn->buff_out.append_back(len_arr, 4);
    conn->buff_out.append_back(reinterpret_cast<const uint8_t*>(rsp.data()), rsp_len);
    conn->want_write = true;
    handle_write(conn);
  }
}

void Util::handle_write(Util::Conn *conn)
{
  if(conn->buff_out.empty()) return;
  std::cout << "handle_write called\n";
  int rv = write(conn->fd, conn->buff_out.data(), conn->buff_out.size());
  if(rv < 0 && EAGAIN == errno)
  {
    return;
  }
  else if(rv <= 0)
  {
    perror("write returned negative not yet closing the connection");
    std::cout << "[CRITICAL] rv=" << rv << ", errno=" << errno << "\n";
    return;
  }
  conn->buff_out.erase_front(rv);
  if(conn->buff_out.empty()) conn->want_write = false;

  handle_write(conn);
}
