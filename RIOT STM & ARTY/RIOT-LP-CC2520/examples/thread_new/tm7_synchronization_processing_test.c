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
/**   Synchronization Processing Test                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include <stdio.h>
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 7)

/* Define the counters used in the demo application...  */
volatile uint32_t tm_synchronization_processing_counter = 0;

/* Define the test thread prototypes.  */
void * tm_synchronization_processing_thread_0_entry(void * arg);

/* Define the reporting thread prototype.  */
void * tm_synchronization_processing_thread_report(void * arg);

/* Define the stacks for each thread.  */
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];

/*Defenition of thread id's*/
kernel_pid_t pid_thread_0 = 0;
kernel_pid_t pid_thread_report = 0;

/*Declaration of semaphore*/
sema_t semaphore;


/* Define the synchronization processing test initialization.  */
void tm_synchronization_processing_initialize(void)
{
    /* Create thread 0 at priority 10.  */
    pid_thread_0 = tm_thread_create(0, 8, tm_synchronization_processing_thread_0_entry, 
                                    stack_thread_0, "tm_thread_0");

    /* Create a semaphore for the test.  */
    tm_semaphore_create(&semaphore);

    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(0, 7, tm_synchronization_processing_thread_report, 
                                         stack_thread_report, "tm_thread_report");
 
    tm_thread_resume(pid_thread_report);
    
    /* Resume thread 0.  */
    tm_thread_resume(pid_thread_0);
}


/* Define the synchronization processing thread.  */
void * tm_synchronization_processing_thread_0_entry(void* arg)
{
    (void)arg;
  int status;

    while(1)
    {
        /* Get the semaphore.  */
        tm_semaphore_get(&semaphore);

        /* Release the semaphore.  */
        status = tm_semaphore_put(&semaphore);

        /* Check for semaphore put error.  */
        if (status != TM_SUCCESS)
            break;

        /* Increment the number of semaphore get/puts.  */
        tm_synchronization_processing_counter++;
    }
    return 0;
}


/* Define the synchronization test reporting thread.  */
void*  tm_synchronization_processing_thread_report(void *arg)
{
    (void)arg;
  uint32_t   last_counter = 0;
  uint32_t   counter[TM_TEST_CYCLES] = {0};
  uint32_t   counter_index = 0;

    while(1)
    {

        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);
        
        /* See if there are any errors.  */
        if (tm_synchronization_processing_counter == last_counter)
        {
          //abort test
           printf("ERROR: Invalid counter value(s). Error allocating/deallocating memory!\n");
           tm_thread_suspend();
        }
       
       //end of test, print values and exit
       if(counter_index == TM_TEST_CYCLES)
       {
            printf("\n");
            for(uint32_t i = 0; i < counter_index; i++)
            //printf("test %d:  %lu\n", i, counter[i]);
            printf("%lu\n", counter[i]);
         
            print_packets();
            printf("End of Test!\n\n");
			
			ps();
	
            tm_thread_suspend();
       }

        /* Save the last counter.  */
        counter[counter_index++] = tm_synchronization_processing_counter - last_counter;
        last_counter =  tm_synchronization_processing_counter;
                
        /* Show the time period total.  */
    }
}

#endif
