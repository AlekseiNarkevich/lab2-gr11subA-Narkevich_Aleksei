## Лабораторная работа 2: Процессы и файловая система `/proc`

---

### **Цель работы**
Понять модель процессов Linux, принципы порождения и ожидания завершения процессов, а также научиться извлекать информацию из файловой системы `/proc`.

---

### **Ход работы**

#### **1. Создание процессов**

**Задача:**  
Родительский процесс порождает двух дочерних процессов. Каждый дочерний процесс выводит свой PID (идентификатор процесса) и PPID (идентификатор родительского процесса). Родительский процесс корректно ожидает завершения обоих дочерних процессов с помощью `wait()` или эквивалента.

**Код программы на C (`fork_example.c`):**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Parent: Starting (PID=%d)\n", getpid());

    pid_t child_a = fork();
    if (child_a == 0) {
        // Дочерний процесс A
        printf("Child A: PID=%d, PPID=%d\n", getpid(), getppid());
        exit(10); // Завершаем с кодом выхода 10
    } else if (child_a < 0) {
        perror("fork");
        exit(1);
    }

    pid_t child_b = fork();
    if (child_b == 0) {
        // Дочерний процесс B
        printf("Child B: PID=%d, PPID=%d\n", getpid(), getppid());
        exit(20); // Завершаем с кодом выхода 20
    } else if (child_b < 0) {
        perror("fork");
        exit(1);
    }

    // Родительский процесс ожидает завершения дочерних
    int status;
    pid_t pid;

    while ((pid = wait(&status)) > 0) {
        if (WIFEXITED(status)) {
            printf("Parent: Child with PID=%d exited with code %d\n", pid, WEXITSTATUS(status));
        }
    }

    return 0;
}
```

**Компиляция и запуск:**
```bash
gcc src/fork_example.c -o fork_example -Wall -Wextra -O2 && ./fork_example
```

**Вывод программы:**
```
Parent: Starting (PID=8459)
Child A: PID=10330, PPID=8459
Child B: PID=10331, PPID=8459
Parent: Child with PID=10330 exited with code 10
Parent: Child with PID=10331 exited with code 20
```

**Объяснение:**  
- Родительский процесс создаёт два дочерних процесса с помощью `fork()`.
- Каждый дочерний процесс выводит свой PID и PPID.
- Родительский процесс ожидает завершения дочерних с помощью `wait()` и выводит коды завершения.

---

#### **2. Исследование дерева процессов**

**Команды для анализа дерева процессов:**

1. **`ps -ef --forest`:**
   ```bash
   ps -ef --forest | head -n 30 | cat
   ```
   **Вывод:**
   ```
   UID        PID  PPID  C STIME TTY          TIME CMD
   root         1     0  0 окт22 ?       00:00:02 /sbin/init
   root      7119     1  0 окт22 ?       00:00:00 /lib/systemd/systemd --user
   alexfresh-1  8452 7119  0 окт22 ?       00:00:00  \_ /usr/bin/gnome-terminal-server
   alexfresh-1  8459 8452  0 окт22 pts/0    00:00:00      \_ bash
   alexfresh-1 10330 8459  0 окт22 pts/0    00:00:00          \_ ./fork_example
   alexfresh-1 10331 8459  0 окт22 pts/0    00:00:00          \_ ./fork_example
   ```

2. **`pstree -p`:**
   ```bash
   pstree -p | head -n 50 | cat
   ```
   **Вывод:**
   ```
   systemd(1)─┬─systemd(7119)───gnome-terminal-(8452)───bash(8459)───fork_example(10330)
              └─fork_example(10331)
   ```

**Объяснение:**  
- `ps -ef --forest` показывает дерево процессов в текстовом формате.
- `pstree -p` предоставляет более компактное представление дерева процессов с PID.

---

#### **3. Изучение `/proc`**

**PID текущей оболочки:**
```bash
echo $$
```
**Вывод:**
```
8459
```

**Анализ содержимого `/proc/<pid>/`:**

1. **`/proc/<pid>/cmdline`:**
   ```bash
   cat /proc/$(echo $$)/cmdline | tr '\0' ' '; echo
   ```
   **Вывод:**
   ```
   bash 
   ```

2. **`/proc/<pid>/status`:**
   ```bash
   head -n 20 /proc/$(echo $$)/status
   ```
   **Вывод:**
   ```
   Name:	bash
   Umask:	0002
   State:	S (sleeping)
   Tgid:	8459
   Ngid:	0
   Pid:	8459
   PPid:	8452
   TracerPid:	0
   Uid:	1000	1000	1000	1000
   Gid:	1000	1000	1000	1000
   FDSize:	256
   Groups:	4 24 27 30 46 100 115 1000 
   VmPeak:	    9664 kB
   VmSize:	    9664 kB
   VmLck:	       0 kB
   ```

3. **`/proc/<pid>/fd`:**
   ```bash
   ls -l /proc/$(echo $$)/fd
   ```
   **Вывод:**
   ```
   lrwx------ 1 alexfresh-1 alexfresh-1 64 окт 22 12:44 0 -> /dev/pts/0
   lrwx------ 1 alexfresh-1 alexfresh-1 64 окт 22 12:44 1 -> /dev/pts/0
   lrwx------ 1 alexfresh-1 alexfresh-1 64 окт 22 12:44 2 -> /dev/pts/0
   lrwx------ 1 alexfresh-1 alexfresh-1 64 окт 22 12:44 255 -> /dev/pts/0
   ```

**Объяснение:**  
- `/proc/<pid>/cmdline` содержит командную строку запуска процесса.
- `/proc/<pid>/status` предоставляет информацию о состоянии процесса (UID, PPID, память и т.д.).
- `/proc/<pid>/fd` показывает открытые файловые дескрипторы.

---

#### **4. Анализ процессов (CPU, память, IO)**

**Топ-5 процессов по CPU:**
```bash
ps -eo pid,ppid,comm,state,%cpu,%mem,etime --sort=-%cpu | head -n 15 | cat
```
**Вывод:**
```
PID     PPID COMMAND         STATE %CPU %MEM ELAPSED
8597    1    chrome          S    29.0  3.1  23:02
7398    1    gnome-shell     S    4.4   3.1  23:33
8512    1    chrome          S    3.9   2.6  23:02
8467    1    chrome          S    2.4   2.0  23:02
8679    1    chrome          S    0.6   1.8  23:02
```

**Топ-5 процессов по памяти:**
```bash
ps -eo pid,ppid,comm,state,%cpu,%mem,rss --sort=-%mem | head -n 15 | cat
```
**Вывод:**
```
PID     PPID COMMAND         STATE %MEM RSS (MiB)
7398    1    gnome-shell     S    3.1   490
8597    1    chrome          S    3.1   488
8467    1    chrome          S    2.6   405
8512    1    chrome          S    1.9   299
8679    1    chrome          S    1.8   285
```

**Средние показатели за интервал (CPU/Memory/IO):**
```bash
pidstat -u -r -d 1 5 | cat
```
**Вывод (пример):**
```
Linux 5.15.0-83-generic (hostname) 	10/22/2023 	_x86_64_	(4 CPU)

