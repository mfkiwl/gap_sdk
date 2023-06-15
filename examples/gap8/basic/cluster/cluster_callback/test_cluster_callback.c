/* PMSIS includes */
#include "pmsis.h"

/* Variables used. */
#define BUFFER_SIZE_PER_CORE ( 256 )

struct cl_args_s
{
    uint32_t size;
    uint8_t *l1_buffer;
    uint8_t *l2_in;
    uint8_t *l2_out;
};

PI_L2 static struct cl_args_s cl_arg;

/* Task executed by cluster cores. */
void cluster_dma(void *arg)
{
    struct cl_args_s *dma_args = (struct cl_args_s *) arg;
    uint8_t *l1_buffer = dma_args->l1_buffer;
    uint8_t *l2_in = dma_args->l2_in;
    uint8_t *l2_out = dma_args->l2_out;
    uint32_t buffer_size = dma_args->size;
    uint32_t coreid = pi_core_id(), start = 0, end = 0;

    /* Core 0 of cluster initiates DMA transfer from L2 to L1. */
    if (!coreid)
    {
        printf("Core %d requesting DMA transfer from l2_in to l1_buffer.\n", coreid);
        pi_cl_dma_copy_t copy;
        copy.dir = PI_CL_DMA_DIR_EXT2LOC;
        copy.merge = 0;
        copy.size = buffer_size;
        copy.id = 0;
        copy.ext = (uint32_t) l2_in;
        copy.loc = (uint32_t) l1_buffer;

        pi_cl_dma_memcpy(&copy);
        pi_cl_dma_wait(&copy);
        printf("Core %d : Transfer done.\n", coreid);
    }

    start = (coreid * (buffer_size / pi_cl_cluster_nb_pe_cores()));
    end = (start - 1 + (buffer_size / pi_cl_cluster_nb_pe_cores()));

    /* Barrier synchronisation before starting to compute. */
    pi_cl_team_barrier(0);
    /* Each core computes on specific portion of buffer. */
    for (uint32_t i=start; i<=end; i++)
    {
        l1_buffer[i] = (l1_buffer[i] * 3);
    }
    /* Barrier synchronisation to wait all cores. */
    pi_cl_team_barrier(0);

    /* Core 0 of cluster initiates DMA transfer from L1 to L2. */
    if (!coreid)
    {
        printf("Core %d requesting DMA transfer from l1_buffer to l2_out.\n", coreid);
        pi_cl_dma_copy_t copy;
        copy.dir = PI_CL_DMA_DIR_LOC2EXT;
        copy.merge = 0;
        copy.size = buffer_size;
        copy.id = 0;
        copy.ext = (uint32_t) l2_out;
        copy.loc = (uint32_t) l1_buffer;

        pi_cl_dma_memcpy(&copy);
        pi_cl_dma_wait(&copy);
        printf("Core %d : Transfer done.\n", coreid);
    }
}

/* Cluster main entry, executed by core 0. */
void master_entry(void *arg)
{
    printf("Cluster master core entry\n");
    /* Task dispatch to cluster cores. */
    pi_cl_team_fork(pi_cl_cluster_nb_pe_cores(), cluster_dma, arg);
    printf("Cluster master core exit\n");
}

void test_cluster_callback(void)
{
    printf("Entering main controller\n");
    uint32_t errors = 0;
    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;

    uint32_t nb_cl_pe_cores = pi_cl_cluster_nb_pe_cores();
    uint32_t buffer_size = nb_cl_pe_cores * (uint32_t) BUFFER_SIZE_PER_CORE;
    uint8_t *l2_in = pi_l2_malloc(buffer_size);
    if (l2_in == NULL)
    {
        printf("l2_in buffer alloc failed !\n");
        pmsis_exit(-1);
    }

    uint8_t *l2_out = pi_l2_malloc(buffer_size);
    if (l2_out == NULL)
    {
        printf("l2_out buffer alloc failed !\n");
        pmsis_exit(-2);
    }

    /* L2 Array Init. */
    for (uint32_t i=0; i<buffer_size; i++)
    {
        l2_in[i] = i;
        l2_out[i] = 0;
    }

    /* Init cluster configuration structure. */
    pi_cluster_conf_init(&conf);
    conf.id = 0;            /* Set cluster ID. */
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-3);
    }

    uint8_t *l1_buffer = pi_cl_l1_malloc(&cluster_dev, buffer_size);
    if (l1_buffer == NULL)
    {
        printf("l1_buffer alloc failed !\n");
        pi_cluster_close(&cluster_dev);
        pmsis_exit(-4);
    }

    /* Init arg struct. */
    cl_arg.size = buffer_size;
    cl_arg.l1_buffer = l1_buffer;
    cl_arg.l2_in = l2_in;
    cl_arg.l2_out = l2_out;

    /* Prepare cluster tasks and send them to cluster. */
    struct pi_cluster_task cl_task, cl_task2, cl_task3;
    pi_cluster_task(&cl_task, master_entry, &cl_arg);
    pi_cluster_task(&cl_task2, master_entry, &cl_arg);
    pi_cluster_task(&cl_task3, master_entry, &cl_arg);

    pi_task_t wait_task, wait_task2;
    pi_task_block(&wait_task);
    pi_task_block(&wait_task2);

    pi_cluster_send_task_to_cl_async(&cluster_dev, &cl_task, &wait_task);
    pi_cluster_send_task_to_cl_async(&cluster_dev, &cl_task2, &wait_task2);

    pi_task_wait_on(&wait_task);
    pi_task_wait_on(&wait_task2);
    /* Need to wait end of those two calls, FIFO can take only two calls. */

    #if defined(ASYNC)
    pi_task_t wait_task3;
    pi_task_block(&wait_task3);
    pi_cluster_send_task_to_cl_async(&cluster_dev, &cl_task3, &wait_task3);
    printf("Wait end of cluster task\n");
    pi_task_wait_on(&wait_task3);
    printf("End of cluster task\n");
    #else
    pi_cluster_send_task_to_cl(&cluster_dev, &cl_task3);
    #endif  /* ASYNC */

    pi_cl_l1_free(&cluster_dev, l1_buffer, buffer_size);
    pi_cluster_close(&cluster_dev);

    /* Verification. */
    for (uint32_t i=0; i<buffer_size; i++)
    {
        if (l2_out[i] != (uint8_t) (l2_in[i] * 3))
        {
            errors++;
            printf("%d) %x-%x ", i, l2_out[i], (uint8_t) (l2_in[i] * 3));
        }
    }

    pi_l2_free(l2_out, buffer_size);
    pi_l2_free(l2_in, buffer_size);

    printf("\nCluster callback done with %d error(s) !\n", errors);

    pmsis_exit(errors);
}

/* Program Entry. */
int main(void)
{
    printf("\n\n\t *** PMSIS Cluster Callback Test ***\n\n");
    return pmsis_kickoff((void *) test_cluster_callback);
}
