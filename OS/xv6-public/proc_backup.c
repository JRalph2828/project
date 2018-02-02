#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
int TimePeriod[3] = {5, 10, 20};    // timeslice cor each level
double MAX_STRIDE = 1000;              // value for decide stride
double Cpu_Total = 0;                  // total cpu_share
double MLFQ_pass = 0;
double STRIDE_pass = 0;

struct spinlock set_cpu_share_lock; // lock for set_cpu_share
struct spinlock LOCK;

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);
extern void trapret(void);
static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  initlock(&set_cpu_share_lock, "cpu_share");
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.

static struct proc*
allocproc(void)
{
  struct proc *p;
  struct proc *temp;
  char *sp;
  int i;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;
found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->Stride = 0;        // initialize proc comes
  p->Pass = 0;
  p->Origin_cpu = 0;
  p->Flag = 0;
  p->TFlag = 0;
  p->tid = -1;
  p->CFlag = 0;
  for(i = 0; i < 63; i++)
      p->tspace[i] = 0;
  for(i = 0; i < 63; i++)
      p->tstackaddr[i] = 0;
       
  // prevent starvation if there is new proc in STRIDE
  for(temp = ptable.proc; temp < &ptable.proc[NPROC]; temp++){
      if(temp->state != RUNNABLE);
        continue;
      temp->Pass = 0;
  }
  release(&ptable.lock);
  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;
  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  return p;
}
//PAGEBREAK: 32
// Set up first user process.

void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S
  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  release(&ptable.lock);
}
// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;

  sz = proc->sz;

  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  if(proc->TFlag == 1)
      proc->parent->sz = sz;
  else
      proc->sz = sz;
  switchuvm(proc);
  return 0;
}
// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if(proc->TFlag == 0){
      if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
      }
  }
  // If thread calls fork() then copy rest empty space too
  else{
      if((np->pgdir = copyuvm_thread(proc->pgdir, proc->sz)) == 0){
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
      }
  }


  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;
  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  pid = np->pid;
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  return pid;
}
// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;
  if(proc == initproc)
    panic("init exiting");
  // If thread calls exit() then close master thread openfiles
  if(proc->TFlag == 1){
      for(fd = 0; fd < NOFILE; fd++){
        if(proc->parent->ofile[fd]){
          fileclose(proc->parent->ofile[fd]);
          proc->parent->ofile[fd] = 0;
        }
      }
      begin_op();
      iput(proc->parent->cwd);
      end_op();
      proc->parent->cwd = 0;
  }
  // Close current process openfiles
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }
  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  // Close rest threads openfiles
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->pid == proc->pid && p->TFlag == 1 && p->tid != proc->tid){
          for(fd = 0; fd < NOFILE; fd++){
            if(p->ofile[fd]){
              fileclose(p->ofile[fd]);
              p->ofile[fd] = 0;
            }
          }
          begin_op();
          iput(p->cwd);
          end_op();
          p->cwd = 0;
      }
  }

  acquire(&ptable.lock);
  // Parent might be sleeping in wait().
  // If thread calls exit wakeup master thread's parent
  if(proc->TFlag == 1)
      wakeup1(proc->parent->parent);
  // Else wake up parent
  else 
      wakeup1(proc->parent);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    // If thread calls exit() then makes other threads as orphan
    if(proc->TFlag == 1)
        if(p->TFlag == 1 && p->pid == proc->pid){
            p->Orphan = 1;
            p->state = ZOMBIE;
        }

    if(p->parent == proc){
        // If there are orphan threads
        if(p->TFlag == 1){
            p->Orphan = 1;
            p->state = ZOMBIE;
        }

        // Pass abandoned children to init.
        else{
            p->parent = initproc;
            if(p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }
  }
  // If thread calls exit() make parent as orphan
  if(proc->TFlag == 1){
      proc->parent->state = ZOMBIE;
      proc->parent->Orphan = 1;
  }
  // Jump into the scheduler, never to return.
  // decrease total cpu_share and initialze origin_cpu when proc end
  Cpu_Total -= proc->CPU;
  proc->Origin_cpu = 0;
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}
// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  acquire(&ptable.lock);
  for(;;){
    havekids = 0;
    // Orphan threads go here
    for(p = ptable.proc; p<&ptable.proc[NPROC]; p++){
        if(p->Orphan == 1 && p->TFlag == 1 && p->state == ZOMBIE){
            deallocuvm(p->pgdir, p->tstack +2*PGSIZE, p->tstack);
            kfree(p->kstack);
            p->kstack = 0;
            p->Orphan = 0;
            p->name[0] = 0;
            p->killed = 0;
            p->TFlag = 0;
            p->parent = 0;
            p->state = UNUSED;
        }
     }
    // Scan through table looking for exited children.
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      // Only process go here, not for thread
      if(p->state == ZOMBIE && p->TFlag ==0){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->TFlag = 0;
        p->state = UNUSED;
       
        release(&ptable.lock);
        return pid;
      }
    }
    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
