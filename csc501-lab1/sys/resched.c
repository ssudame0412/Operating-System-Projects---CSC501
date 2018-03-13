/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

int schedule_class =0;
int epoch = 0;
long timestamp = 0;

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	int random_prio;
	int total_sum;
	int new_proc;
	int iproc;
	/* no switch needed if current process priority higher than next*/
	if( schedule_class == RANDOMSCHED){
		optr= &proctab[currpid];
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}		
		total_sum = total_sum_in_ready();

		iproc = q[rdytail].qprev;
		if(total_sum!=0){
			random_prio = rand()%(total_sum-1);	
			while(q[iproc].qkey < random_prio && (iproc < NPROC)){
				random_prio = random_prio - q[iproc].qkey;
				iproc = q[iproc].qprev;	
			}}
		nptr = &proctab[ (currpid = iproc) ];
		dequeue(currpid);
		nptr->pstate = PRCURR;	
#ifdef  RTCLOCK
		preempt = QUANTUM;              /* reset preemption counter     */
#endif
	} else if(schedule_class == LINUXSCHED){
		optr= &proctab[currpid];
		int old_priority; 
		int max_goodness =0;
		
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}
		//set the counter //
		if(epoch > 0) {
			epoch = epoch - (optr->pcounter - preempt);
			old_priority = optr->pgoodness - optr->pcounter;
			optr->pcounter = preempt;	
			if(optr->pcounter == 0)		
				optr->pgoodness = 0;		
			else 
				optr->pgoodness = optr->pcounter + old_priority;
		}
		if(epoch <= 0){
			start_epoch();
		}	
		new_proc = choose_process();
		
		nptr = &proctab[(currpid = new_proc)];
		dequeue(currpid);
		nptr->pstate = PRCURR;
		if(currpid == NULLPROC) {
			epoch = 0;
#ifdef  RTCLOCK
			preempt = QUANTUM;
#endif
		} else {
                // incrementing my local timestamp to make sure round robin works.
			 nptr->ptimer = timestamp;
			timestamp++;
#ifdef  RTCLOCK		
			preempt = nptr->pcounter;
#endif
		}
	} else {
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
				(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}

		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	}
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

	/* The OLD process returns here when resumed. */
	return OK;

}
void start_epoch()
{
	int epoch_duration = 0;
	struct pentry *proc;
	int i;
	for(i = 0; i < NPROC; i++) {

		proc = &proctab[i];
		if(proc->pstate != PRFREE){
			proc->pquantum = proc->pcounter/2 + proc->pprio;
			proc->pcounter = proc->pquantum;
			proc->pgoodness = proc->pcounter + proc->pprio;
			epoch_duration += proc->pquantum;
		}
	}
	epoch = epoch_duration;

}

int choose_process()
{
	int max_goodness =0;
	int new_proc;
	int iproc;
	iproc = q[rdytail].qprev;
	int num = 0;
	long last_scheduled = 0;
	// if no process in the ready queue except ready queue
	if((q[0].qprev > NPROC) && (q[0].qnext > NPROC)) return NULLPROC;
	
	while( iproc != rdyhead)
	{
		kprintf("time %d\n",proctab[iproc].ptimer  );
		/* mainting a new variable "ptimer" make sure if processed have same goodness, algorithm  select a process which was executed last. 
		/ so we are maintaining a time counter which increments whenever a process is rescheduled. This way every proccess will have different 
		/ ptimer entries.  we select the process which has least ptimer value. ie. it was executed last among the processes with same goodness.
		*/
		if(proctab[iproc].pgoodness > max_goodness || (proctab[iproc].pgoodness == max_goodness && proctab[iproc].ptimer < last_scheduled)){
			new_proc = iproc;
			max_goodness = proctab[iproc].pgoodness;
			last_scheduled = proctab[iproc].ptimer;
		}
		iproc = q[iproc].qprev; 
		num++;
	}
	//all processes have either used up their quantums for this epoch restart epoch
	if(max_goodness == 0 )
	{
		start_epoch();
		return choose_process();
	}
	return new_proc;
}

int total_sum_in_ready()
{
	int total_sum = 0;	
	struct  qent  *iptr; 

	iptr = &q[rdyhead];
	iptr = &q[iptr->qnext];
	while(iptr->qnext != EMPTY){
		//kprintf("%d \n", iptr->qkey);
		total_sum = total_sum + iptr->qkey;
		iptr = &q[iptr->qnext];

	}	
	return total_sum;
}

void setschedclass(int sched_class){	

	schedule_class = sched_class;
}

int getschedclass() {
	return schedule_class;
}
