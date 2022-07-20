#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
  cout << "hello world!" << endl;
  // create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return 1;
  }

  // create a hint stucture for the server we're connecting with
  int port = 54000;
  string ipAddress = "127.0.0.1";

  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

  // connect to the server on the socket
  int connectRes = connect(sock, (sockaddr *)&hint, sizeof(hint));
  if (connectRes == -1) {
    return 1;
  }

  char buf[4096];
  string userIn;

  // while loop
  do {
    // enter lines of text
    cout << "> ";
    getline(cin, userIn);

    // send to server
    int sendRes = send(sock, userIn.c_str(), userIn.size() + 1, 0);
    // TODO: check if that failes!!
    if (sendRes == -1) {
      cout << "Could not send";
      continue;
    }
    // wait for response
    memset(buf, 0, 4096);
    int bytesRecv = recv(sock, buf, 4096, 0);
    // display response
    cout << "SERVER> " << string(buf, bytesRecv) << "\r\n";
  } while (true);
  // close socket
  close(sock);
  return 0;
}
