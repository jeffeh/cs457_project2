/*
 * Main.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: wilsonno
 */
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
#include <string>
#include <signal.h>
using namespace std;
void wget(char* arg);
void removeF(const char* arg);
vector<pair<string, int>> readChainFile(char* filee);
pair<const char*, const char*> handleArgs(int argc, char* argv[]);
std::string basename(std::string const& pathname);
inline bool fileExists(const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}
void merror(const string msg);
pair<string, int> popRandom(vector<pair<string, int>>& Stones);
void error(const string msg);
int sockett;
void my_handler(int s){
	close(sockett);
	printf("caught signal %d\n", s);
	exit(1);
}

typedef struct p_SSInfo {
	char url[1000];
	int remainingSS;
	char SSList[1000];
} SSInfo;

typedef struct p_File {
	int seqNum;
	char Data[1400];
} FilePac;
typedef struct filePacket {
	char mes[1200];
}FILEInfo;

string convertListToString(vector<pair<string, int>> Stones);

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

	if(connect(socketFileDesc, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
		error("couldn't connect");
	}
	sockett = socketFileDesc;
	return socketFileDesc;
}
void sendMessage(int fileDesc, SSInfo* pak){
	char buffer[4000];
	memcpy(buffer, pak, sizeof(p_SSInfo));
	int n;
	n = send(fileDesc, buffer, sizeof(buffer), 0);
	if(n < 0){
		error("error writing to socket");
	}
}
char* get_all_buf(int sock, std::string & output) {
    char buffer[1200];

    int n;
    while((errno = 0, (n = recv(sock, buffer, sizeof(buffer), 0))>0) ||
          errno == EINTR)
    {
        if(n>0)
            output.append(buffer, n);
    }

    if(n < 0){
        /* handle error - for example throw an exception*/
    }
    return const_cast<char*>(output.c_str());
};
filePacket* recievePacket(int socketFileDesc, filePacket* OfType){

	int n = 0;
	char buffer[1200];
	string out;
	n = recv(socketFileDesc, buffer, sizeof(buffer), MSG_WAITALL);
//	while(out.length()<1200){
//		n = recv(socketFileDesc, buffer, sizeof(buffer), 0);
//		cout << "Gen" << endl;
//		out.append(buffer);
//		bzero(buffer, 1200);
//	}
//
	cout << "n was " << n << endl;
	filePacket* pac = (filePacket*) buffer;
	cout << "buffer size: " << strlen(buffer) << endl;
	cout << "buffer len: " << strlen(pac->mes) << endl;
	if(n<0){
		error("error reading from the socket");
	}
	return pac;
}
char* recieveMessage(int socketFileDesc){
	int n=0;
	char* buffer = (char*)malloc(1200*sizeof(char));
	n = recv(socketFileDesc, buffer, 1200, MSG_WAITALL);
	cout << "n " << n << endl;
	cout << strlen(buffer) << endl;
	return buffer;
}
void recieveFile(int fileDesc, const char* name);
int cconnect(int port, char* hn, char* url, vector<pair<string, int>> ss){
	int socketFileDesc = connectToServer(hn, port);


	SSInfo* pak = (SSInfo*)malloc(sizeof(SSInfo));

	strcpy(pak->url, url);
	pak->remainingSS = ss.size();
	strcpy(pak->SSList, convertListToString(ss).c_str());

	sendMessage(socketFileDesc, pak);
	recieveFile(socketFileDesc, basename(url));

	free(pak);
	return socketFileDesc;
}

int main(int argc, char* arv[]){

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);


	cout << "awget:" <<endl;
	pair<const char*, const char*> a = handleArgs(argc, arv);

	if(fileExists(a.second)==false){
		string m = "File ";
		string o = a.second;
		string p = " not found";
		merror((m+o+p).c_str());
	}
	cout << "Request: " << a.first << endl;
	cout << "chainlist is" << endl;
	vector<pair<string, int>> s = readChainFile(const_cast<char *>(a.second));
	for(pair<string, int> i:s){
		cout << i.first << ", " << i.second << endl;
	}
	pair<string, int>firstStone = popRandom(s);
	cout << "next SS is " << firstStone.first << ", " << firstStone.second << endl;
	cout << "waiting for file..." << endl;
	cconnect(firstStone.second, const_cast<char *>(firstStone.first.c_str()),const_cast<char *>(a.first), s);
	cout << "Goodbye!" << endl;

}





















