#include "lwp.h"
#include "rr.h"
#include <stdlib.h>

#define NEXT sched_one 




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



static thread tail = NULL;
static int ready_threads = 0;
                        
void rr_admit(thread new) {
  if (tail == NULL) {
    tail = new;
    new->NEXT = new;
  }
  else {
    new->NEXT = tail->NEXT;
    tail->NEXT = new;
    tail = new;
  }
  ready_threads++;
}

void rr_remove(thread victim) {
  /* should only unlink from queue, decrement ready, and clear its sched_one
  */
  if (tail == NULL) {
    return;
  }
  /*check if victim is only thread in queue */
  if (tail == victim && tail->NEXT == victim) {
    tail = NULL;
  }
  else {
    thread temp_thread = tail;
    while (temp_thread->NEXT != victim && temp_thread->NEXT != tail) {
      temp_thread = temp_thread->NEXT;
    }
    /*here, temp_thread->NEXT is either the victim or tail*/
    if (temp_thread->NEXT == victim) {
      temp_thread->NEXT = victim->NEXT;
      if (victim == tail) {
        tail = temp_thread;
      }
      victim->NEXT = NULL;
      ready_threads--;
    }
    else {
      /*victim not found in queue*/
      return;
    }

  }
}

thread rr_next(void) { /*assuming this executes current*/
  if (tail == NULL) {
    return NULL;
  }
  tail = tail->NEXT;
  return tail;
}

int rr_qlen(void) {
  return ready_threads;
}


/* Helper function adding removed threads to exited list.
   Exited threads will be added to a list that is freed during shutdown.*/

void rr_terminate(thread victim) {
  if (victim == NULL) {
    return;
  }
  victim->status = TERMINATED;
  victim->NEXT = NULL;
}