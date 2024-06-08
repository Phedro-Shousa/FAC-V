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
/** Application Interface (API)                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    tm_api.h                                            PORTABLE C      */ 
/*                                                           4.2          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the basic Application Interface (API)             */ 
/*    implementation source code for the Thread-Metrics performance       */
/*    test suite. All service prototypes and data structure definitions   */
/*    are defined in this file.                                           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  03-01-2004     William E. Lamie         Initial Version 4.0           */
/*  03-05-2007     William E. Lamie         CMP Release New Banner 4.1    */ 
/*  05-15-2016     William E. Lamie         Updated comments 4.2          */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef  TM_API_H
#define  TM_API_H

/*******************RIOT Includes********************/
#include "xtimer.h"
#include "thread.h"
#include "sema.h"
#include "msg.h"
#include <stdlib.h>

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef   __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {
#endif
  
  
/* ********************** */
/* Define test to run!!  */
#define THREADMETRIC 10


#if THREADMETRIC == 1
#define TEST "1 \"Basic Processing Test\""
#elif THREADMETRIC == 2
#define TEST "2 \"Cooperative Scheduling Test\""
#elif THREADMETRIC == 3
#define TEST "3 \"Preemptive Scheduling Test\""
#elif THREADMETRIC == 4
#define TEST "4 \"Interrupt Processing Test\""
#elif THREADMETRIC == 5
#define TEST "5 \"Interrupt Preemption Processing Test\""
#elif THREADMETRIC == 6
#define TEST "6 \"Message Processing Test\""
#elif THREADMETRIC == 7
#define TEST "7 \"Synchronization Processing Test\""
#elif THREADMETRIC == 8
#define TEST "8 \"RTOS Memory allocation\""
#elif THREADMETRIC == 9
#define TEST "9 \"No thread Test\""
#elif THREADMETRIC == 10
#define TEST "10 \"No thread STM Test\""
#else
#warning Please select a test! 
#endif

/* Define API constants.  */

#define TM_SUCCESS  0
#define TM_ERROR    1
#define ENABLE_PRINTF 


/* Define the time interval in seconds. This can be changed with a -D compiler option.  */
// Time period should be 30 seconds.
#ifndef TM_TEST_DURATION
#define TM_TEST_DURATION    30
#endif
  
/*Defines the number of cycles that the test will execute*/
// number of cycles should be 5.
#ifndef TM_TEST_CYCLES
#define TM_TEST_CYCLES    5
#endif


/* Define RTOS Neutral APIs. RTOS vendors should fill in the guts of the following
   API. Once this is done the Thread-Metric tests can be successfully run.  */

void tm_initialize(void (*test_initialization_function)(void));
int tm_thread_create(int flag, int priority, thread_task_func_t entry_function, char *thread_stack, char *name);
int tm_thread_resume(int thread_id);
int tm_thread_suspend(void);
void tm_thread_relinquish(void);
void tm_thread_sleep(int seconds);
int tm_queue_create(msg_t *queue, int queue_size);
int tm_queue_send(int queue_id, msg_t *message_ptr);
int tm_queue_receive(int queue_id, msg_t *message_ptr);
int tm_semaphore_create(sema_t *semaphore);
int tm_semaphore_get(sema_t *semaphore);
int tm_semaphore_put(sema_t *semaphore);
int tm_memory_pool_create(int pool_id);
int tm_memory_pool_allocate(int pool_id, unsigned char **memory_ptr);
int tm_memory_pool_deallocate(int pool_id, unsigned char *memory_ptr);

/* Define test initialization prototypes.  */

void tm_basic_processing_initialize(void);
void tm_cooperative_scheduling_initialize(void);
void tm_preemptive_scheduling_initialize(void);
void tm_interrupt_processing_initialize(void);
void tm_interrupt_preemption_processing_initialize(void);
void tm_message_processing_initialize(void);
void tm_synchronization_processing_initialize(void);
void tm_memory_allocation_initialize(void);


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif
