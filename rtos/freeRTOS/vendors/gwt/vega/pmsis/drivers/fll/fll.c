/*
 * Copyright (c) 2018, GreenWaves Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include "pmsis.h"
#include "pmsis/implem/drivers/fll/fll.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * FreqOut = (Fref * Mult)/2^(Div-1)
 * With Mult on 16 bits and Div on 4 bits
 */

/* Maximum Log2(DCO Frequency) */
#define LOG2_MAXDCO     29
/* Maximum Log2(Clok Divider) */
#define LOG2_MAXDIV     15
/* Log2(FLL_REF_CLK=32768) */
#define LOG2_REFCLK     (ARCHI_FLL_REF_CLOCK_LOG2)
/* Maximum Log2(Multiplier) */
#define LOG2_MAXM       (LOG2_MAXDCO - LOG2_REFCLK)

#define FLL_REF_CLK     (1 << LOG2_REFCLK)

/*******************************************************************************
 * Driver data
 ******************************************************************************/

static volatile uint32_t g_fll_frequency[ARCHI_NB_FLL] = {0};

static pi_freq_cb_t *g_freq_cb = NULL;

/*******************************************************************************
 * Internal functions
 ******************************************************************************/

static uint32_t __pi_fll_mult_div_from_frequency_get(uint32_t freq, uint32_t *mult,
                                                     uint32_t *div)
{
    uint32_t fref = (uint32_t) FLL_REF_CLK;
    uint32_t Log2M = __FL1(freq) - __FL1(fref);
    uint32_t D =  __MAX(1, (LOG2_MAXM - Log2M)>>1);
    uint32_t M = ((freq << D) / fref);
    *mult = M;
    *div  = (D + 1);
    uint32_t fres = (((fref * M) + (1 << (D - 1))) >> D); /* Rounding. */
    //printf("Set freq: mult=%lx, div=%lx, real_freq=%ld, freq=%ld\n", *mult, *div, fres, freq);
    return fres;
}

static uint32_t __pi_fll_frequency_from_mult_div_get(uint32_t mult, uint32_t div)
{
    /* FreqOut = Fref * Mult/2^(Div-1)    With Mult on 16 bits and Div on 4 bits */
    uint32_t fref = (uint32_t) FLL_REF_CLK;
    uint32_t fres = ((fref * mult) >> (div - 1));
    //printf("Get freq: mult=%lx, div=%lx, real_freq=%ld\n", mult, div, fres);
    return fres;
}

/*******************************************************************************
 * Function implementation
 ******************************************************************************/

void pi_fll_init(uint8_t fll_id, uint32_t frequency)
{
    uint32_t mult = 0, div = 0;
    fll_ctrl_conf1_t conf1 = {0};
    fll_ctrl_conf2_t conf2 = {0};
    conf1.raw = hal_fll_conf1_get(fll_id);

    /* Special setting to assign periph FLL to periph domain and FC FLL to FC domain. */
    if (fll_id == FLL_ID_FC)
    {
        hal_soc_ctrl_clk_sel_set(FLL_ID_FC, FLL_ID_CL);
    }

    /* Don't set the gain and integrator in case it has already been set by the boot code */
    /* as it totally blocks the fll on the RTL platform. */
    /* The boot code is anyway setting the same configuration. */
    if (conf1.mode == 0)
    {
        conf2.raw = hal_fll_conf2_get(fll_id);
        //conf2.loop_gain = 0x7;
        //conf2.de_assert_cycles = 0x10;
        conf2.assert_cycles = 0x6;
        conf2.lock_tolerance = 0x50;
        //conf2.config_clock_sel = 0x0;
        //conf2.open_loop = 0x0;
        //conf2.dithering = 0x1;
        hal_fll_conf2_mask_set(fll_id, conf2.raw);

        /* We are in open loop, prime the fll forcing dco input, approx 50 MHz */
        /* Set int part to 332 */
        fll_ctrl_integrator_t integrator = { .raw = hal_fll_integrator_get(fll_id) };
        integrator.int_part = 332;
        hal_fll_integrator_set(fll_id, integrator.raw);

        /* Lock FLL. */
        conf1.output_lock_enable = 1;
        conf1.mode = 1;
        hal_fll_conf1_mask_set(fll_id, conf1.raw);
    }

    /* Set frequency. */
    pi_fll_frequency_set(fll_id, frequency, 0);
}

