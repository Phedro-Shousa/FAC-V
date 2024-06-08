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
/**   Cooperative Scheduling Test                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 2)

/* Define the counters used in the demo application...  */
/* maximum size supported in this region*/
volatile uint32_t tm_cooperative_thread_0_counter = 0;
volatile uint32_t tm_cooperative_thread_1_counter = 0;
volatile uint32_t tm_cooperative_thread_2_counter = 0;
volatile uint32_t tm_cooperative_thread_3_counter = 0;
volatile uint32_t tm_cooperative_thread_4_counter = 0;


/* Define the test thread prototypes.  */

void* tm_cooperative_thread_0_entry(void * arg);
void* tm_cooperative_thread_1_entry(void * arg);
void* tm_cooperative_thread_2_entry(void * arg);
void* tm_cooperative_thread_3_entry(void * arg);
void* tm_cooperative_thread_4_entry(void * arg);


/* Define the stacks for each thread.  */
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_1[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_2[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_3[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_4[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];


/*Defenition of thread id's*/
kernel_pid_t pid_thread_0 = NULL;
kernel_pid_t pid_thread_1 = NULL;
kernel_pid_t pid_thread_2 = NULL;
kernel_pid_t pid_thread_3 = NULL;
kernel_pid_t pid_thread_4 = NULL;
kernel_pid_t pid_thread_report = NULL;


/* Define the reporting thread prototype.  */
void* tm_cooperative_thread_report(void* arg);


/* Define the cooperative scheduling test initialization.  */
void tm_cooperative_scheduling_initialize(void)
{
    /* Create all 5 threads at priority 8.  */
    pid_thread_0 = tm_thread_create(0, 8, tm_cooperative_thread_0_entry, stack_thread_0, "tm_thread_0");
    pid_thread_1 = tm_thread_create(0, 8, tm_cooperative_thread_1_entry, stack_thread_1, "tm_thread_1");
    pid_thread_2 = tm_thread_create(0, 8, tm_cooperative_thread_2_entry, stack_thread_2, "tm_thread_2");
    pid_thread_3 = tm_thread_create(0, 8, tm_cooperative_thread_3_entry, stack_thread_3, "tm_thread_3");
    pid_thread_4 = tm_thread_create(0, 8, tm_cooperative_thread_4_entry, stack_thread_4, "tm_thread_4");

    tm_thread_resume(pid_thread_0);
    tm_thread_resume(pid_thread_1);
    tm_thread_resume(pid_thread_2);
    tm_thread_resume(pid_thread_3);
    tm_thread_resume(pid_thread_4);
    
    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(1, 7, tm_cooperative_thread_report, stack_thread_report, "tm_thread_report");

    /* Resume all threads.  */
    tm_thread_resume(pid_thread_report);
}

/* Define the first cooperative thread.  */
void* tm_cooperative_thread_0_entry(void* arg)
{
    while(1)
    {
        /* Relinquish to all other threads at same priority.  */
        tm_thread_relinquish();

        /* Increment this thread's counter.  */
        tm_cooperative_thread_0_counter++;
    }
}

/* Define the second cooperative thread.  */
void* tm_cooperative_thread_1_entry(void *arg)
{
    while(1)
    {
        /* Relinquish to all other threads at same priority.  */
        tm_thread_relinquish();

        /* Increment this thread's counter.  */
        tm_cooperative_thread_1_counter++;
    }
}

/* Define the third cooperative thread.  */
void* tm_cooperative_thread_2_entry(void *arg)
{
    while(1)
    {
        /* Relinquish to all other threads at same priority.  */
        tm_thread_relinquish();

        /* Increment this thread's counter.  */
        tm_cooperative_thread_2_counter++;
    }
}

/* Define the fourth cooperative thread.  */
void* tm_cooperative_thread_3_entry(void *arg)
{
    while(1)
    {
        /* Relinquish to all other threads at same priority.  */
        tm_thread_relinquish();

        /* Increment this thread's counter.  */
        tm_cooperative_thread_3_counter++;
    }
}

/* Define the fifth cooperative thread.  */
void* tm_cooperative_thread_4_entry(void *arg)
{
    while(1)
    {
        /* Relinquish to all other threads at same priority.  */
        tm_thread_relinquish();

        /* Increment this thread's counter.  */
        tm_cooperative_thread_4_counter++;
    }
}

/* Define the cooperative test reporting thread.  */
void* tm_cooperative_thread_report(void *arg)
{
    uint32_t   total = 0;
    uint32_t   last_total = 0;
    uint32_t   average = 0;
    uint32_t   counter[TM_TEST_CYCLES] = {0};
    uint32_t   counter_index = 0;
    
    while(1)
    {
        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);
        /* Calculate the total of all the counters.  */
        
        total = tm_cooperative_thread_0_counter + \
                tm_cooperative_thread_1_counter + \
                tm_cooperative_thread_2_counter + \
                tm_cooperative_thread_3_counter + \
                tm_cooperative_thread_4_counter;

        /* Calculate the average of all the counters.  */
        average =  total/5;

        /* See if there are any errors.  */
        if ((tm_cooperative_thread_0_counter < (average - 1)) || 
            (tm_cooperative_thread_0_counter > (average + 1)) ||
            (tm_cooperative_thread_1_counter < (average - 1)) || 
            (tm_cooperative_thread_1_counter > (average + 1)) ||
            (tm_cooperative_thread_2_counter < (average - 1)) || 
            (tm_cooperative_thread_2_counter > (average + 1)) ||
            (tm_cooperative_thread_3_counter < (average - 1)) || 
            (tm_cooperative_thread_3_counter > (average + 1)) ||
            (tm_cooperative_thread_4_counter < (average - 1)) || 
            (tm_cooperative_thread_4_counter > (average + 1)))
        {
          
          printf("ERROR: Invalid counter value(s). Cooperative counters should not be more that 1 different than the average!\n");
        }    

	/* Save the last counter.  */
       counter[counter_index++] = total - last_total;
       last_total =  total;
	   
        //end of test, print values and exit
       if(counter_index == TM_TEST_CYCLES)
       {
            printf("\n");
            for(int i = 0; i < counter_index; i++)
            //printf("test %d:  %lu\n", i, counter[i]);
            printf("%lu\n", counter[i]);
         
            print_packets();
            printf("End of Test!\n\n");
			
			ps();
			
            tm_thread_suspend();
       }
      }
}

#endif