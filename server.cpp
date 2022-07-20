// TCP server

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(int argc, char const *argv[]) {
  cout << "server Start!!" << endl;
  // create socket
  int listening = socket(AF_INET, SOCK_STREAM, 0);
  if (listening == -1) {
    cout << "Can't Create A Socket!!";
    return -1;
  }

  // Bind the socket to a IP / port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1) {
    cout << "Can't bind to IP/port";
    return -2;
  }

  // mark the socket for listening in
  if (listen(listening, SOMAXCONN) == -1) {
    cout << "Can't LIsten!";
    return -3;
  }

  // Accept the Call
  sockaddr_in client;
  socklen_t clientSize = sizeof(client);
  char host[NI_MAXHOST];
  char svc[NI_MAXSERV]; // client port

  int clientSocket = accept(listening, (sockaddr *)&client, &clientSize);
  cout << "here";
  if (clientSocket == -1) {
    cout << "Problem with client connecting!";
    return -4;
  }

  // close the listening socket
  close(listening);

  memset(host, 0, NI_MAXHOST);
  memset(svc, 0, NI_MAXSERV);

  int result = getnameinfo((sockaddr *)&client, sizeof(client), host,
                           NI_MAXHOST, svc, NI_MAXSERV, 0);

  if (result) {
    cout << host << " connected on " << svc << endl;
  } else {
    inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
    cout << host << " connected on " << ntohs(client.sin_port) << endl;
  }

  // while receiving
  char buf[4096];
  while (true) {
    // Clear the buffer
    memset(buf, 0, 4096);
    // Wait for a message
    int bytesRecv = recv(clientSocket, buf, 4096, 0);
    if (bytesRecv == -1) {
      cout << "there was a connection issue!";
      break;
    }

    if (bytesRecv == 0) {
      cout << "The Client disconnected" << endl;
    }

    // Display message
    cout << "Recieved: " << string(buf, 0, bytesRecv) << endl;
    // resend message
    send(clientSocket, buf, bytesRecv + 1, 0);
  }

  close(clientSocket);

  return 0;
}
