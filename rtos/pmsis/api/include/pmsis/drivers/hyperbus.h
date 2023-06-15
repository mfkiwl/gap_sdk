/*
 * Copyright (C) 2019 GreenWaves Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PI_PMSIS_DRIVERS_HYPERBUS_H__
#define __PI_PMSIS_DRIVERS_HYPERBUS_H__

#include "pmsis/pmsis_types.h"

#if defined(__GAP9__)
#include "pmsis/drivers/aes_utils.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PI_INLINE_HYPER_LVL_0
#define PI_INLINE_HYPER_LVL_0
#endif

/**
* @ingroup groupDrivers
*/



/**
 * @defgroup Hyperbus Hyperbus
 *
 * The Hyperbus driver provides support for transferring data between an
 * external Hyperbus chip and the processor running this driver.
 *
 * This is a driver for the Hyperbus interface. Higher-level drivers can be
 * built on top of this one to target specific devices such as Hyperflash or
 * Hyperram. Please refer to the PMSIS BSP documentation for such drivers.
 *
 */

/**
 * @addtogroup Hyperbus
 * @{
 */

/**@{*/

/** \enum pi_hyper_type_e
 * \brief Type of the device connected to the hyperbus interface.
 *
 * This is used to know if the device is a flash or a RAM.
 */
typedef enum
{
    PI_HYPER_TYPE_FLASH,  /*!< Device is an Hyperflash. */
    PI_HYPER_TYPE_RAM     /*!< Device is an Hyperram. */
} pi_hyper_type_e;

/** \struct pi_hyper_conf
 * \brief Hyperbus configuration structure.
 *
 * This structure is used to pass the desired Hyperbus configuration to the
 * runtime when opening the device.
 */
typedef struct pi_hyper_conf
{
    pi_device_e device;  /*!< Interface type. */
    signed char id;      /*!< Hyperbus interface where the device is connected.
     */
#if defined(__GAP9__)
    uint8_t xip_en;      /*!< Specify whether xip is on */
    pi_aes_utils_conf_t* aes_conf; /*!< pointer to the AES configuration for
                                        on-the-fly encryption/decryption */
#endif
    uint32_t cs;         /*!< Chip select where the device is connected. */
    pi_hyper_type_e type;/*!< Type of device connected on the hyperbus
    interface. */
    uint32_t baudrate;   /*!< Baudrate (in bytes/second). */
    int32_t burst_length; /*< Maximum burst length in ns. */
    int latency;          /*< Number of latency cycles during read and write
    requests. */
}pi_hyper_conf_t;

/** \brief Hyperbus cluster request structure.
 *
 * This structure is used by the runtime to manage a cluster remote copy with
 * the Hyperbus. It must be instantiated once for each copy and must be kept
 * alive until the copy is finished. It can be instantiated as a normal
 * variable, for example as a global variable, a local one on the stack,
 * or through a memory allocator.
 */
typedef struct pi_cl_hyper_req_s pi_cl_hyper_req_t;

/** \brief Initialize an Hyperbus configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the
 * device is opened.
 *
 * \param conf A pointer to the Hyperbus configuration.
 */
void pi_hyper_conf_init(struct pi_hyper_conf *conf);

/** \brief Open an Hyperbus device.
 *
 * This function must be called before the Hyperbus device can be used.
 * It will do all the needed configuration to make it usable and initialize
 * the handle used to refer to this opened device when calling other functions.
 *
 * \param device    A pointer to the device structure of the device to open.
 *   This structure is allocated by the called and must be kept alive until the
 *   device is closed.
 * \return          0 if the operation is successfull, -1 if there was an error.
 */
int32_t pi_hyper_open(pi_device_t *device);

/** \brief Close an opened Hyperbus device.
 *
 * This function can be called to close an opened Hyperbus device once it is
 * not needed anymore, in order to free all allocated resources. Once this
 * function is called, the device is not accessible anymore and must be opened
 * again before being used.
 *
 * \param device    The device structure of the device to close.
 */
void pi_hyper_close(pi_device_t *device);

/** IOCTL command */
enum pi_hyper_ioctl_cmd
{
    /** @brief Set the device latency.
     *
     * This command can be used when the interface has been opened to configure
     * the latency suitable for the device.
     */
    PI_HYPER_IOCTL_SET_LATENCY,
#if defined(__GAP9__)
    /** @brief Enable AES
     *
     * This command can be used to enable/disable the on-the-fly AES at runtime
     */
    PI_HYPER_IOCTL_ENABLE_AES,
#endif
};

/**
 * \brief Dynamically change the device configuration.
 *
 * This function can be called to change part of the device configuration after
 * it has been opened or to control it.
 *
 * \param device  A pointer to the structure describing the device.
 * \param cmd      The command which specifies which parameters of the driver
 *   to modify and for some of them also their values. The command must be one
 *   of those defined in pi_hyper_ioctl_e.
 * \param arg       An additional value which is required for some parameters
 *   when they are set.
 */