void Usage(char* argg){
	cout << "Usage: " << endl;
	cout << "./" << argg << " [-c chainfile] [url]" << endl;
	exit(1);
}
pair<const char*, const char*> handleArgs(int argc, char* argv[]){
	if(argc == 2){
		//handle the case where argv[1] is a url
		return {argv[1], "chaingang.txt"};
	}
	else if(argc == 4){
		if(strcmp(argv[1],"-c") == 0){
			//handle the case where argv[2] is a chainfile
			return {argv[3], argv[2]};
		}
		else if(strcmp(argv[2],"-c") == 0){
			return {argv[1], argv[3]};
			//handle the case where argv[3] is a chainfile
		}
		else{
			cout << argv[1] << " " << argv[2] << endl;
			Usage(argv[0]);
			return {"",""};
		}
	}
	else{
		Usage(argv[0]);
		return {"",""};
	}
}
bool isValidIP(string s){
	return std::regex_match(s, std::regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"));
}
void error(const string msg)
{
 perror((msg.c_str()));
 exit(1);
}
void merror(const string msg){
	cerr << msg << endl;
	exit(1);
}
bool isInt(string s){
	std::istringstream ss(s);
	int i = 0;
	if(ss >> i){
		return true;
	}
	else
		return false;
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

void wget(char* arg){
	std::string w = "wget ";
	std::string g = arg;
	std::string l = w.append(g);

	system(l.c_str());

}
void removeF(const char* arg){
	if( std::remove(arg) != 0){
		perror("Error deleting file");
	}
	else
		cout << "File successfully deleted" << endl;

}

vector<pair<string, int>> readChainFile(char* filee){
	 std::ifstream file(filee);
	 std::string str;
	 int SteppingStonesCount = 0;
	 int count = 0;
	 vector<pair<string, int>> Stones;
	 while (std::getline(file, str))
	 {
		 if(count == 0){
			 if(isInt(str)==false){
				 merror("Chainfile format error");
			 }
			 SteppingStonesCount = atoi(str.c_str());
		 }
		 else{
			 std::string t = str;
			 std::string host = t.substr(0, t.find(" "));
			 std::string port = t.substr(t.find(" ")+1);
			 //cout << host << ", " << port << endl;
			 if(isInt(port)==false){
				 merror("Chainfile format error");
			 }
			 if(isValidIP(host)==false){
				 merror("Chainfile format error");
			 }
			 Stones.push_back({host, atoi(port.c_str())});
		 }
		 count ++;
	 }
	 if(SteppingStonesCount != Stones.size()){
		 merror("Chainfile format error");
	 }

	 return Stones;
}











pair<string, int> popRandom(vector<pair<string, int>>& Stones){

	srand(time(NULL));
	int elem = rand() % Stones.size();

	//cout << "p: " << Stones.at(elem).first << " s: " << Stones.at(elem).second << endl;
	pair<string, int> ret = make_pair(Stones.at(elem).first, Stones.at(elem).second);
	Stones.erase(Stones.begin()+elem);
	return ret;
}










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


void sendAck(int fileDesc){
	char a[4] = "ack";
	int n = send(fileDesc, a, sizeof(a), 0);
	if(n < 0){
		perror("error on ack");
	}
}

void recieveFile(int fileDesc, const char* name){


	int counter = 0;
	int n = 1;
	char buffer[1200];
	n = recv(fileDesc, buffer, 1200, MSG_WAITALL);
	FILE * out = fopen(buffer, "wb");
	while(n){
		char buffer[1200];
		n = recv(fileDesc, buffer, 1200, MSG_WAITALL);



		counter ++;

		if(strcmp(buffer, "END")==0){
			n = 0;
		} else {
			fwrite(buffer, 1200, 1, out);
		}
	}
	fclose(out);
	close(fileDesc);
	cout << "Received file "  << buffer << endl;
}