// scheduler for MLFQ
void
MLFQ_scheduler(void)
{
  struct proc *p;   
  struct proc *temp;
  struct proc *pp;      
  // proc pointer for execute same proc during time quantum
  double MLFQ_stride = MAX_STRIDE/(100-(Cpu_Total));
  // define MLFQ stride
  MLFQ_pass += MLFQ_stride;
  int HIGH_P = 5;     // variable for search highest priority
  temp = 0;
  pp = 0;
  // priority boosting
  // if boosting occur then initialize priority, ticket and age
  // Age will be increased in trap.c when time interrupt occured
  if(Age == 100){
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
          p->Priority = 0;
          p->Ticket = 0;
      }
      Age = 0;
//      cprintf("[do boosting!]\n");  
  }
  // search the highest priority proc
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE || pp == p || p->Flag != 0)
          continue;
 
      if(p->Priority < HIGH_P){
          HIGH_P = p->Priority;
          temp = p;
      }
  }  
  p = temp;
  // switch to chosen process
  if(p && p->state == RUNNABLE){
      proc = p;
      pp = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, p->context);
      switchkvm();
      proc = 0;
      // if proc used time quantum priority will be decrease
      // and go to next level
      // priority 0 is highest priority
      if(p->Priority < 2 && p->Ticket == TimePeriod[p->Priority]){
          p->Priority++;
          p->Ticket = 0;
      }
  }
}
// scheduler for STRIDE
void
STRIDE_scheduler(void)
{
  struct proc *p;
  struct proc *temp;
  // for prevent divided by 0, +1 Cpu_Total
  double STRIDE_stride = MAX_STRIDE/(Cpu_Total+1);
  STRIDE_pass += STRIDE_stride;
    
    int MAX_P = 10000000;       // variable for search lowest pass proc
    temp = 0;
    // search the lowest pass proc
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE || p->Flag != 1)
           continue;
        if(p->Pass < MAX_P){
            MAX_P = p->Pass;
            temp = p;
        }
    }
    
    // switch to chosen process
    p = temp;
    if(p && p->state == RUNNABLE){
        p->Pass += p->Stride;
        proc = p;
        switchuvm(p);
        p->state = RUNNING;
        swtch(&cpu->scheduler, p->context);
        switchkvm();
        proc = 0;
    }
}
// scheduler for MLFQ and STRIDE
void
scheduler(void)
{
  for(;;){
    sti();
    acquire(&ptable.lock);
    if(MLFQ_pass <= STRIDE_pass)
        MLFQ_scheduler();
    else
        STRIDE_scheduler();
    release(&ptable.lock);
  }
}
// systemcall set_cpu_share
int
set_cpu_share(int cpu_share)
{
    struct proc *p;
    double cntt = 0;
    // lock for prevent total cpu is over 80%
    acquire(&set_cpu_share_lock);
    double temp = Cpu_Total + cpu_share;
    release(&set_cpu_share_lock);
    if(temp > 80){                             // fail set cpu
        cprintf("[Total CPU share > 80, Error!]\n");
        return -1;
    }
    else
    {
        acquire(&set_cpu_share_lock);          // lock
        Cpu_Total += cpu_share;
        proc->CPU = cpu_share;      // save cpu_share
        release(&set_cpu_share_lock);
        
        acquire(&ptable.lock);
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->pid == proc->pid){
                p->Flag = 1;    // Change proc MLFQ to STRIDE
                cntt++;     // count number of threads
            }
        }
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->pid == proc->pid){
                p->Origin_cpu = cpu_share/cntt; // Save cpu_share
                p->Stride = MAX_STRIDE/p->Origin_cpu;   // Decide proc stride
            }
        }
        release(&ptable.lock);
        return 0;
    }
}
// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}
// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}
// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }
  // Return to "caller", actually trapret (see allocproc).
}
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");
  if(lk == 0)
    panic("sleep without lk");
  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();
  // Tidy up.
  proc->chan = 0;
  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}
