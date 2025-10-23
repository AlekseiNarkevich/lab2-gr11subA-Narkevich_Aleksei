#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Parent: Starting (PID=%d)\n", getpid());
    fflush(stdout); // Сброс буфера

    pid_t child1 = fork(); // Первый fork()

    if (child1 == 0) {
        // Код первого дочернего процесса
        printf("Child A: PID=%d, PPID=%d\n", getpid(), getppid());
        fflush(stdout);
        sleep(2); // Имитация работы
        return 10; // Код завершения
    } else {
        pid_t child2 = fork(); // Второй fork()

        if (child2 == 0) {
            // Код второго дочернего процесса
            printf("Child B: PID=%d, PPID=%d\n", getpid(), getppid());
            fflush(stdout);
            sleep(3); // Имитация работы
            return 20; // Код завершения
        } else {
            // Код родительского процесса
            int status1, status2;
            pid_t pid1 = wait(&status1); // Ожидание первого дочернего процесса
            pid_t pid2 = wait(&status2); // Ожидание второго дочернего процесса

            printf("Parent: Child with PID=%d exited with code %d\n", pid1, WEXITSTATUS(status1));
            printf("Parent: Child with PID=%d exited with code %d\n", pid2, WEXITSTATUS(status2));
            fflush(stdout);
        }
    }

    return 0;
}
