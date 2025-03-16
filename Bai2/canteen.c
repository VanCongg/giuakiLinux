#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_KEY 1234 // Key cho shared memory
#define SEM_KEY 5678 // Key cho semaphore

// Cấu trúc dữ liệu lưu trong shared memory
typedef struct
{
    int k;
    int p, c, d;                         // Số lượng món khai vị (P), món chính (C), bánh ngọt (D)
    int holding_p, holding_c, holding_d; // Số lượng món phục vụ đang cầm
    int served;                          // Số thực khách đã phục vụ
} SharedData;

void sem_op(int sem_id, int sem_num, int op)
{
    struct sembuf sb = {sem_num, op, 0};
    semop(sem_id, &sb, 1);
}

void chef(int shmid, int semid)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL) ^ getpid());

    while (data->served < data->k)
    {
        sem_op(semid, 0, -1);
        if (data->served >= data->k)
            break;
        while (data->p + data->c + data->d < 3 * data->k)
        {
            int choice = rand() % 3;
            if (choice == 0 && data->p < data->k)
            {
                data->p++;
                break;
            }
            else if (choice == 1 && data->c < data->k)
            {
                data->c++;
                break;
            }
            else if (choice == 2 && data->d < data->k)
            {
                data->d++;
                break;
            }
        }
        printf("Đầu bếp nấu: P=%d, C=%d, D=%d\n", data->p, data->c, data->d);
        sem_op(semid, 1, 1);
        sleep(1);
    }
    sem_op(semid, 1, 1);
    shmdt(data);
}

void waiter(int shmid, int semid)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL) ^ getpid());

    while (data->served < data->k)
    {
        sem_op(semid, 1, -1);
        if (data->holding_p + data->holding_c + data->holding_d >= data->k * 2 + 1)
            break;

        while (data->p - data->served - data->holding_p > 0 || data->c - data->served - data->holding_c > 0 || data->d - data->served - data->holding_d > 0)
        {
            int choice = rand() % 3;
            if (choice == 0 && data->p - data->served - data->holding_p > 0)
            {
                data->holding_p++;
                break;
            }
            else if (choice == 1 && data->c - data->served - data->holding_c > 0)
            {
                data->holding_c++;
                break;
            }
            else if (choice == 2 && data->d - data->served - data->holding_d > 0)
            {
                data->holding_d++;
                break;
            }
        }
        printf("Phục vụ cầm: P=%d, C=%d, D=%d\n", data->holding_p, data->holding_c, data->holding_d);
        sem_op(semid, 2, 1);
        sleep(1);
    }
    sem_op(semid, 2, 1);
    shmdt(data);
}

void customer(int shmid, int semid)
{
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL) ^ getpid());

    while (1)
    {
        sem_op(semid, 2, -1);
        if (data->served >= data->k)
            break;

        if (data->holding_p > 0 && data->holding_c > 0 && data->holding_d > 0)
        {
            data->holding_p--;
            data->holding_c--;
            data->holding_d--;
            data->served++;
        }
        printf("%d/%d Khách đã nhận đủ thức ăn từ phục vụ và rời đi.\n---------------\n", data->served, data->k);
        sem_op(semid, 0, 1);
        sleep(1);
    }
    sem_op(semid, 0, 1);
    shmdt(data);
}

int main(int argc, char *argv[])
{
    time_t start, end;
    start = time(NULL);
    if (argc != 2)
    {
        fprintf(stderr, "Cách dùng: %s K\n", argv[0]);
        return 1;
    }

    int K = atoi(argv[1]);
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    int semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);
    semctl(semid, 1, SETVAL, 0);
    semctl(semid, 2, SETVAL, 0);
    // Khởi tạo bộ nhớ dùng chung
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
    srand(time(NULL));
    data->p = rand() % (K + 1);
    data->c = rand() % (K + 1);
    data->d = rand() % (K + 1);
    data->k = K;
    data->served = 0;
    data->holding_p = 0;
    data->holding_c = 0;
    data->holding_d = 0;
    shmdt(data);
    // Tạo các tiến trình đầu bếp, phục vụ và thực khách

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

    if (fork() == 0)
    {
        customer(shmid, semid);
        exit(0);
    }
    // Chờ tất cả tiến trình con kết thúc
    for (int i = 0; i < 3; i++)
        wait(NULL);

    // Giải phóng tài nguyên
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID, 0);

    end = time(NULL);
    double cooking_time = difftime(end, start);

    printf("✅ Tất cả thực khách đã được phục vụ. Thời gian phục vụ: %.2f giây.\n", cooking_time);
    return 0;
}