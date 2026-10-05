#include "lwp.h"
#include "rr.h"
#include <stdlib.h>

#define next sched_one




/* Tuple that describes a scheduler */
typedef struct scheduler {
  void   (*init)(void);            /* initialize any structures     */
  void   (*shutdown)(void);        /* tear down any structures      */
  void   (*admit)(thread new);     /* add a thread to the pool      */
  void   (*remove)(thread victim); /* remove a thread from the pool */
  thread (*next)(void);            /* select a thread to schedule   */
  int    (*qlen)(void);            /* number of ready threads       */
} *scheduler;


/* DEFINED IN rr.h
struct scheduler RR = {NULL,
                      NULL,
                      rr_admit,
                      rr_remove,
                      rr_next,
                      rr_qlen}; */



static thread tail;
static thread exit_queue;
static int ready_threads = 0;
                        
void rr_admit(thread new) {
  if (tail == NULL) {
    tail = new;
    new->next = new;
  }
  else {
    new->next = tail->next;
    tail->next = new;
    tail = new;
  }
  tail->exited = exit_queue;
  ready_threads++;
}

void rr_remove(thread victim) {
  if (tail == NULL) {
    return;
  }
  /*check if victim is only thread in queue */
  if (tail == victim && tail->next == victim) {
    tail = NULL;
  }
  else {
    thread temp_thread = tail;
    while (temp_thread->next != victim && temp_thread.next != tail) {
      temp_thread = temp_thread->next;
    }
    /*here, temp_thread->next is either the victim or tail*/
    if (temp_thread->next == victim) {
      temp_thread->next = victim->next;
      if (victim == tail) {
        tail = temp_thread;
      }
      ready_threads--;
      rr_terminate(victim);
    }
    else {
      /*victim not found in queue*/
      return;
    }

  }
}

thread rr_next(void) {
  return RR.next();
}

int rr_qlen(void) {
  return RR.qlen();
}


/* Helper function adding removed threads to exited list.
   Exited threads will be added to a list that is freed during shutdown.*/

void rr_terminate(thread victim) {
  if (victim == NULL) {
    return;
  }
  victim->status = TERMINATED;
  victim->next = NULL;
}