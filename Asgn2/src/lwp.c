#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "lwp.h"
#include <sys/resource.h>

static tid_t global_count = 0;
static thread current_thread = NULL;
static thread all_threads_head = NULL;
static thread terminated_newest = NULL;
static thread terminated_oldest = NULL;
static thread waiting_newest = NULL;
static thread waiting_oldest = NULL;

/*thread wrapper*/

static void lwp_wrap(lwpfun fun, void *arg) {
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

/*fifo queue setups*/

// terminated_oldest -> oldest thread -> ... -> newest thread <- terminated_newest
static void terminated_fifo_queue(thread queued_thread){
    queued_thread->lib_two = NULL; //point new thread to nothing in fifo
    if (terminated_newest == NULL) { //if queue is empty
        terminated_oldest = queued_thread; //thread is oldest
        terminated_newest = queued_thread; //and newest
    } 
    else { //if queue isn't empty
        terminated_newest->lib_two = queued_thread; //older newest points to new thread
        terminated_newest = queued_thread; // new thread is now the queue tail
    }
}
static thread terminated_fifo_dequeue(void) {
    if (terminated_oldest == NULL) { //if queue is empty before dequeue
        return NULL;
    }
    thread oldest = terminated_oldest; //save oldest thread
    terminated_oldest = oldest->lib_two; //next oldest is now oldest
    if (terminated_oldest == NULL) { //if queue is now empty
        terminated_newest = NULL; //make it so
    }
    oldest->lib_two = NULL; //remove thread from queue
    return oldest; //return oldest thread
}

static void wait_fifo_queue(thread queued_thread){
    queued_thread->lib_two = NULL; //point new thread to nothing in fifo
    if (waiting_newest == NULL) { //if queue is empty
        waiting_oldest = queued_thread; //thread is oldest
        waiting_newest = queued_thread; //and newest
    } 
    else { //if queue isn't empty
        waiting_newest->lib_two = queued_thread; //older newest points to new thread
        waiting_newest = queued_thread; // new thread is now the queue tail
    }
}
static thread wait_fifo_dequeue(void) {
    if (waiting_oldest == NULL) { //if queue is empty before dequeue
        return NULL;
    }
    thread oldest = waiting_oldest; //save oldest thread
    waiting_oldest = oldest->lib_two; //next oldest is now oldest
    if (waiting_oldest == NULL) { //if queue is now empty
        waiting_newest = NULL; //make it so
    }
    oldest->lib_two = NULL; //remove thread from queue
    return oldest; //return oldest thread
}

/*thread removal helper to remove terminated thread from all threads list*/

static void thread_removal(thread removed_thread) {
    thread previous = NULL;
    thread current = all_threads_head;
    while (current != NULL) {
        if (current == removed_thread) {
            if (previous == NULL) {
                all_threads_head = current->lib_one;
            } else {
                previous->lib_one = current->lib_one;
            }
            current->lib_one = NULL;
            return;
        }
        previous = current;
        current = current->lib_one;
    }
}

/*terminated thread clearer helper for lwp_wait*/

static tid_t thread_clearer(thread terminated, int *status) {
    if (terminated == NULL) {
        return NO_THREAD;
    }
    //save tid and status of terminated thread
    tid_t tid = terminated->tid;
    if (status != NULL) {
        *status = terminated->status;
    }
    //remove thread from all threads list
    thread_removal(terminated);

    //free up stack space and heap for terminated thread
    if (terminated->stack != NULL) {
        munmap(terminated->stack, terminated->stacksize);
    }
    free(terminated);

    return tid;
}


/*lwp functions*/

extern tid_t lwp_create(lwpfun fun, void *arg){
    thread new_thread = calloc(1, sizeof *new_thread);
    if (new_thread == NULL) {
        return NO_THREAD;
    }
    new_thread->tid = ++global_count;
    new_thread->status = LWP_LIVE;
    new_thread->state.fxsave = FPU_INIT;

    /*stacksize stuff*/
    struct rlimit limit;
    size_t stacksize;   
    if (getrlimit(RLIMIT_STACK, &limit) == -1 || limit.rlim_cur == RLIM_INFINITY) { //If RLIMIT_STACK does not exist or if its value is RLIM_INFINITY
        stacksize = 8 * 1024 * 1024;   //set to 8MB
    } else {
        stacksize = (size_t)limit.rlim_cur; //set to current soft limit
    }
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        free(new_thread);
        return NO_THREAD;
    }
    if (stacksize % (size_t)page_size != 0) { //is if not a multiple
        stacksize += (size_t)page_size - (stacksize % (size_t)page_size); //round up
    }

    /*allocate thread stack*/
    void * s = mmap(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    if (s == MAP_FAILED){
        free(new_thread);
        return NO_THREAD;
    }
    else{
        new_thread->stack = s; //stack base address
        new_thread->stacksize = stacksize;
        /*fun stuff*/
        unsigned long *top = new_thread->stack + (stacksize / sizeof(unsigned long)); //start at the high end
        unsigned long *frame = top - 3; //pretend there was a context switch before
        frame[0] = (unsigned long)0; //no previous frame for rbp
        frame[1] = (unsigned long)lwp_wrap; //return address for swap_rfiles
        frame[2] = (unsigned long)0; //return address for lwp_wrap (nothing at start)
        new_thread->state.rbp = (unsigned long)frame;
        new_thread->state.rsp = (unsigned long)frame;
        new_thread->state.rdi = (unsigned long)fun;
        new_thread->state.rsi = (unsigned long)arg;
        
        new_thread->lib_one = all_threads_head; //add last created thread to front of list
        all_threads_head = new_thread; //update next thread head with currently created one
        
        
        
        
        //2. Admit it to the active scheduler
        return new_thread->tid;
    }
    return NO_THREAD;

        
    
    



}
extern void  lwp_exit(int status){
    
}

extern tid_t lwp_gettid(void){
    if (current_thread == NULL){
        return NO_THREAD;
    }
    else{
        return current_thread->tid;
    }
}

extern void  lwp_yield(void){

}

extern void  lwp_start(void){
    thread original_thread = calloc(1, sizeof *original_thread);
    if (original_thread == NULL) {
        exit(EXIT_FAILURE);
    }
    original_thread->tid = ++global_count;
    original_thread->status = LWP_LIVE;
    original_thread->stack = NULL;
    original_thread->stacksize = 0;
    original_thread->lib_one = all_threads_head;
    all_threads_head = original_thread;
    current_thread = original_thread;
    //admit to scheduler
    lwp_yield();
    
}

extern tid_t lwp_wait(int *status){

}

extern void  lwp_set_scheduler(scheduler fun){

}

extern scheduler lwp_get_scheduler(void){

}

extern thread tid2thread(tid_t tid){
    thread threads_copy = all_threads_head;
    while(threads_copy != NULL){
        if (threads_copy->tid == tid){
            return threads_copy;
        }
        else{
            threads_copy = threads_copy->lib_one;
        }
    }
    return NULL;
}