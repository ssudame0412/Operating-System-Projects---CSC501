
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
extern unsigned long ctr1000;

SYSCALL lock(int ldes1, int type, int priority)
{
	STATWORD ps;   
	struct lentry *lptr;
	struct pentry *pptr;
// 1add check for lock deleted
// 2 if lock free
	disable(ps);

	
	if(isbadlock(ldes1) || (lptr = &locktab[ldes1])->lstate == LFREE)
	{
			restore(ps);
			return(SYSERR);
		
	};

	
	lptr = &locktab[ldes1];
	pptr = &proctab[currpid];

	// process was waiting on thi slock desriptor already , but the lock was deleted	
	if(pptr->plwaitlid == ldes1 && pptr->pwaitret == DELETED)
		return SYSERR;

	// if lock is created but not acquired by any process 
	if(lptr->lcnt == 1) {
			/* set proc lock params*/
		lptr->lcnt--;
		pptr->plockbitmap[ldes1] = 1;
		pptr->pnlock++;
		lptr->pidbitmap[currpid] = PROC_HOLD;
		lptr->pid = currpid;
		lptr->type = type;
		if(type == READ){
			lptr->nareaders++;
		}
		restore(ps);
        return OK; 
	}
	
	if(type == WRITE){
		 lptr->nqwriters++;
                        goto enqueue;
	} else if(type == READ){
			//1. REader has already acquired a lock
			 if (lptr->type == READ){
				//if any writer with higher priority in the queue add to queue
				int highest_prio_writer = highest_prio_writer_in_queue(ldes1);
				if(highest_prio_writer < priority){
					lptr->nareaders++;
					lptr->pidbitmap[currpid] = PROC_HOLD;
					pptr->plockbitmap[ldes1] = 1;
					pptr->pnlock++;
					restore(ps);
                                        return(OK);
				}
			}
		lptr->nqreaders++;
               	goto enqueue;
	}

	enqueue:
		update_priorities_if_required(currpid, ldes1);
		// update lprio for lock
		int prio_c;
  		prio_c = getprio(currpid);
		if(prio_c > lptr->lprio){
			lptr->lprio = prio_c;
		}	
		lptr->pidbitmap[currpid] = PROC_WAIT;	
		lptr->lcnt--;
			// set process info
		 pptr->pstate = PRWAIT;
		 pptr->plwaitlid = ldes1;
		 pptr->plprio = priority;
		 pptr->pltimer = ctr1000;
		 pptr->plwaittype = type;
		 	// end setting params for process before inserting to wait queue
		insert(currpid,lptr->lqhead, priority);

		pptr->pwaitret = OK;
		resched();
		restore(ps);
		return pptr->pwaitret;
	
}

int highest_prio_writer_in_queue(int lid){
	struct lentry *lptr, *iterator;
	struct pentry *pptr;
	
	int max_writer_priority = -2147483647 - 1;
	lptr = &locktab[lid];
	int literator;
	literator = q[lptr->lqtail].qprev;
	
	while(literator != lptr->lqhead){
		pptr = &proctab[literator];
		
		if(pptr->plwaittype == WRITE){	
			if(pptr->plprio > max_writer_priority)
				max_writer_priority = pptr->plprio;
		}
		literator=q[literator].qprev;
	}
	return max_writer_priority;
}

