#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define K 3           // Số thực khách
#define M (2 * K + 1) // Số món tối đa người phục vụ có thể mang
#define SHM_KEY 1234  // Key cho shared memory
#define SEM_KEY 5678  // Key cho semaphore

// Cấu trúc dữ liệu lưu trong shared memory
typedef struct
{
    int p, c, d; // Số lượng món khai vị (P), món chính (C), bánh ngọt (D)
    int served;  // Số thực khách đã phục vụ
} SharedData;

// Hàm thao tác semaphore
void sem_op(int sem_id, int sem_num, int op)
{
    struct sembuf sb = {sem_num, op, 0};
    semop(sem_id, &sb, 1);
}

void chef(int shmid, int semid)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL));

    while (data->served < K)
    {
        sem_op(semid, 0, -1); // Wait
        if (data->p + data->c + data->d < 3 * K)
        {
            int choice = rand() % 3;
            if (choice == 0)
                data->p++;
            else if (choice == 1)
                data->c++;
            else
                data->d++;
            printf("Đầu bếp nấu: P=%d, C=%d, D=%d\n", data->p, data->c, data->d);
        }
        sem_op(semid, 0, 1); // Signal
        sleep(1);
    }
    shmdt(data);
}

void waiter(int shmid, int semid)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    int holding_p = 0, holding_c = 0, holding_d = 0;

    while (data->served < K)
    {
        sem_op(semid, 0, -1); // Wait
        if (holding_p + holding_c + holding_d < M)
        {
            if (data->p > 0)
            {
                data->p--;
                holding_p++;
            }
            if (data->c > 0)
            {
                data->c--;
                holding_c++;
            }
            if (data->d > 0)
            {
                data->d--;
                holding_d++;
            }
        }
        printf("Phục vụ cầm: P=%d, C=%d, D=%d\n", holding_p, holding_c, holding_d);
        sem_op(semid, 0, 1); // Signal
        sleep(1);
    }
    shmdt(data);
}

void customer(int shmid, int semid, int id)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    while (1)
    {
        sem_op(semid, 0, -1); // Wait
        if (data->served >= K)
        {
            sem_op(semid, 0, 1); // Signal
            break;
        }
        if (data->p > 0 && data->c > 0 && data->d > 0)
        {
            data->p--;
            data->c--;
            data->d--;
            data->served++;
            printf("Khách %d đã nhận đủ thức ăn và rời đi.\n", id);
        }
        sem_op(semid, 0, 1); // Signal
        sleep(1);
    }
    shmdt(data);
}

int main()
{
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL));
    data->p = rand() % 5;
    data->c = rand() % 5;
    data->d = rand() % 5;
    data->served = 0;
    shmdt(data);

    if (fork() == 0)
    {
        chef(shmid, semid);
        exit(0);
    }
    if (fork() == 0)
    {
        waiter(shmid, semid);
        exit(0);
    }
    for (int i = 0; i < K; i++)
    {
        if (fork() == 0)
        {
            customer(shmid, semid, i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < K + 2; i++)
        wait(NULL);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("Tất cả thực khách đã được phục vụ. Dọn dẹp tài nguyên.\n");
    return 0;
}
