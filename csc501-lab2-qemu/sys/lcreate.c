/* lcreate.c - lcreate, newsem */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();


int lcreate()
{
        STATWORD ps;
        int     lockid;

        disable(ps);
        if ( (lockid=newlock())==SYSERR ) {
                restore(ps);
                return(SYSERR);
        }
        /*need to check  sqhead and sqtail were initialized at system startup */
        restore(ps);
        return(lockid);
}

int newlock()
{
        int     lockid;
        int     i;

        for (i=0 ; i<NLOCK ; i++) {
                lockid=nextlock--;
                if (nextlock < 0)
                        nextlock = NLOCK-1;
                if (locktab[lockid].lstate==LFREE) {
		      	locktab[lockid].lstate = LUSED;
			return(lockid);
                }
        }
        return(SYSERR);
}
