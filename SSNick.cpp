
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include <sstream>
#include <fstream>
#include <regex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <signal.h>
using namespace std;
int sockett;
//std::mutex lock;
//vector<int> socketts;
//void addSock(int i){
//	lock.lock();
//	socketts.push_back(i);
//	lock.unlock();
//
//}
//void removeSock(int i){
//	lock.lock();
//	for(int j=0; j<socketts.size(); j++){
//		if(i==socketts.at(j)){
//			socketts.erase(socketts.begin()+j);
//			lock.unlock();
//			return;
//		}
//	}
//	lock.unlock();
//}
void chunkFile(char *fullFilePath, unsigned long chunkSize, int fileDesc);
/* Packet Structure for Chainfile and URL */
typedef struct p_SSInfo {
	char url[1000];
	int remainingSS;
	char SSList[1000];
} SSInfo;
typedef struct filePacket {
	char mes[1200];
} FILEInfo;

/* Prints the IP Address of the local machine */
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
/* Print an error message */
void error(const string msg)
{
 perror((msg.c_str()));
 exit(1);
}

/* Takes a random pair<string, int> from the vector, removes it and returns it */
pair<string, int> popRandom(vector<pair<string, int>>& Stones){
	/* random seed everytime */
	srand(time(NULL));
	/* elem is index of random element */
	int elem = rand() % Stones.size();
	pair<string, int> ret = make_pair(Stones.at(elem).first, Stones.at(elem).second);
	Stones.erase(Stones.begin()+elem);
	return ret;
}








