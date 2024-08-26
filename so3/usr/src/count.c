#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define ACTIVE_WAITING 0

int main(int argc, char **argv) {
    int cnt = 0;
    pid_t pid;
#if ACTIVE_WAITING
    uint32_t i;
#endif

    pid = getpid();

	while (1) {
        printf("[%d]: %d\n", pid, cnt++);

#if ACTIVE_WAITING
        for (i = 0; i < 80000000; ++i) {
            i++;
            i--;
        }
#else
        sleep(1);
#endif
    }

    return 0;
}
