/*
 * Copyright (c) 2013, Shanjin Yang.<sjyangv0@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Shanjin Yang.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the FOS.
 *
 * The latest version of FOS download by <https://github.com/yangshanjin/YSJ_OS>
 *
 */

#include <var_define.h>

TICK tick_queue;
ULONG fos_tick;

void tick_queue_init()
{
    list_init(&tick_queue.list);
}

void tick_queue_insert( TICK *tick )
{
    list_insert_behind(&tick_queue.list, &tick->list); 
}

void tick_queue_delete( TICK *tick )
{
    list_delete(&tick->list);
}

ULONG tick_get()
{
    return fos_tick;
}

void hardware_timer()
{
    hw_timer_clear_interrupt();

    U32 cpu = interrupt_disable();

    TICK *tick_tmp; 

    LIST *tmp = &tick_queue.list;

    fos_tick++;

    while ( !is_list_last(tmp) ) {
        
        tick_tmp = list_entry(tmp->next, TICK, list);
        tmp = tmp->next;
 
        if ( tick_tmp->timeout == TIMEOUT )
        {
            if ( tick_tmp->style == SOFTWARE_TIMER ){
  
                tick_tmp->func(tick_tmp->func_arg);
                
                if ( tick_tmp->period == CYCLE ){

                    tick_tmp->timeout = tick_tmp->timeout_copy;
                     
                    if ( is_list_last(tmp) ){

                        schedule();
                        interrupt_enable(cpu);
                        return;
                    }
                }
            }

            if ( tick_tmp->style == DELAY )
            {
                tick_tmp->tcb->state = CAN_RUNNING_STATE;

                /*Put the delay task to ready queue head*/ 
                prio_ready_queue_delete(tick_tmp->tcb);
                prio_ready_queue_insert_head(tick_tmp->tcb);

                if ( is_list_last(tmp) ){

                    tick_queue_delete(tick_tmp);
                    schedule();
                    interrupt_enable(cpu);
                    return;
                }
            }

            tick_queue_delete(tick_tmp);
        }

        (tick_tmp->timeout)--;
    } 

    schedule();
    interrupt_enable(cpu);
}

void timer_req( TICK *timer, FUNC_PTR func, U32 timeout, BOOL period, void *arg )
{
    U32 cpu = interrupt_disable();
    timer->func         = func;
    timer->timeout      = timeout;
    timer->period       = period;
    timer->timeout_copy = timeout;
    timer->style        = SOFTWARE_TIMER;
    timer->func_arg = arg;
    tick_queue_insert(timer);
    interrupt_enable(cpu);
}

void os_delay( U32  timeslice )
{   
    U32 cpu = interrupt_disable();
    TICK *timer_delay               = &new_task->delay;
    timer_delay->tcb                = new_task;
    timer_delay->tcb->state         = NON_RUNNING_STATE;
    timer_delay->timeout            = timeslice;
    timer_delay->timeout_copy       = timeslice;
    timer_delay->style              = DELAY;
  
    /*Put the delay task to ready queue tail*/ 
    prio_ready_queue_delete(timer_delay->tcb);
    prio_ready_queue_insert_tail(timer_delay->tcb);

    tick_queue_insert(timer_delay);
    schedule();
    
    interrupt_enable(cpu);
}

/*
 * Test softtimer for tick-queue function
 *
 * */
#if 0
void my_timer(void  *arg)
{
    os_printf("my_timer runing\n");
}

TICK my_timer_str;

void test_tick(void *arg)
{ 
    timer_req(&my_timer_str, my_timer, 100, 1); 
}
#endif
