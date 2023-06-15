/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 */

#ifndef __MODEL_CREATE_H__
#define __MODEL_CREATE_H__

#include "AutoTilerLibTypes.h"
#include "AutoTilerLib.h"

/* Tiler internal */

extern int ByteConvert(int IsBit, int64_t X);

extern void SetSymbolDefaultNames();

extern Kernel_T *GetCurrentKernel();

extern Kernel_T *GetCurrentKernelGroup();

extern unsigned int KernelInstanceCount(
		NameT *KerName);

extern Kernel_T *UserKernelLookup(
		NameT *KerName);

extern int KernelArgIndexLookup(
		Kernel_T *Ker,
		NameT *KerArgName);

extern Kernel_Arg_T *KernelArgLookup(
		Kernel_T *Ker,
		NameT *KerArgName);

extern int KernelCArgIndexLookup(
		Kernel_T *Ker,
		NameT *CArgName);

extern CKernel_Arg_T *KernelCArgLookup(
		Kernel_T *Ker,
		NameT *CArgName);

extern int KernelCGroupArgIndexLookup(
		Kernel_T *Ker,
		NameT *CArgName);


extern KernelLib_T *KerLibLookup(
		NameT *KerName);

extern NameT **KernelArguments(
		int NameCount,
		...);

extern void CreateLibKernelTemplate();

extern char *StrToUpper(char *Str);

extern char *FileNamePrefix(char *Name);

extern int GetUserSymbolValue(NameT *Sym, int *Index);
extern UserSymbol_T *UserSymbolLookup(
	Kernel_T *Ker,
	NameT *Sym,
	int *Index);

extern void InitKernelIterInfos(int OldStyle);

extern CKernel_Arg_T *CNNGraphNodeArgAliasOfLookup(
	CNNGraph_T *Graph,
	GraphNode_T *Node,
	NameT *Name);

extern uint64_t GetCompressedSizeOf(Kernel_Arg_T *Arg, uint64_t Size, unsigned int ChunkSize, int FullSpace);
extern uint64_t GetCompressedSizeOfObj(Object_T *Arg, uint64_t Size, unsigned int ChunkSize, int FullSpace);

extern uint64_t MaybeCompressedKerArgSize(Kernel_Arg_T *Arg, uint64_t Size);
extern uint64_t MaybeCompressedKerArgSpaceSize(Kernel_Arg_T *Arg);

extern int IsKerArgRelated(
	KernelArgSelect_T Sel);

extern KernelIteratorDescrT *IsInArgFixedSpace(Kernel_T *Ker, Kernel_Arg_T *Arg, KernelIteratorT ItSpace);

extern KernelArgOneDimDescrT *ContainsTiledSpace(
	Kernel_T *Ker,
	KernelArgDimDescrT *K,
	Object_T *Obj,
	Kernel_Arg_T *Arg,
	KernelIteratorT *TiledSpace);

extern int IterSpaceIsDefined(
	Kernel_T *Ker,
	KernelIteratorT ItSpace);

/* Deprecated */
extern CKernel_Arg_T *CArg(
	char *ArgType,
	char *ArgName,
	unsigned int ArgRef,
	KernelArgSelect_T ArgSelect);

extern ArgBindingDescr_T *Bind(
	char *TargetArgName,
	ArgBindingT BindType,
	char *SourceArgName,
	KernelArgSelect_T ArgSelect);

extern ArgBindingDescr_T *BindImm(
	char *TargetArgName,
	int Value,
	KernelArgSelect_T ArgSelect);

extern Object_T *KerArgAliased(
	char *KerArgName,
	KernelArgDimDescrT *KerArgSpace,
	unsigned int Alias,
	Object_Type_T ObjType,
	unsigned int W,
	unsigned int H,
	unsigned int ItemSize,
	int TileOverlap,
	KernelArgConstraints_T Constraint,
	unsigned int PreferedTileSize,
	char *CArgName);

extern KernelGroup_T *GetKernelGroup(
	NameT *GroupName,
	int *GIndex);

extern void SetL3DeviceUsed(
	Kernel_T *Ker);

extern char *StrToUpper(
	char *Str);

extern int IsInTensorList(
	StackedTensors_T *List,
	int Count,
	NameT *Name);

extern int IsInListTensorList(
	StackedTensors_T *List,
	NameT *Name);

extern CKernel_Arg_T *StackedTensorsLookup(
	StackedTensors_T *List,
	NameT *Name);

extern StackedTensors_T *SetStackedInTensorSize(
	StackedTensors_T *List,
	NameT *Name,
	int Size);

extern void SetStackedInTensorsOffset(
	StackedTensors_T *List);

extern StackedTensors_T *GetStackedInTensorOffset(
	StackedTensors_T *List,
	NameT *Name,
	int *Offset,
	int MustExist);

extern int VaArgPointerArraySize(int Dim, va_list Va);

extern Kernel_Arg_T *ArgInGroup(Kernel_T *Ker, Kernel_Arg_T *Arg, int *IsGroupRep);
extern int ArgPosInGroup(Kernel_T *Ker, Kernel_Arg_T *Arg);

extern int LUTLen(KerArgLUT_T *LUT);

#endif
