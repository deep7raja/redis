//#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
//#include <errno.h>
#include <stdio.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include "util.hpp"

int main()
{
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd > 0 && "socket call failed");

  int sock_opt_val = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt_val, sizeof(sock_opt_val));

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(0);
  addr.sin_port = htons(1234);

  int rv = bind(listen_fd, (const struct sockaddr*)(&addr), sizeof(addr));
  if(rv != 0)
  {
    perror("bind failed");
    return -1;
  }

  rv = listen(listen_fd, 10);
  if(rv != 0)
  {
    perror("listen failed");
    return -1;
  }

  Util::setNoBlockFd(listen_fd);

  std::vector<Util::Conn*> fd2conn;
  std::vector<struct pollfd> poll_args {{listen_fd, POLLIN, 0}};

  while(true)
  {
    std::cout << "Polling for new data......." << std::flush;
    rv = poll(poll_args.data(), poll_args.size(), -1);                   //thread blocking call
    assert(rv > 0);
    std::cout << "success\n";

    sleep(1);
    
    if(poll_args[0].revents & POLLIN)
    {
      rv = accept(listen_fd, nullptr, nullptr);   //rv is socket fd of accepted conntection
      assert(rv != -1);
      if(fd2conn.size() < rv+1) fd2conn.resize(rv+1, nullptr);
      fd2conn[rv] = new Util::Conn();
      fd2conn[rv]->fd = rv;
      fd2conn[rv]->want_read = true;
      std::cout << "new connection accepted with fd=" << rv << "\n";
      Util::setNoBlockFd(rv);
    }
    //poll_args[0].revents = 0;

    for(int i=1; i<poll_args.size(); ++i){
      bool match_found = false;
      if(poll_args[i].revents & POLLIN){
        match_found = true;
        handle_request(fd2conn[poll_args[i].fd]);
      }
      if(poll_args[i].revents & POLLOUT){
        match_found = true;
        handle_write(fd2conn[poll_args[i].fd]);
      }
      if(poll_args[i].revents & POLLERR){
        match_found = true;
        std::cout << "PollError detected for fd =" << poll_args[i].fd << "\n";
        fd2conn[poll_args[i].fd]->want_close = true;
        return -1;
      }
      if(!match_found)
      {
        std::cout << "invalid poll result match not found\n";
      }
      //poll_args[i].revents = 0;
    }
    poll_args.clear();
    poll_args.push_back({listen_fd, POLLIN, 0});

    for(Util::Conn *conn: fd2conn)
    {
      if(nullptr == conn) continue;
      struct pollfd curfd = {conn->fd, 0, 0};
      if(conn->want_write)
      {
        curfd.events |= POLLOUT;
      }
      if(conn->want_read)
      {
        curfd.events |= POLLIN;
      }
      if(conn->want_close)
      {
        std::cout << "closing the connection on fd=" << conn->fd << "\n";
        close(conn->fd);
      }
      else 
      {
        poll_args.push_back(curfd);
      }
    }
  }
}
















