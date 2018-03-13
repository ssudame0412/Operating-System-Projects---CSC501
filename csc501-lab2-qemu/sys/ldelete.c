/* ldelete.c */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int ldelete(int lockid)
{
        STATWORD ps;
        int     pid;
        struct  lentry  *lptr;
        disable(ps);
        if (isbadlock(lockid) || locktab[lockid].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
        lptr = &locktab[lockid];
        lptr->lstate = LFREE;
	
        if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    proctab[pid].pwaitret = DELETED;
                    ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);
}

