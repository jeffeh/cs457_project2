
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
using namespace std;
int sockett;

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
	int c = atoi(l.at(0).c_str());
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
	return socketFileDesc;
}
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
	filePacket* pack = (filePacket*)pak;

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
void wget(char* arg){
	std::string w = "wget -q ";
	std::string g = arg;
	std::string l = w.append(g);

	system(l.c_str());

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
	//cout << "recv got: " << pac->last << endl;
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
	char* a = "ack";
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

int main(int argc, char* argv[]){
	cout << sizeof(filePacket) << endl;
	filePacket* p = (filePacket*)malloc(sizeof(filePacket));
	//cout << p->last << endl;
	filePacket* p2 = (filePacket*)malloc(sizeof(filePacket));
		//cout << p2->last << endl;
	int port = 20001;
	int socketFileDesc = setUpServer(port);
	int n;
	char buffer[4000];

	cout << "Stepping Stone Started on "; getmyip();cout << " port " << port << endl;

	int newSocketFileDesc = listenAndAccept(socketFileDesc);


	p_SSInfo* pac;
	pac = recievePacket(newSocketFileDesc, pac);








	printf("Url recieved: %s\n", pac->url);
	printf("Chainlist recieved:\n");
	vector<pair<string, int>> listr = convertStringToList(pac->SSList);

	// If the recieved chainfile is not empty, forward to the next stone
	if(!listr.empty()){
		for(pair<string, int> i:listr){
					cout << "IP: " << i.first << ", Port:" << i.second << endl;
				}
		pair<string, int> next = popRandom(listr);
				int nextFD = cconnect(next.second, const_cast<char*>(next.first.c_str()), pac->url, listr);
				lo:
				filePacket* f;
				//cout << f->last << "(before)" << endl;
				recievePacket(nextFD, f);
				sendMessage(newSocketFileDesc, f);
				//cout <<"last: " << f->last << endl;


//				if(f->last == 0){
//					f = NULL;
//					goto lo;
//				}
//				else{
//					cout << "last no zero" << endl;
//				}

	}

	// If the recieved chainfile is empty, this is the last one, call wget
	else{
		cout << "this is the end, no chainlist" << endl;
		cout << "calling wget" << endl;
		wget(pac->url);
		char* localFile = basename(pac->url);
		chunkFile(localFile, 1200, newSocketFileDesc);
	}

	if(n<0){
		error("couldn't write to socket");
	}




		return 0;
}


void chunkFile(char *fullFilePath, unsigned long chunkSize, int fileDesc) {
	ifstream fileStream;
	fileStream.open(fullFilePath, ios::in | ios::binary);

	if (fileStream.is_open()) {
		ofstream output;
		int counter = 1;



		char *buffer = new char[chunkSize];


		while (!fileStream.eof()) {
				char* buffer = new char[chunkSize];
				cout << "starting read" << endl;
				fileStream.read(buffer,chunkSize);
				cout << "read " << endl;
				counter++;
				filePacket* p = (filePacket*)malloc(sizeof(filePacket));


				strcpy(p->mes, buffer);



				sendMessage(fileDesc, p);
				receiveAck(fileDesc);

				cout << "recieved ack" << endl;
				//bzero(buffer, chunkSize);
				//free(p);
				cout << "endloop " << counter-1 << endl;
		}
		filePacket* p = (filePacket*)malloc(sizeof(filePacket));
		//bzero(buffer, chunkSize);
//		buffer = "x";
//		strcpy(p->last, buffer);

		char mes[1200] = "END";
		strcpy(p->mes, mes);
		sendMessage(fileDesc, p);
		cout << "sent " << counter-1 << endl;

		//free(p);

		//delete(buffer);


		fileStream.close();
		cout << "Chunking complete! " << counter - 1 << " files created." << endl;
		while(true);
	}
	else { cout << "Error opening file!" << endl; }
}


//http://www.coderslexicon.com/file-chunking-and-merging-in-c/
