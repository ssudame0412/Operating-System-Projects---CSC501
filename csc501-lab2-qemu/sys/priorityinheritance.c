#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


void update_priorities_if_required(int pid_requesting, int lock_requested)
{
	struct lentry *lptr, *lptr2;
	struct pentry *pptr, *ptr;

	/* check all the processess acquiring LOCK requested
	 *  * if pinh of processes < pinh of pid_requesting - >update recursively
	 *   *   else return
	 *    */

	if(isbadlock(lock_requested) || (lptr = &locktab[lock_requested])->lstate == LFREE)
                        return;

        

	lptr = &locktab[lock_requested];
	int pid_req_prio = getprio(pid_requesting);
	int p,  p_prio;

	for(p =0 ; p < NPROC; p++){
		// if process : p acquired lock_requested
		if(lptr->pidbitmap[p] == PROC_HOLD){
			ptr = &proctab[p];
			p_prio = getprio(p);

			// the process p is waiting for another lock, recussively update pinh of processess waiting on lock acquired by p
			if(ptr->plwaitlid != PR_NOLOCK && ptr->pstate == PRWAIT){

				if(p_prio < pid_req_prio){
					ptr->pinh = pid_req_prio;
					lptr2= &locktab[ptr->plwaitlid];
					if(ptr->pinh > lptr2->lprio) {
						lptr2->lprio = ptr->pinh;
					}
				}
				update_priorities_if_required(p, ptr->plwaitlid);
			}
			// the process p currently executing: update the pinh, so that it can be scheduled based on this prio
			if(ptr->plwaitlid == PR_NOLOCK) {
				if( p_prio < pid_req_prio)
					ptr->pinh = pid_req_prio;
			}
		}
	}
}

void update_lprio(int lockid){
	int max_prio = 0;
	struct lentry *lptr;
	struct pentry *ptr;

	if(isbadlock(lockid) || (lptr = &locktab[lockid])->lstate == LFREE)
                   return;

	lptr = &locktab[lockid];
	int p;
	p = q[lptr->lqtail].qprev; 
	int count =0 ;
	while(p != (lptr->lqhead)){

		count++;
		ptr = &proctab[p];
		int prio;
		prio =  getprio(p);
		if(max_prio < prio ){
			max_prio = prio;
		}
		p = q[p].qprev;
	}
	lptr->lprio = max_prio;
	return;
}

void reset_priority_after_release(int pid){

	struct pentry *pptr, *ptr;
	struct lentry *lptr;
	pptr = &proctab[pid];
	int max_prio = pptr->pprio;
	int i;

	for(i = 0; i < NLOCK ; i++){
		if (pptr->plockbitmap[i] == 1){	
			lptr = &locktab[i];
			if(max_prio < lptr->lprio)
				max_prio = lptr->lprio;

		}
	}
	if(max_prio > pptr->pprio){
		pptr->pinh = max_prio;
	} else {
		pptr->pinh = 0;
	}
	return;
}

void reverse_priority(int lid){
	//called when a process nnwaiting in lid is either killed(removed from the Q) or changed priority
struct lentry *lptr;
	if(isbadlock(lid) || (lptr = &locktab[lid])->lstate == LFREE)
                        return;

	update_lprio(lid);
	// for all processes holding lid, update their priority

	lptr = &locktab[lid];
	int p;
	for(p = 0; p < NPROC; p++){

		if(lptr->pidbitmap[p] == PROC_HOLD){

			// first calculte selfs new prio
			reset_priority_after_release(p);
			// change priorities of procs holding lock; which is waiting for; also upadte the lprio of lock p is waiting for
			if(proctab[p].plwaitlid != -1&& proctab[p].plwaitlid>0){
				reverse_priority(proctab[p].plwaitlid);
			}
		}	
	}
	return;
}