int pi_hyper_ioctl(pi_device_t *device, uint32_t cmd, void *arg);

/** \brief Enqueue a read copy to the Hyperbus (from Hyperbus to processor).
 *
 * The copy will make a transfer between the Hyperbus and one of the processor
 * memory areas.
 * The caller is blocked until the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_read(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size);

/** \brief Enqueue an asynchronous read copy to the Hyperbus (from Hyperbus
 * to processor).
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param task        The task used to notify the end of transfer.
   See the documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_read_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_task_t *task);

/** \brief Enqueue a write copy to the Hyperbus (from processor to Hyperbus).
 *
 * The copy will make a transfer between the Hyperbus and one of the processor
 * memory areas.
 * The caller is blocked until the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_write(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size);

/** \brief Enqueue an asynchronous write copy to the Hyperbus (from processor
 * to Hyperbus).
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param task        The task used to notify the end of transfer. See the
 *   documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_write_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_task_t *task);

/** \brief Enqueue a 2D read copy (rectangle area) to the Hyperbus (from
 * Hyperbus to processor).
 *
 * The copy will make a transfer between the Hyperbus and one of
 * the processor memory areas.
 * The caller is blocked until the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_read_2d(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length);

/** \brief Enqueue an asynchronous 2D read copy (rectangle area) to the
 * Hyperbus (from Hyperbus to processor).
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 * the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param task        The task used to notify the end of transfer. See the
 * documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_read_2d_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, pi_task_t *task);

/** \brief Enqueue a 2D write copy (rectangle area) to the Hyperbus (from
 * processor to Hyperbus).
 *
 * The copy will make a transfer between the Hyperbus and one of the processor
 * memory areas.
 * The caller is blocked until the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 * the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_write_2d(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length);

/** \brief Enqueue an asynchronous 2D write copy (rectangle area) to the
 * Hyperbus (from processor to Hyperbus).
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param task        The task used to notify the end of transfer. See the
 *   documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_write_2d_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, pi_task_t *task);

/** \brief Enqueue an asynchronous copy to the Hyperbus.
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * The transfer can be either a read or a write.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 * The hyper address specified with this function must be added the offset
 * returned by pi_hyper_offset.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus, which must contain the hyper offset.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param ext2loc     1 if the copy is from HyperBus to the chip or 0 for the
 *   contrary.
 * \param task        The task used to notify the end of transfer. See the
 *   documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_copy_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, int ext2loc,
  pi_task_t *task);

/** \brief Enqueue an asynchronous 2D copy (rectangle area) to the Hyperbus.
 *
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A task must be specified in order to specify how the caller should be
 * notified when the transfer is finished.
 * The transfer can be either a read or a write.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 * The hyper address specified with this function must be added the offset
 * returned by pi_hyper_offset.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus, which must contain the hyper offset.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param ext2loc     1 if the copy is from HyperBus to the chip or 0 for the
 *   contrary.
 * \param task        The task used to notify the end of transfer. See the
 *   documentation of pi_task_t for more details.
 */
PI_INLINE_HYPER_LVL_0 void pi_hyper_copy_2d_async(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, int ext2loc, pi_task_t *task);

/** \brief Enqueue a read copy to the Hyperbus from cluster side (from Hyperbus
 * to processor).
 *
 * This function is a remote call that the cluster can do to the
 * fabric-controller in order to ask for an HyperBus read copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_read(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyper_req_t *req);

/** \brief Enqueue a 2D read copy (rectangle area) to the Hyperbus from cluster
 * side (from Hyperbus to processor).
 *
 * This function is a remote call that the cluster can issue to the
 * fabric-controller in order to ask for an HyperBus read copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy.
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_read_2d(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, pi_cl_hyper_req_t *req);

/** \brief Wait until the specified hyperbus request has finished.
 *
 * This blocks the calling core until the specified cluster remote copy is
 * finished.
 *
 * \param req       The request structure used for termination.
 */
static inline void pi_cl_hyper_read_wait(pi_cl_hyper_req_t *req);

/** \brief Enqueue a write copy to the Hyperbus from cluster side (from
 * Hyperbus to processor).
 *
 * This function is a remote call that the cluster can issue to the
 * fabric-controller in order to ask for an HyperBus write copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_write(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyper_req_t *req);

/** \brief Enqueue a 2D write copy (rectangle area) to the Hyperbus from
 * cluster side (from Hyperbus to processor).
 *
 * This function is a remote call that the cluster can issue to the
 * fabric-controller in order to ask for an HyperBus write copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_write_2d(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, pi_cl_hyper_req_t *req);

/** \brief Wait until the specified hyperbus request has finished.
 *
 * This blocks the calling core until the specified cluster remote copy is
 * finished.
 *
 * \param req       The request structure used for termination.
 */
