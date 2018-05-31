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
char*
alloc_shm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  if(newsz >= KERNBASE)
    return 0;
  if(newsz < oldsz)
    return oldsz;
  uint a;
  a = PGROUNDUP(OLDSZ); 
  mem = kalloc();
  if(mem == 0){
    cprintf("allocuvm out of memory(1)\n");
    return 0;
  }
  memset(mem, 0, PGSIZE);
  if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
    cprintf("allocuvm out of memory (2)\n");  
    return 0;
  }
  return mem;
}
int shm_open(int id, char **pointer) {
  struct proc *curproc = myproc();
  uint sz;
  int i;
  char *mem;
//you write this
  sz = PGROUNDUP(curproc->sz);
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    if(shm_table.shm_pages[i].id == id){
      if(mappages(curproc->pgdir, (char*)sz, PGSIZE, v2p(shm_table.shm_pages[i].frame), PTE_W|PTE_U) < 0)      {
        cprintf("mappages failed \n");
        return 0;
      }
      shm_table.shm_pages[i].refcnt++;
      *pointer = (char *)curproc->sz; 
      curproc->sz += PGSIZE;
      return 0;
    }
  }
  for(i = 0, i < 64; i++){
    if(shm_table.shm_pages[i].id == 0){
      shm_table.shm_pages[i].id = id;
      shm_table.shm_pages[i].refcnt ++;
      shm_table.shm_pages[i].frame = alloc_shm(curproc->pgdir, sz, sz + PGSIZE); 
      curproc->sz += PGSIZE;
      shm_table.shm_pages[i].refcnt++;
      *pointer = (char *)sz;
      break;
    }

  }
  release(&(shm_table.lock));
  if(!*pointer)
  cprintf("shm_open fail");
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
  if(shm_table.shm_pages[i].id == id){
    if(shm_table.shm_pages[i].refcnt > 0)
      shm_table.shm_pages[i].refcnt--;
    else
      shm_table.shm_pages[i].refcnt = 0;
  }

  return 0; //added to remove compiler warning -- you should decide what to return
}
