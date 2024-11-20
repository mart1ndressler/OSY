all: producer consumer server

producer: producer.cpp
	g++ producer.cpp -o producer -pthread -std=c++11

consumer: consumer.cpp
	g++ consumer.cpp -o consumer -pthread -std=c++11

server: server.cpp
	g++ server.cpp -o server -pthread -std=c++11

clean:
	rm -f producer consumer server