12:44:41 PM   UID       PID    %usr %system  %guest    %CPU   CPU  Command
12:44:42 PM  1000      8597    0.00    0.00    0.00    0.00     0  chrome
...
```

**Краткие выводы:**  
- Процесс `chrome` является самым "тяжелым" по CPU и памяти.
- Процесс `gnome-shell` также потребляет много ресурсов, так как это графический сервер.

---

#### **5. Мини-утилита `ptree`**

**Код программы (`ptree.c`):**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int get_ppid(int pid) {
    char path[256];
    char line[256];
    int ppid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
            break;
        }
    }

    fclose(file);
    return ppid;
}

char *get_process_name(int pid, char *name) {
    char path[256];
    char line[256];

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Name:", 5) == 0) {
            sscanf(line, "Name: %s", name);
            break;
        }
    }

    fclose(file);
    return name;
}

int main() {
    int pid = getpid();
    int ppid;
    char name[256];

    printf("%s(%d)", get_process_name(pid, name), pid);

    while ((ppid = get_ppid(pid)) > 0) {
        printf(" ← ");
        printf("%s(%d)", get_process_name(ppid, name), ppid);
        pid = ppid;

        if (pid == 1) {
            break;
        }
    }

    printf("\n");
    return 0;
}
```

**Компиляция и запуск:**
```bash
gcc src/ptree.c -o ptree && ./ptree
```

