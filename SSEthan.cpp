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
void getmyip();
void error(const string msg);
void sig_handler(int signal);
void Usage(char* argv);

void splitUrlName (const std::string &url);

vector<pair<string, int>> convertStringToList(char* Stones);
string convertListToString(vector<pair<string, int>> Stones);
void *unpackForNextSteppingStoneConnection(void *SSInfoArg);
void split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
void cconnect(int port, char* hn, char* url, vector<pair<string, int>> ss);
void wget(char* arg);
pair<string, int> popRandom(vector<pair<string, int>>& Stones);
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
	char url[1000];
	int remainingSS;
	char SSList[1000];
} SSInfo;

// Main Function
int main(int argc, char* argv[]){

	if(signal(SIGINT,sig_handler)==SIG_ERR){
		printf("can't signal");
	}
	if(checkArgs(argc, argv)==0){
		Usage(argv[0]);
		exit(-1);
	}
	switch(argc){
		case 1:
			server(20000);break;
		case 2:
			Usage(argv[0]); break;
		default:
			//client(port, host);
			cout << "default" << endl;
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
	char buffer[4000];
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

	cout << "Stepping stone has been setup!" << endl;
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


    cout << "A connection has been made!" << endl;
		cout << "Found a friend! You recieve first." << endl;

		// **************Testing recv of struct p_SSInfo and unpacking

		SSInfo* testingSS = (SSInfo*)malloc(sizeof(SSInfo));

		//bzero(buffer, 1000);

		int n = recv(newSocketFileDesc, buffer, sizeof(buffer), 0);

		//pac = (packet_t*)buffer;
		//testingSS = (SSInfo *)buffer;
		memcpy(testingSS, buffer, sizeof(p_SSInfo));
		cout << endl;
		cout << "testingSS remaining stepping stones: " << testingSS->remainingSS << endl;
		cout << endl;

		// Checks for recv socket error
		if(n < 0){
			error("error reading from the socket");
		}


    // sanity check to see if structure is correct
		printf("Requested: %s\n", testingSS->url);
		printf("Chainlist is:\n");
    // end of structure check

		// ******************

		// pthread_create
		cout << "Starting creation of thread" << endl;
		pthread_create(&cThreads[threadCounter], NULL, unpackForNextSteppingStoneConnection, (void *)testingSS);
		threadCounter++;

    //if wget is called, we need to read and send the file

	} // End of first while

	cout << "Threading part completed" << endl;

	for (int i = 0; i < 3; i++) {
		pthread_join(cThreads[i], NULL);
	}

	return 0;
}

// ----- creating a new thread start routine
void *unpackForNextSteppingStoneConnection(void *SSInfoArg) {
	cout << "Thread No: " << pthread_self() << endl;

	SSInfo *ss_data;

	ss_data = (SSInfo *)SSInfoArg;

  // Convert ss info list to vector<pair< ip address, port>>
  vector<pair<string, int>> listr = convertStringToList(ss_data->SSList);

  if(!listr.empty()){
    // print out remaining stone lists
    for(pair<string, int> i:listr){
      cout << "\tIP: " << i.first << ", Port:" << i.second << endl;
    }
    // remove the randomly chosen stone from list
    pair<string, int> next = popRandom(listr);
    // connect to the randomly chosen stone
    cconnect(next.second, const_cast<char*>(next.first.c_str()), ss_data->url, listr);
  }
  else{
    cout << "this is the end, no chainlist" << endl;
    cout << "calling wget" << endl;
    wget(ss_data->url);
    cout << "wget is done calling!" << endl;
    splitUrlName(ss_data->url);
  }
}

void splitUrlName (const std::string &url) {
	cout << "Splitting url: " << url << endl;
	std::size_t found = url.find_last_of("/\\");
	cout << "File Name: " << url.substr(found + 1) << endl;
}

// ----- ending new thread routine

vector<pair<string, int>> convertStringToList(char* Stones){
	vector<pair<string, int>> ret;
	const string st = Stones;
	vector<string> l = split(st, '\n');
	int c = atoi(l.at(0).c_str());
	l.erase(l.begin(), l.begin()+1);
	for (string i:l){
		std::string host = i.substr(0, i.find(" "));
		std::string port = i.substr(i.find(" ")+1);
		ret.push_back({host, atoi(port.c_str())});
	}
	return ret;
}

string convertListToString(vector<pair<string, int>> Stones){
	string i = "";
	i+=std::to_string(Stones.size());
	cout << "convertListToString before for loop and stone size is : " << Stones.size() << endl;
	i+="\n";
	for(pair<string, int> p:Stones){
		i+=p.first;
		i+= " ";
		i+=std::to_string(p.second);
		i+="\n";
	}

	return const_cast<char*>( i.substr(0, i.size()-1).c_str());
}


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}


// (c)lient connect
void cconnect(int port, char* hn, char* url, vector<pair<string, int>> ss){
	int socketFileDesc;

	int n;
	struct sockaddr_in serverAddress;
	struct hostent *server;
	char buffer[4000];

		// Open a socket
		socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
		sockett = socketFileDesc;
		if(socketFileDesc < 0){
			perror("couldn't open the socket");
			exit(0);
		}
		server = gethostbyname(hn);
		if(server == NULL){
			perror("error no host");
			exit(0);
		}

		// Populating serverAddress struct
		bzero((char *) &serverAddress, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
		serverAddress.sin_port = htons(port);

		cout << "Connecting to server... ";
		if(connect(socketFileDesc, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
			perror("couldn't connect");
		}
		cout << "Connected!" << endl;

			SSInfo* pak = (SSInfo*)malloc(sizeof(SSInfo));

	strcpy(pak->url, url);
	pak->remainingSS = ss.size();
	char tos[1000];
	cout << convertListToString(ss)<<endl;
	string stringzz = convertListToString(ss).c_str();
	strcpy(pak->SSList, convertListToString(ss).c_str());

	memcpy(buffer, pak, sizeof(p_SSInfo));

	SSInfo* pakk = (p_SSInfo*)buffer;

	cout << "Sending url" << endl;
	cout << "The url is: " << pakk -> url << endl;
	n = send(socketFileDesc, buffer, sizeof(buffer), 0);

		if(n < 0){
			perror("error writing to socket");
		}
		free(pak);
}



void wget(char* arg){
	std::string w = "wget -q ";
	std::string g = arg;
	std::string l = w.append(g);
	
	system(l.c_str());

}

pair<string, int> popRandom(vector<pair<string, int>>& Stones){

	srand(time(NULL));
	int elem = rand() % Stones.size();

	cout << "next SS is " << Stones.at(elem).first << " " << Stones.at(elem).second << endl;
	pair<string, int> ret = make_pair(Stones.at(elem).first, Stones.at(elem).second);
	Stones.erase(Stones.begin()+elem);
	return ret;
}

