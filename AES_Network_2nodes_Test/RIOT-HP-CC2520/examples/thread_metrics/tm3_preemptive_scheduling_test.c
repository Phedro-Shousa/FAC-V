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
/**   Preemptive Scheduling Test                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 3)

/* Define the counters used in the demo application...  */
volatile uint32_t tm_preemptive_thread_0_counter;
volatile uint32_t tm_preemptive_thread_1_counter;
volatile uint32_t tm_preemptive_thread_2_counter;
volatile uint32_t tm_preemptive_thread_3_counter;
volatile uint32_t tm_preemptive_thread_4_counter;

/* Define the test thread prototypes.  */
void* tm_preemptive_thread_0_entry(void * arg);
void* tm_preemptive_thread_1_entry(void * arg);
void* tm_preemptive_thread_2_entry(void * arg);
void* tm_preemptive_thread_3_entry(void * arg);
void* tm_preemptive_thread_4_entry(void * arg);

/* Define the reporting thread prototype.  */
void* tm_preemptive_thread_report(void * arg);

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

/* Define the preemptive scheduling test initialization.  */

void tm_preemptive_scheduling_initialize(void)
{
    /* Create thread 0 at priority 10.  */
    pid_thread_0 = tm_thread_create(0, 12, tm_preemptive_thread_0_entry, stack_thread_0, "tm_thread_0");

    /* Create thread 1 at priority 9.  */
    pid_thread_1 = tm_thread_create(0, 11, tm_preemptive_thread_1_entry, stack_thread_1, "tm_thread_1");

    /* Create thread 2 at priority 8.  */
    pid_thread_2 = tm_thread_create(0, 10, tm_preemptive_thread_2_entry, stack_thread_2, "tm_thread_2");

    /* Create thread 3 at priority 7.  */
    pid_thread_3 = tm_thread_create(0, 9, tm_preemptive_thread_3_entry, stack_thread_3, "tm_thread_3");

    /* Create thread 4 at priority 6.  */
    pid_thread_4 = tm_thread_create(0, 8, tm_preemptive_thread_4_entry, stack_thread_4, "tm_thread_4");

    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(1, 7, tm_preemptive_thread_report, stack_thread_report, "tm_thread_report");
    tm_thread_resume(pid_thread_report);
    
    /* Resume just thread 0.  */
    tm_thread_resume(pid_thread_0);
}

/* Define the first preemptive thread.  */
void* tm_preemptive_thread_0_entry(void * arg)
{
  
    while(1)
    {
        /* Resume thread 1.  */
        tm_thread_resume(pid_thread_1);

        /* We won't get back here until threads 1, 2, 3, and 4 all execute and
           self-suspend.  */

        /* Increment this thread's counter.  */
        tm_preemptive_thread_0_counter++;
    }
}

/* Define the second preemptive thread.  */
void* tm_preemptive_thread_1_entry(void * arg)
{

    while(1)
    {
        /* Resume thread 2.  */
        tm_thread_resume(pid_thread_2);

        /* We won't get back here until threads 2, 3, and 4 all execute and
           self-suspend.  */

        /* Increment this thread's counter.  */
        tm_preemptive_thread_1_counter++;

        /* Suspend self!  */
        tm_thread_suspend();
    }
}

/* Define the third preemptive thread.  */
void* tm_preemptive_thread_2_entry(void * arg)
{

    while(1)
    {
        /* Resume thread 3.  */
        tm_thread_resume(pid_thread_3);

        /* We won't get back here until threads 3 and 4 execute and
           self-suspend.  */

        /* Increment this thread's counter.  */
        tm_preemptive_thread_2_counter++;

        /* Suspend self!  */
        tm_thread_suspend();
    }
}


/* Define the fourth preemptive thread.  */
void* tm_preemptive_thread_3_entry(void * arg)
{

    while(1)
    {
        /* Resume thread 4.  */
        tm_thread_resume(pid_thread_4);

        /* We won't get back here until thread 4 executes and
           self-suspends.  */

        /* Increment this thread's counter.  */
        tm_preemptive_thread_3_counter++;

        /* Suspend self!  */
        tm_thread_suspend();
    }
}


/* Define the fifth preemptive thread.  */
void* tm_preemptive_thread_4_entry(void * arg)
{

    while(1)
    {
        /* Increment this thread's counter.  */
        tm_preemptive_thread_4_counter++;

        /* Self suspend thread 4.  */
        tm_thread_suspend();
    }
}


/* Define the preemptive test reporting thread.  */
void* tm_preemptive_thread_report(void * arg)
{

uint32_t total = 0;
//uint32_t relative_time = 0;
uint32_t last_total = 0;
uint32_t average = 0;
uint32_t counter[TM_TEST_CYCLES] = {0};
uint32_t counter_index = 0;

    while(1)
    {
        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);

        /* Calculate the total of all the counters.  */
        total =  tm_preemptive_thread_0_counter + tm_preemptive_thread_1_counter + tm_preemptive_thread_2_counter
                    + tm_preemptive_thread_3_counter + tm_preemptive_thread_4_counter;

        /* Calculate the average of all the counters.  */
        average =  total/5;

        /* See if there are any errors.  */
        if ((tm_preemptive_thread_0_counter < (average - 1)) || 
            (tm_preemptive_thread_0_counter > (average + 1)) ||
            (tm_preemptive_thread_1_counter < (average - 1)) || 
            (tm_preemptive_thread_1_counter > (average + 1)) ||
            (tm_preemptive_thread_2_counter < (average - 1)) || 
            (tm_preemptive_thread_2_counter > (average + 1)) ||
            (tm_preemptive_thread_3_counter < (average - 1)) || 
            (tm_preemptive_thread_3_counter > (average + 1)) ||
            (tm_preemptive_thread_4_counter < (average - 1)) || 
            (tm_preemptive_thread_4_counter > (average + 1)))
        {
#ifdef ENABLE_PRINTF
            printf("ERROR: Invalid counter value(s). Preemptive counters should not be more that 1 different than the average!\n");
#else
            while(1)
            {

            }
#endif
        }
                
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
            
       /* Save the last counter.  */
       counter[counter_index++] = total - last_total;
       last_total =  total;
      }
}

#endif
