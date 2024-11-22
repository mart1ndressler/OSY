#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <cstring>
#include <ctime>

using namespace std;

#define N 5
#define LEN 32

struct SharedMemory 
{
    char items[N][LEN];
    int in = 0;
    int out = 0;
    long file_position = 0;
};

void cout_time(string message) 
{
    time_t now = time(0);
    cout << ctime(&now) << message << endl;
}

void insert_item(SharedMemory *shared_memory, char *item) 
{
    strncpy(shared_memory->items[shared_memory->in], item, LEN);
    if(strlen(item) < LEN) memset(shared_memory->items[shared_memory->in] + strlen(item), '\0', LEN - strlen(item));
    shared_memory->in = (shared_memory->in + 1) % N;
}

int main(int argc, char *argv[]) 
{
    bool initialize = (argc > 1 && string(argv[1]) == "-i");
    if(initialize) 
    {
        sem_unlink("/mutex");
        sem_unlink("/empty");
        sem_unlink("/full");
        sem_unlink("/file_access");
        shm_unlink("/shared_memory");
    }

    sem_t *mutex = sem_open("/mutex", O_CREAT, 0644, 1);
    sem_t *empty = sem_open("/empty", O_CREAT, 0644, N);
    sem_t *full = sem_open("/full", O_CREAT, 0644, 0);
    sem_t *file_access = sem_open("/file_access", O_CREAT, 0644, 1);

    int shm = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if(initialize) ftruncate(shm, sizeof(SharedMemory));
    SharedMemory *shared_memory = (SharedMemory *) mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);

    FILE *file = fopen("names.txt", "r");
    char line[LEN];
    while(1)
    {
        sem_wait(file_access);
        fseek(file, shared_memory->file_position, SEEK_SET);

        if(!fgets(line, sizeof(line), file)) {sem_post(file_access); break;}
        shared_memory->file_position = ftell(file);
        sem_post(file_access);
        line[strlen(line) - 1] = '\0';

        sem_wait(empty);
        sem_wait(mutex);
        insert_item(shared_memory, line);
        sem_post(mutex);
        sem_post(full);
        cout_time("Producer: Inserted item '" + string(line) + "' into buffer.\n");
        sleep(6);
    }
    fclose(file);

    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    sem_close(file_access);
    munmap(shared_memory, sizeof(SharedMemory));
    close(shm);

    if(initialize) 
    {
        sem_unlink("/mutex");
        sem_unlink("/empty");
        sem_unlink("/full");
        sem_unlink("/file_access");
        shm_unlink("/shared_memory");
    }
    return 0;
}