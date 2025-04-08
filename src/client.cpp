#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

const int MAX_MSG_SIZE = 4*1024;

int read_full(int fd, char* buff, int len)
{
  while(len > 0)
  {
    errno = 0;
    ssize_t rv = read(fd, buff, len);
    if(rv < 0)
    {
      if(EINTR == errno) continue;
      std::cerr << "read failed ,strerr[" << strerror(errno) << "], errono=[" << errno << "], rv=[" << rv << "]\n";
      return -1;
    }
    else if(0 == rv)
    {
      std::cerr << "Unexpected EOF encountered\n";
      return -1;
    }
    len -= rv;
    buff += rv;
  }
  return 0;
}

int write_all(int fd, char* buff, int len)
{
  while(len > 0)
  {
    errno = 0;
    ssize_t rv = write(fd, buff, len);
    if(EINTR == errno) continue;
    else if(rv <= 0)
    {
      std::cerr << "write failed ,strerr[" << strerror(errno) << "], errono=[" << errno << "], rv=[" << rv << "]\n";
      return -1;
    }
    len -= rv;
    buff += rv;
  }
  return 0;
}

int send_msg(int fd, const char* msg, int msg_len)
{
  if(msg_len > MAX_MSG_SIZE)
  {
    std::cerr << "msg_len > MAX_MSG_SIZE\n";
    return -1;
  }
  char buff[4 + MAX_MSG_SIZE];
  *(uint32_t*)buff = (uint32_t)msg_len;         //assuming little endian
  memcpy(buff+4, msg, msg_len);
  int rv = write_all(fd, buff, 4+msg_len);
  if(rv)
  {
    std::cerr << "write_all failed in send_msg\n";
    return -1;
  }
  return 0;
}

int recv_msg(int fd, char* msg, uint32_t& msg_len)
{
  int rv = read_full(fd, (char*)(&msg_len), 4);
  if(rv)
  {
    std::cerr << "read_full failed in recv_msg for length\n";
    return -1;
  }
  rv = read_full(fd, msg, msg_len);
  if(rv)
  {
    std::cerr << "read_full failed in recv_msg for data\n";
  }
  return 0;
}

int main()
{
  std::cout << "\n-----------------starting client--------------------\n";
  int fd = socket(AF_INET, SOCK_STREAM, 0); 
  std::cout << "socket is created with fd=" << fd << "\n";

  int sock_opt_val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(sock_opt_val));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int rval = connect(fd, (const struct sockaddr*)(&addr), sizeof(addr));
  if(rval)
  {
    perror("call to connect failed");
  }
  else
  {
    std::cout << "connect success\n";
  }

  if(true)  //jft
  {
    std::cout << "press enter to send msg\n";
    std::cin.get();
    std::string str = "hello there";
    int rv = send_msg(fd, str.data(), str.size());
    str = "general kenobi";
    rv = send_msg(fd, str.data(), str.size());
    {
      char reply[MAX_MSG_SIZE+1] = {0};
      uint32_t reply_len = 0;
      int rv =  recv_msg(fd, reply, reply_len);
      std::cout << "reply len=" << reply_len << "\n";
      reply[reply_len] = 0;
      std::cout << "server responded with [" << reply << "]\n";
    }
    {
      char reply[MAX_MSG_SIZE+1] = {0};
      uint32_t reply_len = 0;
      int rv =  recv_msg(fd, reply, reply_len);
      std::cout << "reply len=" << reply_len << "\n";
      reply[reply_len] = 0;
      std::cout << "server responded with [" << reply << "]\n";
    }
  }

  while(true)
  {
    std::cout << "Enter the message to send to the server\n";
    std::string str;
    std::getline(std::cin, str);
    //std::cin >> str;
    int rv = send_msg(fd, str.data(), str.size());
    if(rval < 0)
    {
      std::cerr << "send_msg failed in while loop\n";
    }
    else
    {
      std::cout << "msg send success\n";
    }
    char reply[MAX_MSG_SIZE+1] = {0};
    uint32_t reply_len = 0;
    rv =  recv_msg(fd, reply, reply_len);
    std::cout << "reply len=" << reply_len << "\n";
    reply[reply_len] = 0;
    if(rval < 0)
    {
      std::cerr << "send_msg failed in while loop\n";
    }
    else
    {
      std::cout << "server responded with [" << reply << "]\n";
    }
  }
}
