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
}

void test1 ()
{
 int lck;
 int pid1;
 int pid2;

 kprintf("\nTest 1: readers can share the rwlock\n");
 lck  = lcreate ();
 assert (lck != SYSERR, "Test 1 failed");

 pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
 pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

 resume(pid1);
 resume(pid2);
 
 sleep (5);
 ldelete (lck);
 kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[10];
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
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
  " lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

 rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
 rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
 rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
 rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 25);
 
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


        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABDDCCEE",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
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
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
 assert (getprio(wr1) == 25, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
 sleep (1);
 assert (getprio(wr1) == 30, "Test 3 failed");
 
 kprintf("-kill reader B, then sleep 1s\n");
 kill (rd2);
 sleep (1);
 assert (getprio(wr1) == 25, "Test 3 failed");

 kprintf("-kill reader A, then sleep 1s\n");
 kill (rd1);
 sleep(1);
 assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}
void reader5 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer5 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test5 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 5: Test priority change\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 5 failed");

        rd1 = create(reader5, 2000, 25, "reader5", 2, "reader A", lck);
        rd2 = create(reader5, 2000, 30, "reader5", 2, "reader B", lck);
        wr1 = create(writer5, 2000, 20, "writer5", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
 assert (getprio(wr1) == 25, "Test 5-1 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
 sleep (1);
 assert (getprio(wr1) == 30, "Test 5-2 failed");
 
 kprintf("-change priority for reader A, then sleep 1s\n");
 chprio (rd1, 40);
 sleep (1);
 assert (getprio(wr1) == 40, "Test 5-3 failed");

 kprintf("-change priority reader B, then sleep 1s\n");
 chprio (rd2, 50);
 sleep(1);
 assert(getprio(wr1) == 50, "Test 5-4 failed");

        sleep (8);
        kprintf ("Test 5 OK\n");
}
void readerA (char *msg, int lck1)
{
        int     ret;

        kprintf ("  %s: to acquire lock lck1\n", msg);
        lock (lck1, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck1\n", msg);
        kprintf ("  %s: to release lock lck1\n", msg);
        releaseall (1, lck1);
}

void readerB (char *msg, int lck2)
{
        int     ret;

        kprintf ("  %s: to acquire lock lck2\n", msg);
        lock (lck2, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck2\n", msg);
        kprintf ("  %s: to release lock lck2\n", msg);
        releaseall (1, lck2);
}

void writerC (char *msg, int lck1, int lck2, int lck3)
{
        kprintf ("  %s: to acquire lock lck1\n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck1\n", msg);
 kprintf ("  %s: to acquire lock lck2\n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck2, sleep 30s\n", msg);
        sleep (30);
 
 kprintf ("  %s: to acquire lock lck3\n", msg);
        lock (lck3, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck3\n", msg);

        kprintf ("  %s: to release lock lck1 lck2 lck3\n", msg);
        releaseall (1, lck1, lck2, lck3);
}

void writerD (char *msg, int lck3)
{
        kprintf ("  %s: to acquire lock lck3\n", msg);
        lock (lck3, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck3, sleep 50s\n", msg);
        sleep (50);
        kprintf ("  %s: to release lock lck3\n", msg);
        releaseall (1, lck3);
}
void test6(){
 int     lck1, lck2, lck3;
        int     rdA, rdB;
        int     wrC, wrD;

        kprintf("\nTest 6: More transitivity - Priority inheritance\n");
        lck1  = lcreate ();
 lck2  = lcreate ();
 lck3  = lcreate ();

        assert (lck1 != SYSERR, "Test 6 failed");
        assert (lck2 != SYSERR, "Test 6 failed");
 assert (lck3 != SYSERR, "Test 6 failed");

        rdA = create(readerA, 2000, 20, "readerA", 2, "reader A", lck1);
        rdB = create(readerB, 2000, 40, "readerB", 2, "reader B", lck1);
        wrC = create(writerC, 2000, 10, "writerC", 4, "writer C", lck1, lck2, lck3);
        wrD = create(writerD, 2000, 5,  "writerD", 2, "writer D", lck3);

        kprintf("-start writer C, then sleep 1s. lock granted to write (prio 10)\n");
        resume(wrC);
 assert (getprio(wrC) == 10, "Test 6-1 failed");
        sleep (1);

 kprintf("-start reader A, then sleep 1s (prio 20)\n");
 resume(rdA);
 assert (getprio(wrC) == 20, "Test 6-2 failed");
 sleep(1);

 kprintf("-start writer D, then sleep 1s. lock granted to writer (prio 5)");
 resume(wrD);
 assert( getprio(wrD) == 5, "Test 6-3 failed");
 sleep(1);

 kprintf("Wait for writer C to request lck3\n");
 sleep(40);
 assert(getprio(wrD) == 20, "Test 6-4 failed\n");

 kprintf("-start reader B, then sleep 1s (prio 40)\n");
 resume(rdB);
 assert (getprio(wrC) == 40, "Test 6-5 failed");
 assert (getprio(wrD) == 40, "Test 6-6 failed");
 sleep(1);

        sleep (8);
        kprintf ("Test 6 OK\n");

}
reader4 (char *msg, int lck1)
{
        int     ret;

        kprintf ("  %s: to acquire lock lck1\n", msg);
        lock (lck1, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck1\n", msg);
        kprintf ("  %s: to release lock lck1\n", msg);
        releaseall (1, lck1);
}

void writer4 (char *msg, int lck1, int lck2)
{
        kprintf ("  %s: to acquire lock lck1\n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck1, sleep 10s\n", msg);
        sleep (10);
 
 kprintf ("  %s: to acquire lock lck2\n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck2, sleep 10s\n", msg);

        kprintf ("  %s: to release lock lck1 lck2\n", msg);
        releaseall (1, lck1, lck2);
}

void writer6 (char *msg, int lck2)
{
        kprintf ("  %s: to acquire lock lck2\n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock lck2, sleep 30s\n", msg);
        sleep (30);
        kprintf ("  %s: to release lock lck2\n", msg);
        releaseall (1, lck2);
}

void test4 (){
 int     lck1, lck2;
        int     rdA;
        int     wrC, wrD;

        kprintf("\nTest 4: test the transitivity for priority inheritence\n");
        lck1  = lcreate ();
 lck2  = lcreate ();
        assert (lck1 != SYSERR, "Test 4 failed");
        assert (lck2 != SYSERR, "Test 4 failed");

        rdA = create(reader4, 2000, 40, "reader4", 2, "reader A", lck1);
        wrC = create(writer4, 2000, 20, "writer4", 3, "writer C", lck1, lck2);
        wrD = create(writer6, 2000, 10, "writer6", 2, "writer D", lck2);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wrC);
        sleep (1);

 assert (getprio(wrC) == 20, "Test 4-1 failed");


 kprintf("-start reader, then sleep 1s. lock granted to write (prio 40)\n");
        resume(rdA);
        sleep (1);
 assert (getprio(wrC) == 40, "Test 4-2 failed");

 kprintf("-start writer, then sleep 1s. lock granted to write (prio 10)\n");
        resume(wrD);
        sleep (1);

 assert(getprio(wrD) == 10, "Test 4-3 failed");

 sleep(10);

 assert(getprio(wrD) == 40, "Test 4-4 failed");

        sleep (8);
        kprintf ("Test 4 OK\n");

}
int main( )
{
        
 test1();
 test2();
 test3();
test5();
test6();
test4();        
        shutdown();
}




