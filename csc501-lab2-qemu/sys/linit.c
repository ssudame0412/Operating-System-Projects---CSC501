
#include <lock.h>
#include <conf.h>
#include <q.h>

void linit(){
     struct lentry *lptr;
     int i;
     for(i = 0 ; i < NLOCK ; i++) {
		lptr = &locktab[i];
	 	lptr->lstate = LFREE;
 		lptr->lcnt = 1;
		lptr->type = 0;;
		lptr->nareaders = 0;
		lptr->nqreaders = 0;
		lptr->nqwriters = 0;
		lptr->lprio = 0; /*lowest priority*/
		lptr->lqtail = 1 + (lptr->lqhead= newqueue());
		int j;
		for(j = 0 ; j < NPROC; j++){
			lptr->pidbitmap[j] = PROC_NOTHOLD;
		}
	}
}
