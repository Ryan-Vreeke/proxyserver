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
#include <regex>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
// ssl for https connections
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

cSock clients[50];
std::mutex myMutex;
SSL *ssl;
int sock;

int RecvPacket(char* buf)
{
  int len = 100;
  do
  {
    len = SSL_read(ssl, buf, 100);
    buf[len] = 0;
    printf("%s\n", buf);
    //        fprintf(fp, "%s",buf);
  } while (len > 0);
  if (len < 0)
  {
    int err = SSL_get_error(ssl, len);
    if (err == SSL_ERROR_WANT_READ)
      return 0;
    if (err == SSL_ERROR_WANT_WRITE)
      return 0;
    if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL)
      return -1;
  }
}

int SendPacket(const char *buf)
{
  int len = SSL_write(ssl, buf, strlen(buf));
  if (len < 0)
  {
    int err = SSL_get_error(ssl, len);
    switch (err)
    {
    case SSL_ERROR_WANT_WRITE:
      return 0;
    case SSL_ERROR_WANT_READ:
      return 0;
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SYSCALL:
    case SSL_ERROR_SSL:
    default:
      return -1;
    }
  }
}

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
      close(innerClient);
      break;
    }

    if (bytesRecv == 0)
    {
      cout << "The Client disconnected" << endl;
      close(innerClient);
      break;
    }

    // display message
    string req = string(clients[innerClient].buf, 0, bytesRecv);
    vector<string> tokens;
    split_regex(tokens, req, boost::regex("(\r\n)+"));

    for (auto t : tokens)
    {
      cout << t << endl;
    }

    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
      printf("Error creating socket. \n");
      break;
    }
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("172.217.15.228");
    sa.sin_port = htons(443);
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen))
    {
      printf("Error connecting to server.\n");
      break;
    }

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ctx);
    if (!ssl)
    {
      printf("Error creating SSL.\n");
      break;
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0)
    {
      printf("Error creating SSL connection.  err=%x\n", err);
      fflush(stdout);
      break;
    }
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));

    char *request = "GET https://www.google.com/ HTTP/1.1\r\n\r\n";       
    SendPacket(request);
    RecvPacket(clients[innerClient].buf);
    //send(send to this socket, from this buf, of this length, flag)
    send(innerClient, clients[innerClient].buf, 4096, 0);
  }

  std::lock_guard<std::mutex> guard(myMutex);
  cSock tempC;
  clients[innerClient] = tempC;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cerr << "Arguments missing! Port Required! proxyserver [port]" << endl;
    // return -1;
  }
  // int port = atoi(argv[1]);

  int listening = socket(AF_INET, SOCK_STREAM, 0);
  if (listening == -1)
  {
    cout << "Can't Create A Socket!!";
    return -1;
  }

  // Bind the socket to a IP / port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1)
  {
    cout << "Can't bind to IP/port";
    return -2;
  }
  // mark the socket for listening in
  if (listen(listening, SOMAXCONN) == -1)
  {
    cout << "Can't Listen!";
    return -3;
  }
  cout << "listenging" << endl;
  // cout << "Listening on port " << port << endl;

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
    std::lock_guard<std::mutex> guard(myMutex);
    clients[newClient.clientSocket] = newClient;
    new thread(recvLoop, newClient.clientSocket);
  }

  return 0;
}
