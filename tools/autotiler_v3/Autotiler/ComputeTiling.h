/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#ifndef __COMPUTE_TILING_H__
#define __COMPUTE_TILING_H__

#include "AutoTilerLibTypes.h"

extern unsigned int ObjectAllocSize(
	Object_T *Obj,
	int DoubleBuffer,
	int LastIter,
	int Rem);

extern Kernel_Arg_T **CreateKernelArguments(
	Kernel_T *Ker,
	Object_T **ObjList,
	unsigned int NObj,
	Kernel_Arg_T **KernelArgList,
	unsigned int *Rem);

extern void CreateKernelGroupArguments(
	Kernel_T *Ker,
	Object_T **ObjList,
	unsigned int NObj,
	Kernel_Arg_T **KernelArgList);


extern unsigned int ProcessKernelTiling(
	Kernel_T *Ker,
	Object_T **ObjList,
	unsigned int NObj,
	BasicObjectType_T IterSpace,
	uint64_t BaseMemory,
	uint64_t MemorySize,
	unsigned int *DimRem,
	uint64_t *L1TopMemory);

extern void PartitionMemory(
	Kernel_T *Ker,
	Object_T **ObjList,
	unsigned int NObj,
	uint64_t MemorySize,
	uint64_t *PartitionSize);

uint64_t L3_PartitionUsage(
	Kernel_T *Ker,
	Object_T **ObjList,
	unsigned int NObj,
	BasicObjectType_T IterSpace);

extern void ProcessL3_Arguments(
	unsigned int NArg,
	Kernel_Arg_T **KernelArgList,
	unsigned int *AllocatedL2);

extern int LastNonL1Space(
	Kernel_Arg_T *Arg);

extern KerIteratorParT *ArgParametricSpace(
	Kernel_T *Ker,
	Kernel_Arg_T *Arg,
	int DimOrder,
	KerIteratorParT **ParS1,
	KernelIteratorT *KerItSpace0,
	KernelIteratorT *KerItSpace1,
	int *MultFactor,
	KernelIteratorT *FullSpace);

extern KerIteratorParT *ArgParametricSpace1(
	Kernel_T *Ker,
	Kernel_Arg_T *Arg,
	int DimOrder,
	KerIteratorParT **ParS1,
	KernelArgOneDimDescrT **ArgD0,
	KernelArgOneDimDescrT **ArgD1,
	int *MultFactor,
	KernelIteratorT *FullSpace);

extern KernelArgOneDimDescrT *GetArgSubSpaceDescriptor(
	Object_T *Obj,
	Kernel_Arg_T *Arg,
	KernelIteratorT Space);

extern int NumberOfTiles(
	Kernel_T *Ker,
	Kernel_Arg_T *Arg,
	int *N_LogicalTiles,
	int Traversed);

extern int LastNonL1Space1(
	KernelArgDimDescrT *ArgSpace);

extern void DumpKerObjTileStructure(Kernel_T *Ker, Object_T **ObjList, unsigned int NObj);
extern void DumpKerArgTileStructure(Kernel_T *Ker);

extern int KerArgTileNDim(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg);
extern int EvalArgTileSize(Kernel_T *Ker, Kernel_Arg_T *Arg, int WhichD1, int WhichD0, int EvalMinBufferSize, int AlignPadding);
extern int EvalArgTileLength(Kernel_T *Ker, Kernel_Arg_T *Arg, int WhichD0);

extern void KerArgTileSize(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg, int64_t *Cst, KernelIteratorT *D0, KernelIteratorT *D1, int AfterBuffering);
extern void KerArgTileSizePadded(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg, int64_t *Cst, KernelIteratorT *D0, KernelIteratorT *D1, int AfterBuffering, int Pure);
extern void KerArgTileLen(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg, int64_t *Cst, KernelIteratorT *D0, int Pure);
extern void KerArgTileHeight(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg, int64_t *Cst, KernelIteratorT *D0);

extern int ArgIsPadded(Kernel_T *Ker, Object_T *Obj, Kernel_Arg_T *Arg, int *Pad);

extern int EvalArgAlignedSize(Kernel_T *Ker, Kernel_Arg_T *Arg, int Align);





#endif
