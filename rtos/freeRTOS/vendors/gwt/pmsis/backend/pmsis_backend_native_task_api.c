#include "pmsis/backend/pmsis_backend_native_types.h"
#include "pmsis.h"
#include "driver/gap_io.h"


void pi_task_timer_enqueue(struct pi_task *task, uint32_t delay_us);

/**
 * Kickoff the first "main" os task and scheduler
 */
int __os_native_kickoff(void *arg)
{
    BaseType_t xTask;
    TaskHandle_t xHandler0 = NULL;

    uint32_t stack_size = (uint32_t) MAIN_APP_STACK_SIZE;
    stack_size /= sizeof(configSTACK_DEPTH_TYPE);
    xTask = xTaskCreate(arg, "main", stack_size,
                        NULL, tskIDLE_PRIORITY + 1, &xHandler0);
    if (xTask != pdPASS)
    {
        printf("main is NULL !\n");
        pmsis_exit(-4321);
    }

    /* Enable IRQ for context switch. */
    NVIC_EnableIRQ(PENDSV_IRQN);

    __enable_irq();

    struct pmsis_event_kernel_wrap *wrap;
    pmsis_event_kernel_init(&wrap, pmsis_event_kernel_main);

    pmsis_event_set_default_scheduler(wrap);

    hal_compiler_barrier();


    extern SemaphoreHandle_t g_printf_mutex;
    g_printf_mutex = xSemaphoreCreateMutex();
    if (g_printf_mutex == NULL)
    {
        printf("Error : printf mutex not created !\n", g_printf_mutex);
        pmsis_exit(-4322);
    }
    /*
     * This should be used in case of printf via uart before scheduler has started.
     * Output will be on terminal instead of uart. After scheduler has started, output
     * will be via uart.
     */
    extern uint8_t g_freertos_scheduler_started;
    g_freertos_scheduler_started = 1;

    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    vTaskStartScheduler();

    hal_compiler_barrier();

    /* Exit FreeRTOS */
    return 0;
}

void pi_time_wait_us(int time_us)
{
    //time_us--;
    if (time_us > 0)
    {
#ifdef __GAP8__
        // correct the setup time if wait is short, -10 us ==> about 500 cycles max
        // also do not go below 100 us resolution

            uint32_t wait_time = time_us < 100 ? 100 : time_us;
            pi_task_t task_block;
            pi_task_block(&task_block);
            pi_task_timer_enqueue(&task_block, wait_time);
            pi_task_wait_on(&task_block);
#else
        /* Wait less than 1 ms. */
        if (time_us < 1000)
        {
#ifdef __VEGA__
            int irq = pi_irq_disable();
            for (volatile int i = 0; i < time_us; i++){};
            pi_irq_restore(irq);
#else
            pi_task_t task_block;
            pi_task_block(&task_block);
            extern struct pi_device sys_timer_hi_prec;
            pi_timer_task_add(&sys_timer_hi_prec, time_us, &task_block);
            pi_task_wait_on(&task_block);
#endif
        }
        else
        {
            pi_task_t task_block;
            pi_task_block(&task_block);
            pi_task_delayed_fifo_enqueue(&task_block, time_us);
            pi_task_wait_on(&task_block);
        }
#endif
    }
}

unsigned int pi_time_get_us()
{
    #if defined(__GAP8__) || defined (__VEGA__)
    uint64_t time = 0;
    uint32_t irq = pi_irq_disable();
    uint32_t cur_tick = xTaskGetTickCount();
    uint64_t cur_timer_val = pi_timer_value_read(SYS_TIMER);
    uint32_t freq_timer = system_core_clock_get();
    cur_timer_val = (cur_timer_val * 1000000) / freq_timer;
    time = (cur_tick * 1000);
    time += cur_timer_val;
    //time += 95; /* Around 95us between main() and kernel start. */
    pi_irq_restore(irq);
    return time;
    #else
    uint32_t time = 0;
    uint32_t irq = pi_irq_disable();
    uint32_t cur_timer_val = 0;
    extern struct pi_device sys_timer_hi_prec;
    int32_t status = pi_timer_current_value_read(&sys_timer_hi_prec, &cur_timer_val);
    uint32_t freq_timer = pi_timer_clock_freq_get(&sys_timer_hi_prec);
    time = ((float)cur_timer_val * 1000000) / freq_timer;
    //time += cur_timer_val;
    pi_irq_restore(irq);
    return time;
    #endif  /* __GAP8__ */
}

