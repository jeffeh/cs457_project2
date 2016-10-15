/* file: ss.cpp */
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <vector>
using namespace std;

// Signal handler
void sig_handler(int signal) {
  //Do whatever I want
  cout << endl;
  exit(1);
}

// Converting hostname into an ip format
int hostname_to_ip(char * hostname , char* ip)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;

  if ( (he = gethostbyname( hostname ) ) == NULL) {
    // get the host info
    herror("gethostbyname");
    return 1;
  }

  addr_list = (struct in_addr **) he->h_addr_list;

  for(i = 0; addr_list[i] != NULL; i++) {
    //Return the first one;
    strcpy(ip , inet_ntoa(*addr_list[i]) );
    return 0;
  }

  return 1;
}

struct stoneInfo {
  char ip[INET_ADDRSTRLEN];
  int port;
};

struct chainFileInfo {
  int numOfStones;
  vector<stoneInfo> listOfStones;
  string url;
};

int main(int argc, char* argv[]) {

  // The optional argument


  // Getting the ip address in url format
  struct addrinfo hints, *info, *p;
  int gai_result;

  char hostname[1024];
  char ip[INET_ADDRSTRLEN];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
    exit(1);
  }

  for(p = info; p != NULL; p = p->ai_next) {
    //printf("hostname: %s\n", p->ai_canonname);
  }

  freeaddrinfo(info);

  // Convert the url address into ip dot notation format _._._._
  hostname_to_ip(hostname, ip);

  cout << "Welcome to Chat!" << endl;

  // -------------------- Server Side --------------------

  struct sockaddr_in serverAddress;
  socklen_t size;

  int client, server;
  int portNumber = 61234;
  int bufferSize = 140;
  char ipAddressValue[INET_ADDRSTRLEN];

  // 1. Creating the server socket and get descriptor

  client = socket(AF_INET, SOCK_STREAM, 0);

  if (client < 0) {
    cerr << "Error: could not establish a connection" << endl;
    exit(-1);
  }

  // Server socket created
  // 2. Binding the socket to the port

  // This is the optional argument
  if (argc == 2) {
    portNumber = atoi(argv[1]);
    // Populating the struct addr_in serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY); // This should be the ip address of where the server is being hosted (s_addr is binary format)
    serverAddress.sin_port = htons(portNumber); // This is the port to the server ip address being hosted

    // populating ip binary and ipAddressValue with the ip address i think... tho it' sprobably already in char ip
    inet_pton(AF_INET, ip, &(serverAddress.sin_addr));
    inet_ntop(AF_INET, &(serverAddress.sin_addr), ipAddressValue, INET_ADDRSTRLEN);

    int bindingValue = bind(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if (bindingValue < 0) {
      cout << " -- Error binding the socket. It may have already been established" << endl;
      return -1;
    }
  } else {
    // This is default
    // Populating the struct addr_in serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY); // This should be the ip address of where the server is being hosted (s_addr is binary format)
    serverAddress.sin_port = htons(portNumber); // This is the port to the server ip address being hosted

    // populating ip binary and ipAddressValue with the ip address i think... tho it' sprobably already in char ip
    inet_pton(AF_INET, ip, &(serverAddress.sin_addr));
    inet_ntop(AF_INET, &(serverAddress.sin_addr), ipAddressValue, INET_ADDRSTRLEN);

    int bindingValue = bind(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if (bindingValue < 0) {
      cout << " -- Error binding the socket. It may have already been established" << endl;
      return -1;
    }
  }
  size = sizeof(serverAddress);

  //cout << "Server socket binding established" << endl;


  // 3. Listening on the port the socket was connected to

  cout << "Waiting for a connection on " << ip << " port " << portNumber << endl;

  int listenValue = listen(client, 1);

  int acceptValue = accept(client, (struct sockaddr*)&serverAddress, &size);

  //TODO
  // Proj 2 step 4. Create a loop statement and set the socket in listen mode
  while (true) {

  }

  return 0;
}
