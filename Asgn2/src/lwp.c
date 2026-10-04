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
static thread terminated_head = NULL;
static thread terminated_tail = NULL;
static thread waiting_head = NULL;
static thread waiting_tail = NULL;



static void lwp_wrap(lwpfun fun, void *arg) {
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}


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

extern tid_t lwp_create(lwpfun fun, void *arg){
    thread new_thread = calloc(1, sizeof *new_thread);
    if (new_thread == NULL) {
        return NO_THREAD;
    }
    new_thread->tid = ++global_count;
    new_thread->status = LWP_LIVE;
    new_thread->state.fxsave = FPU_INIT;
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