void pi_fll_frequency_value_update(uint8_t fll_id, uint32_t freq)
{
    g_fll_frequency[fll_id] = freq;
}

int32_t pi_fll_frequency_set(uint8_t fll_id, uint32_t frequency, uint8_t check)
{
    uint32_t irq =  __disable_irq();

    uint32_t mult = 0, div = 0;
    fll_ctrl_conf1_t conf1 = {0};

    /* Frequency calculation from theory */
    uint32_t real_freq = __pi_fll_mult_div_from_frequency_get(frequency, &mult, &div);
    conf1.mult_factor = mult;
    conf1.clock_out_divider = div;
    hal_fll_conf1_mult_div_update(fll_id, mult, div);

    real_freq = pi_fll_frequency_get(fll_id, 1);
    g_fll_frequency[fll_id] = frequency;

    if (fll_id == FLL_ID_FC)
    {
        system_core_clock_update(frequency);
        //pi_freq_callback_exec();
    }
    __restore_irq(irq);

    return real_freq;
}

uint32_t pi_fll_frequency_get(uint8_t fll_id, uint8_t real)
{
    uint32_t real_freq = 0;

    if (real)
    {
        /* Frequency calculation from real world */
        uint32_t mult = hal_fll_status_mult_factor_get(fll_id);
        uint32_t div = hal_fll_conf1_div_get(fll_id);
        real_freq = __pi_fll_frequency_from_mult_div_get(mult, div);
        /* printf("Get freq: domain=%d @ %lx mult=%lx, div=%lx, real_freq=%ld\n", */
        /*        fll_id, fll(fll_id), mult, div, real_freq); */
        /* Update Frequency */
        pi_fll_frequency_value_update(fll_id, real_freq);
    }
    else
    {
        real_freq = g_fll_frequency[fll_id];
    }
    return real_freq;
}

int pi_freq_callback_add(pi_freq_cb_t *cb)
{
    if (cb == NULL)
    {
        //printf("Error : callback is NULL !\n");
        return -1;
    }

    pi_freq_cb_t *temp_cb = g_freq_cb;
    if (g_freq_cb == NULL)
    {
        g_freq_cb = cb;
    }
    else
    {
        while (temp_cb->next != NULL)
        {
            temp_cb = temp_cb->next;
        }
        temp_cb->next = cb;
        cb->prev = temp_cb;
    }
    return 0;
}

int pi_freq_callback_remove(pi_freq_cb_t *cb)
{
    if (cb == NULL)
    {
        //printf("Error : callback is NULL !\n");
        return -1;
    }

    if (g_freq_cb == NULL)
    {
        //printf("Error : callback list is NULL !\n");
        return -2;
    }

    /* Callback at the head. */
    if (g_freq_cb == cb)
    {
        g_freq_cb = g_freq_cb->next;
        g_freq_cb->prev = NULL;
        return 0;
    }

    /* Callback in list. */
    pi_freq_cb_t *temp_cb = g_freq_cb;
    while ((temp_cb != cb) && (temp_cb->next != NULL))
    {
        temp_cb = temp_cb->next;
    }
    if (temp_cb != cb)
    {
        //printf("Error : callback not found !\n");
        return -3;
    }
    if (temp_cb->next != NULL)
    {
        temp_cb->next->prev = temp_cb->prev;
    }
    if (temp_cb->prev != NULL)
    {
        temp_cb->prev->next = temp_cb->next;
    }
    return 0;
}

void pi_freq_callback_exec(void)
{
    pi_freq_cb_t *temp_cb = g_freq_cb;
    while (temp_cb  != NULL)
    {
        temp_cb->function(temp_cb->args);
        temp_cb = temp_cb->next;
    }
}

void pi_freq_set_value(uint8_t fll_id, uint32_t freq)
{
    g_fll_frequency[fll_id] = freq;
}
