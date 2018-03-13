//#include <limit.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>
extern void reset_priority_after_release(int pid);
extern void update_lprio(int lockid);
int releaseall (numlocks, args)
	int	numlocks;			/* number of args that follow	*/
	long	args;
{
	STATWORD ps;

	int a;
	int i;
	struct lentry *lptr; 
	struct pentry *pptr;
	
	int status = OK;
	disable(ps);
	
	pptr = &proctab[currpid];

	for(i = numlocks-1; numlocks > 0 ; numlocks--)
	{
		a = (int)*(&args) + i;

		if(pptr->plockbitmap[a] == 1){
			lptr = &locktab[a];

			if(lptr->lstate == LFREE || isbadlock(a)){
						status = SYSERR;
						continue;
			}
			pptr->plockbitmap[a] = 0;
			pptr->pnlock--;
			lptr->pidbitmap[currpid] = PROC_NOTHOLD;
				//currproc not reading anymore, can decrement the nareader count for lock a
			if(lptr->type == READ){
				lptr->nareaders--;

				/*if more readers have acquired the lock*/
				if(lptr->nareaders > 0){
						
					//update inherited priority
					reset_priority_after_release(currpid);
					continue;
				}
			}
			awakenfromqueue(a);
			reset_priority_after_release(currpid);
	
		} else {
			status = SYSERR;
		}
	}
	restore(ps);
	return status;

}
// called when a process is killed
int release_lock_for_pid(int pid, int lid){
	struct lentry *lptr; 
	struct pentry *pptr;
	pptr = &proctab[pid];
	
	if(pptr->plockbitmap[lid] == 1){
	
			lptr = &locktab[lid];

			if(lptr->lstate == LFREE || isbadlock(lid)){
				return SYSERR;
			}

			pptr->plockbitmap[lid] = 0;
			pptr->pnlock--;
			lptr->pidbitmap[pid] = PROC_NOTHOLD;
				//pid not reading anymore, can decrement the nareader count for lock a
			if(lptr->type == READ){
				lptr->nareaders--;

				/*if more readers have acquired the lock*/
				if(lptr->nareaders > 0){
					//proc not holding this lock anymore
					//update inherited priority
					reset_priority_after_release(pid);
					return OK;
				}
			}

		awakenfromqueue(lid);
		reset_priority_after_release(pid);
	}
	return OK;
}

void awakenfromqueue(int lockid)
{
	int temp;
	struct lentry *lptr; 
	struct pentry *pptr;
	lptr = &locktab[lockid];

//nothing in queue, no process waiting on lock
	if(q[lptr->lqhead].qnext == lptr->lqtail){
		/* queue is empty*/
		// reset lock params
		lptr->nqreaders = 0;
		lptr->nqwriters = 0;
		lptr->lprio = 0;
		lptr->lcnt++;
		return;
	}

	int highestpriow = get_highest_prio_writer_pid(lockid);
	
	if(highestpriow == -1){
		// no writer.. release all readers
		int proc;		
		proc = q[lptr->lqtail].qprev;
		
		while(proc != (lptr->lqhead)){
			// awake this process -start params changes
			pptr = &proctab[proc];
			set_acquire_params(proc, lockid);
				
			int prevproc = q[proc].qprev;
			temp = proc;		
			if(lptr->nareaders == 0){
                                lptr->pid = proc;
				lptr->lcnt++;
                        }	
			//these process now hold the lock
			lptr->pidbitmap[temp] = PROC_HOLD;	
				ready(dequeue(temp), RESCHYES);

			lptr->nareaders++;
			lptr->nqreaders--;
			// end params changes
			
			proc = prevproc;	
		}
		lptr->type = READ;	

	} else {
		// release process 
		// 1. if process prio > highest reader prio
		// 2. if process prio == highest reader prio && wait time < 0.5
		int proc;		
		proc = q[lptr->lqtail].qprev;
		struct pentry *hwproc;
		hwproc = &proctab[highestpriow];
		int temp;
		int flag = 0;
		while(proc != (lptr->lqhead)){
			
			// awake this process -start params changes
			pptr = &proctab[proc];
			int prevproc = q[proc].qprev;
			temp = proc;
			proc = q[proc].qprev;
			if(pptr != hwproc){
			if ( (pptr->plprio > hwproc->plprio) || ((pptr->plprio == hwproc->plprio) && ((hwproc->pltimer - pptr->pltimer) < 500)) ){
				flag = 1;

				set_acquire_params(temp, lockid);
				if(lptr->nareaders == 0){
                                        lptr->pid = proc;
					lptr->lcnt++;
                                }
			        lptr->pidbitmap[temp] = PROC_HOLD;		
					ready(dequeue(temp), RESCHYES);

				// set lock params // set pid of first reader

				lptr->nareaders++;
				lptr->nqreaders--;

				lptr->type = READ;	
				// end params changes
			} }
			
		}

		if(!flag){
			// release writer
				set_acquire_params(highestpriow, lockid);
				lptr->pidbitmap[highestpriow] = PROC_HOLD;
				if((lptr->lcnt++) < 0)
					ready(dequeue(highestpriow), RESCHYES);

				lptr->pid = highestpriow;
				lptr->nqwriters--;

				lptr->type = WRITE;	
		}
		
	}
	update_lprio(lockid);
	return;
}

int get_highest_prio_writer_pid(int lockid){
	struct lentry *lptr;
	struct pentry *pptr;
	int max_writer_priority = -2147483647 - 1;
	lptr = &locktab[lockid];
	int literator;
	int max_writer_pid = -1;
	literator = q[lptr->lqtail].qprev;
	
	while(literator != lptr->lqhead){
		pptr = &proctab[literator];
		
		if(pptr->plwaittype == WRITE){	
			if(pptr->plprio > max_writer_priority){
				max_writer_pid = literator;
				max_writer_priority = pptr->plprio;
			}
		}
		literator=q[literator].qprev;
	}
	return max_writer_pid;
}

void set_acquire_params(int pid, int lockid){
	struct pentry *pptr = &proctab[pid];
	
	pptr->pnlock++;
	pptr->plockbitmap[lockid]=1;

				// not waiting anymore reset wait parameters
	pptr->plprio = 0;
	pptr->pltimer = 0;
	pptr->plwaitlid = PR_NOLOCK;
	pptr->plwaittype = 0;

}