/* converts the list to a string */
string convertListToString(vector<pair<string, int>> Stones){
	string i = "";
	i+=std::to_string(Stones.size());
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
/* converts a chainfile string to a list */
vector<pair<string, int>> convertStringToList(char* Stones){
	vector<pair<string, int>> ret;
	const string st = Stones;
	vector<string> l = split(st, '\n');
	l.erase(l.begin(), l.begin()+1);
	for (string i:l){
		std::string host = i.substr(0, i.find(" "));
		std::string port = i.substr(i.find(" ")+1);
		ret.push_back({host, atoi(port.c_str())});
	}
	return ret;
}

int connectToServer(char* hostname, int port){
	int socketFileDesc;
	struct sockaddr_in serverAddress;
	struct hostent *server;
	socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
	sockett = socketFileDesc;
	if(socketFileDesc < 0){
		perror("couldn't open the socket");
		exit(0);
	}
	server = gethostbyname(hostname);
	if(server == NULL){
		perror("error no host");
		exit(0);
	}
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
	serverAddress.sin_port = htons(port);
	cout << "Connecting to server... ";
	if(connect(socketFileDesc, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		perror("couldn't connect");
	}
	cout << "Connected!" << endl;
	sockett = socketFileDesc;
	return socketFileDesc;
}
void sendMessage(int sock, char mes[1200]);
void sendMessage(int fileDesc, SSInfo* pak){
	char buffer[4000];
	memcpy(buffer, pak, sizeof(p_SSInfo));
	int n;
	n = send(fileDesc, buffer, sizeof(buffer), 0);
	if(n < 0){
		perror("error writing to socket");
	}
}
void sendMessage(int fileDesc, filePacket* pak){
	char buffer[1200];

	memcpy(buffer, pak, sizeof(filePacket));
	int n = 0;

	cout << "buff size: " << sizeof(buffer) << endl;
	n = send(fileDesc, buffer, sizeof(buffer), 0);
	if(n < 0){
		perror("error writing to socket");
	}
}
int cconnect(int port, char* hn, char* url, vector<pair<string, int>> ss){
	int socketFileDesc = connectToServer(hn, port);


	SSInfo* pak = (SSInfo*)malloc(sizeof(SSInfo));

	strcpy(pak->url, url);
	pak->remainingSS = ss.size();
	strcpy(pak->SSList, convertListToString(ss).c_str());

	sendMessage(socketFileDesc, pak);

	free(pak);
	return socketFileDesc;

}
std::string wget(char* arg){

	char buffer[128];
	std::string w = "wget ";
	std::string g = arg;
	std::string l = w.append(g);
	l = l.append(" 2>&1");
	std::string result = "";
	char* cmd = const_cast<char*>(l.c_str());
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	sleep(1);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != NULL){
			result += buffer;
		}

	}

	std::string s = "Saving to: ";
	std::size_t loc = result.find(s, 0);
	loc += 14;
	result = result.substr(loc);
	result = result.substr(0, result.find("’", 0));





	return result;

}
int setUpServer(int port){





	struct sockaddr_in serverAddress;

	int socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);

	if(socketFileDesc < 0){
		error("could not open the socket");
	}
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if(bind(socketFileDesc, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
		error("could not bind");
	}
	return socketFileDesc;
}
int listenAndAccept(int socketFileDesc){
	listen(socketFileDesc, 5);
	struct sockaddr_in clientAddress;
	unsigned int sizeOfAddrClient = sizeof(clientAddress);
	int newSocketFileDesc = accept(socketFileDesc, (struct sockaddr *) &clientAddress, &sizeOfAddrClient);

	if(newSocketFileDesc < 0){
		error("error on acceptance");
	}
	cout << "Connection recieved." << endl;
	return newSocketFileDesc;
}
p_SSInfo* recievePacket(int socketFileDesc, p_SSInfo* OfType){
	int n = 0;
	char buffer[4000];
	n = recv(socketFileDesc, buffer, sizeof(buffer), 0);

	p_SSInfo* pac = (p_SSInfo*) buffer;

		if(n < 0){
			error("error reading from the socket");
		}
		return pac;
}
filePacket* recievePacket(int socketFileDesc, filePacket* OfType){
	int n = 0;
	char buffer[4000];
	n = recv(socketFileDesc, buffer, sizeof(buffer), 0);

	filePacket* pac = (filePacket*) buffer;
	if(n<0){
		error("error reading from the socket");
	}
	OfType = pac;
	return pac;
}
struct MatchPathSeparator
{
    bool operator()( char ch ) const
    {
        return ch == '\\' || ch == '/';
    }
};
std::string
basename( std::string const& pathname )
{
    return std::string(
        std::find_if( pathname.rbegin(), pathname.rend(),
                      MatchPathSeparator() ).base(),
        pathname.end() );
}

