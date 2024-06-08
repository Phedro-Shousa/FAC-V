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
/**   Interrupt Preemption Processing Test                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "periph/gpio.h"
#include <stdio.h>
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 5)

//#define SEMAPHORES

#define OUT_PIN (GPIO_PIN(0, 9))
#define IN_PIN (GPIO_PIN(0, 10))

/* Define the counters used in the demo application...  */
volatile uint32_t tm_interrupt_preemption_thread_0_counter;
volatile uint32_t tm_interrupt_preemption_thread_1_counter;
volatile uint32_t tm_interrupt_preemption_handler_counter;


/* Define the test thread prototypes.  */
void * tm_interrupt_preemption_thread_0_entry(void * arg);
void * tm_interrupt_preemption_thread_1_entry(void * arg);
void * tm_interrupt_preemption_handler_entry(void * arg);


/* Define the reporting thread prototype.  */
void * tm_interrupt_preemption_thread_report(void * arg);

/* Define the interrupt handler.  This must be called from the RTOS.  */
void tm_interrupt_preemption_handler(void * arg);

/* Define the stacks for each thread.  */
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_1[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];

/*Defenition of thread id's*/
kernel_pid_t pid_thread_0 = 0;
kernel_pid_t pid_thread_1 = 0;
kernel_pid_t pid_thread_report = 0;

#ifdef SEMAPHORES
sema_t tm_semaphore;
#endif //SEMAPHORES


/* Define the interrupt processing test initialization.  */
void tm_interrupt_preemption_processing_initialize(void)
{
  
    /* Initialize the output/input pins  */
    printf("  > Configuring output pin...\n");
    gpio_init(OUT_PIN, GPIO_OUT);
    gpio_set(OUT_PIN);
    
    gpio_init_int(IN_PIN, GPIO_IN, GPIO_FALLING, tm_interrupt_preemption_handler, NULL);

    #ifdef SEMAPHORES
    printf("  > Creating a semaphore...\n");     
    tm_semaphore_create(&tm_semaphore);
    
    #endif //SEMAPHORES
    
    /* Create interrupt thread at priority 3.  */
    pid_thread_0 = tm_thread_create(0, 9, tm_interrupt_preemption_thread_0_entry, stack_thread_0, "tm_thread_0");

    /* Create thread that generates the interrupt at priority 10.  */
    pid_thread_1 = tm_thread_create(0, 10, tm_interrupt_preemption_thread_1_entry, stack_thread_1, "tm_thread_1");

    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(1, 8, tm_interrupt_preemption_thread_report, stack_thread_report, "tm_thread_report");
    
    tm_thread_resume(pid_thread_report);

    tm_thread_resume(pid_thread_0);

    /* Resume just thread 1.  */
    tm_thread_resume(pid_thread_1);    
    
    #ifdef SEMAPHORES
    tm_semaphore_put(&tm_semaphore);    
    #endif //SEMAPHORES
}

/* Define the interrupt thread.  This thread is resumed from the 
   interrupt handler.  It runs and suspends.  */
void * tm_interrupt_preemption_thread_0_entry(void * arg)
{
    (void)arg;
    #ifdef SEMAPHORES
  	int status;
    status = tm_semaphore_get(&tm_semaphore);
    if (status != TM_SUCCESS)
        return 0;
    #endif //SEMAPHORES
    while(1)
    {
        /* Increment this thread's counter.  */
        tm_interrupt_preemption_thread_0_counter++;


        #ifdef SEMAPHORES
        status = tm_semaphore_get(&tm_semaphore);
        if (status != TM_SUCCESS)
            return 0;
        #else
        /* Suspend. This will allow the thread generating the 
           interrupt to run again.  */
        tm_thread_suspend();
        #endif //SEMAPHORES
    }
}

/* Define the thread that generates the interrupt.  */
void * tm_interrupt_preemption_thread_1_entry(void * arg)
{
    (void)arg;

    while(1)
    {
        /* Force an interrupt. */
	 
		gpio_clear(OUT_PIN);
	  	
		thread_yield_higher();
      

        /* Increment this thread's counter.  */
        tm_interrupt_preemption_thread_1_counter++;
    }
}


/* Define the interrupt handler.  This must be called from the RTOS trap handler.
   To be fair, it must behave just like a processor interrupt, i.e. it must save
   the full context of the interrupted thread during the preemption processing. */
   
/* void  tm_interrupt_preemption_handler(void)  */
 void tm_interrupt_preemption_handler(void * arg)        /* This is Cortex-M specific  */
{
    (void)arg;
    /* Clear the output pin in order to enebla future interrupts from thread 0  */

	  
	    gpio_set(OUT_PIN);
     //*/  /* Increment the interrupt count.  */	
	tm_interrupt_preemption_handler_counter++; 
	
    #ifdef SEMAPHORES
    tm_semaphore_put(&tm_semaphore);
    #else
    /* Resume the higher priority thread from the ISR.  */
    tm_thread_resume(pid_thread_0);
    #endif //SEMAPHORES
}


/* Define the interrupt test reporting thread.  */
void * tm_interrupt_preemption_thread_report(void * arg)
{
    (void)arg;
  uint32_t total;
//uint32_t relative_time;
  uint32_t last_total;
  uint32_t average;
  uint32_t counter[TM_TEST_CYCLES] = {0};
  uint32_t counter_index = 0;

  /* Initialize the last total.  */
  last_total =  0;

  /* Initialize the relative time.  */
//relative_time =  0;

    while(1)
    {
        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);

        /* Increment the relative time.  */
        // relative_time =  relative_time + TM_TEST_DURATION;
#ifdef xENABLE_PRINTF
        /* Print results to the stdio window.  */
        printf("**** Thread-Metric Interrupt Preemption Processing Test **** Relative Time: %lu\n", relative_time);
#endif
        /* Calculate the total of all the counters.  */
        total =  tm_interrupt_preemption_thread_0_counter + tm_interrupt_preemption_thread_1_counter + tm_interrupt_preemption_handler_counter;

        /* Calculate the average of all the counters.  */
        average =  total/3;

	
        /* See if there are any errors.  */
        if ((tm_interrupt_preemption_thread_0_counter < (average - 1)) || 
            (tm_interrupt_preemption_thread_0_counter > (average + 1)) ||
            (tm_interrupt_preemption_thread_1_counter < (average - 1)) || 
            (tm_interrupt_preemption_thread_1_counter > (average + 1)) ||
            (tm_interrupt_preemption_handler_counter < (average - 1)) || 
            (tm_interrupt_preemption_handler_counter > (average + 1)))
        {
          
#ifdef ENABLE_PRINTF
            printf("ERROR: Invalid counter value(s). Interrupt processing test has failed!\n");
#else
//            while(1)
//            {
//
//            }
#endif
        }
#ifdef ENABLE_PRINTF
        /* Show the total interrupts for the time period.  */
       // printf("Time Period Total:  %lu\n\n", tm_interrupt_preemption_handler_counter - last_total);
#endif
        
        /* Save the last total number of interrupts.  */
        counter[counter_index++] = total - last_total;
        last_total =  total;
		
		//printf("\n\n ------------------->>> ITERATION %ld <<<-------------------", counter_index);
		//printf("\ncounter0: %ld \ncounter1: %ld \ncounter_isr: %ld\n\n",tm_interrupt_preemption_thread_0_counter,tm_interrupt_preemption_thread_1_counter,tm_interrupt_preemption_handler_counter);

        
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
    }
}

#endif