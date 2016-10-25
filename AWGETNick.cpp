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



typedef struct p_SSInfo {
	char url[1000];
	int remainingSS;
	char SSList[1000];
	//vector<pair<string, int>> SSList;
} SSInfo;

string convertListToString(vector<pair<string, int>> Stones);
int sockett;

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
		cout << "hostname is: " << gethostbyname(hn) << endl;

			SSInfo* pak = (SSInfo*)malloc(sizeof(SSInfo));



		  strcpy(pak->url, url);

		  pak->remainingSS = ss.size();
		  char tos[1000];
		  //cout << convertListToString(ss)<<endl;
		  strcpy(pak->SSList, convertListToString(ss).c_str());
		  //pak->SSList = tos;
		memcpy(buffer, pak, sizeof(p_SSInfo));

		SSInfo* pakk = (p_SSInfo*)buffer;

		cout << "Sending url" << endl;
		n = send(socketFileDesc, buffer, sizeof(buffer), 0);

		if(n < 0){
			perror("error writing to socket");
		}

		free(pak);
}

int main(int argc, char* arv[]){
	pair<const char*, const char*> a = handleArgs(argc, arv);
	//wget("https://www.cs.colostate.edu/~cs457/yr2016sp/more_assignments/proj2.html");
	if(fileExists(a.second)==false){
		string m = "File ";
		string o = a.second;
		string p = " not found";
		merror((m+o+p).c_str());
	}

	vector<pair<string, int>> s = readChainFile(const_cast<char *>(a.second));
	pair<string, int>firstStone = popRandom(s);
	cout << "Connecting to stepping stone: " << firstStone.first << " on port: " << firstStone.second << endl;
	cconnect(firstStone.second, const_cast<char *>(firstStone.first.c_str()),const_cast<char *>(a.first), s);

	//begin connection
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
	std::string w = "wget -q ";
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
//	 cout << "Number of stones: " << SteppingStonesCount << endl;
//	 cout << "Stones:" << endl;
//	 for(pair<string, int> t:Stones){
//		 cout << "IP: " << t.first << ", Port: " << t.second << endl;
//	 }
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
	int c = atoi(l.at(0).c_str());
	l.erase(l.begin(), l.begin()+1);
	for (string i:l){
		std::string host = i.substr(0, i.find(" "));
		std::string port = i.substr(i.find(" ")+1);
		ret.push_back({host, atoi(port.c_str())});
	}
	return ret;
}
