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
    int in;
    int out;
};

void cout_time(string message) 
{
    time_t now = time(0);
    cout << ctime(&now) << message << endl;
}

void remove_item(SharedMemory *shared_memory, char *item) 
{
    strncpy(item, shared_memory->items[shared_memory->out], LEN);
    shared_memory->out = (shared_memory->out + 1) % N;
}

int main() 
{
    sem_t *mutex = sem_open("/mutex", 0);
    sem_t *empty = sem_open("/empty", 0);
    sem_t *full = sem_open("/full", 0);

    int shm = shm_open("/shared_memory", O_RDWR, 0666);
    SharedMemory *shared_memory = (SharedMemory *) mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    char item[LEN];

    while(1) 
    {
        cout_time("Consumer is waiting for a full slot...\n");
        sem_wait(full);
        sem_wait(mutex);
        remove_item(shared_memory, item);
        sem_post(mutex);
        sem_post(empty);

        cout_time("Consumer: Removed item '" + string(item) + "' from buffer.\n");
        sleep(3);
    }

    sem_close(mutex);
    sem_close(empty);
    sem_close(full);

    munmap(shared_memory, sizeof(SharedMemory));
    close(shm);
    return 0;
}