#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Добавлено для getpid()

// Функция для получения PPID из /proc/<pid>/status
int get_ppid(int pid) {
    char path[256];
    char line[256];
    int ppid = -1;

    // Формируем путь к файлу status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    // Открываем файл
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    // Читаем файл построчно
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
            break;
        }
    }

    fclose(file);
    return ppid;
}

// Функция для получения имени процесса из /proc/<pid>/status
char *get_process_name(int pid, char *name) { // Убран параметр size
    char path[256];
    char line[256];

    // Формируем путь к файлу status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    // Открываем файл
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    // Читаем файл построчно
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Name:", 5) == 0) {
            // Извлекаем имя процесса
            sscanf(line, "Name: %s", name);
            break;
        }
    }

    fclose(file);
    return name;
}

int main() {
    int pid = getpid(); // Текущий PID
    int ppid;
    char name[256];

    printf("%s(%d)", get_process_name(pid, name), pid);

    // Поднимаемся по дереву процессов
    while ((ppid = get_ppid(pid)) > 0) {
        printf(" ← ");
        printf("%s(%d)", get_process_name(ppid, name), ppid);
        pid = ppid;

        // Останавливаемся, если достигли PID 1
        if (pid == 1) {
            break;
        }
    }

    printf("\n");
    return 0;
} 