//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}
// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}
// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}
//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
int
thread_create(thread_t *thread, void *(*start_routine)(void*), void *arg)
{
    int i;
    double cnt,cpu;
    struct proc *p;
    struct proc *pp;
    p = 0;
    pp = 0;

    if((p = allocproc()) ==0)
        return -1;
    nextpid--;
    // allocate new thread same Pid with parent
    // tid will increase from 0
    p->pid = proc->pid;
    // set thread parent for main thread
    if(proc->TFlag == 0)
        p->parent = proc;
    else
        return -1;
    // If threadcreate() called first time then
    // Save stack address in tstackaddr
    if(proc->CFlag == 0){
        for(i =0; i < 64; i++)
            proc->tstackaddr[i] = proc->sz + (i+1)*2*PGSIZE;
        if(proc->Flag == 1){
            proc->CPU = proc->Origin_cpu;
            cpu = proc->Origin_cpu;
        }
    }
    // find a empty space for allocate threadstack
    // 0 is empty, 1 is full
    for(i = 0; i < 64; i++){
        if(p->parent->tspace[i] == 0)
        {
            p->tid = i;
            p->parent->tspace[i] = 1;
            break;
        }
    }
    // this process is thread
    p->TFlag = 1;
    p->Orphan = 0;
    // Set thread stack base address    
    p->tstack = proc->tstackaddr[p->tid]-2*PGSIZE;
    // Set master thread size
    proc->sz = proc->tstackaddr[p->tid];
    *p->tf = *proc->tf;
    p->tf->eax = 0;
    // copy parent pdgir address to thread
    p->pgdir = proc->pgdir;
    // modified return address to start_routine
    p->tf->eip = (uint)start_routine;
    // allocate new page to thread
    p->sz = allocuvm(p->pgdir, p->tstack, p->tstack + 2*PGSIZE);
    p->tf->esp = p->tstack + 4092;
    *((uint*)(p->tf->esp)) = (uint)arg;
    *((uint*)(p->tf->esp-4)) = 0xFFFFFFFF;
    p->tf->esp -=4;
    // tid will be saved
    *thread = p->tid;
    // Master thread will never set tstackaddr[] again
    proc->CFlag = 1;

    // If go STRIDE sheduler go here
    cnt = 0;
    if(proc->Flag == 1){
        for(pp = ptable.proc; pp < &ptable.proc[NPROC]; pp++){
            if(pp->pid == proc->pid){
                pp->Flag = 1;
                cnt++;  // count number of threads 
            }
        }
        for(pp = ptable.proc; pp < &ptable.proc[NPROC]; pp++){
            if(pp->pid == proc->pid){
                pp->Origin_cpu = cpu/cnt;   // distribute cpu
                pp->Stride = MAX_STRIDE/pp->Origin_cpu; // set stride
            }
        }
    }
    for(i = 0; i < NOFILE; i++)
        if(proc->ofile[i])
            p->ofile[i] = filedup(proc->ofile[i]);
    p->cwd = idup(proc->cwd);
    safestrcpy(p->name, proc->name, sizeof(proc->name));
    acquire(&ptable.lock);
    p->state = RUNNABLE;
    release(&ptable.lock);
    return 0;
}
int
thread_join(thread_t thread, void**retval)
{
    struct proc *p;
    int havethreads,i;
    int count;

    acquire(&ptable.lock);
    for(;;){
        havethreads = 0;
        // scan through table looking for exited thread (TFlag == 1)
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->parent != proc || p->TFlag != 1)
                continue;
            havethreads = 1;
            // find exited thread who has tid same as thread
            if(p->state == ZOMBIE && p->tid == thread){
                // toss the returnvalue form mainthread array to *retval 
               *retval = proc->t_val[p->tid];
                deallocuvm(p->pgdir, p->tstack + 2*PGSIZE, p->tstack);
                kfree(p->kstack);
                count = 0;
                // Set masterthread stack size as last threadstack address
                for(i = 0; i < 64; i++)
                    if(proc->tspace[i] !=0)
                        proc->sz = proc->tstackaddr[i];
                p->kstack = 0;
                proc->tspace[p->tid] = 0;
                p->tid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                p->TFlag = 0;       //change the flag
                p->state = UNUSED;
                // If there aren't any threads then reset masterthread sz
                for(i = 0; i <64; i++)
                    if(proc->tspace[i] == 0)
                        count++;
                if(count == 64)
                    proc->sz = proc->tstackaddr[0]-2*PGSIZE;
                release(&ptable.lock);
                return 0;
            }
        }
        //no point waiting if we don't have any children
        if(!havethreads || proc->killed){
            release(&ptable.lock);
            return -1;
        }
        // wait for tread to thread_exit
        sleep(proc, &ptable.lock);
    }
}

void
thread_exit(void *retval)
{
    int fd;
    int tid = proc->tid;
    // close all open files
    for(fd = 0; fd < NOFILE; fd++){
        if(proc->ofile[fd]){
            fileclose(proc->ofile[fd]);
            proc->ofile[fd] = 0;
        }
    }
    begin_op();
    iput(proc->cwd);
    end_op();
    proc->cwd = 0;
    
    // save return value in main thread array
    proc->parent->t_val[tid] = retval; 
    acquire(&ptable.lock);
    
    // main thread might be sleeping in thread_join()
    wakeup1(proc->parent);
    // decrease total cpu_share and initialze origin_cpu when proc end
    if(proc->Flag == 1){
        proc->parent->Origin_cpu += proc->Origin_cpu;
        proc->Origin_cpu = 0;
        if(proc->parent->Origin_cpu != proc->parent->CPU)
            proc->parent->Origin_cpu = proc->parent->CPU;
    }
    proc->Origin_cpu = 0;
   
    // jump into the scheduler, never to return
    proc->state = ZOMBIE;
    sched();
    panic("thread zombie exit");
}
