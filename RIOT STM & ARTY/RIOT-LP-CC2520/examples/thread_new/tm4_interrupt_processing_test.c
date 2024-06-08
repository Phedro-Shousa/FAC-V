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
/**   Interrupt Processing Test                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "periph/gpio.h"
#include <stdio.h>
#include "udp/udp-server.h"
#include "ps.h"
#include "sema.h"

#include "vendor/riscv_csr.h"

#if (THREADMETRIC == 4)
#define ENABLE_PRINTF

#define OUT_PIN (GPIO_PIN(0, 9))
#define IN_PIN (GPIO_PIN(0, 10))

/* Define the counters used in the demo application...  */
volatile uint32_t tm_interrupt_thread_0_counter;
volatile uint32_t tm_interrupt_handler_counter;

/* Define the semaphore...  */
sema_t tm_semaphore;

/* Define the test thread prototypes.  */
void * tm_interrupt_thread_0_entry(void * arg);

/* Define the reporting thread prototype.  */
void * tm_interrupt_thread_report(void * arg);

/* Define the interrupt handler.  This must be called from the RTOS.  */
void tm_interrupt_handler(void * arg);

/* Define the stacks for each thread.  */
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];

/*Defenition of thread id's*/
kernel_pid_t pid_thread_0 = 0;
kernel_pid_t pid_thread_report = 0;

/* Define the interrupt processing test initialization.  */

void tm_interrupt_processing_initialize(void)
{        
//    /* Initialize the output/input pins  */
  
    printf("  > Configuring output pin...\n");
    gpio_init(OUT_PIN, GPIO_OUT);
    gpio_set(OUT_PIN);
    
    printf("  > Configuring input pin and the interrupt...\n");

    gpio_init_int(IN_PIN, GPIO_IN, GPIO_FALLING, tm_interrupt_handler, NULL);  
 

    printf("  > Creating interrupt thread...\n");    
    /* Create thread that generates the interrupt at priority 10.  */
    pid_thread_0 = tm_thread_create(0, 8, tm_interrupt_thread_0_entry, stack_thread_0, "tm_thread_0");

    printf("  > Creating a semaphore...\n");     
    /* Create a semaphore that will be posted from the interrupt 
       handler.  */
    tm_semaphore_create(&tm_semaphore);

    printf("  > Creating the reporting thread...\n");     
    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(1, 7, tm_interrupt_thread_report, stack_thread_report, "tm_thread_report");

    printf("  > Resuming threads to start the test...\n");     
    tm_thread_resume(pid_thread_report);
    
    /* Resume just thread 0.  */
    tm_thread_resume(pid_thread_0);
    
    /* By default, the semaphore must be initialized to 1 by default.  */
    tm_semaphore_put(&tm_semaphore);
   
}

//#pragma required=irq_enable
//#pragma required=irq_disable
//#pragma optimize=none
/* Define the thread that generates the interrupt.  */
void* tm_interrupt_thread_0_entry(void * arg)
{
(void)arg;
int status;
    /* Pickup the semaphore since it is initialized to 1 by default. */
    status = tm_semaphore_get(&tm_semaphore);

    /* Check for good status.  */
    if (status != TM_SUCCESS)
        return 0;
	//printf("			> MS: Thread 0\n");
    while(1)
    {
      
        /* Force an interrupt.*/
	  	
		gpio_clear(OUT_PIN);  
	        
        /* We won't get back here until the interrupt processing is complete,
           including the setting of the semaphore from the interrupt 
           handler.  */

        /* Pickup the semaphore set by the interrupt handler. */
        status = tm_semaphore_get(&tm_semaphore);

        /* Check for good status.  */
        if (status != TM_SUCCESS)
            return 0;

        /* Increment this thread's counter.  */
        tm_interrupt_thread_0_counter++;
    }
}


/* void  tm_interrupt_preemption_handler(void)  */
void tm_interrupt_handler(void * arg)        /* This is Cortex-M specific  */
{
    /* Clear the output pin in order to enebla future interrupts from thread 0  */
    (void)arg;
    gpio_set(OUT_PIN);
     
    tm_interrupt_handler_counter++;   /* Increment the interrupt count.  */

   /* Put the semaphore from the interrupt handler.  */
    tm_semaphore_put(&tm_semaphore);
}

/* Define the interrupt test reporting thread.  */
//#pragma optimize=none
void* tm_interrupt_thread_report(void * arg)
{
	(void)arg;
volatile  uint32_t total;
volatile  uint32_t last_total;
//uint32_t relative_time;
volatile  uint32_t average;
volatile  uint32_t counter[TM_TEST_CYCLES] = {0};
volatile  uint32_t counter_index = 0;

    /* Initialize the last total.  */
  last_total =  0;

    /* Initialize the relative time.  */
//relative_time =  0;
	printf("			> MS: Thread Report\n");
    while(1)
    {
        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);

        /* Increment the relative time.  */
       // relative_time =  relative_time + TM_TEST_DURATION;
#ifdef xENABLE_PRINTF
        /* Print results to the stdio window.  */
        printf("**** Thread-Metric Interrupt Processing Test **** Relative Time: %lu\n", relative_time);
#endif
        /* Calculate the total of all the counters.  */
        total =  tm_interrupt_thread_0_counter + tm_interrupt_handler_counter;

        /* Calculate the average of all the counters.  */
        average =  total/2;

        //printf("\n\n ------------------->>> ITERATION %ld <<<-------------------", counter_index);
		//printf("\ncounter0: %ld \ncounter_isr: %ld\n\n", tm_interrupt_thread_0_counter, tm_interrupt_handler_counter);

        

        /* See if there are any errors.  */
        if ((tm_interrupt_thread_0_counter < (average - 1)) || 
            (tm_interrupt_thread_0_counter > (average + 1)) ||
            (tm_interrupt_handler_counter < (average - 1)) || 
            (tm_interrupt_handler_counter > (average + 1)))
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
       /* Save the last counter.  */
       counter[counter_index] = total - last_total;
	   counter_index++;
       last_total =  total;
       
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