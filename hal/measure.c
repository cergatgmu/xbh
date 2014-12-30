#include <inttypes.h>
#include <stdbool.h>

#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include "hal/measure.h"
#include "hal/isr_prio.h"

#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>
#include <task.h>


// Static variables 

/** Sampling task handle */
TaskHandle_t pwr_sample_task_handle;

// Timer variables/*{{{*/
static volatile uint32_t wrap_cnt;
static volatile uint32_t wrap_cap_cnt;
static volatile uint64_t t_start;
static volatile uint64_t t_stop;
static volatile uint32_t cap_cnt;
/*}}}*/



// Execution timer stuff/*{{{*/

/**
 * Sets up execution timer
 */
void exec_timer_setup(void){/*{{{*/
    MAP_IntPrioritySet(INT_TIMER2A, TIMER_CAP_ISR_PRIO);
    MAP_IntPrioritySet(INT_TIMER2B, TIMER_WRAP_ISR_PRIO);

    // PM0 and PM1 are attached to timer2
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER2);
    MAP_GPIOPinConfigure(GPIO_PM0_T2CCP0);
    
    // Split needed for timer capture
    // We use timer A for capture and timer B to count wraparounds of timer A
    MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_CAP_TIME_UP|TIMER_CFG_B_PERIODIC_UP);

    // Synchronize both counters
    MAP_TimerSynchronize(TIMER0_BASE, TIMER_2A_SYNC|TIMER_2B_SYNC);

    // Detect both rising and falling edges
    MAP_TimerControlEvent(TIMER2_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

}/*}}}*/

/**
 * Starts execution timer, enabling interrupts
 */
void exec_timer_start(void){/*{{{*/
    wrap_cnt = 0;
    t_start= 0;
    t_stop= 0;
    cap_cnt = 0;
    exec_started = false;
    MAP_TimerIntEnable(TIMER2_BASE, TIMER_CAPA_EVENT);
    MAP_TimerIntEnable(TIMER2_BASE, TIMER_TIMB_TIMEOUT);
    MAP_TimerEnable(TIMER2_BASE, TIMER_BOTH);
}/*}}}*/

/**
 * Records time that execution started and stopped, triggered by rising/falling
 * edge on timer capture pin
 * Not atomic, since can be interrupted by wrap_isr
 */
void exec_timer_cap_isr(void){/*{{{*/
    uint32_t wrap_cnt_snap;
    uint32_t wrap_cap_cnt_snap;
    uint32_t cap_time;

    TimerIntClear(TIMER2_BASE, TIMER_CAPA_EVENT);
    //Disable interrupts so snap and time read are atomic
    IntMasterDisable();{
        wrap_cnt_snap = wrap_cnt;
        wrap_cap_cnt_snap = wrap_cap_cnt;
        cap_time = MAP_TimerValueGet(TIMER2_BASE, TIMER_A);
    } IntMasterEnable();
    

    // If cap count when wrap happens equal to timer A value when wrap_cnt was incremented was equal to cap timer, then
    // wrap must have been triggered during or after cap timer
    if(wrap_cap_cnt_snap == cap_cnt){
        // If capture time was shortly before wraparound point, let's say halfway
        // through counter (0xFFFF/2), than wraparound probably happened after
        // and thus decrement to compensate
        if(cap_time > (0xFFFF >> 1)){
            --wrap_cnt_snap;
        }  
    }


    if(cap_cnt == 0){
        t_start = (wrap_cnt_snap << 16) | cap_time;
    }else if (cap_cnt == 1){
        t_stop = (wrap_cnt_snap << 16) | cap_time;
        MAP_TimerDisable(TIMER2_BASE, TIMER_BOTH);
    }

    IntMasterDisable();{
        ++cap_cnt;
    } IntMasterEnable();


}/*}}}*/

/**
 * Counts how many times capture timer wraps around
 * This ISR should be of higher priority than exec_timer_cap_isr 
 */
void exec_timer_wrap_isr(void){/*{{{*/
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);
    
    ++wrap_cnt; 
    wrap_cap_cnt = cap_cnt;
}/*}}}*/
/*}}}*/

// Power measurement stuff 

void pwr_sample_timer_isr(void){
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

void pwr_sample_task(void *args){
    uint32_t wrap_cnt_snap;
    uint32_t wrap_time_snap; 
    
    while(1){
        //Disable interrupts so we get both values together atomically
        //Really only need to disable exec_timer_cap and exec_timer_wrap, but we
        //kill all just to be sure. Plus cpsid/cpsie is faster than masking
        //individual interrupts
        IntMasterDisable();
        wrap_cnt_snap = wrap_cnt;
        wrap_cnt_snap = wrap_time;
        IntMasterEnable();
    }


}

