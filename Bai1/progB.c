#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#define SEM_KEY 1234

void reverse_words(const char *input_file, int fd_out)
{
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in < 0)
    {
        perror("Lỗi mở file F3");
        exit(1);
    }

    char buffer[1024], *words[100];
    ssize_t bytes = read(fd_in, buffer, sizeof(buffer) - 1);
    close(fd_in);

    buffer[bytes] = '\0';
    int count = 0;
    char *token = strtok(buffer, " \n");
    while (token)
    {
        words[count++] = token;
        token = strtok(NULL, " \n");
    }

    for (int i = count - 1; i >= 0; i--)
    {
        write(fd_out, words[i], strlen(words[i]));
        write(fd_out, " ", 1);
    }
    write(fd_out, "\n", 1);
}

void first_last_words(const char *input_file, int fd_out)
{
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in < 0)
    {
        perror("Lỗi mở file F3");
        exit(1);
    }

    char buffer[1024], *words[100];
    ssize_t bytes = read(fd_in, buffer, sizeof(buffer) - 1);
    close(fd_in);

    buffer[bytes] = '\0';
    int count = 0;
    char *token = strtok(buffer, " \n");
    while (token)
    {
        words[count++] = token;
        token = strtok(NULL, " \n");
    }

    if (count > 0)
    {
        write(fd_out, words[0], strlen(words[0]));
        write(fd_out, " ", 1);
        if (count > 1)
        {
            write(fd_out, words[count - 1], strlen(words[count - 1]));
        }
        write(fd_out, "\n", 1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Cách dùng: %s F3 F4 time\n", argv[0]);
        return 1;
    }

    char *file3 = argv[1];
    char *file4 = argv[2];
    int sleep_time = atoi(argv[3]);

    // Đợi semaphore từ progA
    int sem_id = semget(SEM_KEY, 1, 0666);
    struct sembuf sem_op = {0, -1, 0};
    semop(sem_id, &sem_op, 1);

    int fd_out = open(file4, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0)
    {
        perror("Lỗi mở file F4");
        return 1;
    }

    pid_t pid3 = fork();
    if (pid3 == 0)
    {
        reverse_words(file3, fd_out);
        exit(0);
    }

    pid_t pid4 = fork();
    if (pid4 == 0)
    {
        first_last_words(file3, fd_out);
        exit(0);
    }

    close(fd_out);
    wait(NULL);
    wait(NULL);
    printf("Chương trình B đã hoàn thành. Kết quả đã ghi vào result.txt\n");
    sleep(sleep_time);

    return 0;
}
