#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;
  
  if(proc->TFlag ==1)
      proc->sz = proc->parent->sz;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// syscall_yield
int
sys_yield(void)
{
    yield();
    return 0;
}

//syscall_getlev
int
sys_getlev(void)
{
    return proc->Priority;
}

//syscall_set_cpu_share
int 
sys_set_cpu_share(void)
{
   int cpu_share;
   if (argint(0, &cpu_share) < 0)
       return -1;
   return set_cpu_share(cpu_share);
}

// syscall_thread_create
int
sys_thread_create(void)
{
    thread_t *thread;
    void *(*fn)(void*);
    void *arg;
    int i;
    if (argint(0, &i) < 0)
        return -1;

    thread = (thread_t*)i;

    if (argint(1, &i) < 0)
        return -1;

    fn = (void*)i;

    if (argint(2, &i) < 0)
        return -1;

    arg = (void*)i;
    

    return thread_create(thread, fn, arg);
}

// syscall_thread_exit
int
sys_thread_exit(void)
{
    void *retval;
    int i;

    if(argint(0, &i) < 0)
        return-1;
    retval = (void*)i;

    thread_exit(retval);
    return 0;
}

// syscall_thread_join
int
sys_thread_join(void)
{
    thread_t thread;
    void **retval;
    int i;

    if(argint(0,&i) < 0)
        return -1;
    thread = i;
    
    if (argint(1, &i) < 0)
        return -1;
    retval = (void**)i;
    return thread_join(thread, retval);
}
