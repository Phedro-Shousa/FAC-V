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
/**   Basic Processing Test                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 1)

/* Define the test thread prototypes.  */
void * tm_basic_processing_thread_0_entry(void * arg);

/* Define the reporting thread prototype.  */
void * tm_basic_processing_thread_report(void * arg);

/*each of the threads need a single space for stack*/
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];

/* Define the counters used in the demo application...  */
volatile uint32_t tm_basic_processing_counter = 0;

/* Test array.  We will just do a series of calculations on the 
   test array to eat up processing bandwidth. The idea is that 
   all RTOSes should produce the same metric here if everything
   else is equal, e.g. processor speed, memory speed, etc.  */

/*Due to the linker script is not possible to use a uint32_t*/
volatile uint16_t tm_basic_processing_array[1024] = { 0 };


/* Define the basic processing test initialization.  */
void tm_basic_processing_initialize(void)
{
    kernel_pid_t pid_thread_0 = 0;
    kernel_pid_t pid_thread_report = 0;
    
    /* Create thread 0 at priority 8.  */
    pid_thread_0 = tm_thread_create(0, 8, tm_basic_processing_thread_0_entry, stack_thread_0, "tm_thread_0");


    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    //higher priority
    pid_thread_report = tm_thread_create(0, 7, tm_basic_processing_thread_report, stack_thread_report, "tm_thread_report");

    /* Resume threads.  */
    tm_thread_resume(pid_thread_report);
    tm_thread_resume(pid_thread_0);
    printf("initialize()\n");
}


/* Define the basic processing thread.  */
void * tm_basic_processing_thread_0_entry(void * arg)
{
    (void)arg;
  int i;

    /* Initialize the test array.   */
    for (i = 0; i < 1024; i++)
    {
        /* Clear the basic processing array.  */
        tm_basic_processing_array[i] =  0;
    }

    printf("thread_0_entry()\n");
    while(1)
    {
        /* Loop through the basic processing array, add the previous 
           contents with the contents of the tm_basic_processing_counter
           and xor the result with the previous value...   just to eat 
           up some time.  */
        for (i = 0; i < 1024; i++)
        {
            /* Update each array entry.  */
            tm_basic_processing_array[i] =  (tm_basic_processing_array[i] + tm_basic_processing_counter) ^ tm_basic_processing_array[i];
        }

        /* Increment the basic processing counter.  */
        tm_basic_processing_counter++;
    }
}


/* Define the basic processing reporting thread.  */
void * tm_basic_processing_thread_report(void* arg)
{
  (void)arg;
  uint32_t   last_counter = 0;
  uint32_t   counter[TM_TEST_CYCLES] = {0};
  uint32_t   counter_index = 0;
    
        printf("thread_report()\n");
    while(1)
    {

        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);
        
        /* See if there are any errors.  */
        if (tm_basic_processing_counter == last_counter)
        {       
#ifdef ENABLE_PRINTF
          printf("ERROR: Invalid counter value(s). Basic processing thread died!\n");
#endif
          while (1)
          {
          
          }
        }

		
        /* Save the last counter.  */
		counter[counter_index++] = tm_basic_processing_counter - last_counter;
        last_counter =  tm_basic_processing_counter;
			   
		//end of test, print values and exit
       if(counter_index == TM_TEST_CYCLES)
       {
            printf("=T2=\n");
            for(uint32_t i = 0; i < counter_index; i++)
            //printf("test %d:  %lu\n", i, counter[i]);
            printf("%lu\n", counter[i]);
         
            print_packets();
            printf("End of Test!\n\n");
			
			ps();
			
            tm_thread_suspend();
       }
                
        /* Show the time period total.  */
    }
}

#endif
