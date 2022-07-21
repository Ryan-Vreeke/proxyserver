#include "cSock.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>

using namespace std;

cSock clients[50];
std::mutex myMutex;

void recvLoop(int innerClient)
{
  while (true)
  {
    // clear buffer
    clients[innerClient].clearBuffer();
    // wait for message
    int bytesRecv = recv(innerClient, clients[innerClient].buf, 4096, 0);
    if (bytesRecv == -1)
    {
      cout << "there was a connection issue!";
      break;
    }

    if (bytesRecv == 0)
    {
      cout << "The Client disconnected" << endl;
    }

    // display message
    cout << innerClient << ">" << string(clients[innerClient].buf, 0, bytesRecv) << endl;
    // send(send to this socket, from this buf, of this length, flag)
    std::lock_guard<std::mutex> guard(myMutex);
    for (auto c : clients)
    {
      if (c.clientSocket != -1)
      {
        send(c.clientSocket, clients[innerClient].buf, bytesRecv + 1, 0);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int listening = socket(AF_INET, SOCK_STREAM, 0);
  if (listening == -1)
  {
    cout << "Can't Create A Socket!!";
    return -1;
  }

  // Bind the socket to a IP / port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(5000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1)
  {
    cout << "Can't bind to IP/port";
    return -2;
  }
  // mark the socket for listening in
  if (listen(listening, SOMAXCONN) == -1)
  {
    cout << "Can't LIsten!";
    return -3;
  }

  cout << "Listening..." << endl;

  while (true)
  {
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket(accept(listening, (sockaddr *)&client, &clientSize));
    cSock newClient(clientSocket);

    if (newClient.clientSocket == -1)
    {
      cout << "Problem with client connecting!";
      return -4;
    }
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr *)&client, sizeof(client), host,
                             NI_MAXHOST, svc, NI_MAXSERV, 0);

    if (result)
    {
      cout << "nothinhg";
    }
    else
    {
      inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
      newClient.setIP(host);
      // newClient.setPort(ntohs(client.sin_port));
    }

    cout << newClient.ip << " Connected" << endl;
    clients[newClient.clientSocket] = newClient;
    new thread(recvLoop, newClient.clientSocket);
  }

  return 0;
}
