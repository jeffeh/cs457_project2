//
// Created by Ethan Loza on 10/19/16.
//
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <signal.h>
#include <regex>
#include <pthread.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Function Definitions//
bool checkArgs(int argc, char* argv[]);
int server(int port);
int client(int port, char* hn);
void getmyip();
void error(const string msg);
void sig_handler(int signal);
void Usage(char* argv);
void *PrintHello (void *dummyPt);

// Global Variables
char* host;
int port;
int sockett;
int sockett2;
static int newSocketFileDesc;

// Structures
typedef struct packet_t{
	short version, messageLength;
	char message[140];
}packet;

typedef struct p_SSInfo {
  char* url;
  int remainingSS;
  vector<pair<string, int>> SSList;
} SSInfo;

// Main Function
int main(int argc, char* argv[]){

  /*
  SSInfo stuff;
  stuff.remainingSS = 10;
  cout << "remaining ss is : " << stuff.remainingSS << endl;
  vector<pair<string, int>> wsup = stuff.SSList;
  if (wsup.empty()) {
    cout << "is vector empty " << endl;
  }
  */


	if(signal(SIGINT,sig_handler)==SIG_ERR){
		printf("can't signal");
	}
	if(checkArgs(argc, argv)==0){
		Usage(argv[0]);
		exit(-1);
	}
	switch(argc){
		case 1:
			server(6676);break;
		case 2:
			Usage(argv[0]); break;
		default:
			client(port, host);
	}

	//cout << checkArgs(argc, argv) << endl;
}
bool is_number(const std::string& s);
bool is_number_or_dot(const std::string& s);
void error(const string msg)
{
	perror((msg.c_str()));
	exit(1);
}