static inline void pi_cl_hyper_write_wait(pi_cl_hyper_req_t *req);

/** \brief Enqueue a copy with the Hyperbus from cluster side.
 *
 * This function is a remote call that the cluster can issue to the
 * fabric-controller in order to ask for an HyperBus copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param ext2loc     1 if the copy is from HyperBus to the chip or 0 for the
 *   contrary.
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_copy(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, int ext2loc,
  pi_cl_hyper_req_t *req);

/** \brief Enqueue a 2D copy (rectangle area) with the Hyperbus from cluster
 * side.
 *
 * This function is a remote call that the cluster can issue to the
 * fabric-controller in order to ask for an HyperBus copy.
 * The copy will make an asynchronous transfer between the Hyperbus and one of
 * the processor memory areas.
 * A pointer to a request structure must be provided so that the runtime can
 * properly do the remote call.
 * Depending on the chip, there may be some restrictions on the memory which
 * can be used. Check the chip-specific documentation for more details.
 *
 * \param device      The device descriptor of the Hyperbus chip on which to do
 *   the copy.
 * \param hyper_addr  The address of the copy in the Hyperbus.
 * \param addr        The address of the copy in the processor.
 * \param size        The size in bytes of the copy
 * \param stride      2D stride, which is the number of bytes which are added
 *   to the beginning of the current line to switch to the next one.
 * \param length      2D length, which is the number of transferred bytes after
 *   which the driver will switch to the next line.
 * \param ext2loc     1 if the copy is from HyperBus to the chip or 0 for the
 *   contrary.
 * \param req         A pointer to the HyperBus request structure. It must be
 *   allocated by the caller and kept alive until the copy is finished.
 */
static inline void pi_cl_hyper_copy_2d(pi_device_t *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride,
  uint32_t length, int ext2loc, pi_cl_hyper_req_t *req);


/** \brief Forbid XIP refills
 *
 * This function can be called to prevent the hyperbus from triggering any
 * XIP refill transfer. This can be used to do an operation in a device which
 * would make an XIP refill fail, like an erase operation.
 * Be careful that locking XIP refills can lead to a deadlock if XIP code is
 * executed so only local code must be execyted when the XIP refill
 * is locked.
 * This will only apply to the new transfer enqueued after calling this
 * function, not to the pending transfers enqueued before.
 *
 * \param device    The device structure of the device to close.
 */
void pi_hyper_xip_lock(pi_device_t *device);


/** \brief Allow XIP refills
 *
 * This function can be called to allow again XIP refills after they have been
 * forbidden.
 *
 * \param device    The device structure of the device to close.
 */
void pi_hyper_xip_unlock(pi_device_t *device);

/** \brief Return hyperbus address offset
 *
 * Some functions of the API require that an offset is added to the hyperbus address.
 * This functions can be called to retrieve the offset to be applied.
 * See the documentation of each function to see which ones need this offset.
 *
 * \param device    The device structure of the device.
 */
PI_INLINE_HYPER_LVL_0 uint32_t pi_hyper_offset(struct pi_device *device);


//!@}

/**
 * @} end of Hyperbus
 */

/// @cond IMPLEM

/* DEPRECATED, don't use these API below */

typedef struct pi_cl_hyperram_alloc_req_s pi_cl_hyperram_alloc_req_t;

typedef struct pi_cl_hyperram_free_req_s pi_cl_hyperram_free_req_t;

uint32_t pi_hyperram_alloc(pi_device_t *device, uint32_t size);

int32_t pi_hyperram_free(pi_device_t *device, uint32_t chunk,
  uint32_t size);

void pi_cl_hyperram_alloc(pi_device_t *device, uint32_t size,
  pi_cl_hyperram_alloc_req_t *req);

void pi_cl_hyperram_free(pi_device_t *device, uint32_t chunk,
  uint32_t size, pi_cl_hyperram_free_req_t *req);

void pi_hyper_flash_erase(pi_device_t *device, uint32_t hyper_addr);

void pi_hyper_flash_write(pi_device_t *device, uint32_t hyper_addr,
                          void *addr, uint32_t size);

void pi_hyper_flash_read(pi_device_t *device, uint32_t hyper_addr,
                         void *addr, uint32_t size);

void pi_hyper_flash_sync(pi_device_t *device);

/* DEPRECATED, don't use these API above */



/// @endcond

#ifdef __cplusplus
}
#endif
#endif  /* __PI_PMSIS_DRIVERS_HYPERBUS_H__ */