void sendAck(int fileDesc){
	char a[4] = "ack";
	int n = send(fileDesc, a, sizeof(a), 0);
	if(n < 0){
		perror("error on ack");
	}
}
void receiveAck(int fileDesc){
	char buffer[4];
	int n = recv(fileDesc, buffer, sizeof(buffer), 0);
	if(n < 0){
			error("error reading from the socket");
		}
}
void removeF(const char* arg){
	if( std::remove(arg) != 0){
		perror("Error deleting file");
	}

}
void mt(int socketFileDesc, int newSocketFileDesc){
	int n = 1;


			if(newSocketFileDesc < 0){
				error("error on acceptance");
			}


		p_SSInfo* pac;
		pac = recievePacket(newSocketFileDesc, pac);









		vector<pair<string, int>> listr = convertStringToList(pac->SSList);

		if(!listr.empty()){
			printf("Request: %s\n", pac->url);
			printf("chainlist is\n");
			for(pair<string, int> i:listr){
						cout << "IP: " << i.first << ", Port:" << i.second << endl;
					}
			pair<string, int> next = popRandom(listr);
			cout << "next SS is " << next.first << ", " << next.second << endl;
			cout << "waiting for file..." << endl;
					int nextFD = cconnect(next.second, const_cast<char*>(next.first.c_str()), pac->url, listr);
					int n = 1;
					char fnbuffer[1200];
					n = recv(nextFD, fnbuffer, 1200, MSG_WAITALL);
					if(n<0){
						cout << "Error recieving filename" << endl;
					}
					sendMessage(newSocketFileDesc, fnbuffer);
					cout << "Relaying file ..." << endl;
					while(n){
							char buffer[1200];
							n = recv(nextFD, buffer, 1200, MSG_WAITALL);

							if(strcmp(buffer, "END")==0){
								sendMessage(newSocketFileDesc, buffer);
								n = 0;


							} else {
								sendMessage(newSocketFileDesc, buffer);

							}
							cout << n << endl;
						}
					cout << "Goodbye!" << endl;
					close(nextFD);

		}

		// If the recieved chainfile is empty, this is the last one, call wget
		else{
			cout << "Request: " << pac->url << endl;
			cout << "chainlist is empty" << endl;
			std::string filename = wget(pac->url);
			cout << "issueing wget for file " << filename << endl;
			cout << "File received" << endl;
			cout << "Relaying file ..." << endl;
			char tos[1200];
			strcpy(tos, filename.c_str());
			sendMessage(newSocketFileDesc, tos);

			char* localFile = const_cast<char*>(filename.c_str());
			chunkFile(localFile, 1200, newSocketFileDesc);
			removeF(localFile);
			cout << "Goodbye!"<<endl;
		}

		if(n<0){
			error("couldn't write to socket");
		}
}
bool is_number(const std::string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it))) ++it;
    return !s.empty() && it == s.end();
}
int handleArgs(int argc, char* argv[]){
	if(argc != 3){
		error("argument error");
	}
	else if(strcmp(argv[1], "-p")!=0){
		error("argument error");
	}
	std::string three = argv[2];
	if(is_number(three)==false){
		error("argument error");
	}
	return atoi(argv[2]);

}
void do_join(std::thread& t)
{
    t.join();
}

void join_all(std::vector<std::thread>& v)
{
    std::for_each(v.begin(),v.end(),do_join);
}
vector<int> socketts;
void my_handler(int s){

           for (int i:socketts){
        	   close(i);
           }
           exit(1);
}

int main(int argc, char* argv[]){
	struct sigaction sigIntHandler;

	   sigIntHandler.sa_handler = my_handler;
	   sigemptyset(&sigIntHandler.sa_mask);
	   sigIntHandler.sa_flags = 0;

	   sigaction(SIGINT, &sigIntHandler, NULL);
	int port = handleArgs(argc, argv);
	int socketFileDesc = setUpServer(port);
	socketts.push_back(socketFileDesc);
	cout << "ss  "; getmyip();cout << ", " << port << ":" << endl;
	std::vector<std::thread> ts;

	while(true){

	listen(socketFileDesc, 5);
	struct sockaddr_in clientAddress;
			unsigned int sizeOfAddrClient = sizeof(clientAddress);
			int newSocketFileDesc = accept(socketFileDesc, (struct sockaddr *) &clientAddress, &sizeOfAddrClient);
			socketts.push_back(newSocketFileDesc);

	ts.push_back(std::thread(mt, socketFileDesc, newSocketFileDesc));

	}
	join_all(ts);




		return 0;
}
void sendMessage(int sock, char mes[1200]){
	int n = send(sock, mes, 1200, 0);
	if(n<0)
		error("couldn't write to socket");
}

// MSG_WAITALL
void chunkFile(char *fullFilePath, unsigned long chunkSize, int fileDesc) {
	ifstream fileStream;
	fileStream.open(fullFilePath, ios::in | ios::binary);

	if (fileStream.is_open()) {
		ofstream output;
		int counter = 1;






		while (!fileStream.eof()) {
				char* buffer = new char[chunkSize];

				fileStream.read(buffer,chunkSize);

				counter++;




				sendMessage(fileDesc, buffer);




		}



		char mes[1200] = "END";
		sendMessage(fileDesc, mes);



		fileStream.close();


	}
	else { cout << "Error opening file!" << endl; }
}

