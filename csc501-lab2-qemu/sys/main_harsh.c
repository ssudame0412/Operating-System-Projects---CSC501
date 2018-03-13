#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
    lock (lck, READ, DEFAULT_LOCK_PRIO);
    kprintf ("  %s: acquired lock, sleep 2s\n", msg);
    sleep (2);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
	kprintf("lock released\n");
}

void test1 ()
{
    int    lck;
    int    pid1;
    int    pid2;

    kprintf("\nTest 1: readers can share the rwlock\n");
    lck  = lcreate ();
    assert (lck != SYSERR, "Test 1 failed");

    pid1 = create(reader1, 2000, 30, "reader a", 2, "reader a", lck);
   pid2 = create(reader1, 2000, 50, "reader b", 2, "reader b", lck);
 kprintf("resume pi1i\n");
    resume(pid1);
    kprintf("resume pi2i\n");
    resume(pid2);
   kprintf("sleeping\n"); 
    sleep (5);
	kprintf("deleting lck");
    ldelete (lck);
    kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[30];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
    releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
    kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1, wr2, wr3;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
        " lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

    rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
    rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
    rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
    rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'Z', lck, 35);
        wr2 = create(writer2, 2000, 20, "writer2", 3, 'Y', lck, 15);
        wr3 = create(writer2, 2000, 20, "writer2", 3, 'X', lck, 25);
    
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
    resume (rd3);
    resume (rd4);
    resume(wr2);
    resume(wr3);
    resume(wr1);


        sleep (25);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABDDCCEE",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/
/*void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (15);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}*/
/*
void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed from here");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
    assert (getprio(wr1) == 25, "Test 3 failed from her 2");
	kprintf("%d reader1", getprio(rd1) );
        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
    sleep (1);
  
    assert (getprio(wr1) == 30, "Test 3 failed from here 3");
    
    kprintf("-kill reader B, then sleep 1s\n");
    kill (rd2);
    sleep (1);
kprintf("%d\n pri ", getprio(wr1));
    assert (getprio(wr1) == 25, "Test 3 failed from here 4");

    kprintf("-kill reader A, then sleep 1s\n");
    kill (rd1);
    sleep(1);
    assert(getprio(wr1) == 20, "Test 3 failed from her 5");

        sleep (8);
        kprintf ("Test 3 OK\n");
}
*/
/*
void test3(){
int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 15, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
 assert (getprio(wr1) == 20, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
 sleep (1);
 assert (getprio(wr1) == 30, "Test 3 failed");
 kprintf("-change writer , then sleep 1s\n");
 chprio (wr1,15);
 sleep (1);
 assert (getprio(wr1) == 30, "Test 3 failed");
 
 kprintf("-change writer , then sleep 1s\n");
 chprio (wr1,40);
 sleep (1);
 assert (getprio(wr1) == 40, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (rd2,45);
 sleep (1);
 assert (getprio(wr1) == 45, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (rd2,25);
 sleep (1);
 assert (getprio(wr1) == 40, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (rd1,25);
 sleep (1);
 assert (getprio(wr1) == 40, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (rd2,45);
 sleep (1);
 assert (getprio(wr1) == 45, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (wr1,55);
 sleep (1);
 kprintf("value %d\n",getprio(wr1));
 assert (getprio(wr1) == 55, "Test 3 failed");
 
 kprintf("-change reader B, then sleep 1s\n");
 chprio (rd2,45);
 sleep (1);
 kprintf("value %d\n",getprio(wr1));
 assert (getprio(wr1) == 55, "Test 3 failed");
 
 
        sleep (10);
        kprintf ("Test 3 OK\n");

}*/
void reader3 (char *msg, int lck1,int lck2)
{
        int     ret;

        kprintf ("  %s: to acquire lock1 \n", msg);
        lock (lck1, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock1\n", msg);
        kprintf ("  %s: to acquire lock2 \n", msg);
        lock (lck2, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock2\n", msg);
        releaseall(2,lck1,lck2);
        kprintf ("  \n%s: to release lock1\n", msg);
kprintf ("  \n%s: to release lock2\n", msg);
        
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (15);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test9()
{
        int     lck1,lck2;
        int     rd1;
        int     wr1,wr2;

        kprintf("\nTest 8: Transitive property");
        lck1 = lcreate ();
        lck2 = lcreate();
        assert (lck1 != SYSERR, "Syser lock creation");
        assert (lck2 != SYSERR, "Syser lock creation");
        
        
        rd1 = create(reader3, 2000, 10, "reader3", 3, "reader A", lck1,lck2);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer B", lck2);
        wr2 = create(writer3, 2000, 30, "writer3", 2, "writer C ", lck1);

        kprintf("\n-start writer B with Priority 20, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (2);

        kprintf("\n-start reader A with priority 10, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (2);
        assert (getprio(wr1) == 20, "Test 3 failed");

        kprintf("\n-start writer C with Priority 30, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr2);
        sleep (2);
        assert (getprio(rd1) == 30, "Reader A not updated");
        assert (getprio(wr1) == 30, "Writer B not  updated");

        sleep(45);


        kprintf("\n Transitive working ");







}
int main( )
{
        /* These test cases are only used for test purpose.
         * The provided results do not guarantee your correctness.
         * You need to read the PA2 instruction carefully.
         */
//    test1();
  //  test2();
    test9();

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        shutdown();
}







