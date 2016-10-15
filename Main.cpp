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
using namespace std;
void wget(char* arg);
void removeF(const char* arg);
//void readChainFile();
std::string basename(std::string const& pathname);
int main(int argc, char* arv[]){
	wget("https://www.cs.colostate.edu/~cs457/yr2016sp/more_assignments/proj2.html");
	//readChainFile();
	//removeF(basename("https://www.cs.colostate.edu/~cs457/yr2016sp/more_assignments/proj2.html").c_str());
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
//
//void readChainFile(){
//	 std::ifstream file("Read.txt");
//	 std::string str;
//	 int c = 0;
//	 int count = 0;
//	 while (std::getline(file, str))
//	 {
//	    cout << str << endl;
//	    if(c == 0){
//	    	count = stoi(str);
//	    }
//	    else{
//
//	    }
//	    c++;
//	 }
//}