PI_FC_L1 struct pi_task_delayed_s delayed_task = {0};

void pi_task_delayed_fifo_enqueue(struct pi_task *task, uint32_t delay_us)
{
#if defined(__GAP9__)
    if (delay_us == 0)
    {
        /* task should be executed as soon as possible */
        pi_task_push(task);
    }
    else
    {
        /* put minimum delay to timer resolution */
        //FIXME use a defined value instead of hardcoded one
        delay_us = (delay_us < 100) ? 100 : delay_us;

        // All gap9 chips should be fine, as it just requires having two physical
        // timers
        extern struct pi_device sys_timer_hi_prec;
        pi_timer_task_add(&sys_timer_hi_prec, delay_us, task);
    }
#else
    task->data[8] = (delay_us/1000)/portTICK_PERIOD_MS;
    /* Add 1ms, in case the next SysTick IRQ is too close. This will ensure at
       least 1ms has passed. */
    task->data[8]++;
    task->next = NULL;
    if (delayed_task.fifo_head == NULL)
    {
        delayed_task.fifo_head = task;
    }
    else
    {
        delayed_task.fifo_tail->next = task;
    }
    delayed_task.fifo_tail = task;
#endif
}

#if defined(__GAP8__)
PI_FC_L1 struct pi_task_delayed_s timer_task = {0};

// TODO: use a proper define for ref clk (does not exist yet)
#define ref_clk_us (1000000/(32768/2))

void pi_task_timer_enqueue(struct pi_task *task, uint32_t delay_us)
{
    task->data[8] = ((delay_us)/ref_clk_us)
        + (((delay_us)%ref_clk_us) > 0);
    //printf("ticks: %i\n ref_clk_us: %i\n rem: %i\n",task->data[8]
    //        ,ref_clk_us
    //        ,(((delay_us)%ref_clk_us) > 0));
    task->next = NULL;
    if (delayed_task.fifo_head == NULL)
    {
        delayed_task.fifo_head = task;
        // IRQ might have been disabled due to no timer pending
        system_setup_timer();
        NVIC_ClearPendingIRQ(FC_IRQ_TIMER0_HI_EVT);
        NVIC_EnableIRQ(FC_IRQ_TIMER0_HI_EVT);
    }
    else
    {
        delayed_task.fifo_tail->next = task;
    }
    delayed_task.fifo_tail = task;
}


// push timer pi task to unlock
// --> No callback is allowed here, only timed waits
void __pi_task_timer_irq(void)
{
    struct pi_task *task = delayed_task.fifo_head;
    struct pi_task *prev_task = delayed_task.fifo_head;
    while (task != NULL)
    {
        task->data[8]--;
        if ((int32_t) task->data[8] <= 0)
        {
            if (task == delayed_task.fifo_head)
            {
                delayed_task.fifo_head = task->next;
            }
            else
            {
                prev_task->next = task->next;
            }
            // pi task is mandatorily a blocking task
            pi_task_release(task);
        }
        prev_task = task;
        task = task->next;
    }
    if(!delayed_task.fifo_head)
    {// no tasks at all --> disable irq
        pi_timer_stop(FC_TIMER_1);
        NVIC_DisableIRQ(FC_IRQ_TIMER0_HI_EVT);
    }
}
#endif  /* __GAP8__ */

// return value allows to skip some OS logic when a switch has already been triggered
int pi_task_delayed_increment_push(void)
{
    struct pi_task *task = delayed_task.fifo_head;
    struct pi_task *prev_task = delayed_task.fifo_head;
    int ret = 1;
    while (task != NULL)
    {
        task->data[8]--;
        if ((int32_t) task->data[8] <= 0)
        {
            if (task == delayed_task.fifo_head)
            {
                delayed_task.fifo_head = task->next;
            }
            else
            {
                prev_task->next = task->next;
            }
            __pi_irq_handle_end_of_task(task);
            ret = 0;
        }
        prev_task = task;
        task = task->next;
    }
    return ret;
}
