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
	kprintf("highprioProc is going to acquire sem\n");
	wait(sem);
	kprintf ("highprioProc: %s: acquired sem %d, sleep 2sec\n", msg, ctr1000 - start);
	sleep(2);
	signal(sem);
	kprintf ("highprioProc: %s: released sen\n", msg);
}

void mediumprioproc(char *msg){
	int start = ctr1000; 

	kprintf("\nmediumprioproc is going to start\n");
	int i;
	while(ctr1000 - start < 100) {
		for(i = 0; i< 15; i++){
			kprintf(".");
			sleep(1);
		
		}
}
}
void lowprioProc_sem(char *msg, int sem){
	int start = ctr1000; 
	int i;
	kprintf("lowprioProc_sem is going to acquire sem \n");
	wait(sem);
	kprintf ("lowprioProc_sem: %s: acquired sem , running\n", msg);
	while(ctr1000 - start < 800) {
		for(i = 0; i< 150000000; i++);
			kprintf("!");
			//sleep(1);
		
	}
	signal(sem);
	kprintf("\nlowprioProc_sem  released lock\n");
}


void highprioProc_Lock(char *msg, int lck){
	int start = ctr1000;
	kprintf("highprioProc is going to acquire lock\n");
	lock(lck, WRITE, 10);
	kprintf ("highprioProc: %s: acquired lock %d, sleep 7sec\n", msg,ctr1000 - start);
	sleep(7);
	releaseall(1, lck);
	kprintf("highprioProc released lock\n");
}
void lowprioProc_Lock(char *msg, int lck){
	int start = ctr1000; 
	int i;
	kprintf("lowprioProc_lock is going to acquire lock\n", start);
	lock(lck, WRITE, 15);
	kprintf ("lowprioProc_Lock: %s: acquired lock, running\n", msg);
	while(ctr1000 - start < 800) {
		for(i = 0; i< 150000000; i++);
			kprintf("!");
			//sleep(1);
		
	}
	releaseall(1, lck);
	kprintf("\nlowprioProc_lock released lock\n");
}

void test_sem(){
	int sem, sem2;
	int l, h, m, m2;

	kprintf("\nTest sem: creating a sem\n");
	sem = screate(1);

	assert (sem != SYSERR, "Test sem failed\n");


	l = create(lowprioProc_sem,  2000, 10, "lowprio", 2, "lowprio" , sem );
	m = create(mediumprioproc,   2000, 30, "medprio", 2, "medprio" ,  sem);
	m2 = create(mediumprioproc, 2000,  35, "medprio2", 2, "medprio2", sem);
	h = create(highprioProc_sem, 2000, 50, "highprio", 2, "highprio" , sem);

	kprintf("\nStarting lowprio proc\n");
	resume(l);
	sleep(1);
	kprintf("\nStarting highprio proc\n");
	resume(h);

	kprintf("\nPrio of lowprioProcess changed to : %d\n", getprio(l));

	kprintf("\nStarting medprio proc\n");
	resume(m);
	resume(m2);
	sleep( 20);
	kprintf("\nEnd test\n");
}


void test_lock(){
	int lock;
	int l, h, m, m2;

	kprintf("\nTest sem: creating a lock\n");
	lock = lcreate();
	//lock2 = lcreate();
	assert (lock != SYSERR, "Test lock failed\n");


	l = create(lowprioProc_Lock,  2000, 10, "lowprio", 2, "lowprio" , lock );
	m = create(mediumprioproc,   2000, 30, "medprio", 2, "medprio" ,  lock);
	m2 = create(mediumprioproc, 2000,  35, "medprio2", 2, "medprio2", lock);
	h = create(highprioProc_Lock, 2000, 40, "highprio", 2, "highprio" , lock);

	kprintf("\n--Starting lowprio proc\n");
	resume(l);
	sleep(1); 
	kprintf("\n--Starting highprio proc\n");
	resume(h);
	kprintf("\n--Prio of lowprioproc change to : %d\n", getprio(l));
 	assert (getprio(l) == 40, "Test lock failed");
	kprintf("\n--Starting medprio proc\n");
	resume(m);
	resume(m2);
	sleep(20);
	kprintf("end test\n\n");
}

int main(){

	test_lock();

	 test_sem();
	shutdown();


}



