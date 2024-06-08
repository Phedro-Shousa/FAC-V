/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2016 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This Original Work may be modified, distributed, or otherwise used in */ 
/*  any manner with no obligations other than the following:              */ 
/*                                                                        */ 
/*    1. This legend must be retained in its entirety in any source code  */ 
/*       copies of this Work.                                             */ 
/*                                                                        */ 
/*    2. This software may not be used in the development of an operating */
/*       system product.                                                  */ 
/*                                                                        */  
/*  This Original Work is hereby provided on an "AS IS" BASIS and WITHOUT */ 
/*  WARRANTY, either express or implied, including, without limitation,   */ 
/*  the warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A  */ 
/*  PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF this         */ 
/*  ORIGINAL WORK IS WITH the user.                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Porting Layer (Must be completed with RTOS specifics)               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include <string.h>
#include <assert.h>
#include "tm_api.h"
#include "thread.h"
#include "msg.h"
#include <errno.h>

#define THREAD_TM_PRIORITY_BASE THREAD_PRIORITY_MAIN + 5


/* This function called from main performs basic RTOS initialization, 
   calls the test initialization function, and then starts the RTOS function.  */
void  tm_initialize(void (*test_initialization_function)(void))
{
    (void)test_initialization_function;

}


/* This function takes a thread ID and priority and attempts to create the
   file in the underlying RTOS.  Valid priorities range from 1 through 31, 
   where 1 is the highest priority and 31 is the lowest. If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.   */
/** RIOT needs a stack for each thread that should be specified before the
  * creation of thread 
  * returns the thread pid, it's attributed by the kernel **/
int  tm_thread_create( int flag, int priority, thread_task_func_t entry_function, char *thread_stack, char *name)
{
  
    kernel_pid_t thread_pid = 0;      /*initialize the value of the thread*/
    
    /*we don't care about the thread_id, is not used in RIOT*/
    /* lower mean higher priority, we don't need main to execute after the
       creation of the threads*/
    
    /*reduced size for threads stack*/
    if(!flag){
      thread_pid = thread_create(thread_stack, THREAD_STACKSIZE_DEFAULT,
                  priority, THREAD_CREATE_SLEEPING, 
                  entry_function, NULL, name);
    }
    else{ 
        thread_pid = thread_create(thread_stack, THREAD_STACKSIZE_MAIN,
                  priority, THREAD_CREATE_SLEEPING,
                  entry_function, NULL, name);  
    }
    if( thread_pid == -EOVERFLOW || thread_pid == EINVAL)
       return TM_ERROR;
    else
       return thread_pid;
}


/* This function resumes the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_resume(int thread_id)
{
   if ( thread_wakeup(thread_id) == 1)
     return TM_SUCCESS;
   else
     return TM_ERROR;
}


/* This function suspends the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
/** is just used to suspend the own thread and not other threads*/
int tm_thread_suspend(void)
{
   thread_sleep(); /*puts the current thread into sleep mode*/
   
   return TM_SUCCESS;
}


/* This function relinquishes to other ready threads at the same
   priority.  */
void tm_thread_relinquish(void)
{
    thread_yield();
}


/* This function suspends the specified thread for the specified number
   of seconds.  If successful, the function should return TM_SUCCESS. 
   Otherwise, TM_ERROR should be returned.  */
void tm_thread_sleep(int seconds)
{
   xtimer_sleep(seconds);  
}


/* This function creates the specified queue.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_create(msg_t *queue, int queue_size)
{
     /*queue size must be a powe of two*/
     msg_init_queue(queue, queue_size);
     return TM_SUCCESS;
}


/* This function sends a 16-byte message to the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_send(int queue_id, msg_t *message_ptr)
{
    //Asynchronous IPC  non-blocking
    //the message will be sent to the own thread
    (void)queue_id;
    if( msg_send_to_self(message_ptr) == 0 )
       return TM_ERROR;
    return TM_SUCCESS;
}


/* This function receives a 16-byte message from the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_receive(int queue_id, msg_t *message_ptr)
{
    (void)queue_id;
    if( msg_receive(message_ptr) < 0)
      return TM_ERROR;   
    return TM_SUCCESS;
}


/* This function creates the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int tm_semaphore_create(sema_t *semaphore)
{
  sema_create(semaphore , 0);
  
  return TM_SUCCESS;
}


/* This function gets the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_get(sema_t *semaphore)
{
   if( _sema_wait_xtimer(semaphore, 1, 10000) < 0 )
       return TM_ERROR;
   
   return TM_SUCCESS;

}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put(sema_t *semaphore)
{
  if( sema_post(semaphore) < 0)
    return TM_ERROR;
  
  return TM_SUCCESS;
}


/* This function creates the specified memory pool that can support one or more
   allocations of 128 bytes.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_memory_pool_create(int pool_id)
{
  //no implementation needed 
  (void)pool_id;
  return TM_SUCCESS;
}


/* This function allocates a 128 byte block from the specified memory pool.  
   If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_allocate(int pool_id, unsigned char **memory_ptr)
{
    (void)pool_id;
    (*memory_ptr)= (unsigned char *)malloc(sizeof(char));
    
    if(memory_ptr == 0)
      return TM_ERROR;
    
    return TM_SUCCESS;
}


/* This function releases a previously allocated 128 byte block from the specified 
   memory pool. If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_deallocate(int pool_id, unsigned char *memory_ptr)
{
    (void)pool_id;
    free(memory_ptr);
    return TM_SUCCESS;
}


