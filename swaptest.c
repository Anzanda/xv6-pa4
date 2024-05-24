#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "mmu.h"


int main () {
    int nr, nw;
    test();
    swapstat(&nr, &nw);
    printf(1, "nr: %d\nnw: %d\n", nr, nw);
    sbrk(PGSIZE*56100);
    swapstat(&nr, &nw);
    printf(1, "nr: %d\nnw: %d\n", nr, nw);
    test();
    exit();
}
