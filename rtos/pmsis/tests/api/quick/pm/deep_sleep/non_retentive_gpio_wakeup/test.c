#include <stdio.h>
#include <pmsis.h>
#include "testbench.h"


#ifdef USE_HYPERFLASH
#include <bsp/flash/hyperflash.h>
#else
#include <bsp/flash/spiflash.h>
#endif

#define TB_FREQ 50000000
#define TB_GPIO_PULSE_CYCLES 150000
#define TB_GPIO_PULSE_WIDTH 2500


#if defined(__PLATFORM_RTL__)
#define NB_EVENTS 20
#else
#define NB_EVENTS 5
#endif


static pi_device_t bench;

void testbench_prepare_pads()
{
    struct pi_testbench_conf conf;
    pi_testbench_conf_init(&conf);

    conf.uart_id = TESTBENCH_UART_ID;
    conf.uart_baudrate = TESTBENCH_UART_BAUDRATE;

    pi_testbench_prepare_pads(&conf);

}

void testbench_exit(int status)
{
    static pi_device_t bench;
    struct pi_testbench_conf conf;
    pi_testbench_conf_init(&conf);

    conf.uart_id = TESTBENCH_UART_ID;
    conf.uart_baudrate = TESTBENCH_UART_BAUDRATE;

    pi_open_from_conf(&bench, &conf);

    if (pi_testbench_open(&bench))
        return;

    pi_testbench_set_status(&bench, status);
    while(1);
}



static PI_L2 char counts[NB_EVENTS];

static int flash_read(struct pi_device *flash)
{
    pi_flash_read(flash, 0x00080000, (void *)counts, sizeof(counts));
    for (int i=0; i<NB_EVENTS; i++)
    {
        if (counts[i] != 0)
            return i;
    }
}

static void flash_write(struct pi_device *flash, int count)
{
    counts[count-1] = 0;
    pi_flash_program(flash, 0x00080000, (void *)counts, sizeof(counts));
}


static int open_flash(struct pi_device *flash)
{
#ifdef USE_HYPERFLASH
    struct pi_hyperflash_conf flash_conf;
    pi_hyperflash_conf_init(&flash_conf);
#elif USE_MRAM
    struct pi_mram_conf flash_conf;
    pi_mram_conf_init(&flash_conf);
#else
#if defined(CONFIG_ATXP032)
    struct pi_atxp032_conf flash_conf;
    pi_atxp032_conf_init(&flash_conf);
#else
    struct pi_spiflash_conf flash_conf;
    pi_spiflash_conf_init(&flash_conf);
#endif
#endif

    pi_open_from_conf(flash, &flash_conf);

    if (pi_flash_open(flash))
        return -1;

    return 0;
}


int test_entry()
{
    struct pi_device flash;

    // Setup the Pads now to avoid triggering some random communication with the testbench
    testbench_prepare_pads();

    // And then release the outputs that we forced in case we come back from deep sleep
    pi_pad_sleep_cfg_force(0);

    if (open_flash(&flash))
        return -1;

    if (pi_pmu_boot_state_get() == PI_PMU_DOMAIN_STATE_OFF)
    {
        printf("STA\n");

        struct pi_testbench_conf tb_conf;

        pi_testbench_conf_init(&tb_conf);

        tb_conf.uart_id = TESTBENCH_UART_ID;
        tb_conf.uart_baudrate = TESTBENCH_UART_BAUDRATE;

        pi_open_from_conf(&bench, &tb_conf);

        if (pi_testbench_open(&bench))
            return -1;

        pi_testbench_gpio_pulse_gen(&bench, 42, 1, TB_GPIO_PULSE_CYCLES, TB_GPIO_PULSE_WIDTH, TB_GPIO_PULSE_CYCLES);

        pi_flash_erase(&flash, 0x00080000, 4);

        // Force outputs during deep sleep to avoid random communication
        pi_pad_sleep_cfg_force(1);

        pi_pmu_wakeup_control(PI_PMU_WAKEUP_GPIO | PI_PMU_WAKEUP_RTC, 1);

        pi_pmu_domain_state_change(PI_PMU_DOMAIN_CHIP, PI_PMU_DOMAIN_STATE_DEEP_SLEEP, 0);
    }
    else
    {
        pi_pmu_wakeup_control(0, 0);

        int count = flash_read(&flash);
        count++;

        #ifdef VERBOSE
        printf("Wakeup from deep sleep count=%d\n", count);
        #endif

        if (count == NB_EVENTS)
        {
            printf("TOK\n");
            testbench_exit(0);
            return 0;
        }

        flash_write(&flash, count);

        // Ref clock edge is 15 us, wait a different number of edges at each iteration
        pi_time_wait_us(((count % 10)+1)*500);

        //for (volatile int i=0; i<count*6000; i++);

        pi_pad_sleep_cfg_force(1);

        pi_pmu_wakeup_control(PI_PMU_WAKEUP_GPIO, 1);

        pi_pmu_domain_state_change(PI_PMU_DOMAIN_CHIP, PI_PMU_DOMAIN_STATE_DEEP_SLEEP, 0);
    }

    printf("TKO\n");
    testbench_exit(-1);
    return -1;
}

static void test_kickoff(void *arg)
{
    int ret = test_entry();
    pmsis_exit(ret);
}

int main()
{
    return pmsis_kickoff((void *) test_kickoff);
}
