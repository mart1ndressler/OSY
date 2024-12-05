#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    int port = atoi(argv[1]), client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    cout << "[CLIENT] Connected to the server on port " << port << "." << endl;

    if(port % 2 == 1) 
    {
        string input;
        while(1) 
        {
            cout << "Enter coin value (END to stop): ";
            cin >> input;
            send(client_socket, input.c_str(), input.size() + 1, 0);
            if (input == "END") break;

            char buffer[1024] = {0};
            int bytes_read = read(client_socket, buffer, sizeof(buffer));
            if (bytes_read > 0) cout << "[SERVER] " << buffer;
        }
    } 
    else 
    {
        while(1) 
        {
            char buffer[1024] = {0};
            int bytes_read = read(client_socket, buffer, sizeof(buffer));
            if (bytes_read > 0) cout << "[SECURITY] " << buffer;
        }
    }

    cout << "[CLIENT] Disconnecting..." << endl;
    close(client_socket);
    return 0;
}