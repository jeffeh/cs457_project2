all: ss awget
ss:
	g++ -std=c++0x -pthread -Wall -I. -lX11 SS.cpp -o ss
awget:
	g++ -std=c++0x -pthread -Wall -I. -lX11 Awget.cpp -o awget

