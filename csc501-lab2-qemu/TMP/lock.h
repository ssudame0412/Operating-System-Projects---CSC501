/* lock.h - isbadlock */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCK
#define NLOCK		50 	/*no . of locks*/
#endif

#ifndef	NPROC				/* set the number of processes	*/
#define	NPROC		30		/*  allowed if not already done	*/
#endif

#define LFREE 	'\01'
#define LUSED 	'\02'
#define LDELETED '\03'

#define ACQUIRED_BY_NONE 0
#define READ 	1
#define WRITE 	2

#define PROC_NOTHOLD  0
#define PROC_HOLD   1
#define PROC_WAIT   2


struct lentry	{
	int		type; //currently acquired by Reader : READ, writer : WRITE, none : ACQUIRED_BY_NONE
	int		pid; // acquired by process
	char	lstate;
	int 	lcnt;
	int		lqhead;
	int 	lqtail;
	int		nareaders; // no of readers acquired access
	int		nqreaders;	// no of readers in q
	int 	nqwriters; // no of writers waiting in q
	int 	pidbitmap[NPROC]; 
	int 	lprio;
};



extern struct lentry locktab[];
extern int nextlock;

#define isbadlock(l)	(l<0 || l>=NLOCK)

void linit();
int lcreate();
int ldelete (int lockdescriptor);
int lsignal(int lock);
int lwait();
int lock (int ldes1, int type, int priority);
int releaseall(int numlocks,long arg, ...);
int highest_prio_writer_in_queue(int lid);
void awakenfromqueue(int lockid);
void set_acquire_params(int proc, int lockid);
int get_highest_prio_writer_pid(int lockid);
void update_lprio(int lockid);
void reset_priority_after_release(int pid);
void update_priorities_if_required(int pid_requesting, int lock_requested);
void reverse_priority(int lid);
#endif
