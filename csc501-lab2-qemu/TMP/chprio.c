/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	
	// added for PA2
	// if new priority is greater than the inherited priority, inherited prio should not be used anymore.
	if(pptr->pprio >= pptr->pinh)
		pptr->pinh = 0;

	// there is priority change, the process must update the lprio of the lock, pid is waiting on	
	int lock_waiting_on = pptr->plwaitlid;
	if(lock_waiting_on != -1){
		reverse_priority(lock_waiting_on);
	}

	restore(ps);
	return(newprio);
}
