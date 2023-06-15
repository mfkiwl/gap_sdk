/*
 * Copyright (C) 2020  GreenWaves Technologies, SAS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#pragma once

#include <vp/vp.hpp>
#include "../udma_impl.hpp"
#include <vp/itf/io.hpp>
#include <vp/itf/qspim.hpp>
#include <vp/itf/uart.hpp>
#include <vp/itf/cpi.hpp>
#include <vp/itf/wire.hpp>
#include <vp/itf/hyper.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <udma_hyper/udma_hyper_regs.h>
#include <udma_hyper/udma_hyper_regfields.h>
#include <udma_hyper/udma_hyper_gvsoc.h>
#include "../udma_mem_refill.hpp"

typedef enum
{
    HYPER_STATE_IDLE,
    HYPER_STATE_DELAY,
    HYPER_STATE_CS,
    HYPER_STATE_CA,
    HYPER_STATE_DATA,
    HYPER_STATE_CS_OFF,
} hyper_state_e;

typedef enum
{
    HYPER_CHANNEL_STATE_IDLE,
    HYPER_CHANNEL_STATE_SEND_ADDR,
    HYPER_CHANNEL_STATE_SEND_SIZE,
    HYPER_CHANNEL_STATE_SEND_LENGTH,
    HYPER_CHANNEL_STATE_SEND_CFG,
    HYPER_CHANNEL_STATE_STOP
} hyper_channel_state_e;

class Hyper_read_request
{
public:
    void set_next(Hyper_read_request *next) { this->next = next; }
    Hyper_read_request *get_next() { return next; }
    Hyper_read_request *next;

    uint32_t data;
    int size;
    int requested_size;
};

class Hyper_periph;

class Hyper_rx_channel : public Udma_rx_channel
{
public:
    Hyper_rx_channel(udma *top, Hyper_periph *periph, string name);

private:
    Hyper_periph *periph;
};

class Hyper_tx_channel : public Udma_tx_channel
{
    friend class Hyper_periph;

public:
    Hyper_tx_channel(udma *top, Hyper_periph *periph, string name);
    void push_data(uint8_t *data, int size);

protected:
private:
    Hyper_periph *periph;
};

class Hyper_periph : public Udma_periph
{
    friend class Hyper_tx_channel;
    friend class Hyper_rx_channel;

public:
    Hyper_periph(udma *top, int id, int itf_id);
    vp::io_req_status_e custom_req(vp::io_req *req, uint64_t offset);
    static void rx_sync(void *__this, int data);
    bool push_to_udma();
    void reset(bool active);
    static void refill_req(void *__this, udma_refill_req_t *req);
    static void handle_pending_word(void *__this, vp::clock_event *event);
    static void handle_check_state(void *__this, vp::clock_event *event);
    static void handle_pending_channel(void *__this, vp::clock_event *event);
    static void handle_push_data(void *__this, vp::clock_event *event);
    void check_state();
    void push_data(uint8_t *data, int size);

protected:
    vp::wire_slave<udma_refill_req_t *> refill_itf;
    vp::hyper_master hyper_itf;
    Hyper_tx_channel *tx_channel;
    Hyper_rx_channel *rx_channel;
    vp::wire_master<bool> irq_itf;

private:
    void trans_cfg_req(uint64_t reg_offset, int size, uint8_t *value, bool is_write);
    void enqueue_transfer(uint32_t ext_addr, uint32_t l2_addr, uint32_t transfer_size, uint32_t length, uint32_t stride, bool is_write, int address_space);
    void check_read_req_ready();

    vp_regmap_udma_hyper regmap;

    int eot_event;
    int pending_bytes;
    int nb_bytes_to_read;
    bool pending_is_write;
    vp::clock_event *pending_word_event;
    vp::clock_event *check_state_event;
    vp::clock_event *pending_channel_event;
    vp::clock_event *push_data_event;
    int64_t next_bit_cycle;
    vp::io_req *pending_req;

    uint32_t pending_word;
    bool pending_word_ready;
    int pending_word_size;

    Udma_queue<Hyper_read_request> *read_req_free;
    Udma_queue<Hyper_read_request> *read_req_ready;
    Udma_queue<Hyper_read_request> *read_req_waiting;

    std::queue<uint32_t> push_data_fifo_data;
    std::queue<int> push_data_fifo_size;
    std::queue<int64_t> push_data_fifo_cycles;

    bool iter_2d;
    int transfer_size;
    vp::reg_32 state;
    vp::reg_8 active;
    vp::reg_1 busy;
    hyper_channel_state_e channel_state;
    int delay;
    int ca_count;
    bool pending_tx;
    bool pending_rx;
    uint32_t pending_length;
    uint32_t length;
    uint32_t stride;
    uint32_t pending_burst;
    uint32_t l2_addr;
    uint32_t pending_ext_addr;
    uint32_t ext_addr;
    udma_refill_req_t *pending_refill_req;
    bool is_refill_req;
    int address_space;
    union
    {
        struct
        {
            unsigned int low_addr : 3;
            unsigned int reserved : 13;
            unsigned int high_addr : 29;
            unsigned int burst_type : 1;
            unsigned int address_space : 1;
            unsigned int read : 1;
        } __attribute__((packed));
        uint8_t raw[6];
    } ca;

    vp::trace trace;
};
