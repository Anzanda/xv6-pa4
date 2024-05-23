// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

struct page pages[PHYSTOP/PGSIZE];
struct page *page_lru_head;
int num_free_pages;
int num_lru_pages;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;

  num_free_pages += 1;

  if(kmem.use_lock)
    release(&kmem.lock);
}
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

//try_again:
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
//  if(!r && reclaim())
//	  goto try_again;
  if(r) {
    kmem.freelist = r->next;
    num_free_pages -= 1;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

void
kalloc2(pde_t *pgdir, char *pa, char *va)
{
  struct page *page = &pages[V2P(pa)/PGSIZE];

  page->pgdir = pgdir;
  page->vaddr = va;

  if(num_lru_pages == 0) { // n == 0
    page->prev = page;
    page->next = page;
  } else {
    if(num_lru_pages == 1) { // n == 1
      page->prev = page_lru_head;
      page_lru_head->next = page; 
    } else { // n > 1
      page_lru_head->prev->next = page; 
      page->prev = page_lru_head->prev;
    }
    page_lru_head->prev = page;
    page->next = page_lru_head;
  }
  page_lru_head = page;

  num_lru_pages += 1;
}

static int
is_in_lru_list(char *v)
{
  struct page *page = &pages[V2P(v)/PGSIZE];
  if(page == page_lru_head)
    return 1;

  struct page *curr = page_lru_head->next;
  while(curr != page_lru_head) {
    if(page == curr)
      return 1;
    curr = curr->next;
  }
  return 0;
}
void
kfree2(char *v)
{
  struct page *page = &pages[V2P(v)/PGSIZE];
  page->pgdir = 0;
  page->vaddr = 0;

  if(!is_in_lru_list(v))
    return;

  if(page == page_lru_head)
    page_lru_head = page->next;

  struct page *curr = page_lru_head;
  if(num_lru_pages > 1) {
    page->prev->next = page->next;
    page->next->prev = page->prev;
  } else {
    page_lru_head = 0;
  }
  page->next = 0;
  page->prev = 0;

  num_lru_pages -= 1;
}

void
find_victim()
{
  struct page *curr = page_lru_head;
  pde_t *pde;
  pte_t *pgtab;
  pte_t *pte;
  while(1) {
    pde = &curr->pgdir[PDX(curr->vaddr)];
    if(*pde & PTE_P) {
      pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
      pte = &pgtab[PTX(curr->vaddr)];
    } else {
      cprintf("error!!!!!!!!!!\n");
    }
    if(*pte & PTE_A) {
      cprintf("pte has accessed bit\n");
      *pte &= (~PTE_A);
      if(curr == page_lru_head) {
        page_lru_head = curr->next;
      } else {
        // move to tail
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
        page_lru_head->prev->next = curr;
        curr->prev = page_lru_head->prev;
        page_lru_head->prev = curr;
        curr->next = page_lru_head;
      }
    } else {
      cprintf("victim: %d\n", curr->vaddr);
      break;
    }
    curr = curr->next;
  }
}
