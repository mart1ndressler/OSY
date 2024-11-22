all: server client

server: server.cpp
	g++ server.cpp -o server -pthread -std=c++11

client: client.cpp
	g++ client.cpp -o client -pthread -std=c++11

clean:
	rm -f server client