//Error Checking Functions
bool checkArgs(int argc, char* argv[]){
	if(argc == 1){
		return true;
	}
	else if(argc == 2){
		return !strcmp(argv[1], "-h");
	}
	else if(argc != 5){
		return 0;
	}
	else {


		if(!strcmp(argv[1], "-p")&&!strcmp(argv[3], "-s")){
			if(is_number(argv[2])&&is_number_or_dot(argv[4])){
				if(atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535 || std::regex_match(string(argv[4]), std::regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"))==false){
					return false;
				}
				port = atoi(argv[2]); host = argv[4];
				return true;
			}
		}
		if(!strcmp(argv[1], "-s")&&!strcmp(argv[3], "-p")){
			if(is_number(argv[4])&&is_number_or_dot(argv[2])){
				if(atoi(argv[4]) < 1024 || atoi(argv[4]) > 65535 || std::regex_match(string(argv[2]), std::regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"))==false){
					return false;
				}
				port = atoi(argv[4]); host = argv[2];
				return true;
			}
		}

		return false;
	}
}

void sig_handler(int signal){
	if(sockett)
		close(sockett);
	if(sockett2)
		close(sockett2);
	exit(0);
}
void Usage(char* argv){
	cout << "Server Usage: " << argv << endl;
	cout << "Client Usage: " << argv << " -s ip -p port" << endl;
	cout << "Help Usage: " << argv << " -h" << endl;
}

bool is_number(const std::string& s)
{
	string::const_iterator it = s.begin();
	while (it != s.end() && (std::isdigit(*it))) ++it;
	return !s.empty() && it == s.end();
}
bool is_number_or_dot(const std::string& s)
{
	string::const_iterator it = s.begin();
	while (it != s.end() && (std::isdigit(*it)||*it=='.')) ++it;
	return !s.empty() && it == s.end();
}

void getmyip(){
	char buffer[128];
	std::string result = "";
	std::shared_ptr<FILE> pipe(popen("ip addr show em1", "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != NULL)
			result += buffer;
	}
	result = result.substr(result.find("inet")+5);
	result = result.substr(0, result.find("/"));
	cout << result;
}

// Server and Client Functions
int server(int port){
	int socketFileDesc;
	//int newSocketFileDesc;

	unsigned int sizeOfAddrClient;
	int n;
	char buffer[1000];
	struct sockaddr_in serverAddress, clientAddress;

  // Socket creation
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	sockett = socketFileDesc;
	if(socketFileDesc < 0){
		error("could not open the socket");
	}

  // Populating serverAddress with IP info and port #
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Binding the socket
	if(bind(socketFileDesc, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
		error("could not bind");
	}

  // Listen to the socket
	listen(socketFileDesc, 5);

	cout << "Welcome to Chat!" << endl;
	cout << "Waiting for connection on ";
	getmyip();
	cout << " port ";
	cout << port << endl;


  // Declare/initialize pthread array
  pthread_t cThreads[3];
  int threadCounter = 0;

  // 4. While loop statement
  while (threadCounter < 3){

    // Accept clients
    // sizeOfAddrClient is unsigned int. cplucplus example declared it to be socklen_t
    sizeOfAddrClient = sizeof(clientAddress);
    newSocketFileDesc = accept(socketFileDesc, (struct sockaddr *) &clientAddress, &sizeOfAddrClient);

    // Check if client was accepted
    sockett2 = newSocketFileDesc;
    if(newSocketFileDesc < 0){
      error("error on acceptance");
    }


    cout << "Found a friend! You recieve first." << endl;

    // **************Testing recv of struct p_SSInfo and unpacking

    SSInfo* testingSS;

    //bzero(buffer, 1000);

    int n = recv(newSocketFileDesc, buffer, sizeof(buffer), 0);

    //pac = (packet_t*)buffer;
    testingSS = (SSInfo *)buffer;

    // Checks for recv socket error
    if(n < 0){
      error("error reading from the socket");
    }

    // Test printing structure SSInfo
    /*
    typedef struct p_SSInfo {
      char* url;
      int remainingSS;
      vector<pair<string, int>> SSList;
    } SSInfo;
    */

    cout << "url is: " << testingSS->url << endl;
    cout << "# of remaing stones = " << testingSS->remainingSS << endl;
    cout << "SS list size is = " << testingSS->SSList.size() << endl;

    cout << "Finished unpacking" << endl;

    // ******************

    // pthread_create
    cout << "Starting creation of thread" << endl;
    pthread_create(&cThreads[threadCounter], NULL, PrintHello, NULL);
    threadCounter++;


  } // End of first while

  for (int i = 0; i < 3; i++) {
    pthread_join(cThreads[i], NULL);
  }


	return 0;
}


//------------------- thread start routine


void *PrintHello (void *dummyPt) {
  cout << "Thread No: " << pthread_self() << endl;
  //char test[300];
  //bzero(test, 301);
  bool loop = false;
	char buffer[1000];
  while(!loop){

    //bzero(test, 301);


    //read(connFd, test, 300);
    // our code
    //packet_t* pac;

    //printf("Friend: %s", pac->message);
label2:
    printf("You: ");
    //bzero(buffer,1000);
    fgets(buffer,1000,stdin);
    packet_t pak2 = {};

    if(strlen(buffer) > 140){
      cout << "message too long" << endl;
      goto label2;

    }

    strcpy(pak2.message, buffer);
    pak2.messageLength = strlen(buffer);
    pak2.version = 457;

    int n = send(newSocketFileDesc, (char*)&pak2, sizeof(packet_t), 0);


    if(n<0){
      error("couldn't write to socket");
    }


    //string tester (test);
    //cout << tester << endl;

    //if(tester == "exit")
     // break;
  }
  cout << "\nClosing thread and conn" << endl;
  close(newSocketFileDesc);
}

//------------------- end of thread start routine




int client(int port, char* hn){
	int socketFileDesc;

	int n;
	struct sockaddr_in serverAddress;
	struct hostent *server;
	char buffer[1000];

	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	sockett = socketFileDesc;
	if(socketFileDesc < 0){
		error("couldn't open the socket");
		exit(0);
	}
	server = gethostbyname(hn);
	if(server == NULL){
		error("error no host");
		exit(0);
	}
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
	serverAddress.sin_port = htons(port);
	cout << "Connecting to server... ";
	if(connect(socketFileDesc, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		error("couldn't connect");
	}
	cout << "Connected!" << endl;
	cout <<"Connected to a friend! You send first." << endl;
	while(true){
		packet_t* pak = (packet_t*)malloc(sizeof(packet_t));
label:
		printf("You: ");
		//bzero(buffer,1000);
		fgets(buffer,1000,stdin);

		if(strlen(buffer)>140){
			cout << "error string too long" << endl;
			goto label;
		}


		strcpy(pak->message, buffer);

		pak->messageLength = strlen(pak->message);
		pak->version = 457;

		memcpy(buffer, pak, sizeof(packet_t));

		n = send(socketFileDesc, buffer, sizeof(buffer), 0);

		if(n < 0){
			error("error writing to socket");
		}
		//bzero(buffer, 1000);
		packet_t* pak2;
		n = recv(socketFileDesc, buffer, 1000, 0);
		pak2 = (packet_t*)buffer;

		if (n < 0){
			error("error reading from socket");
		}

		printf("Friend: %s", pak2->message);

		free(pak);
	}
	return 0;
}
