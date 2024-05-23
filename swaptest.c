#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


int main () {
	// int a, b;

    int* a = malloc(4);
    printf(1, "addr: 0x%x\n", a);
    a[0] = 574;
    test();
    printf(1, "%d\n", a[0]);
    exit();
}
