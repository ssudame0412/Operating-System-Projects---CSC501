#define RANDOMSCHED 1 
#define LINUXSCHED 2

extern void setschedclass(int sched_class);
extern int getschedclass();
extern int total_sum_in_ready();
extern int choose_process();
extern void start_epoch();

