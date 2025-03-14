#include <stdio.h>     // Thư viện chuẩn cho nhập/xuất
#include <stdlib.h>    // Thư viện cho các hàm như exit(), atoi()
#include <unistd.h>    // Chứa các system call như fork(), sleep(), write()
#include <fcntl.h>     // Thư viện để thao tác file (open(), close())
#include <sys/types.h> // Chứa kiểu dữ liệu hệ thống như pid_t
#include <sys/wait.h>  // Chứa các hàm chờ tiến trình con kết thúc (wait())
#include <sys/ipc.h>   // Thư viện hỗ trợ giao tiếp liên tiến trình (IPC)
#include <sys/sem.h>   // Hỗ trợ semaphore để đồng bộ tiến trình

#define SEM_KEY 1234 // Khóa semaphore

// Hàm đọc dữ liệu từ file input_file và ghi vào file fd_out
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
    int sleep_time = atoi(argv[4]) // Thời gian chờ (chuyển từ string sang số nguyên)

        // O_WRONLY - Mở file ở chế độ ghi.
        // O_CREAT - Nếu file chưa tồn tại,tạo mới file.
        // O_TRUNC - Nếu file đã tồn tại, xóa hết dữ liệu cũ.
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
    printf("Chương trình A đã hoàn thành. Dữ liệu đã ghi vào output.txt\n");
    sleep(sleep_time);

    // Mở semaphore để thông báo progB có thể chạy
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666); // lấy semaphore dựa vào key, nếu chưa có thì tạo mới, cho phép đọc và ghi
    struct sembuf sem_op = {0, 1, 0}; // Tăng giá trị semaphore lên 1
    semop(sem_id, &sem_op, 1); //báo hiệu A hoàn thành, bắt đầu chạy B

    return 0;
}
