#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <sem.h>
extern unsigned long ctr1000;
#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

void highprioProc_sem(char *msg, int sem){
 int start = ctr1000; 

 kprintf("highprioProc is going to acquire sem at %d\n", start);
 wait(sem);
 kprintf ("highprioProc: %s: acquired sem, sleep 2sec\n", msg);
 sleep(5);
 kprintf ("highprioProc: %s: to release sen\n", msg);
 signal(sem);
 kprintf("total time for highprioProc : %d\n", ctr1000 - start);
}

void mediumprioproc(char *msg){
 int start = ctr1000; 

 kprintf("mediumprioproc is going to start%d\n", start);
 int i;
// int start = ctr1000;
while(ctr1000 - start < 2000) {
	for(i = 0; i< 1000; i++){
		kprintf(".");
		sleep(1);
	}
	
}
 kprintf("total time for mediumprioproc : %d\n", ctr1000 - start);
}

void lowprioProc_sem(char *msg, int sem){
int start = ctr1000; 

 kprintf("lowprioProc_sem is going to acquire sem at %d\n", start);
 wait(sem);
 kprintf ("lowprioProc_sem: %s: acquired sen, sleep 2sec\n", msg);
 sleep(7);
 kprintf ("lowprioProc_sem: %s: to release sem\n", msg);
 signal(sem);
 kprintf("total time for lowprioProc_sem : %d\n", ctr1000 - start);
}


void highprioProc_Lock(char *msg, int lck){
int start  = ctr1000;
 kprintf("highprioProc is going to acquire lock at %d\n", start);
 lock(lck, WRITE, 10);
 kprintf ("highprioProc: %s: acquired lock, sleep 2sec\n", msg);
 sleep(7);
 kprintf ("highprioProc: %s: to release lock\n", msg);
 releaseall(1, lck);
 kprintf("total time for highprioProc : %d\n", ctr1000 - start);
}
void lowprioProc_Lock(char *msg, int lck){
int start = ctr1000; 

 kprintf("lowprioProc_lock is going to acquire lock at %d\n", start);
  lock(lck, WRITE, 15);
 kprintf ("lowprioProc_Lock: %s: acquired lock, sleep 2sec\n", msg);
 sleep(7);
 kprintf ("lowprioProc_lock: %s: to release lock\n", msg);
 releaseall(1, lck);
 kprintf("total time for lowprioProc_lock : %d\n", ctr1000 - start);
}

void test_sem(){
 int sem;
 int l, h, m;

 kprintf("\nTest sem: creating a sem\n");
 sem = screate(1);
 assert (sem != SYSERR, "Test sem failed\n");


 l = create(lowprioProc_sem,  2000, 10, "lowprio", 2, "lowprio" , sem );
 m = create(mediumprioproc,   2000, 30, "medprio", 2, "medprio" ,  sem);
 h = create(highprioProc_sem, 2000, 40, "highprio", 2, "highprio" , sem);

 kprintf("Starting lowprio proc\n");
 resume(l);
 sleep(1);
 kprintf("Starting highprio proc\n");
 resume(h);

kprintf("prio of low change to : %d\n", getprio(l));

 kprintf("Starting medprio proc\n");
 resume(m);
sleep(20);
kprintf("end test");
}


void test_lock(){
 int lock;
 int l, h, m;

 kprintf("\nTest sem: creating a lock\n");
 lock = lcreate();
 assert (lock != SYSERR, "Test lock failed\n");


 l = create(lowprioProc_Lock,  2000, 10, "lowprio", 2, "lowprio" , lock );
 m = create(mediumprioproc,   2000, 30, "medprio", 2, "medprio" ,  lock);
 h = create(highprioProc_Lock, 2000, 40, "highprio", 2, "highprio" , lock);

 kprintf("Starting lowprio proc\n");
 resume(l);
sleep(1);
 kprintf("Starting highprio proc\n");
 resume(h);
kprintf("prio of low change to : %d\n", getprio(l));
 kprintf("Starting medprio proc\n");
 resume(m);
 sleep(20);
 kprintf("end test");
}

int main(){
 test_sem();

test_lock();

shutdown();


}



