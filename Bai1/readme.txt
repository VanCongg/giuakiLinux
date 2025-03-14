Hướng dẫn sử dụng:
------------------
1. Build chương trình:
   $ make
2. Chạy chương trình A:
   $ ./progA file1.txt file2.txt output.txt 5

3. Chạy chương trình B (sau khi A chạy xong):
   $ ./progB output.txt result.txt 3
4. Chạy cả hai
    make && ./progA file1.txt file2.txt output.txt 5 && ./progB output.txt result.txt 3
5. Xóa file biên dịch:
   $ make clean
