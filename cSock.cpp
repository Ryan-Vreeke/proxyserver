#include "cSock.h"
#include <cstdint>
#include <cstdio>

cSock::cSock(int clientSocket) { this->clientSocket = clientSocket; }
void cSock::setIP(char ip[]) { this->ip = ip; }
void cSock::clearBuffer() { memset(buf, 0, 4090); }
