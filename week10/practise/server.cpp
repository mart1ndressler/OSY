#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <poll.h>
#include <cstring>
#include <algorithm>
#include <sys/msg.h>

using namespace std;

struct Message 
{
    long priority;
    char text[1024];
};

int msg_queue_id;
void broadcast_message(const string &message, const vector<int> &client_sockets, int sender_socket)
{
    for(int socket : client_sockets) 
    {
        if(socket != sender_socket) if(send(socket, message.c_str(), message.size(), 0) < 0) perror("Send Failed!");
    }
}

void cleanup_disconnected_client(vector<int> &client_sockets, vector<pollfd> &poll_fds, int client_socket)
{
    client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
    poll_fds.erase(remove_if(poll_fds.begin(), poll_fds.end(),
        [client_socket](pollfd const &pfd) {return pfd.fd == client_socket;}),poll_fds.end());
    close(client_socket);
    cout << "Cleaned Up Disconnected Client [" << client_socket << "]" << endl;
}

void handle_client(int client_socket, int msg_queue_id) 
{
    char buffer[1024];
    string accumulated_message;
    int empty_line_count = 0;
    bool multiline_mode = false;

    while(1) 
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if(bytes_read <= 0) 
        {
            Message msg = {10, ""};
            snprintf(msg.text, sizeof(msg.text), "%d", client_socket);
            if(msgsnd(msg_queue_id, &msg, sizeof(msg.text), 0) < 0) perror("msgsnd: Failed for Disconnect Message!");
            close(client_socket);
            exit(0);
        }

        buffer[bytes_read] = '\0';
        if(strcmp(buffer, "\n") == 0) 
        {
            empty_line_count++;
            if(empty_line_count == 2) 
            {
                if(multiline_mode) 
                {
                    Message msg = {1, ""};
                    snprintf(msg.text, sizeof(msg.text), "Client [%d]:\n%s", client_socket, accumulated_message.c_str());
                    if(msgsnd(msg_queue_id, &msg, sizeof(msg.text), 0) < 0) perror("msgsnd: Failed for Multiline Message!");
                    accumulated_message.clear();
                    multiline_mode = false;
                } 
                else multiline_mode = true;
                empty_line_count = 0;
            }
        } 
        else 
        {
            empty_line_count = 0;
            if(multiline_mode) accumulated_message += string(buffer) + "\n";
            else 
            {
                Message msg = {1, ""};
                snprintf(msg.text, sizeof(msg.text), "Client [%d]: %.1000s", client_socket, buffer);
                if(msgsnd(msg_queue_id, &msg, sizeof(msg.text), 0) < 0) perror("msgsnd: Failed for Normal Message!");
            }
        }
    }
}

int main() 
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0), opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    msg_queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    cout << "Server is running..." << endl;
    vector<int> client_sockets;
    vector<pollfd> poll_fds;

    pollfd server_poll_fd = {server_socket, POLLIN, 0};
    poll_fds.push_back(server_poll_fd);

    while(1) 
    {       
        int poll_count = poll(poll_fds.data(), poll_fds.size(), 1);
        Message msg;
        while(msgrcv(msg_queue_id, &msg, sizeof(msg.text), 0, IPC_NOWAIT) > 0) 
        {
            if(msg.priority == 1) 
            {
                cout << msg.text << endl;
                broadcast_message(msg.text, client_sockets, -1);
            } 
            else if(msg.priority == 10) 
            {
                int client_socket = atoi(msg.text);
                cleanup_disconnected_client(client_sockets, poll_fds, client_socket);
            }
        }

        for(int i = 0; i < poll_fds.size(); i++) 
        {
            if(poll_fds[i].revents & POLLIN) 
            {
                if(poll_fds[i].fd == server_socket) 
                {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
                    cout << "New Client Connected: Socket [" << client_socket << "]" << endl;

                    if(fork() == 0)
                    {
                        for(auto &pfd : poll_fds) close(pfd.fd);
                        handle_client(client_socket, msg_queue_id);
                    }
                    else 
                    {
                        client_sockets.push_back(client_socket);
                        pollfd client_poll_fd = {client_socket, POLLIN, 0};
                        poll_fds.push_back(client_poll_fd);
                    }
                }
            }
        }
    }
    msgctl(msg_queue_id, IPC_RMID, NULL);
    close(server_socket);
    return 0;
}