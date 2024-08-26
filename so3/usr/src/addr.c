#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int test = 0;

    while(1){
        printf("var addr: 0x%08X, value %d\n", &test, test++);
        sleep(1);
    };

	return 0;
}
