#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>

using namespace std;

void handle_client(int client_socket) 
{
    char buffer[64];
    while(1)
    {
        memset(buffer, 0, 64);
        int bytes_received = read(client_socket, buffer, 64);
        if(bytes_received <= 0) {close(client_socket); exit(0);}

        if(strncmp(buffer, "push", 4) == 0) 
        {
            char *command = buffer + 5;
            command[strcspn(command, "\n\r ")] = '\0';

            cout << "PUSH command: " << command << endl;
            if(fork() == 0) 
            {
                execl("./producer", "./producer", command, NULL);
                exit(1);
            } 
            else send(client_socket, "OK\n", 3, 0);
        }
        else if(strncmp(buffer, "pop", 3) == 0) 
        {
            int pipefd[2];
            pipe(pipefd);

            cout << "POP command!\n";
            if(fork() == 0) 
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                execl("./consumer", "./consumer", NULL);
                exit(1);
            } 
            else 
            {
                close(pipefd[1]);
                char result[64];
                memset(result, 0, 64);
                read(pipefd[0], result, 64);
                close(pipefd[0]);
                send(client_socket, result, strlen(result), 0);
            }
        }

        close(client_socket);
        return;
    }
}

int main() 
{
    cout << "Starting server...\n";
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3333);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    while(1) 
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if(fork() == 0) 
        {
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        } 
        else close(client_socket);
    }
    close(server_socket);
    return 0;
}