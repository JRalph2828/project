#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

int
getppid(void)
{
    return proc->parent->pid;
}

int
sys_getppid(void)
{
    return getppid();
}


