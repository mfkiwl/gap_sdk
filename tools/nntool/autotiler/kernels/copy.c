/*
 * Copyright 2021 GreenWaves Technologies, SAS
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


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "nntool_extra_kernels.h"

static int CoreCountDynamic = 1;
static int ActiveCore = gap_ncore();

static inline unsigned int __attribute__((always_inline)) ChunkSize(unsigned int X)

{
	unsigned int NCore;
	unsigned int Log2Core;
	unsigned int Chunk;

	if (CoreCountDynamic) NCore = ActiveCore; else NCore = gap_ncore();
	Log2Core = gap_fl1(NCore);
	Chunk = (X>>Log2Core) + ((X&(NCore-1))!=0);
	return Chunk;
}

#define B_CLR(x, bits)	((x)&(~((1<<(bits))-1)))

// void CNN_Copy_fps(KerCopy_fps_T * Arg)
// {
// 	unsigned int Size = Arg->W * Arg->H;
// 	unsigned int CoreId = gap_coreid();
// 	unsigned int Chunk = ChunkSize(Size), First = Chunk*CoreId, Last = Min(First+Chunk, Size);
// 	unsigned int Iter = Max(0, Last-First);
// 	signed char *__restrict__ From = Arg->In;
// 	signed char *__restrict__ To = Arg->Out;

// 	int *pFrom = (int *) (From+First), *pTo = (int *) (To+First);
// 	for (int i=0; i<Iter/8; i++) {
// 		int V0 = pFrom[2*i], V1 = pFrom[2*i+1];
// 		pTo[2*i] = V0; pTo[2*i+1] = V1;
// 	}
// 	if (Iter & 0x4) *((int *) (To + First + B_CLR(Iter, 3))) = *((int *) (From + First + B_CLR(Iter, 3)));
// 	if (Iter & 0x2) *((short int *) (To + First + B_CLR(Iter, 2))) = *((short int *) (From + First + B_CLR(Iter, 2)));
// 	if (Iter & 0x1) *((signed char *) (To + First + Iter - 1)) = *((signed char *) (From + First + Iter - 1));
// }

#pragma GCC diagnostic pop
