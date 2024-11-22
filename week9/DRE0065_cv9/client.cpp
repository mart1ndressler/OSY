#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <cstring>

using namespace std;

void receive_messages(int client_socket) 
{
    char buffer[1024];
    while(1) 
    {
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if(bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            cout << buffer << endl;
            fflush(stdout);
        }
    }
}

int main() 
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3333);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr));

    thread receive_thread(receive_messages, client_socket);
    receive_thread.detach();

    string command;
    while(1) 
    {
        getline(cin, command);
        if(command == "exit") break;
        send(client_socket, command.c_str(), command.length(), 0);
    }

    close(client_socket);
    return 0;
}