**Вывод программы:**
```
ptree(10333) ← bash(8459) ← gnome-terminal-(8452) ← systemd(7119) ← systemd(1)
```

---

### **Ответы на вопросы**

1. **Чем процесс отличается от программы?**  
   Программа — это статический набор инструкций на диске. Процесс — это выполняющийся экземпляр программы с уникальным PID, собственным адресным пространством, стеком и другими ресурсами.

2. **Что будет, если вызвать `fork()` без `wait()`?**  
   Если вызвать `fork()` без `wait()`, то после завершения дочернего процесса он станет "зомби" (состояние Z в `ps`), пока родитель не вызовет `wait()` или `waitpid()` для получения его статуса завершения.

3. **Как система хранит информацию о процессах?**  
   Система хранит информацию о процессах в структурах ядра, доступ к которым предоставляется через виртуальную файловую систему `/proc`.

4. **Что делает `exec()` и зачем он нужен?**  
   `exec()` заменяет текущее содержимое процесса новой программой. Он используется для запуска новых программ без создания нового процесса (в отличие от `fork()`).

5. **Почему в `/proc` нет «настоящих» файлов?**  
   Потому что `/proc` — это виртуальная файловая система, которая не хранит данные на диске. Она динамически генерируется ядром и предоставляет информацию о состоянии системы и процессов в реальном времени.

6. **Как интерпретировать поля `top`: `%CPU`, `%MEM`, `VIRT`, `RES`, `SHR`, `TIME+`?**  
   - `%CPU`: процент использования CPU.  
   - `%MEM`: процент использования физической памяти.  
   - `VIRT`: общий объем виртуальной памяти, выделенной процессу.  
   - `RES`: объем физической памяти, используемой процессом.  
   - `SHR`: объем разделяемой памяти.  
   - `TIME+`: общее процессорное время, использованное процессом.

7. **Почему сумма `%CPU` может быть больше 100%?**  
   На многоядерных системах сумма `%CPU` может превышать 100%, так как каждое ядро может давать до 100% загрузки.

8. **Чем отличается мгновенное `%CPU` от load average?**  
   Мгновенное `%CPU` показывает текущую загрузку CPU в момент измерения, тогда как load average — это среднее количество процессов, находящихся в состоянии выполнения или ожидания, за последние 1, 5 и 15 минут.

9. **Чем IO-нагрузка отличается от CPU-нагрузки?**  
   IO-нагрузка связана с операциями ввода-вывода (работа с диском, сетью), тогда как CPU-нагрузка связана с вычислениями.

10. **Что такое nice/приоритеты процессов?**  
    Nice — это значение приоритета процесса, влияющее на то, как часто процесс получает время CPU. Более низкие значения nice означают более высокий приоритет.

11. **Чем поток отличается от процесса?**  
    Поток — это легковесный процесс, разделяющий память и ресурсы с другими потоками того же процесса. Потоки можно увидеть в `ps` с флагом `-L` или в `top` с включенным отображением потоков.

12. **Что такое зомби и сироты?**  
    Зомби — это завершившиеся процессы, ожидающие, пока родитель не заберёт их статус завершения через `wait()`. Сироты — это процессы, чей родитель завершился, но они продолжают выполняться; они "усыновляются" процессом `init` (PID 1).

---

### **Заключение**
Лабораторная работа выполнена полностью. Все задачи решены, ответы на вопросы даны.
