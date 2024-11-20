#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <cstring>

using namespace std;

#define N 5
#define LEN 32

struct SharedMemory 
{
    char items[N][LEN];
    int in = 0;
    int out = 0;
};

void spotreba(SharedMemory *shared_memory, sem_t *mutex, sem_t *empty, sem_t *full) 
{
    sem_wait(full);
    sem_wait(mutex);

    char item[LEN];
    strncpy(item, shared_memory->items[shared_memory->out], LEN);
    shared_memory->out = (shared_memory->out + 1) % N;

    sem_post(mutex);
    sem_post(empty);
    cout << item << endl;
}

int main()
{
    sem_t *mutex = sem_open("/mutex", O_CREAT, 0644, 1);
    sem_t *empty = sem_open("/empty", O_CREAT, 0644, N);
    sem_t *full = sem_open("/full", O_CREAT, 0644, 0);

    int shm = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(shm, sizeof(SharedMemory));
    SharedMemory *shared_memory = (SharedMemory *) mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);

    if(shared_memory->in == 0 && shared_memory->out == 0) memset(shared_memory->items, 0, sizeof(shared_memory->items));
    spotreba(shared_memory, mutex, empty, full);

    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    munmap(shared_memory, sizeof(SharedMemory));
    close(shm);
    return 0;
}