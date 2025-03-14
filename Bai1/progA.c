#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEM_KEY 1234 // Khóa semaphore

void process_file(const char *input_file, int fd_out)
{
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in < 0)
    {
        perror("Lỗi mở file input");
        exit(1);
    }

    char buffer[1024];
    ssize_t bytes;
    while ((bytes = read(fd_in, buffer, sizeof(buffer))) > 0)
    {
        write(fd_out, buffer, bytes);
    }

    close(fd_in);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Cách dùng: %s F1 F2 F3 time\n", argv[0]);
        return 1;
    }

    char *file1 = argv[1];
    char *file2 = argv[2];
    char *file3 = argv[3];
    int sleep_time = atoi(argv[4]);

    int fd_out = open(file3, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0)
    {
        perror("Lỗi mở file F3");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        process_file(file1, fd_out);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        process_file(file2, fd_out);
        exit(0);
    }

    close(fd_out);

    wait(NULL);
    wait(NULL);

    sleep(sleep_time);

    // Mở semaphore để thông báo progB có thể chạy
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    struct sembuf sem_op = {0, 1, 0};
    semop(sem_id, &sem_op, 1);

    return 0;
}
