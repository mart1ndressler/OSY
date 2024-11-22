#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <cstring>
#include <fcntl.h>

#define N 5
#define LEN 32

using namespace std;

struct SharedMemory 
{
    char items[N][LEN];
    int in = 0;
    int out = 0;
};

sem_t *sem_mutex, *sem_empty, *sem_full;
SharedMemory *shared_memory;

void print_memory() 
{
    sem_wait(sem_mutex);
    cout << "Current Memory State: ";
    for(int i=0; i < N; i++) 
    {
        if(strlen(shared_memory->items[i]) > 0) cout << "[" << shared_memory->items[i] << "]";
        else cout << "[EMPTY]";
    }
    cout << endl;
    sem_post(sem_mutex);
}

void handle_client(int client_socket) 
{
    cout << "New Client Connected!" << endl;
    char buffer[1024];

    while(1) 
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if(bytes_received <= 0) 
        {
            cout << "Client Disconnected!" << endl;
            close(client_socket);
            return;
        }

        if(strncmp(buffer, "#write", 6) == 0) 
        {
            char *items = buffer + 7;
            char *word = strtok(items, " ");
            while(word) 
            {
                if(sem_trywait(sem_empty) == 0) 
                {
                    sem_wait(sem_mutex);
                    strncpy(shared_memory->items[shared_memory->in], word, LEN);
                    shared_memory->in = (shared_memory->in + 1) % N;
                    sem_post(sem_mutex);
                    sem_post(sem_full);
                    send(client_socket, "OK\n", 3, 0);
                } 
                else 
                {
                    send(client_socket, "FULL\n", 5, 0);
                    break;
                }
                word = strtok(NULL, " ");
            }
            print_memory();
        } 
        else if(strncmp(buffer, "#read", 5) == 0) 
        {
            if(sem_trywait(sem_full) == 0) 
            {
                sem_wait(sem_mutex);
                char item[LEN];
                strncpy(item, shared_memory->items[shared_memory->out], LEN);
                shared_memory->items[shared_memory->out][0] = '\0';
                shared_memory->out = (shared_memory->out + 1) % N;
                sem_post(sem_mutex);
                sem_post(sem_empty);
                strcat(item, "\n");
                send(client_socket, item, strlen(item), 0);
            } 
            else send(client_socket, "EMPTY\n", 6, 0);
            print_memory();
        } 
        else if(strncmp(buffer, "#readall", 8) == 0) 
        {
            sem_wait(sem_mutex);
            string response;
            while(sem_trywait(sem_full) == 0) 
            {
                response += shared_memory->items[shared_memory->out];
                response += "\n";
                shared_memory->items[shared_memory->out][0] = '\0';
                shared_memory->out = (shared_memory->out + 1) % N;
                sem_post(sem_empty);
            }
            sem_post(sem_mutex);
            send(client_socket, response.c_str(), response.length(), 0);
            print_memory();
        } 
        else send(client_socket, "Invalid Command!\n", 18, 0);
    }
}

void initialize_memory() 
{
    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedMemory));
    shared_memory = (SharedMemory *)mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    memset(shared_memory, 0, sizeof(SharedMemory));
    shm_unlink("/shared_memory");

    sem_mutex = sem_open("/sem_mutex", O_CREAT | O_EXCL, 0644, 1);
    sem_empty = sem_open("/sem_empty", O_CREAT | O_EXCL, 0644, N);
    sem_full = sem_open("/sem_full", O_CREAT | O_EXCL, 0644, 0);

    sem_unlink("/sem_mutex");
    sem_unlink("/sem_empty");
    sem_unlink("/sem_full");
}

int main(int argc, char *argv[]) 
{
    if(argc > 1 && string(argv[1]) == "-i") 
    {
        initialize_memory();
        cout << "Shared Memory and Semaphores Initialized." << endl;
        return 0;
    }

    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedMemory));
    shared_memory = (SharedMemory *)mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_mutex = sem_open("/sem_mutex", O_CREAT, 0644, 1);
    sem_empty = sem_open("/sem_empty", O_CREAT, 0644, N);
    sem_full = sem_open("/sem_full", O_CREAT, 0644, 0);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(3333);

    bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    cout << "Server Started..." << endl;
    while(1) 
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    sem_close(sem_mutex);
    sem_close(sem_empty);
    sem_close(sem_full);
    munmap(shared_memory, sizeof(SharedMemory));
    close(server_socket);
    return 0;
}