#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void reverse_words(char *str)
{
    char *words[100];
    int count = 0;
    char *token = strtok(str, " \n");
    while (token)
    {
        words[count++] = token;
        token = strtok(NULL, " \n");
    }

    for (int i = count - 1; i >= 0; i--)
    {
        printf("%s ", words[i]);
    }
    printf("\n");
}

void process_p3(const char *F3, const char *F4)
{
    int fd = open(F3, O_RDONLY);
    if (fd == -1)
    {
        perror("Open input file failed");
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
    {
        perror("Read error");
        close(fd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0';

    close(fd);

    int fd_out = open(F4, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_out == -1)
    {
        perror("Open output file failed");
        exit(EXIT_FAILURE);
    }

    reverse_words(buffer);
    write(fd_out, buffer, strlen(buffer));
    write(fd_out, "\n", 1);
    close(fd_out);
}

void process_p4(const char *F3, const char *F4)
{
    int fd = open(F3, O_RDONLY);
    if (fd == -1)
    {
        perror("Open input file failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
    {
        perror("Read error");
        close(fd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0';

    char *first = strtok(buffer, " \n");
    char *last = first;
    char *token;
    while ((token = strtok(NULL, " \n")) != NULL)
    {
        last = token;
    }

    close(fd);

    int fd_out = open(F4, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_out == -1)
    {
        perror("Open output file failed");
        exit(EXIT_FAILURE);
    }

    dprintf(fd_out, "%s %s\n", first, last);
    close(fd_out);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s F3 F4 time\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *F3 = argv[1];
    char *F4 = argv[2];
    int time_sleep = atoi(argv[3]);

    pid_t p3 = fork();
    if (p3 == 0)
    {
        process_p3(F3, F4);
        exit(EXIT_SUCCESS);
    }

    pid_t p4 = fork();
    if (p4 == 0)
    {
        process_p4(F3, F4);
        exit(EXIT_SUCCESS);
    }

    wait(NULL);
    wait(NULL);

    sleep(time_sleep);
    return 0;
}
