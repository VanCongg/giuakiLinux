#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void read_and_write(const char *input_file, int pipe_fd)
{
    int fd = open(input_file, O_RDONLY);
    if (fd == -1)
    {
        perror("Open input file failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        write(pipe_fd, buffer, bytes_read);
    }

    close(fd);
    close(pipe_fd);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s F1 F2 F3 time\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *F1 = argv[1];
    char *F2 = argv[2];
    char *F3 = argv[3];
    int time_sleep = atoi(argv[4]);

    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t p1 = fork();
    if (p1 == 0)
    {
        close(pipe_fd[0]);
        read_and_write(F1, pipe_fd[1]);
        exit(EXIT_SUCCESS);
    }

    pid_t p2 = fork();
    if (p2 == 0)
    {
        close(pipe_fd[0]);
        read_and_write(F2, pipe_fd[1]);
        exit(EXIT_SUCCESS);
    }

    close(pipe_fd[1]);
    wait(NULL);
    wait(NULL);

    int output_fd = open(F3, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (output_fd == -1)
    {
        perror("Open output file failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
    {
        write(output_fd, buffer, bytes_read);
    }

    close(output_fd);
    close(pipe_fd[0]);

    sleep(time_sleep);
    return 0;
}
