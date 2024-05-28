#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

#define PGSIZE 4096
#define NUM_PAGES 10000

void
touch_pages(char *base, int num_pages)
{
  int i;
  for (i = 0; i < num_pages; i++) {
    base[i * PGSIZE] = i;
  }
}

void
forkfn() 
{
  char *base;
  int i;
  int a, b; //a swapread, b swapwrite

  base = sbrk(NUM_PAGES * PGSIZE);
  if (base == (char*)-1) {
    printf(1, "sbrk failed\n");
    exit();
  }

  touch_pages(base, NUM_PAGES);

  for (i = 0; i < NUM_PAGES; i++) {
    base[i * PGSIZE] = i;
  }
  swapstat(&a, &b);
  printf(1, "swapstat: %d %d\n", a, b);
}

int
main(void)
{
  int i, pid;
  for(i=0; i<3; i++) {
    pid = fork();
    if(pid==0){
        forkfn();
    }
    else{
        wait();
        printf(1, "swaptest completed\n");
    }
  }
  exit();
}
// #include "param.h"
// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "fs.h"
// #include "fcntl.h"
// #include "syscall.h"
// #include "traps.h"
// #include "memlayout.h"
// #include "mmu.h"


// // 목적: load testing. 10만개 페이지 할당했을 때 어떤 일이 발생하는가?
// void test1()
// {
//     int nr, nw;
//     test();
//     swapstat(&nr, &nw);
//     printf(1, "nr: %d\nnw: %d\n", nr, nw);
//     sbrk(PGSIZE*100000);
//     swapstat(&nr, &nw);
//     printf(1, "nr: %d\nnw: %d\n", nr, nw);
//     test();
// }
// /**
//  * 목적: swap-in이 제대로 이루어지는가?
//  * 60,000개 page만큼 malloc조진다.
//  * 값을 쓸 때 swap-in과 swap-out이 동시에 일어나야 된다.
//  * 그러고는 다시 읽을 때도 swap-in과 swap-out이 동시에 일어나야 한다.
//  * 오래걸림 주의!!!!!!
// */
// void test2()
// {
//     int nr, nw;
//     int *a;
//     swapstat(&nr, &nw);
//     printf(1, "nr: %d\nnw: %d\n", nr, nw);
//     test();

//     int sz = sizeof(int) * PGSIZE * 60000 / 4;
//     a = malloc(sz);
//     swapstat(&nr, &nw);
//     printf(1, "after malloc\nnr: %d\nnw: %d\n", nr, nw);
//     test();
//     for(int i=0; i<sz/4; i++) {
//         a[i] = i;
//     }
//     swapstat(&nr, &nw);
//     printf(1, "after asigned\nnr: %d\nnw: %d\n", nr, nw);
//     test();

//     for(int i=0; i<sz/4; i++) {
//         printf(4, "a[%d]: %d\n", i, a[i]);
//     }
//     swapstat(&nr, &nw);
//     printf(1, "nr: %d\nnw: %d\n", nr, nw);
//     test();
// }
// /**
//  * 목적: fork하고 지웠을 때 할당된 페이지가 잘 지워지는가? lru_pages확인.
// */
// void test3()
// {
//     int nr, nw;
//     // int *a;
//     swapstat(&nr, &nw);
//     printf(1, "nr: %d\nnw: %d\n", nr, nw);
//     test();
//     if(fork() == 0) {
//         sbrk(PGSIZE*10000);
//         swapstat(&nr, &nw);
//         printf(1, "in child\nnr: %d\nnw: %d\n", nr, nw);
//         test();
//         exit();
//     } else {
//         wait();
//     }
//     swapstat(&nr, &nw);
//     printf(1, "in parent\nnr: %d\nnw: %d\n", nr, nw);
//     test();
// }
// // void
// // test4()
// // {
// //     int nr, nw;
// //     int *x;
// //     int *a;
// //     swapstat(&nr, &nw);
// //     printf(1, "nr: %d\nnw: %d\n", nr, nw);
// //     test();
    
// //     int sz = sizeof(int) * PGSIZE * 56002 / 4;
// //     a = malloc(sz);
// //     for(int i=0; i<sz/4; i++) {
// //         a[i] = i;
// //     }

// //     swapstat(&nr, &nw);
// //     printf(1, "nr: %d\nnw: %d\n", nr, nw);
// //     test();

    

// //     swapstat(&nr, &nw);
// //     printf(1, "nr: %d\nnw: %d\n", nr, nw);
// //     test();
// // }
// int main () {
//     test2();
//     // test4();
//     exit();
// }
