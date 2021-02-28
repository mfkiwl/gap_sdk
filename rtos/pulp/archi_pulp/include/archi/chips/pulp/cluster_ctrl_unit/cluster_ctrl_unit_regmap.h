
/* THIS FILE HAS BEEN GENERATED, DO NOT MODIFY IT.
 */

/*
 * Copyright (C) 2020 GreenWaves Technologies
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

#ifndef __ARCHI_CLUSTER_CTRL_UNIT_REGMAP__
#define __ARCHI_CLUSTER_CTRL_UNIT_REGMAP__

#if !defined(LANGUAGE_ASSEMBLY) && !defined(__ASSEMBLER__)

#include <stdint.h>

#endif




//
// REGISTERS GLOBAL STRUCT
//

#if !defined(LANGUAGE_ASSEMBLY) && !defined(__ASSEMBLER__)

/** CLUSTER_CTRL_UNIT_Type Register Layout Typedef */
typedef struct {
    volatile uint32_t reserved_0[1];  // Reserved/Not used.
    volatile uint32_t fetch_en;  // Cluster cores fetch enable configuration register.
    volatile uint32_t reserved_1[5];  // Reserved/Not used.
    volatile uint32_t clock_gate;  // Cluster clock gate configuration register.
    volatile uint32_t reserved_2[7];  // Reserved/Not used.
    volatile uint32_t boot_addr0;  // Cluster core 0 boot address configuration register.
} __attribute__((packed)) cluster_ctrl_unit_t;

#endif  /* LANGUAGE_ASSEMBLY || __ASSEMBLER__ */

#endif
