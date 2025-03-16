#include <stdio.h>     
#include <stdlib.h>    
#include <unistd.h> 
#include <fcntl.h>    
#include <sys/types.h> 
#include <sys/wait.h>  
#include <sys/ipc.h>   
#include <sys/sem.h>  

#define SEM_KEY 1234 

void process_file(const char *input_file, int fd_out)
{
    // Mở file input_file chỉ đọc
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in < 0)
    {
        perror("Lỗi mở file input");
        exit(1);
    }
    //Đọc và ghi 1024 bytes mỗi lần
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
    // Kiểm tra số lượng tham số đầu vào
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
    waitpid(pid1, NULL, 0);
    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        process_file(file2, fd_out);
        exit(0);
    }
    close(fd_out);

    waitpid(pid2, NULL, 0);

    printf("Chương trình A đã hoàn thành. Dữ liệu đã ghi vào F3.\n");
    sleep(sleep_time);

    // Tạo và khởi tạo semaphore
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        perror("Lỗi tạo semaphore");
        exit(1);
    }
    semctl(sem_id, 0, SETVAL, 0);

    struct sembuf sem_op = {0, 1, 0};
    semop(sem_id, &sem_op, 1);

    return 0;
}
