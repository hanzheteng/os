#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

char *
shm_alloc(pde_t *pgdir, uint sz)
{
  char *mem;
  uint a;
  a = PGROUNDUP(sz);
  mem = kalloc(); //would not return 0 since max # of shm is 64 
  if(mem == 0){
    cprintf("allocuvm out of memory\n");
    return 0;
  }
  memset(mem, 0, PGSIZE);
  if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
    cprintf("allocuvm out of memory (2)\n");
    //kfree(mem);
    return 0;
  }
  return mem;
}


int shm_open(int id, char **pointer) {
  int i;
  int exist = 0;
  struct proc *curproc = myproc();
  acquire(&(shm_table.lock));

  for(i=0; i<64; i++){
    if(shm_table.shm_pages[i].id == id){
      if(mappages(curproc->pgdir, (char*)curproc->sz, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U) < 0){ 
        cprintf("allocuvm out of memory (2)\n");
        //kfree(mem);
        return 0;
      }     
      *pointer = (char*)curproc->sz;
      shm_table.shm_pages[i].refcnt += 1;
      curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE; 
      exist = 1;
      break;
    }
  }
 if(!exist){
    for(i=0; i<64; i++){
      if(shm_table.shm_pages[i].id != 0)
        continue;
      shm_table.shm_pages[i].id = id; // should assign the given id
      shm_table.shm_pages[i].frame = shm_alloc(curproc->pgdir, curproc->sz);
      shm_table.shm_pages[i].refcnt = 1;
      *pointer = (char*)curproc->sz;
      curproc->sz = PGROUNDUP(curproc->sz) + PGSIZE;
      break;
    }
    if(shm_table.shm_pages[63].id != 0){
      cprintf("shared memory is full\n");
      return -1;
    }
  }
  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  int i;
  acquire(&(shm_table.lock));
  for(i=0; i<64; i++){
    if(shm_table.shm_pages[i].id == id){
      if((shm_table.shm_pages[i].refcnt -= 1) == 0){
        shm_table.shm_pages[i].id = 0;
        shm_table.shm_pages[i].frame = 0;
      }
    }
  }
  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}

