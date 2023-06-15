/*
 * Copyright (C) 2018 GreenWaves Technologies
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

#ifndef _CNN_AT_MISC_H_
#define _CNN_AT_MISC_H_

#define ALIGN(Value, Size)      ((((Value)+(1<<(Size))-1)/(1<<(Size)))*(1<<(Size)))

/*************************************************************************************************************************************************
        AT book keeping functions
*************************************************************************************************************************************************/

void AT_TileClear(
        char *__restrict__ In,          /**< Tile */
        int W,                          /**< Tile width */
        int H,                          /**< Tile height */
        int Feat,                       /**< Number of features */
        int Size,                       /**< Tile element size in bytes */
        int Pad,                        /**< Height or width of the area to be 0 padded */
        int PadValue,
        int Orientation,                /**< 0: Horizontal tile, 1: Vertical tile */
        int IsHWC                       /**< 0: CHW, 1: HWC */
        );

void AT_DumpTensor(
        char *NodeName,                 /**< Graph Node Name, a User Kernel */
        char *ArgName,                  /**< Argument name of this user kernel */
        int Loc,                        /**< Exec location if this argument, AT_MEM_xyz */
        void *L3_Device,                /**< Pointer to device descriptor in case Loc is external */
        void *L3_Event,                 /**< Pointer to a read event for this device descriptor if any */
        int ItemSize,                   /**< Data type size in bytes */
        int Dim,                        /**< Number of dimensions, up to 5, from D0 most outer to D4 most inner */
        int D0,                         /**< Actual value of this dimension if defined, 1 otherwise */
        int D1,                         /**< Actual value of this dimension if defined, 1 otherwise */
        int D2,                         /**< Actual value of this dimension if defined, 1 otherwise */
        int D3,                         /**< Actual value of this dimension if defined, 1 otherwise */
        int D4,                         /**< Actual value of this dimension if defined, 1 otherwise */
        void *L2_BufferAddr,            /**< In case exec loc is external pointer to a buffer in L2 to host partial copy of Arg */
        unsigned int L2_BufferSize,     /**< Size of this buffer */
        void *Addr                      /**< Address of Arg */
        );

void AT_ChecksumTensor(
        char *NodeName,
        char *ArgName,
        int Loc,
        void *L3_Device,
        void *L3_Event,
        int ItemSize,
        int Dim,
        int D0,
        int D1,
        int D2,
        int D3,
        int D4,
        void *L2_BufferAddr,
        unsigned int L2_BufferSize,
        void *Addr);

void AT_DumpTensorCompressed(
        char *NodeName,
        char *ArgName,
        int Loc,
        void *L3_Device,
        void *L3_Event,
        int LUTBits,
        int Sparse,
        int Dim,
        int D0,
        int D1,
        int D2,
        int D3,
        int D4,
        void *L2_BufferAddr,
        unsigned int L2_BufferSize,
        char *Addr,
        int *ItemAddresses);
#endif
