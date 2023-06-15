/* PMSIS includes */
#include "pmsis.h"
#include "bsp/ram.h"
#include "bsp/ram/aps25xxxn.h"

/* Variables used. */
#define XIP_PAGE_SIZE    (3) //(XIP_PAGE_SIZE_4KB)
#define BUFFER_ALIGN_SIZE      ( 512 << XIP_PAGE_SIZE )
#define BUFFER_SIZE ( BUFFER_ALIGN_SIZE * 3)

#define XIP_NB_DCACHE_PAGE (1)

static uint8_t *buff, *rcv_buff;
static uint32_t octospi_buff = 0;
static struct pi_device ram = {0};
static struct pi_aps25xxxn_conf conf = {0};

static uint8_t *xip_buff = (uint8_t*)0x20000000;

/* -------- */
/* AES data */
/* -------- */

static uint8_t aes_key_ecb_128[16] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F,
};

static pi_aes_utils_conf_t aes_configurations[] = {
    {
        /* ECB 128 */
        .enabled = 1,
        .mode = PI_AES_MODE_ECB,
        .key_len = PI_AES_KEY_128,
        .key = (uint32_t*) aes_key_ecb_128,
        .iv = NULL,
    },
};

/* --------- */
/* Functions */
/* --------- */

void test_spi_ram(void)
{
    printf("Entering main controller\n");


    uint32_t errors = 0;

    buff = (uint8_t *) pmsis_l2_malloc((uint32_t) BUFFER_SIZE*2);
    if (buff == NULL)
    {
        printf("buff alloc failed !\n");
        pmsis_exit(-1);
    }

    rcv_buff = (uint8_t *) pmsis_l2_malloc((uint32_t) BUFFER_SIZE*2);
    if (rcv_buff == NULL)
    {
        printf("rcv_buff alloc failed !\n");
        pmsis_exit(-2);
    }

    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        buff[i] = i & 0xFF;
        rcv_buff[i] = 0;
    }

    /* Init & open ram. */
    pi_aps25xxxn_conf_init(&conf);
    (&conf)->xip_en = 1; // we want to use xip on this device
    (&conf)->baudrate = 50*1000*1000;
    (&conf)->spi_itf = 0;

    /* add aes to octospiram */
    memcpy(&(conf.ram.aes_conf), &aes_configurations[0], sizeof(pi_aes_utils_conf_t));

    printf("baudrate at octospi init %x\n",(&conf)->baudrate);
    pi_open_from_conf(&ram, &conf);
    if (pi_ram_open(&ram))
    {
        printf("Error ram open !\n");
        pmsis_exit(-3);
    }

    uint32_t octospi_buff_raw = 0;
    if (pi_ram_alloc(&ram, &octospi_buff_raw, (uint32_t) BUFFER_SIZE*2+BUFFER_ALIGN_SIZE))
    {
        printf("Ram malloc failed !\n");
        pmsis_exit(-4);
    }
    else
    {
        printf("Ram allocated : %lx %ld.\n", octospi_buff, (uint32_t) BUFFER_SIZE*2+BUFFER_ALIGN_SIZE);
    }
    octospi_buff = (octospi_buff_raw + BUFFER_ALIGN_SIZE) & (~(BUFFER_ALIGN_SIZE-1));
    printf("octospi_buff aligned = %x\n",octospi_buff);

    printf("Octospi write/read\n");
    /* Write a buffer in ram, then read back from ram. */
    pi_ram_write(&ram, octospi_buff, buff, (uint32_t) BUFFER_SIZE);
    printf("Write sync done.\n");
    pi_ram_read(&ram, octospi_buff, rcv_buff, (uint32_t) BUFFER_SIZE);
    printf("Read sync done.\n");

    /* Verification for normal read / write */
    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        if (buff[i] != rcv_buff[i])
        {
            errors++;
            printf("%2x-%2x ", buff[i], rcv_buff[i]);
        }
    }


    printf("Setting up XIP\n");
    uint32_t page_mask = 0;
    pi_device_t *xip_device = pmsis_l2_malloc(sizeof(pi_device_t));
    for( int i = 0; i<XIP_NB_DCACHE_PAGE; i++)
    {
        pi_xip_dcache_page_alloc(i, XIP_PAGE_SIZE);
        printf("dcache page addr= 0x%x\n",hal_xip_dcache_page_l2_addr_get(i));
    }
    pi_xip_free_page_mask_get(&page_mask,XIP_NB_DCACHE_PAGE);
    if(page_mask != (1<<XIP_NB_DCACHE_PAGE)-1)
    {
        printf("page mask = 0x%x / vs 0x%x\n",page_mask,(1<<XIP_NB_DCACHE_PAGE)-1);
    }
    pi_xip_conf_t xip_conf = {0};
    xip_conf.ro = 0;
    xip_conf.per_id = XIP_DEVICE_HYPER0;
    xip_conf.page_size = XIP_PAGE_SIZE;
    xip_conf.page_mask = page_mask;
    printf("xip page alloc done\n");

    uint32_t mount_size = BUFFER_SIZE / (512 << XIP_PAGE_SIZE);

    xip_device->config = &xip_conf;
    int ret = 0;
    if((ret =pi_xip_mount(xip_device,xip_buff,octospi_buff,mount_size,0)))
    {
        printf("xip open failed: %x\n",ret);
        pmsis_exit(-1);
    }
    printf("xip open success\n");

    /* Verification for xip read */
    for (uint32_t i=0; i<(uint32_t) BUFFER_SIZE; i++)
    {
        if (xip_buff[i] != rcv_buff[i])
        {
            errors++;
            printf("%i: %x - %x\n", i, xip_buff[i], rcv_buff[i]);
            pmsis_exit(-1);
        }
    }
    pi_xip_unmount(xip_device);

    pmsis_l2_malloc_free(buff, (uint32_t) BUFFER_SIZE);
    pmsis_l2_malloc_free(rcv_buff, (uint32_t) BUFFER_SIZE);
    pi_ram_free(&ram, octospi_buff_raw, (uint32_t) BUFFER_SIZE+BUFFER_ALIGN_SIZE);

    pi_ram_close(&ram);

    printf("\nOctospiRAM transfer done with %ld error(s) !\n", errors);
    printf("\nTest %s with %ld error(s) !\n", (errors) ? "failed" : "success", errors);

    pmsis_exit(errors);
}

/* Program Entry. */
int main(void)
{
    printf("\n\n\t *** PMSIS XIP-OctospiRAM-AES Test ***\n\n");
    return pmsis_kickoff((void *) test_spi_ram);
}
