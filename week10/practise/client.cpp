#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <thread>

using namespace std;

void receive_messages(int client_socket) 
{
    char buffer[1024];
    while(1) 
    {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if(bytes_read <= 0) {cout << "Disconnected From Server!" << endl; exit(0);}
        buffer[bytes_read] = '\0';
        cout << buffer << endl;
    }
}

int main() 
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(client_socket, (sockaddr *) &server_addr, sizeof(server_addr));

    cout << "Connected To Server!" << endl;
    thread receiver(receive_messages, client_socket);
    receiver.detach();

    string message, input;
    int empty_line_count = 0;
    bool multiline_mode = false;

    while(1) 
    {
        getline(cin, input);
        if(input == "*") {cout << "Exiting..." << endl; break;}

        if(input.empty()) 
        {
            empty_line_count++;
            if(empty_line_count == 2) 
            {
                if(multiline_mode) 
                {
                    send(client_socket, message.c_str(), message.size(), 0);
                    cout << "Multiline Message Sent!" << endl;
                    message.clear();
                    multiline_mode = false;
                }
                else 
                {
                    multiline_mode = true;
                    cout << "Multiline Mode Started:" << endl;
                }
                empty_line_count = 0;
            }
        }
        else 
        {
            empty_line_count = 0;
            if(multiline_mode) message += input + "\n";
            else send(client_socket, input.c_str(), input.size(), 0);
        }
    }
    close(client_socket);
    return 0;
}