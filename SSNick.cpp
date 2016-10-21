
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
typedef struct p_SSInfo {
	char url[1000];
	int remainingSS;
	char SSList[1000];
	//vector<pair<string, int>> SSList;
} SSInfo;
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
void error(const string msg)
{
 perror((msg.c_str()));
 exit(1);
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
	int c = atoi(l.at(0).c_str());
	l.erase(l.begin(), l.begin()+1);
	for (string i:l){
		std::string host = i.substr(0, i.find(" "));
		std::string port = i.substr(i.find(" ")+1);
		ret.push_back({host, atoi(port.c_str())});
	}
	return ret;
}
void cconnect(int port, char* hn, char* url, vector<pair<string, int>> ss){
	int socketFileDesc;

		int n;
		struct sockaddr_in serverAddress;
		struct hostent *server;
		char buffer[4000];

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
		bzero((char *) &serverAddress, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
		serverAddress.sin_port = htons(port);
		cout << "Connecting to server... ";
		if(connect(socketFileDesc, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
			perror("couldn't connect");
		}
		cout << "Connected!" << endl;

		//while(true){
			SSInfo* pak = (SSInfo*)malloc(sizeof(SSInfo));



		  strcpy(pak->url, url);

		  pak->remainingSS = ss.size();
		  char tos[1000];
		  cout << convertListToString(ss)<<endl;
		  strcpy(pak->SSList, convertListToString(ss).c_str());
		  //pak->SSList = tos;
		memcpy(buffer, pak, sizeof(p_SSInfo));

		SSInfo* pakk = (p_SSInfo*)buffer;

		cout << "Sending url" << endl;
		n = send(socketFileDesc, buffer, sizeof(buffer), 0);

		if(n < 0){
			perror("error writing to socket");
		}
		//bzero(buffer, 4000);
//		packet_t* pak2;
//		n = recv(socketFileDesc, buffer, 4000, 0);
//		pak2 = (packet_t*)buffer;
//
//		if (n < 0){
//			error("error reading from socket");
//		}
//
//		printf("Friend: %s", pak2->message);
//
		free(pak);
		//}
}
void wget(char* arg){
	std::string w = "wget -q ";
	std::string g = arg;
	std::string l = w.append(g);

	system(l.c_str());

}
int main(int argc, char* argv[]){
	int port = 20000;
	int sockett, sockett2;
	int socketFileDesc;
		int newSocketFileDesc;

		unsigned int sizeOfAddrClient;
		int n;
		char buffer[4000];
		struct sockaddr_in serverAddress, clientAddress;

		socketFileDesc = socket(AF_INET, SOCK_STREAM, 0);
		sockett = socketFileDesc;
		if(socketFileDesc < 0){
			error("could not open the socket");
		}
		bzero((char *) &serverAddress, sizeof(serverAddress));
		serverAddress.sin_port = htons(port);
		serverAddress.sin_addr.s_addr = INADDR_ANY;

		if(bind(socketFileDesc, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
			error("could not bind");
		}
		listen(socketFileDesc, 5);
		cout << "Stepping Stone Started on ";
			getmyip();
			cout << " port ";
			cout << port << endl;
		sizeOfAddrClient = sizeof(clientAddress);
		newSocketFileDesc = accept(socketFileDesc, (struct sockaddr *) &clientAddress, &sizeOfAddrClient);
		sockett2 = newSocketFileDesc;
		if(newSocketFileDesc < 0){
			error("error on acceptance");
		}
		cout << "Connection recieved." << endl;
			//while(true){
				p_SSInfo* pac;



		//bzero(buffer, 4000);

		n = recv(newSocketFileDesc, buffer, sizeof(buffer), 0);

		pac = (p_SSInfo*) buffer;

		if(n < 0){
			error("error reading from the socket");
		}



		printf("Url recieved: %s\n", pac->url);
		printf("Chainlist recieved:\n");
		vector<pair<string, int>> listr = convertStringToList(pac->SSList);
		//cout << pac->SSList << endl;
		if(!listr.empty()){
			for(pair<string, int> i:listr){
						cout << "IP: " << i.first << ", Port:" << i.second << endl;
					}
			pair<string, int> next = popRandom(listr);
					cconnect(next.second, const_cast<char*>(next.first.c_str()), pac->url, listr);
		}
		else{
			cout << "this is the end, no chainlist" << endl;
			cout << "calling wget" << endl;
			wget(pac->url);
		}

		label:
//		  printf("You: ");
//		  //bzero(buffer,4000);
//		  fgets(buffer,4000,stdin);
//		  packet_t pak2 = {};
//
//		  if(strlen(buffer) > 140){
//			  cout << "message too long" << endl;
//			goto label;
//
//		  }
//
//		  strcpy(pak2.message, buffer);
//		  pak2.messageLength = strlen(buffer);
//		  pak2.version = 457;
//
//		n = send(newSocketFileDesc, (char*)&pak2, sizeof(packet_t), 0);
//

		if(n<0){
			error("couldn't write to socket");
		}


			//}

		return 0;
}



