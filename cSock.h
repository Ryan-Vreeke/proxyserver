#ifndef CSOCK_H
#define CSOCK_H

#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

class cSock {
public:
  int clientSocket;
  std::string ip;
  char buf[4096];

  cSock(int clientSocket);
  void setIP(char ip[]);
  void clearBuffer();
};

#endif
