/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "AutoTilerLib.h"
#include "AutoTilerLibTypes.h"
#include "DSP_Generators.h"
#ifdef __FLOAT_EMUL__
#define F16_SIZE 4
#else
#define F16_SIZE 2
#endif

#define D0      KER_ITER_D0
#define D1      KER_ITER_D1
#define D2      KER_ITER_D2
#define D3      KER_ITER_D3
#define T0      KER_ITER_TILE0
#define T1      KER_ITER_TILE1
#define T2      KER_ITER_TILE2

#ifndef Abs
#define Abs(a) (a>0?a:-a)
#endif

void LoadMFCCLibrary()

{
	LibKernelTemplate("FFT_Arg_T",
			  CArgs(4,
			        TCArg("void * __restrict__", "Data"),
			        TCArg("void * __restrict__", "Twiddles"),
			        TCArg("short int"   , "N_fft"),
			        TCArg("unsigned char", "Inverse")
			        )
		);

	LibKernelTemplate("FFT_scal_Arg_T",
			  CArgs(5,
			        TCArg("void * __restrict__", "Data"),
			        TCArg("void * __restrict__", "Twiddles"),
			        TCArg("short int"   , "N_fft"),
			        TCArg("unsigned char", "Inverse"),
		       		TCArg("signed char * __restrict__", "shift_fft")
			        )
		);


	LibKernelTemplate("RFFT_Arg_T",
			  CArgs(7,
			        TCArg("void * __restrict__", "Data"),
			        TCArg("void * __restrict__", "RFFT_Out"),
			        TCArg("void * __restrict__", "Twiddles"),
			        TCArg("void * __restrict__", "RTwiddles"),
			        TCArg("void * __restrict__", "SwapTable"),
			        TCArg("short int"   , "N_fft"),
			        TCArg("unsigned char", "Inverse")
			        )
		);

	LibKernelTemplate("RFFT_scal_Arg_T",
			  CArgs(9,
			        TCArg("void * __restrict__", "Data"),
			        TCArg("void * __restrict__", "RFFT_Out"),
			        TCArg("void * __restrict__", "Twiddles"),
			        TCArg("void * __restrict__", "RTwiddles"),
			        TCArg("void * __restrict__", "SwapTable"),
			        TCArg("short int"   , "N_fft"),
			        TCArg("unsigned char", "Inverse"),
		       		TCArg("signed char * __restrict__", "shift_fft"),
		       		TCArg("signed char * __restrict__", "shift_rfft")
			        )
		);

	LibKernelTemplate("SwapSamples_Arg_T",
			  CArgs(3,
			        TCArg("void * __restrict__",      "Data"),
			        TCArg("short int * __restrict__", "SwapTable"),
			        TCArg("int", "Ni")
			        )
		);

	LibKernelTemplate("SwapSamples_scal_Arg_T",
			  CArgs(4,
			        TCArg("void * __restrict__",      "Data"),
			        TCArg("short int * __restrict__", "SwapTable"),
			        TCArg("int", "Ni"),
			        TCArg("signed char *__restrict__","shift_fft")
			        )
		);

	LibKernelTemplate("PreEmphasis_T",
			  CArgs(7,
				TCArg("void * __restrict__", "Frame"),
				TCArg("void * __restrict__", "Out"),
				TCArg("short int",           "Prev"),
				TCArg("short int",           "PreempFactor"),
				TCArg("int",                 "FrameSize"),
				TCArg("short int *",         "Shift"),
				TCArg("short int",	     "QIn_FFT")
				)
		);

	LibKernelTemplate("PreEmphasis_f16_T",
			  CArgs(5,
				TCArg("void * __restrict__", "Frame"),
				TCArg("void * __restrict__", "Out"),
				TCArg("F16_DSP",                 "Prev"),
				TCArg("F16_DSP",                 "PreempFactor"),
				TCArg("int",                 "FrameSize")
				)
		);

	LibKernelTemplate("PreEmphasis_f32_T",
			  CArgs(5,
				TCArg("void * __restrict__", "Frame"),
				TCArg("void * __restrict__", "Out"),
				TCArg("float",               "Prev"),
				TCArg("float",               "PreempFactor"),
				TCArg("int",                 "FrameSize")
				)
		);

	LibKernelTemplate("Windowing_T",
			  CArgs(5,
				TCArg("void *__restrict__", "Frame"),
				TCArg("void *__restrict__", "OutFrame"),
				TCArg("void *__restrict__", "Window"),
				TCArg("int", "FrameSize"),
				TCArg("int", "FFT_Dim")
				)
		);

	LibKernelTemplate("CmplxMag_T",
			  CArgs(6,
				TCArg("void *__restrict__", "FrameIn"),
				TCArg("void *__restrict__", "FrameOut"),
				TCArg("int", "Nfft"),
				TCArg("short int"         , "ExtraQ"),
				TCArg("short int"         , "Input_QFormat"),
		       		TCArg("signed char * __restrict__", "shift_fft")
				)
		);

	LibKernelTemplate("MelFilterBank_T",
			  CArgs(9,
				TCArg("void *__restrict__", "FramePower"),
				TCArg("void *__restrict__", "MelSpectr"),
				TCArg("void *__restrict__", "Mel_Coeffs"),
				TCArg("short int *__restrict__", "Mel_FilterBank"),
				TCArg("short int", "Mel_NBanks"),
				TCArg("short int", "Mel_Coeff_dyn"),
				TCArg("signed char", "IsMagSquared"),
				TCArg("signed char *__restrict__", "shift_buff"),
				TCArg("signed char *__restrict__", "shift_fft")
				)
		);

	LibKernelTemplate("MFCC_Log_T",
			  CArgs(10,
				TCArg("void *__restrict__", "FrameIn"),
				TCArg("void *__restrict__", "FrameOut"),
				TCArg("unsigned int", "FrameSize"),
				TCArg("unsigned short int", "Norm"),
				TCArg("short int", "ExtraQ"),
				TCArg("short int", "Q_FFT_Out"),
				TCArg("short int", "Mel_Coeff_Dyn"),
				TCArg("signed char", "IsMagSquared"),
				TCArg("signed char *__restrict__", "shift_buff"),
				TCArg("int", "LogOffset")
				)
		);

	LibKernelTemplate("MFCC_LogF_T",
			  CArgs(10,
				TCArg("void *__restrict__", "FrameIn"),
				TCArg("void *__restrict__", "FrameOut"),
				TCArg("unsigned int", "FrameSize"),
				TCArg("unsigned short int", "Norm"),
				TCArg("short int", "ExtraQ"),
				TCArg("short int", "Q_FFT_Out"),
				TCArg("short int", "Mel_Coeff_Dyn"),
				TCArg("signed char", "IsMagSquared"),
				TCArg("signed char *__restrict__", "shift_buff"),
				TCArg("float", "LogOffset")
				)
		);

	LibKernelTemplate("Norm_Clip_args_T",
			  CArgs(4,
				TCArg("int *__restrict__", "In"),
				TCArg("short int *__restrict__", "Out"),
				TCArg("unsigned shor	t int", "Norm"),
				TCArg("int", "N")
				)
		);

	LibKernelTemplate("MFCC_Clip_32_T",
			  CArgs(8,
		  		TCArg("unsigned int * __restrict__", "In"),
				TCArg("unsigned int * __restrict__", "Out"),
				TCArg("unsigned int"   , "N"),
				TCArg("short int"   , "ExtraQ"),
				TCArg("short int"   , "Q_FFT_Out"),
				TCArg("short int"   , "Mel_Coeff_Dyn"),
				TCArg("signed char" , "IsMagSquared"),
				TCArg("signed char * __restrict__"   , "shift_buff")
				)
		);

	LibKernelTemplate("DCT_II_Arg_T",
			  CArgs(5,
				TCArg("void *__restrict__", "Data"),
				TCArg("void *__restrict__", "FeatList"),
				TCArg("void *__restrict__", "DCTCoeff"),
				TCArg("short int", "n_input"),
				TCArg("short int", "n_dct")
				)
		);

	LibKernelTemplate("Lifter_Arg_T",
			  CArgs(3,
				TCArg("void *__restrict__", "FeatList"),
				TCArg("void *__restrict__", "LiftCoeff"),
				TCArg("void *__restrict__", "FrameSize")
				)
		);

	LibKernelTemplate("MatMul_DSP_T",
			  CArgs(11,
			  	TCArg("void * __restrict__", "In1"),
        			TCArg("void * __restrict__", "In2"),
        			TCArg("void * __restrict__", "Out"),
        			TCArg("void *", 	     "BufferColIn2"),
        			TCArg("unsigned int",  	     "W_In1"),
        			TCArg("unsigned int",  	     "H_In1"),
        			TCArg("unsigned int",  	     "W_In2"),
        			TCArg("unsigned int",  	     "W_Out"),
        			TCArg("unsigned int",  	     "OutFirstCol"),
        			TCArg("int",  		     "ColFirst"),
        			TCArg("int",  		     "Norm")
				)
		);

	LibKernelTemplate("KerMatAdd_DSP_T",
			  CArgs(6,
			  	TCArg("void * __restrict__", "In1"),
        			TCArg("void * __restrict__", "In2"),
        			TCArg("void * __restrict__", "Out"),
        			TCArg("unsigned int",  	     "W"),
        			TCArg("unsigned int",  	     "H"),
        			TCArg("int",  		     "Norm")
				)
		);

	/* FFT Basic Kernels */
	LibKernel("Radix2FFT_DIF_Par_Fix16",	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix2FFT_DIF_Par_Fix32",	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix2FFT_DIF_Par_Fix32_Scal",CALL_PARALLEL, 0, "FFT_scal_Arg_T", NULL);
	LibKernel("Radix2FFT_DIF_Par_f32",  	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix2FFT_DIF_Par_f16",  	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);

	LibKernel("Radix4FFT_DIF_Par_Fix16",	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix4FFT_DIF_Par_Fix32",	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix4FFT_DIF_Par_f32",  	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);
	LibKernel("Radix4FFT_DIF_Par_f16",  	CALL_PARALLEL, 0, "FFT_Arg_T", NULL);

	LibKernel("SwapSamples_Par", 		CALL_PARALLEL, 0, "SwapSamples_Arg_T", NULL);
	LibKernel("SwapSamples_Par_f16", 	CALL_PARALLEL, 0, "SwapSamples_Arg_T", NULL);
	LibKernel("SwapSamples_Par_f32", 	CALL_PARALLEL, 0, "SwapSamples_Arg_T", NULL);
	LibKernel("SwapSamples_Par_Fix32", 	CALL_PARALLEL, 0, "SwapSamples_Arg_T", NULL);
	LibKernel("SwapSamples_scal", 		CALL_PARALLEL, 0, "SwapSamples_scal_Arg_T", NULL);

	LibKernel("RFFT_DIF_Par_Fix16",		CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);
	LibKernel("RFFT_DIF_Par_Fix32_Scal",	CALL_PARALLEL, 0, "RFFT_scal_Arg_T", NULL);
	LibKernel("RFFT_DIF_Par_f32",		CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);
	LibKernel("RFFT_DIF_Par_f16",	  	CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);

	LibKernel("IRFFT_DIF_Par_Fix16",		CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);
	LibKernel("IRFFT_DIF_Par_Fix32_Scal",	CALL_PARALLEL, 0, "RFFT_scal_Arg_T", NULL);
	LibKernel("IRFFT_DIF_Par_f32",		CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);
	LibKernel("IRFFT_DIF_Par_f16",	  	CALL_PARALLEL, 0, "RFFT_Arg_T", NULL);

	/* DSP Basic Kernels */
        LibKernel("PreEmphasis", 		CALL_PARALLEL, 0, "PreEmphasis_T", NULL);
        LibKernel("PreEmphasis_f16", 		CALL_PARALLEL, 0, "PreEmphasis_f16_T", NULL);
        LibKernel("PreEmphasis_f32",		CALL_PARALLEL, 0, "PreEmphasis_f32_T", NULL);

        LibKernel("InvWindowing_Fix16", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_Fix16", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_Fix32", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_f16", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_f32", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_PadCenter_Fix16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_PadCenter_Fix32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_PadCenter_f16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Cmplx_PadCenter_f32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_Fix16", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_Fix32", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_f16", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_f32", 	CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_PadCenter_Fix16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_PadCenter_Fix32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_PadCenter_f16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("WindowingReal2Real_PadCenter_f32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPad_Fix16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPad_Fix32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPad_f16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPad_f32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPadCenter_Fix16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPadCenter_Fix32", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPadCenter_f16", CALL_PARALLEL, 0, "Windowing_T", NULL);
        LibKernel("ZeroPadCenter_f32", CALL_PARALLEL, 0, "Windowing_T", NULL);

        LibKernel("CmplxMagSquared_Fix16", 	CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMagSquared_Fix32_scal",	CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMagSquared_f16", 	CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMagSquared_f32", 	CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMag_Fix32", 		CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMag_Fix32_scal",	CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMag_f16", 		CALL_PARALLEL, 0, "CmplxMag_T", NULL);
        LibKernel("CmplxMag_f32", 		CALL_PARALLEL, 0, "CmplxMag_T", NULL);

        LibKernel("MelFilterBank_Fix32",	CALL_PARALLEL, 0, "MelFilterBank_T", NULL);
        LibKernel("MelFilterBank_Fix32_Scal", 	CALL_PARALLEL, 0, "MelFilterBank_T", NULL);
        LibKernel("MelFilterBank_f16", 		CALL_PARALLEL, 0, "MelFilterBank_T", NULL);
        LibKernel("MelFilterBank_f32", 		CALL_PARALLEL, 0, "MelFilterBank_T", NULL);

        LibKernel("MFCC_ComputeLog_Fix32",	CALL_PARALLEL, 0, "MFCC_Log_T", NULL);
        LibKernel("MFCC_ComputeLog_Fix32_Scal", CALL_PARALLEL, 0, "MFCC_Log_T", NULL);
        LibKernel("MFCC_ComputeLog_f16", 	CALL_PARALLEL, 0, "MFCC_LogF_T", NULL);
        LibKernel("MFCC_ComputeLog_f32", 	CALL_PARALLEL, 0, "MFCC_LogF_T", NULL);
        LibKernel("MFCC_ComputeDB_Fix32", 	CALL_PARALLEL, 0, "MFCC_Log_T", NULL);
        LibKernel("MFCC_ComputeDB_Fix32_Scal", 	CALL_PARALLEL, 0, "MFCC_Log_T", NULL);
        LibKernel("MFCC_ComputeDB_f16", 	CALL_PARALLEL, 0, "MFCC_LogF_T", NULL);
        LibKernel("MFCC_ComputeDB_f32", 	CALL_PARALLEL, 0, "MFCC_LogF_T", NULL);

        LibKernel("norm_clip_16",	 	CALL_PARALLEL, 0, "Norm_Clip_args_T", NULL);
        LibKernel("norm_clip_32_melspect",	CALL_PARALLEL, 0, "MFCC_Clip_32_T", NULL);
        LibKernel("norm_clip_32_melspect_scal",	CALL_PARALLEL, 0, "MFCC_Clip_32_T", NULL);

        LibKernel("MFCC_ComputeDCT_II_Fix16", 	CALL_PARALLEL, 0, "DCT_II_Arg_T", NULL);
        LibKernel("MFCC_ComputeDCT_II_f16", 	CALL_PARALLEL, 0, "DCT_II_Arg_T", NULL);
        LibKernel("MFCC_ComputeDCT_II_f32", 	CALL_PARALLEL, 0, "DCT_II_Arg_T", NULL);

        LibKernel("MFCC_Lifter_Fix16", 		CALL_PARALLEL, 0, "Lifter_Arg_T", NULL);
        LibKernel("MFCC_Lifter_f16", 		CALL_PARALLEL, 0, "Lifter_Arg_T", NULL);
        LibKernel("MFCC_Lifter_f32", 		CALL_PARALLEL, 0, "Lifter_Arg_T", NULL);

        LibKernel("Conjugate_Fix16_Par",   CALL_PARALLEL, CArgs(2, TCArg("v2s   * __restrict__", "Data"), TCArg("int", "Ni")), "SwapSamples_Arg_T", NULL);
        LibKernel("Conjugate_Fix32_Par",   CALL_PARALLEL, CArgs(2, TCArg("int   * __restrict__", "Data"), TCArg("int", "Ni")), "SwapSamples_Arg_T", NULL);
        LibKernel("Conjugate_Float16_Par", CALL_PARALLEL, CArgs(2, TCArg("F16V_DSP   * __restrict__", "Data"), TCArg("int", "Ni")), "SwapSamples_Arg_T", NULL);
        LibKernel("Conjugate_Float32_Par", CALL_PARALLEL, CArgs(2, TCArg("float * __restrict__", "Data"), TCArg("int", "Ni")), "SwapSamples_Arg_T", NULL);

        LibKernel("KerParMatMulDSP_Fix16", CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
        LibKernel("KerParMatMulDSPT_Fix16",CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
        LibKernel("KerParMatMulDSP_fp16",  CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
        LibKernel("KerParMatMulDSPT_fp16", CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
        LibKernel("KerParMatMulDSP_fp32",  CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
        LibKernel("KerParMatMulDSPT_fp32", CALL_PARALLEL, 0, "MatMul_DSP_T", NULL);
}

void PieceWiseGenerator(char *Name, CNN_GenControl_T *Ctrl, char *FunName, int Dim, int DataType, int Inplace)
{
	char * InPointer;
	int InItemSize=2, OutItemSize=2, LUTItemSize=2;
	switch (DataType){
		case FIX16:
			InPointer   = "int16_t * __restrict__"; InItemSize=2; OutItemSize=2; break;
		case FIX32:
			InPointer   = "int * __restrict__"; InItemSize=4; OutItemSize=4; break;
		case FLOAT16:
			InPointer   = "F16_DSP * __restrict__"; InItemSize=F16_SIZE; OutItemSize=F16_SIZE; break;
		case FLOAT32:
			InPointer   = "float * __restrict__"; InItemSize=4; OutItemSize=4; break;
		default:
			return GenTilingError("Data Type %d not known", DataType);
	}
	if (Inplace) LibKernel(FunName, CALL_PARALLEL, CArgs(2, TCArg(InPointer, "In"), TCArg("short int", "N")), "Arg_PieceWise_T", NULL);
	else         LibKernel(FunName, CALL_PARALLEL, CArgs(3, TCArg(InPointer, "In"), TCArg("short int", "N"), TCArg(InPointer, "Out")), "Arg_PieceWise_T", NULL);

	Kernel_T *Kernel = UserKernel(Name,
                KernelIterSpace(1, IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(2,
                	TCArg(InPointer, "In"),
			(Inplace)?TCArg(InPointer, "Out"):AT_NO_C_ARG
                     ),
                Calls(1,
			Call(FunName,LOC_LOOP,
			      Bindings(Inplace?2:3,
			       K_Arg("In",       KER_ARG_TILE),
			       Imm(Dim),
			       Inplace?K_Arg("Out", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     )
		),
		KerArgs(2,
			KerArg("In",  KerArgSpace(1,T0), OBJ_IN_OUT, 1, Dim, InItemSize,  0, 0, 0, "In"),
		Inplace?KerArg("Out", KerArgSpace(1,T0), OBJ_IN_OUT, 1, Dim, OutItemSize, 0, 0, 0, "Out"):AT_NO_KER_ARG
		)
	);
}

int MFCC_Generator_old(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int FrameSize,
	int FrameStride,
	int Nfft,
	int NMelBanks,
	int SizeMelCoeff,
	int Ndct,
	float PreempFactor,
	float LifterCoeff,
	int ForceRadix2FFT,
	int MagSquared,
	int DataType,
	int LogType,
	int OutFFT
	)
{
	AT_PadType PadType = PAD_RIGHT;
	float MfccLogOffset = 0.0;
	if (Ctrl) {
		if (Ctrl->PadType != -1) PadType = Ctrl->PadType;
		if (Ctrl->MfccLogOffset != -1) MfccLogOffset = FIX2FP(Ctrl->MfccLogOffset, 30);
	}
	GenTilingDebug("LOG OFFSET %f\n", MfccLogOffset);
	int MFCC_Coeff_Dyn = 15;
	if (__builtin_popcount(Nfft) != 1) GenTilingError("%s, Incorrect FFTDim: %d, it has to be a a power of 2", Name, Nfft);
	int UseRadix4 = (DataType!=FIX32) && (!ForceRadix2FFT && ((__builtin_ctz(Nfft)%2)==0) && (Nfft>64));
	int OutMelspectrogram = LogType == 0;
	int UseDB = LogType == 2;

	int Log2Nfft = 31 - __builtin_clz(Nfft);
	int Log4Nfft = Log2Nfft>>1;
	int QIn_FFT = UseRadix4?12:13;
	int Q_Out_FFT = 15 - UseRadix4?(Log2Nfft-1):(Log2Nfft-2);
	GenTilingDebug("MFCC_COEF_DYN = %d\nFFT_BITS = %d\nUSE_DB = %d\nDATA_TYPE = %d\n", MFCC_Coeff_Dyn, Log2Nfft, UseDB, DataType);

	char *PreEmpKernel=0, *WinKernel=0, *FFTKernel=0, *SwapKernel=0, *MfccKernel=0, *SpectKernel=0, *LogKernel=0, *DCTKernel=0, *UserKernType=0, *UserKernPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;      

	switch (DataType){
		case FIX16:
			PreEmpKernel = "PreEmphasis";
			WinKernel = PadType==PAD_RIGHT?"WindowingReal2Cmplx_Fix16":"WindowingReal2Cmplx_PadCenter_Fix16";
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_Fix16";
			else  	       FFTKernel = "Radix2FFT_DIF_Par_Fix16";
			SwapKernel = "SwapSamples_Par";
			SpectKernel = MagSquared?"CmplxMagSquared_Fix16":"CmplxMag_Fix32";
			MfccKernel = "MelFilterBank_Fix32";
			LogKernel = UseDB?"MFCC_ComputeDB_Fix32":"MFCC_ComputeLog_Fix32";
			DCTKernel = "MFCC_ComputeDCT_II_Fix16";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2; OutItemSize=OutMelspectrogram?4:2, LUTItemSize=2;
			break;
		case FIX32:
			PreEmpKernel = "PreEmphasis";
			WinKernel = PadType==PAD_RIGHT?"WindowingReal2Cmplx_Fix32":"WindowingReal2Cmplx_PadCenter_Fix32";
			if (UseRadix4) GenTilingError("Radix4 FFT Not implemented for FIX32 Scal");
			else  	       FFTKernel = "Radix2FFT_DIF_Par_Fix32_Scal";
			SwapKernel = "SwapSamples_scal";
			SpectKernel = MagSquared?"CmplxMagSquared_Fix32_scal":"CmplxMag_Fix32_scal";
			MfccKernel = MagSquared?"MelFilterBank_Fix32_Scal":"MelFilterBank_Fix32";
			LogKernel = UseDB?"MFCC_ComputeDB_Fix32_Scal":"MFCC_ComputeLog_Fix32_Scal";
			DCTKernel = "MFCC_ComputeDCT_II_Fix16";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2; OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT16:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f16":0;
			WinKernel = PadType==PAD_RIGHT?"WindowingReal2Cmplx_f16":"WindowingReal2Cmplx_PadCenter_f16";
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f16";
			else 	       FFTKernel = "Radix2FFT_DIF_Par_f16";
			SwapKernel = "SwapSamples_Par_f16";
			SpectKernel = MagSquared?"CmplxMagSquared_f16":"CmplxMag_f16";
			MfccKernel = "MelFilterBank_f16";
			LogKernel = UseDB?"MFCC_ComputeDB_f16":"MFCC_ComputeLog_f16";
			DCTKernel = "MFCC_ComputeDCT_II_Fix16";
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE; OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FLOAT32:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f32":0;
			WinKernel = PadType==PAD_RIGHT?"WindowingReal2Cmplx_f32":"WindowingReal2Cmplx_PadCenter_f32";
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f32";
			else 	       FFTKernel = "Radix2FFT_DIF_Par_f32";
			SwapKernel = "SwapSamples_Par_f32";
			SpectKernel = MagSquared?"CmplxMagSquared_f32":"CmplxMag_f32";
			MfccKernel = "MelFilterBank_f32";
			LogKernel = UseDB?"MFCC_ComputeDB_f32":"MFCC_ComputeLog_f32";
			DCTKernel = "MFCC_ComputeDCT_II_f32";
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			InItemSize=4; OutItemSize=4, LUTItemSize=4;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
	}
	unsigned int LayerOp = Abs(NFrames) * ((PreEmpKernel?(2*FrameSize):0) + (WinKernel?Nfft:0) + Nfft*Log2Nfft*4 + Nfft + Nfft*NMelBanks + (LogKernel?NMelBanks:0) + (DCTKernel?(NMelBanks*Ndct):0));
	unsigned int LayerBandwidth = Abs(NFrames) * (FrameSize*InItemSize + OutItemSize*(DCTKernel?Ndct:NMelBanks)) + LUTItemSize*(FrameSize + (UseRadix4?(3*(Nfft)/4):(Nfft/2)) + Nfft + SizeMelCoeff + Ndct*Ndct) + 2*Nfft/2 + 6*NMelBanks;
	GenTilingDebug("Mfcc:\n");
	GenTilingDebug("PreEmpKernel:	%25s\n", PreEmpKernel?PreEmpKernel:"");
	GenTilingDebug("WinKernel:	%25s\n", WinKernel?WinKernel:"");
	GenTilingDebug("RFFT_Kernel:	%25s\n", FFTKernel?FFTKernel:"");
	GenTilingDebug("SpectKernel:	%25s\n", SpectKernel?SpectKernel:"");
	GenTilingDebug("MfccKernel:	%25s\n", MfccKernel?MfccKernel:"");
	GenTilingDebug("LogKernel:	%25s\n", LogKernel?LogKernel:"");
	GenTilingDebug("DCTKernel:	%25s\n", DCTKernel?DCTKernel:"");
	GenTilingDebug("\tNb Oper: %d\n", LayerOp);
	GenTilingDebug("\tBandwidth: %d\n", LayerBandwidth);
	// GenTilingDebug("MFCC_COEF_DYN = %d\nFFT_BITS = %d\nUSE_DB = %d\nDATA_TYPE = %dLOG OFFSET %f\n", MFCC_Coeff_Dyn, Log2Nfft, UseDB, DataType, MfccLogOffset);


	int ncalls = 10;
	UserSymbols(2, US_Float("LogOffset", MfccLogOffset), US_Float("PreempFactor", PreempFactor));
	int InBuffSize = (PadType!=PAD_RIGHT)?Nfft:FrameSize;
	Kernel_T *Kernel = UserKernel(Name,
                NFrames<0?
                KernelIterSpace(2, IterFixedSpaceDynBound(D0, 10000, "NFrames"), IterTiledSpace(T0)):
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(13,
                	TCArg(UserKernPointer,      "In"),
			(OutMelspectrogram && DataType != FLOAT16 && DataType != FLOAT32)?TCArg("int * __restrict__", "Out"):TCArg(UserKernPointer, "Out"),
			TCArg(UserKernPointer, "Twiddles_fft"),
			TCArg("short int *", "SwapTable_fft"),
			TCArg(UserKernPointer, "WinTable"),
			TCArg("short int *", "Mel_FilterBank"),
			TCArg(UserKernPointer, "Mel_Coeffs"),
			(OutMelspectrogram==0 && DataType != FLOAT16 && DataType != FLOAT32)?TCArg("int", "Norm"):AT_NO_C_ARG,
			Ndct?TCArg(UserKernPointer, "DCT_Coeff"):AT_NO_C_ARG,
			OutFFT?TCArg(UserKernPointer, "FFT_Out"):AT_NO_C_ARG,
			OutFFT&&(DataType==FIX32)?TCArg("signed char *", "FFT_Shift_Buff"):AT_NO_C_ARG,
			OutFFT&&(DataType==FIX16||DataType==FIX32)?TCArg("short int *", "PreempShift"):AT_NO_C_ARG,
			NFrames<0?TCArg("short int", "NFrames"):AT_NO_C_ARG
                ),
                Calls(10,
                	PreEmpKernel?Call(PreEmpKernel,LOC_LOOP,
			     Bindings((DataType==FIX16 || DataType==FIX32)?7:5,
			       K_Arg("In",     KER_ARG_TILE),
			       K_Arg("InOut1", KER_ARG_TILE),
			       Imm(0),
			       (DataType==FIX16 || DataType==FIX32)?Imm(FP2FIX(PreempFactor, 15)):BindKExpr("PreempFactor"),
			       Imm(FrameSize),
			       (DataType==FIX16 || DataType==FIX32)?K_Arg("Shift", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16 || DataType==FIX32)?Imm(QIn_FFT):AT_IGNORE_ARG_BINDING)
			     ):AT_NO_CALL,
			Call(WinKernel,LOC_LOOP,
			     Bindings(5,
			       (PreEmpKernel)?K_Arg("InOut1"  , KER_ARG_TILE):K_Arg("In", KER_ARG_TILE),
			       K_Arg("Out_fft" , KER_ARG_TILE),
			       K_Arg("WinTable" , KER_ARG_TILE ),
			       Imm(FrameSize),
			       Imm(Nfft))
			     ),
			Call(FFTKernel,LOC_LOOP,
			      Bindings(DataType==FIX32?5:4,
			       K_Arg("Out_fft",       KER_ARG_TILE),
			       K_Arg("Twiddles_fft", KER_ARG_TILE ),
			       Imm(Nfft),
			       Imm(0),
			       DataType==FIX32?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING) //shift_fft buffer)
			     ),
			Call(SwapKernel,LOC_LOOP,
			      Bindings(DataType==FIX32?4:3,
			       K_Arg("Out_fft",       KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       DataType==FIX32?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING) //shift_fft buffer)
			     ),
			Call(SpectKernel,LOC_LOOP,
			     Bindings(6,
			       K_Arg("Out_fft"   , KER_ARG_TILE),
			       K_Arg("InOut1"   , KER_ARG_TILE),
			       Imm(Nfft),
			       (DataType==FIX16 || DataType==FIX32)?K_TileOper("Shift", "short int *", '@', 0):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16)?Imm(Q_Out_FFT):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX32)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ),
			Call(MfccKernel, LOC_LOOP,
			     Bindings(9,
			       K_Arg("InOut1"  	      , KER_ARG_TILE),
			       (Ndct==0 && (DataType==FLOAT16 || DataType==FLOAT32))?K_Arg("Out", KER_ARG_TILE):K_Arg("InOut2" , KER_ARG_TILE),
			       K_Arg("Mel_Coeffs"    , KER_ARG_TILE),
			       K_Arg("Mel_FilterBank", KER_ARG_TILE),
			       Imm(NMelBanks),
			       Imm(MFCC_Coeff_Dyn),
			       Imm(MagSquared),
			       (DataType==FIX16 || DataType==FIX32)?K_Arg("shift_buff", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       DataType==FIX32?K_Arg("shift_fft" , KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ),
			(OutMelspectrogram==0)?
			Call(LogKernel,LOC_LOOP,
			     Bindings(10,
			     	(Ndct==0 && (DataType==FLOAT16 || DataType==FLOAT32))?K_Arg("Out", KER_ARG_TILE):K_Arg("InOut2", KER_ARG_TILE),
			 	(Ndct==0)?K_Arg("Out", KER_ARG_TILE):K_Arg("InOut1", KER_ARG_TILE),
				Imm(NMelBanks),
				(DataType==FIX16 || DataType==FIX32)?C_Arg("Norm"):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?K_TileOper("Shift", "short int *", '@', 0):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(Q_Out_FFT):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(MFCC_Coeff_Dyn):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(MagSquared):AT_IGNORE_ARG_BINDING,
			       	(DataType==FIX16 || DataType==FIX32)?K_Arg("shift_buff", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			        (DataType==FIX16 || DataType==FIX32)?Imm(FP2FIX(MfccLogOffset, 30)):BindKExpr("LogOffset"))
			      ):AT_NO_CALL,
			((DataType==FIX32 || DataType==FIX16) && (OutMelspectrogram!=0))?
			Call("norm_clip_32_melspect", LOC_LOOP,
			     Bindings(8,
			        K_Arg("InOut2", KER_ARG_TILE),
			 	K_Arg("Out",    KER_ARG_TILE),
				Imm(NMelBanks),
				K_TileOper("Shift", "short int *", '@', 0), // PreEmphasis Shift
				Imm(Q_Out_FFT),
				Imm(MFCC_Coeff_Dyn),
				Imm(MagSquared),
			       	K_Arg("shift_buff", KER_ARG_TILE)) //shift_bf buffer in case of DataType==FIX32)
			     ):AT_NO_CALL,
			((Ndct>0) && (OutMelspectrogram==0))?
			Call(DCTKernel,LOC_LOOP,
			     Bindings(5,
			       K_Arg("InOut1",    KER_ARG_TILE),
			       K_Arg("Out",       KER_ARG_TILE),
			       K_Arg("DCT_Coeff", KER_ARG_TILE),
			       Imm(NMelBanks),
			       Imm(Ndct))
			     ):AT_NO_CALL,
			(LifterCoeff && (OutMelspectrogram==0))?
			Call("MFCC_Lifter", LOC_LOOP,
			     Bindings(3,
			       K_Arg("InOut2",     KER_ARG_TILE),
			       K_Arg("lift_coeff", KER_ARG_TILE),
			       Imm(Ndct?Ndct:NMelBanks))
			     ):AT_NO_CALL
		),
		KerArgs(14,
			KerArg("In",              KerArgSpace(1,D0), OBJ_IN_DB,	  	       1, InBuffSize, 	  	  InItemSize, InBuffSize-FrameStride, 0, 0, "In"),
			KerArg("Out",         	  KerArgSpace(1,D0), OBJ_OUT_DB,   	       1, Ndct==0?NMelBanks:Ndct, OutItemSize,		          0, 0, 0, "Out"),
			KerArg("InOut1",          KerArgSpace(1,T0), O_BUFF,   		       1, Nfft,   		  sizeof(int), 		          0, 0, 0, ""),
			OutFFT?
			KerArg("Out_fft",         KerArgSpace(1,D0), OBJ_OUT_DB,	       1, 2*Nfft,   		  OutItemSize,    	          0, 0, 0, "FFT_Out"):
			KerArg("Out_fft",         KerArgSpace(1,T0), O_BUFF,		       1, 2*Nfft,   		  OutItemSize,    	          0, 0, 0, ""),
			DataType==FIX32?
			(OutFFT?
			KerArg("shift_fft",       KerArgSpace(1,D0), OBJ_OUT_DB,	       1, Nfft,			  sizeof(signed char), 	          0, 0, 0, "FFT_Shift_Buff"):
			KerArg("shift_fft",       KerArgSpace(1,T0), O_BUFF,		       1, Nfft,			  sizeof(signed char), 	          0, 0, 0, "")):0,
			(DataType==FIX16||DataType==FIX32)?
			(OutFFT?
			KerArg("Shift",		  KerArgSpace(1,D0), OBJ_OUT_DB,	       1, 1, 			  sizeof(short int),              0, 0, 0, "PreempShift"):
			KerArg("Shift",		  KerArgSpace(1,T0), O_BUFF|O_NTILED,	       1, 1, 			  sizeof(short int),              0, 0, 0, "")):0,
			(Ndct||DataType==FIX32||DataType==FIX16)?
			KerArg("InOut2", 	  KerArgSpace(1,T0), O_BUFF, 		       1, NMelBanks, 		  sizeof(int), 			  0, 0, 0, ""):0,
			KerArg("WinTable",        KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST,      1, FrameSize,              LUTItemSize,		          0, 0, 0, "WinTable"),
	      		KerArg("Twiddles_fft", 	  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, UseRadix4?2*(3*Nfft/4):Nfft, LUTItemSize,    	          0, 0, 0, "Twiddles_fft"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST,      1, Nfft,              	  sizeof(short int),              0, 0, 0, "SwapTable_fft"),
			KerArg("Mel_FilterBank",  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST,      1, NMelBanks,          	  6,			          0, 0, 0, "Mel_FilterBank"),
			KerArg("Mel_Coeffs",      KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST,      1, SizeMelCoeff,        	  LUTItemSize,      	          0, 0, 0, "Mel_Coeffs"),
			(DataType==FIX16 || DataType==FIX32)?
			KerArg("shift_buff",   	  KerArgSpace(1,T0), O_BUFF,    	       1, NMelBanks,          	  sizeof(signed char),            0, 0, 0, ""):0,
		  	Ndct?
		  	KerArg("DCT_Coeff",  	  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST,      1, Ndct*Ndct,         	  LUTItemSize, 		          0, 0, 0, "DCT_Coeff"):0
		)
	);
	return 1;
}

int MFCC_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int FrameSize,
	int FrameStride,
	int Nfft,
	int NMelBanks,
	int SizeMelCoeff,
	int Ndct,
	float PreempFactor,
	int NoWindow,
	float LifterCoeff,
	int MagSquared,
	int DataType, 		/* 0: Fixed point 16 bits, 1: Fixed point 32 (old version), 2: float16 (only for gap9), 3: float32 */
	int LogType, 		/* 0: Output melspect without applying log and dct, 1: Natural log, 2: 10log10 */
	int OutFFT 		/* If output FFT beside mel spect */
	)
{
	if (__builtin_popcount(Nfft) != 1 || Nfft < FrameSize) GenTilingError("%s, Incorrect FFTDim: %d, it has to be a a power of 2 and > %d (FrameSize)", Name, Nfft, FrameSize);
	if (DataType==FIX32) return MFCC_Generator_old(Name, Ctrl, NFrames, FrameSize, FrameStride, Nfft, NMelBanks, SizeMelCoeff, Ndct, PreempFactor, LifterCoeff, 0, MagSquared, DataType, LogType, OutFFT);
	AT_PadType PadType = PAD_RIGHT;
	float MfccLogOffset = 0.0;
	if (Ctrl) {
		if (Ctrl->PadType != -1 && Nfft>FrameSize) PadType = Ctrl->PadType;
		if (Ctrl->MfccLogOffset != -1) MfccLogOffset = FIX2FP(Ctrl->MfccLogOffset, 30);
	}
	int MFCC_Coeff_Dyn = 15;
	int UseRadix4 = (DataType!=FIX32) && (((__builtin_ctz(Nfft>>1)%2)==0) && ((Nfft>>1)>64));
	int OutMelspectrogram = LogType == 0;
	int UseDB = LogType == 2;

	int Log2Nfft = 31 - __builtin_clz(Nfft);
	int Log4Nfft = Log2Nfft>>1;
	int Log2Nfft_int = 31 - __builtin_clz(Nfft/2);
	int Log4Nfft_int = Log2Nfft_int>>1;
	int QIn_FFT = UseRadix4?12:13;
	int Q_Out_FFT = 15 - (Log2Nfft_int-1);

	char *PreEmpKernel=0, *WinKernel=0, *RFFT_Kernel=0, *SwapKernel=0, *MfccKernel=0, *SpectKernel=0, *LogKernel=0, *DCTKernel=0, *LifterKern=0, *UserKernType=0, *UserKernPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2; 

	switch (DataType){
		case FIX16:
			PreEmpKernel = "PreEmphasis";
			WinKernel = NoWindow?"ZeroPad_Fix16":(PadType==PAD_RIGHT?"WindowingReal2Real_Fix16":"WindowingReal2Real_PadCenter_Fix16");
			RFFT_Kernel = "RFFT_DIF_Par_Fix16";
			SpectKernel = MagSquared?"CmplxMagSquared_Fix16":"CmplxMag_Fix32";
			MfccKernel = "MelFilterBank_Fix32";
			LogKernel = UseDB?"MFCC_ComputeDB_Fix32":"MFCC_ComputeLog_Fix32";
			DCTKernel = Ndct?"MFCC_ComputeDCT_II_Fix16":0;
			LifterKern = LifterCoeff>0?"MFCC_Lifter_Fix16":0;
			UserKernType = "short int";
			UserKernPointer = "short int * __restrict__";
			InItemSize=2; OutItemSize=OutMelspectrogram?4:2, LUTItemSize=2;
			break;
		case FLOAT16:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f16":0;
			WinKernel = NoWindow?"ZeroPad_f16":(PadType==PAD_RIGHT?"WindowingReal2Real_f16":"WindowingReal2Real_PadCenter_f16");
			RFFT_Kernel = "RFFT_DIF_Par_f16";
			SpectKernel = MagSquared?"CmplxMagSquared_f16":"CmplxMag_f16";
			MfccKernel = "MelFilterBank_f16";
			LogKernel = UseDB?"MFCC_ComputeDB_f16":"MFCC_ComputeLog_f16";
			DCTKernel = Ndct?"MFCC_ComputeDCT_II_f16":0;
			LifterKern = LifterCoeff>0?"MFCC_Lifter_f16":0;
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE; OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FLOAT32:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f32":0;
			WinKernel = NoWindow?"ZeroPad_f32":(PadType==PAD_RIGHT?"WindowingReal2Real_f32":"WindowingReal2Real_PadCenter_f32");
			RFFT_Kernel = "RFFT_DIF_Par_f32";
			SpectKernel = MagSquared?"CmplxMagSquared_f32":"CmplxMag_f32";
			MfccKernel = "MelFilterBank_f32";
			LogKernel = UseDB?"MFCC_ComputeDB_f32":"MFCC_ComputeLog_f32";
			DCTKernel = Ndct?"MFCC_ComputeDCT_II_f32":0;
			LifterKern = LifterCoeff>0?"MFCC_Lifter_f32":0;
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			InItemSize=4; OutItemSize=4, LUTItemSize=4;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
			return 0;
	}
	int OutDataSize = DataType==FLOAT32||(OutMelspectrogram && DataType==FIX16)?4:2;
	unsigned int LayerOp = Abs(NFrames) * ((PreEmpKernel?(2*FrameSize):0) + (WinKernel?Nfft:0) + Nfft*Log2Nfft*4 + Nfft + Nfft*NMelBanks + (LogKernel?NMelBanks:0) + (DCTKernel?(NMelBanks*Ndct):0));
	unsigned int LayerBandwidth = Abs(NFrames) * (FrameSize*InItemSize + OutItemSize*(DCTKernel?Ndct:NMelBanks)) + LUTItemSize*(FrameSize + (UseRadix4?(3*(Nfft)/4):(Nfft/2)) + Nfft + SizeMelCoeff + Ndct*Ndct) + 2*Nfft/2 + 6*NMelBanks;
	GenTilingDebug("Mfcc:\n");
	GenTilingDebug("PreEmpKernel:	%25s\n", PreEmpKernel?PreEmpKernel:"");
	GenTilingDebug("WinKernel:	%25s\n", WinKernel?WinKernel:"");
	GenTilingDebug("RFFT_Kernel:	%25s\n", RFFT_Kernel?RFFT_Kernel:"");
	GenTilingDebug("SpectKernel:	%25s\n", SpectKernel?SpectKernel:"");
	GenTilingDebug("MfccKernel:	%25s\n", MfccKernel?MfccKernel:"");
	GenTilingDebug("LogKernel:	%25s\n", LogKernel?LogKernel:"");
	GenTilingDebug("DCTKernel:	%25s\n", DCTKernel?DCTKernel:"");
	GenTilingDebug("\tN Frames: %d\n", NFrames);
	GenTilingDebug("\tFrame Size: %d\n", FrameSize);
	GenTilingDebug("\tFrame Stride: %d\n", FrameStride);
	GenTilingDebug("\tIn Data Size: %d\n", InItemSize);
	GenTilingDebug("\tTot Input Size: %d\n", FrameStride * (NFrames-1) + FrameSize);
	GenTilingDebug("\tNb Oper: %d\n", LayerOp);
	GenTilingDebug("\tBandwidth: %d\n", LayerBandwidth);
	// GenTilingDebug("MFCC_COEF_DYN = %d\nFFT_BITS = %d\nUSE_DB = %d\nDATA_TYPE = %dLOG OFFSET %f\n", MFCC_Coeff_Dyn, Log2Nfft, UseDB, DataType, MfccLogOffset);

	int InBuffSize = (PadType!=PAD_RIGHT)?Nfft:FrameSize;
	UserSymbols(2, US_Float("LogOffset", MfccLogOffset), US_Float("PreempFactor", PreempFactor));
	Kernel_T *Kernel = UserKernel(Name,
                NFrames<0?
                KernelIterSpace(2, IterFixedSpaceDynBound(D0, 10000, "NFrames"), IterTiledSpace(T0)):
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(13,
                	TCArg(UserKernPointer, "In"),
			TCArg((OutMelspectrogram&&DataType==FIX16)?"unsigned int *":UserKernPointer, "Out"),
			TCArg(UserKernPointer, "Twiddles_fft_int"),
			TCArg(UserKernPointer, "Twiddles_rfft"),
			TCArg("short int *", "SwapTable_fft"),
			!NoWindow?TCArg(UserKernPointer, "WinTable"):AT_NO_C_ARG,
			TCArg("short int *", "Mel_FilterBank"),
			TCArg(UserKernPointer, "Mel_Coeffs"),
			(OutMelspectrogram==0&&DataType==FIX16)?TCArg("int", "Norm"):AT_NO_C_ARG,
			(Ndct)?TCArg(UserKernPointer, "DCT_Coeff"):AT_NO_C_ARG,
			(OutFFT)?TCArg(UserKernPointer, "FFT_Out"):AT_NO_C_ARG,
			(OutFFT&&DataType==FIX16)?TCArg("short int *", "PreempShift"):AT_NO_C_ARG,
			(NFrames<0)?TCArg("short int", "NFrames"):AT_NO_C_ARG
                ),
                Calls(9,
                	PreEmpKernel?
                	Call(PreEmpKernel,LOC_LOOP,
			     Bindings(DataType==FIX16?7:5,
			       K_Arg("In", 	KER_ARG_TILE),
			       K_Arg("In_rfft", KER_ARG_TILE),
			       Imm(0),
			       (DataType==FIX16)?Imm(FP2FIX(PreempFactor, 15)):BindKExpr("PreempFactor"),
			       (PadType==PAD_RIGHT)?Imm(FrameSize):Imm(Nfft),
			       (DataType==FIX16)?K_Arg("Shift", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16)?Imm(QIn_FFT):AT_IGNORE_ARG_BINDING)
			     ):AT_NO_CALL,
			Call(WinKernel,LOC_LOOP,
			     Bindings(5,
			       PreEmpKernel?K_Arg("In_rfft", KER_ARG_TILE):K_Arg("In", KER_ARG_TILE),
			       K_Arg("In_rfft", KER_ARG_TILE),
			       !NoWindow?K_Arg("WinTable" , KER_ARG_TILE ):AT_IGNORE_ARG_BINDING,
			       Imm(FrameSize),
			       Imm(Nfft))
			     ),
			Call(RFFT_Kernel,LOC_LOOP,
			      Bindings(7,
			       K_Arg("In_rfft", KER_ARG_TILE),
			       K_Arg("FFT_Out", 	KER_ARG_TILE),
			       K_Arg("Twiddles_fft_int",  KER_ARG_TILE),
			       K_Arg("Twiddles_rfft", KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       Imm(0))
			     ),
			Call(SpectKernel,LOC_LOOP,
			     Bindings(6,
			       K_Arg("FFT_Out", KER_ARG_TILE),
			       K_Arg("Buff2", KER_ARG_TILE),
			       Imm(Nfft),
			       (DataType==FIX16)?(PreEmpKernel?K_TileOper("Shift", "short int *", '@', 0):0):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16)?Imm(Q_Out_FFT):AT_IGNORE_ARG_BINDING,
			       AT_IGNORE_ARG_BINDING)
			     ),
			Call(MfccKernel, LOC_LOOP,
			     Bindings(9,
			       K_Arg("Buff2", KER_ARG_TILE),
			       (OutMelspectrogram && DataType!=FIX16)?K_Arg("Out" , KER_ARG_TILE):K_Arg("In_rfft" , KER_ARG_TILE),
			       K_Arg("Mel_Coeffs"    , KER_ARG_TILE),
			       K_Arg("Mel_FilterBank", KER_ARG_TILE),
			       Imm(NMelBanks),
			       Imm(MFCC_Coeff_Dyn),
			       Imm(MagSquared),
			       (DataType==FIX16 || DataType==FIX32)?K_Arg("shift_buff", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       AT_IGNORE_ARG_BINDING)
			     ),
			(OutMelspectrogram==0)?
			Call(LogKernel,LOC_LOOP,
			     Bindings(10,
			     	K_Arg("In_rfft", KER_ARG_TILE),
			 	(Ndct==0)?K_Arg("Out", KER_ARG_TILE):K_Arg("Buff2", KER_ARG_TILE),
				Imm(NMelBanks),
				(DataType==FIX16 || DataType==FIX32)?C_Arg("Norm"):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?K_TileOper("Shift", "short int *", '@', 0):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(Q_Out_FFT):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(MFCC_Coeff_Dyn):AT_IGNORE_ARG_BINDING,
				(DataType==FIX16 || DataType==FIX32)?Imm(MagSquared):AT_IGNORE_ARG_BINDING,
			       	(DataType==FIX16 || DataType==FIX32)?K_Arg("shift_buff", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			        (DataType==FIX16 || DataType==FIX32)?Imm(FP2FIX(MfccLogOffset, 30)):BindKExpr("LogOffset"))
			      ):AT_NO_CALL,
			((DataType==FIX32 || DataType==FIX16) && (OutMelspectrogram!=0))?
			Call("norm_clip_32_melspect", LOC_LOOP,
			     Bindings(8,
			        K_Arg("In_rfft", KER_ARG_TILE),
			 	K_Arg("Out",    KER_ARG_TILE),
				Imm(NMelBanks),
				AT_IGNORE_ARG_BINDING,
				AT_IGNORE_ARG_BINDING,
				AT_IGNORE_ARG_BINDING,
				AT_IGNORE_ARG_BINDING,
			       	K_Arg("shift_buff", KER_ARG_TILE)) //shift_bf buffer in case of DataType==FIX32)
			     ):AT_NO_CALL,
			((Ndct>0) && (OutMelspectrogram==0))?
			Call(DCTKernel,LOC_LOOP,
			     Bindings(5,
			       K_Arg("Buff2",    KER_ARG_TILE),
			       K_Arg("Out",       KER_ARG_TILE),
			       K_Arg("DCT_Coeff", KER_ARG_TILE),
			       Imm(NMelBanks),
			       Imm(Ndct))
			     ):AT_NO_CALL,
			LifterKern?
			Call(LifterKern,LOC_LOOP,
			     Bindings(3,
			       K_Arg("Out",    		KER_ARG_TILE),
			       K_Arg("Lifter_Coeff",    KER_ARG_TILE),
			       Imm(NMelBanks))
			     ):AT_NO_CALL
		),
		KerArgs(14,
			KerArg("In",		  KerArgSpace(1,D0), OBJ_IN_DB,		  1, InBuffSize, 	  InItemSize,InBuffSize-FrameStride, 0, 0, "In"),
			KerArg("Out",		  KerArgSpace(1,D0), OBJ_OUT_DB,	  1, Ndct?Ndct:NMelBanks,OutItemSize, 			   0, 0, 0, "Out"),
			KerArg("In_rfft",	  KerArgSpace(1,T0), O_BUFF|O_NTILED,	  1, Nfft, 		InItemSize,    		   	   0, 0, 0, ""),
			OutFFT?
			KerArg("FFT_Out",         KerArgSpace(1,D0), OBJ_OUT_DB, 	  1, 2*(Nfft/2+1),	OutItemSize,			   0, 0, 0, "FFT_Out"):
			KerArg("FFT_Out",         KerArgSpace(1,T0), O_BUFF|O_NTILED,     1, 2*(Nfft/2+1),	OutItemSize,			   0, 0, 0, ""),
			KerArg("Buff2",           KerArgSpace(1,T0), O_BUFF|O_NTILED,     1, 2*(Nfft/2+1),	sizeof(int),			   0, 0, 0, ""),
			(DataType==FIX16?
			(OutFFT?
			KerArg("Shift",		  KerArgSpace(1,D0), OBJ_OUT_DB,	  1, 1,           	sizeof(short int),		   0, 0, 0, "PreempShift"):
			KerArg("Shift",		  KerArgSpace(1,T0), O_BUFF|O_NTILED,	  1, 1,           	sizeof(short int),		   0, 0, 0, "")):0),
			!NoWindow?
			KerArg("WinTable",        KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, FrameSize,		LUTItemSize,			   0, 0, 0, "WinTable"):AT_NO_KER_ARG,
			UseRadix4?
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 3*(Nfft)/4,	LUTItemSize,			   0, 0, 0, "Twiddles_fft_int"):
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft/2,     	LUTItemSize, 			   0, 0, 0, "Twiddles_fft_int"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft/2,      	sizeof(short int),      	   0, 0, 0, "SwapTable_fft"),
			KerArg("Twiddles_rfft",	  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,		LUTItemSize,			   0, 0, 0, "Twiddles_rfft"),
			KerArg("Mel_FilterBank",  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, NMelBanks,		6, /* size of filterbank type */   0, 0, 0, "Mel_FilterBank"),
			KerArg("Mel_Coeffs",      KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, SizeMelCoeff,	LUTItemSize,		   	   0, 0, 0, "Mel_Coeffs"),
			DataType==FIX16?
			KerArg("shift_buff",   	  KerArgSpace(1,T0), O_BUFF,    	  1, NMelBanks,		sizeof(signed char),		   0, 0, 0, ""):0,
		  	Ndct?
		  	KerArg("DCT_Coeff",  	  KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Ndct*Ndct,         LUTItemSize, 		           0, 0, 0, "DCT_Coeff"):0
		)
	);
	if (Kernel) {
		AddKernelInfos(Name, AT_KERINFO_OPER, LayerOp, 0);
		AddKernelInfos(Name, AT_KERINFO_BANDWIDTH, LayerBandwidth, 0);

		if (DataType==FIX32 || DataType==FIX16) {
			AddKernelArgDim(Name, "In",   2, FrameStride*(Abs(NFrames)-1)+FrameSize, InItemSize);
			AddKernelArgDim(Name, "Out",  3, Abs(NFrames), Ndct?Ndct:NMelBanks, OutItemSize);
			if (!NoWindow)
			AddKernelArgDim(Name, "WinTable",   2, FrameSize, LUTItemSize);
			AddKernelArgDim(Name, "Twiddles_fft_int",  2, UseRadix4?(3*(Nfft)/4):(Nfft/2), LUTItemSize);
			AddKernelArgDim(Name, "Twiddles_rfft",  2, Nfft/2, LUTItemSize);
			AddKernelArgDim(Name, "Mel_Coeffs",  2, SizeMelCoeff, LUTItemSize);
			if (Ndct)
			AddKernelArgDim(Name, "DCT_Coeff",  2, Ndct*Ndct, LUTItemSize);
		} else {
			AddKernelFloatArgDim(Name, "In",   2, FrameStride*(Abs(NFrames)-1)+FrameSize, InItemSize);
			AddKernelFloatArgDim(Name, "Out",  3, Abs(NFrames), Ndct?Ndct:NMelBanks, OutItemSize);
			if (!NoWindow)
			AddKernelFloatArgDim(Name, "WinTable",   2, FrameSize, LUTItemSize);
			AddKernelFloatArgDim(Name, "Twiddles_fft_int",  2, UseRadix4?(3*(Nfft)/4):(Nfft/2), LUTItemSize);
			AddKernelFloatArgDim(Name, "Twiddles_rfft",  2, Nfft/2, LUTItemSize);
			AddKernelFloatArgDim(Name, "Mel_Coeffs",  2, SizeMelCoeff, LUTItemSize);
			if (Ndct)
			AddKernelFloatArgDim(Name, "DCT_Coeff",  2, Ndct*Ndct, LUTItemSize);			
		}
		AddKernelArgDim(Name, "SwapTable_fft",  2, Nfft/2, 2);
		AddKernelArgDim(Name, "Mel_FilterBank",  3, NMelBanks, 3, 2);
	}
	return (Kernel!=0);
}

int RFFT_2D_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int FrameSize,
	int FrameStride,
	int Nfft,
	float PreempFactor,
	int SkipPreemp,
	int NoWindow,
	int OutFFT,
	int MagSquared,
	int DataType
	)
{
	if (__builtin_popcount(Nfft) != 1 || Nfft < FrameSize) GenTilingError("%s, Incorrect FFTDim: %d, it has to be a a power of 2 and > %d (FrameSize)", Name, Nfft, FrameSize);
	AT_PadType PadType = PAD_RIGHT;
	if (Ctrl) {
		if (Ctrl->PadType != -1 && Nfft>FrameSize) PadType = Ctrl->PadType;
	}
	if (Nfft < FrameSize) GenTilingError("%s, Incorrect FFTDim: %d, it has to be a greater than FrameSize %d", Name, Nfft, FrameSize);
	int UseRadix4 = (DataType!=FIX32) && (((__builtin_ctz(Nfft>>1)%2)==0) && ((Nfft>>1)>64));

	int Log2Nfft = 31 - __builtin_clz(Nfft);
	int Log4Nfft = Log2Nfft>>1;
	int QIn_FFT = UseRadix4?12:13;
	int Q_Out_FFT = 15 - UseRadix4?(Log4Nfft*2-1):(Log2Nfft-2);

	int OnlyZeroPad = (Nfft > FrameSize) && NoWindow;

	char *PreEmpKernel=0, *WinKernel=0, *RFFT_Kernel=0, *SpectKernel=0, *UserKernType=0, *UserKernPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;
	switch (DataType){
		case FIX16:
			PreEmpKernel = SkipPreemp?0:"PreEmphasis";
			WinKernel = NoWindow?"ZeroPad_Fix16":(PadType==PAD_RIGHT?"WindowingReal2Real_Fix16":"WindowingReal2Real_PadCenter_Fix16");
			RFFT_Kernel = "RFFT_DIF_Par_Fix16";
			SpectKernel = MagSquared?"CmplxMagSquared_Fix16":"CmplxMag_Fix32";
			UserKernType = "short int";
			UserKernPointer = "short int * __restrict__";
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		case FLOAT16:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f16":0;
			WinKernel = NoWindow?"ZeroPad_f16":(PadType==PAD_RIGHT?"WindowingReal2Real_f16":"WindowingReal2Real_PadCenter_f16");
			RFFT_Kernel = "RFFT_DIF_Par_f16";
			SpectKernel = MagSquared?"CmplxMagSquared_f16":"CmplxMag_f16";
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE, OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FIX32:
			PreEmpKernel = SkipPreemp?0:"PreEmphasis";
			WinKernel = NoWindow?"ZeroPad_Fix32":(PadType==PAD_RIGHT?"WindowingReal2Real_Fix32":"WindowingReal2Real_PadCenter_Fix32");
			RFFT_Kernel = "RFFT_DIF_Par_Fix32_Scal";
			SpectKernel = MagSquared?"CmplxMagSquared_Fix32_scal":"CmplxMag_Fix32_scal";
			UserKernType = "short int";
			UserKernPointer = "short int * __restrict__";
			InItemSize=2, OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT32:
			PreEmpKernel = PreempFactor!=0?"PreEmphasis_f32":0;
			WinKernel = NoWindow?"ZeroPad_f32":(PadType==PAD_RIGHT?"WindowingReal2Real_f32":"WindowingReal2Real_PadCenter_f32");
			RFFT_Kernel = "RFFT_DIF_Par_f32";
			SpectKernel = MagSquared?"CmplxMagSquared_f32":"CmplxMag_f32";
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			InItemSize=4, OutItemSize=4, LUTItemSize=4;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
			return 0;
	}
	unsigned int LayerOp = Abs(NFrames) * ((PreEmpKernel?(2*FrameSize):0) + (WinKernel?Nfft:0) + Nfft*Log2Nfft*4 + Nfft);
	unsigned int LayerBandwidth = Abs(NFrames) * (FrameSize*InItemSize + OutItemSize*Nfft) + LUTItemSize*(WinKernel?FrameSize:0 + (UseRadix4?(3*(Nfft)/4):(Nfft/2)) + Nfft) + 2*Nfft/2;
	GenTilingDebug("RFFT2D:\n");
	GenTilingDebug("PreEmpKernel:	%25s\n", PreEmpKernel?PreEmpKernel:"");
	GenTilingDebug("WinKernel:	%25s\n", WinKernel?WinKernel:"");
	GenTilingDebug("RFFT_Kernel:	%25s\n", RFFT_Kernel?RFFT_Kernel:"");
	GenTilingDebug("SpectKernel:	%25s\n", SpectKernel?SpectKernel:"");
	GenTilingDebug("\tNb Oper: %d\n", LayerOp);
	GenTilingDebug("\tBandwidth: %d\n", LayerBandwidth);
	// GenTilingDebug("MFCC_COEF_DYN = %d\nFFT_BITS = %d\nUSE_DB = %d\nDATA_TYPE = %dLOG OFFSET %f\n", MFCC_Coeff_Dyn, Log2Nfft, UseDB, DataType, MfccLogOffset);

	UserSymbols(1, US_Float("PreempFactor", PreempFactor));
	int InBuffSize = (PadType!=PAD_RIGHT)?Nfft:FrameSize;
	Kernel_T *Kernel = UserKernel(Name,
                NFrames<0?
                KernelIterSpace(2, IterFixedSpaceDynBound(D0, 10000, "NFrames"), IterTiledSpace(T0)):
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(9,
                	TCArg(UserKernPointer, "In"),
			TCArg(UserKernPointer, "Out"),
			TCArg(UserKernPointer, "Twiddles_fft_int"),
			TCArg(UserKernPointer, "Twiddles_rfft"),
			TCArg("short int *", "SwapTable_fft"),
			!NoWindow?TCArg(UserKernPointer, "WinTable"):AT_NO_C_ARG,
			NFrames<0?TCArg("short int", "NFrames"):AT_NO_C_ARG,
			OutFFT&&(DataType==FIX16||DataType==FIX32)&&PreEmpKernel?TCArg("short int *", "PreempShift"):AT_NO_C_ARG,
			OutFFT&&DataType==FIX32&&PreEmpKernel?TCArg("signed char *", "shift_rfft"):AT_NO_C_ARG
                ),
                Calls(4,
                	PreEmpKernel?
                	Call(PreEmpKernel,LOC_LOOP,
			     Bindings(DataType==FIX16||DataType==FIX32?7:5,
			       K_Arg("In", 	KER_ARG_TILE),
			       K_Arg("Out", KER_ARG_TILE),
			       Imm(0),
			       (DataType==FIX16||DataType==FIX32)?Imm(FP2FIX(PreempFactor, 15)):BindKExpr("PreempFactor"),
			       Imm(FrameSize),
			       (DataType==FIX16||DataType==FIX32)?K_Arg("Shift", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16||DataType==FIX32)?Imm(QIn_FFT):AT_IGNORE_ARG_BINDING)
			     ):AT_NO_CALL,
			Call(WinKernel,LOC_LOOP,
			     Bindings(5,
			       PreEmpKernel?K_Arg("Out", KER_ARG_TILE):K_Arg("In", KER_ARG_TILE),
			       K_Arg("In_rfft", KER_ARG_TILE),
			       !NoWindow?K_Arg("WinTable" , KER_ARG_TILE ):AT_IGNORE_ARG_BINDING,
			       (PadType==PAD_RIGHT)?Imm(FrameSize):Imm(Nfft),
			       Imm(Nfft))
			     ),
			Call(RFFT_Kernel,LOC_LOOP,
			      Bindings(DataType==FIX32?9:7,
			       K_Arg("In_rfft", KER_ARG_TILE),
			       K_Arg("Out",    	KER_ARG_TILE),
			       K_Arg("Twiddles_fft_int",  KER_ARG_TILE),
			       K_Arg("Twiddles_rfft", KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       Imm(0),
			       (DataType==FIX32)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX32)?K_Arg("shift_rfft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ),
			OutFFT==0?
			Call(SpectKernel,LOC_LOOP,
			     Bindings(6,
			       K_Arg("Out", KER_ARG_TILE),
			       K_Arg("Out",      KER_ARG_TILE),
			       Imm(Nfft),
			       (DataType==FIX16||DataType==FIX32)?K_TileOper("Shift", "short int *", '@', 0):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX16||DataType==FIX32)?Imm(Q_Out_FFT):AT_IGNORE_ARG_BINDING,
			       (DataType==FIX32)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ):AT_NO_CALL
		),
		KerArgs(10,
			KerArg("In",		KerArgSpace(1,D0), OBJ_IN_DB,		1, InBuffSize, 	InItemSize, InBuffSize-FrameStride, 0, 0, "In"),
			KerArg("Out",		KerArgSpace(1,D0), OBJ_OUT_DB,		1, 2*(Nfft/2+1),OutItemSize, 			   	   0, 0, 0, "Out"),
			KerArg("In_rfft",	KerArgSpace(1,T0), O_BUFF|O_NTILED,	1, Nfft, 	InItemSize,    		   		   0, 0, 0, ""),
			((DataType==FIX16||DataType==FIX32)&&PreEmpKernel?
			(OutFFT?
			KerArg("Shift",		KerArgSpace(1,D0), OBJ_OUT_DB,		1, 1,           sizeof(short int),			   0, 0, 0, "PreempShift"):
			KerArg("Shift",		KerArgSpace(1,T0), O_BUFF|O_NTILED,	1, 1,           sizeof(short int),			   0, 0, 0, "")):0),
			DataType==FIX32?
			KerArg("shift_fft",	KerArgSpace(1,D0), OBJ_OUT_DB,		1, Nfft,        sizeof(signed char),			   0, 0, 0, ""):0,
			(DataType==FIX32?
			(OutFFT?
			KerArg("shift_rfft",	KerArgSpace(1,D0), OBJ_OUT_DB,		1, Nfft,        sizeof(signed char),			   0, 0, 0, "shift_rfft"):
			KerArg("shift_rfft",	KerArgSpace(1,T0), O_BUFF|O_NTILED,	1, Nfft,        sizeof(signed char),			   0, 0, 0, "")):0),
			!NoWindow?
			KerArg("WinTable",      KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, FrameSize,	LUTItemSize,				   0, 0, 0, "WinTable"):AT_NO_KER_ARG,
			UseRadix4?
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, 3*(Nfft)/4,  LUTItemSize,				   0, 0, 0, "Twiddles_fft_int"):
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, Nfft/2,     	LUTItemSize,	 			   0, 0, 0, "Twiddles_fft_int"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, Nfft/2,      sizeof(short int),        		   0, 0, 0, "SwapTable_fft"),
			KerArg("Twiddles_rfft",	KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,	LUTItemSize,				   0, 0, 0, "Twiddles_rfft")
		)
	);
	if (Kernel) {
		AddKernelInfos(Name, AT_KERINFO_OPER, LayerOp, 0);
		AddKernelInfos(Name, AT_KERINFO_BANDWIDTH, LayerBandwidth, 0);

		if (DataType==FIX32 || DataType==FIX16) {
			AddKernelArgDim(Name, "In",   2, FrameStride*(Abs(NFrames)-1)+FrameSize, InItemSize);
			AddKernelArgDim(Name, "Out",  3, Abs(NFrames), 2*(Nfft/2+1), OutItemSize);
			if (!NoWindow)
			AddKernelArgDim(Name, "WinTable",   2, FrameSize, LUTItemSize);
			AddKernelArgDim(Name, "Twiddles_fft_int",  2, UseRadix4?(3*(Nfft)/4):(Nfft/2), LUTItemSize);
			AddKernelArgDim(Name, "Twiddles_rfft",  2, Nfft/2, LUTItemSize);
		} else {
			AddKernelFloatArgDim(Name, "In",   2, FrameStride*(Abs(NFrames)-1)+FrameSize, InItemSize);
			AddKernelFloatArgDim(Name, "Out",  3, Abs(NFrames), 2*(Nfft/2+1), OutItemSize);
			if (!NoWindow)
			AddKernelFloatArgDim(Name, "WinTable",   2, FrameSize, LUTItemSize);
			AddKernelFloatArgDim(Name, "Twiddles_fft_int",  2, UseRadix4?(3*(Nfft)/4):(Nfft/2), LUTItemSize);
			AddKernelFloatArgDim(Name, "Twiddles_rfft",  2, Nfft/2, LUTItemSize);			
		}
		AddKernelArgDim(Name, "SwapTable_fft",  2, Nfft/2, 2);
	}
	return (Kernel!=0);
}

int IRFFT_2D_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int Nfft,
	int InvertWindow,
	int DataType
	)
{
	AT_PadType PadType = PAD_RIGHT;
	if (Ctrl) {
		if (Ctrl->PadType != -1) PadType = Ctrl->PadType;
	}
	if (__builtin_popcount(Nfft) != 1) GenTilingError("%s, Incorrect FFTDim: %d, it has to be a a power of 2", Name, Nfft);
	int UseRadix4 = (DataType!=FIX32) && (((__builtin_ctz(Nfft>>1)%2)==0) && ((Nfft>>1)>64));

	char *IRFFT_Kernel=0, *WinKernel=0, *UserKernType=0, *UserKernPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;
	switch (DataType){
		case FIX16:
			IRFFT_Kernel = "IRFFT_DIF_Par_Fix16";
			UserKernType = "short int";
			UserKernPointer = "short int * __restrict__";
			WinKernel = InvertWindow?"InvWindowing_Fix16":0;
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		case FLOAT16:
			IRFFT_Kernel = "IRFFT_DIF_Par_f16";
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			WinKernel = InvertWindow?"WindowingReal2Real_f16":0;
			InItemSize=F16_SIZE, OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FIX32:
			GenTilingError("FIX32 Dtype not supported yet for IRFFT");
			IRFFT_Kernel = "IRFFT_DIF_Par_Fix32_Scal";
			UserKernType = "short int";
			UserKernPointer = "short int * __restrict__";
			WinKernel = InvertWindow?"WindowingReal2Real_Fix16":0;
			InItemSize=2, OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT32:
			IRFFT_Kernel = "IRFFT_DIF_Par_f32";
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			WinKernel = InvertWindow?"WindowingReal2Real_f32":0;
			InItemSize=4, OutItemSize=4, LUTItemSize=4;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
			return 0;
	}

	Kernel_T *Kernel = UserKernel(Name,
                NFrames<0?
                KernelIterSpace(2, IterFixedSpaceDynBound(D0, 10000, "NFrames"), IterTiledSpace(T0)):
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(7,
                	TCArg(UserKernPointer, "In"),
			TCArg(UserKernPointer, "Out"),
			TCArg(UserKernPointer, "Twiddles_fft_int"),
			TCArg(UserKernPointer, "Twiddles_rfft"),
			TCArg("short int *", "SwapTable_fft"),
			WinKernel?
			TCArg(UserKernPointer, "InvWinTable"):AT_NO_C_ARG,
			NFrames<0?
			TCArg("short int", "NFrames"):AT_NO_C_ARG
                ),
                Calls(2,
			Call(IRFFT_Kernel,LOC_LOOP,
				Bindings(7,
					K_Arg("In", KER_ARG_TILE),
					K_Arg("Out",    	KER_ARG_TILE),
					K_Arg("Twiddles_fft_int",  KER_ARG_TILE),
					K_Arg("Twiddles_rfft", KER_ARG_TILE),
					K_Arg("SwapTable_fft", KER_ARG_TILE),
					Imm(Nfft),
					Imm(1)
				)
			),
			WinKernel?Call(WinKernel, LOC_LOOP,
				Bindings(5,
					K_Arg("Out", KER_ARG_TILE),
					K_Arg("Out", KER_ARG_TILE),
					K_Arg("InvWinTable", KER_ARG_TILE),
					Imm(Nfft),
					Imm(Nfft)
				)
			):AT_NO_CALL
		),
		KerArgs(6,
			KerArg("In",		KerArgSpace(1,D0), OBJ_IN_DB,		1, 2*(Nfft/2+1),InItemSize, 	   0, 0, 0, "In"),
			KerArg("Out",		KerArgSpace(1,D0), OBJ_OUT_DB,		1, Nfft,	OutItemSize,	   0, 0, 0, "Out"),
			InvertWindow?
			KerArg("InvWinTable",	KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,	1, Nfft,	LUTItemSize,	   0, 0, 0, "InvWinTable"):AT_NO_KER_ARG,
			UseRadix4?
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, 3*(Nfft)/4,	LUTItemSize,	   0, 0, 0, "Twiddles_fft_int"):
			KerArg("Twiddles_fft_int",KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, Nfft/2,     	LUTItemSize, 	   0, 0, 0, "Twiddles_fft_int"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0),O_IN|O_BUFF|O_CONST,1, Nfft/2,      sizeof(short int), 0, 0, 0, "SwapTable_fft"),
			KerArg("Twiddles_rfft",	KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,	LUTItemSize,	   0, 0, 0, "Twiddles_rfft")
		)
	);
	return 1;
}

void FFT_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int Nfft,
	int ForceRadix2,
	int DataType,
	int IntScal
	)
{
	if (__builtin_popcount(Nfft) != 1) GenTilingError("FFT: %s, Incorrect Nfft: %d, it has to be a a power of 2", Name, Nfft);
	int UseRadix4 = (!ForceRadix2 && ((__builtin_ctz(Nfft)%2)==0) && (Nfft>64));
	char *FFTKernel=0, *SwapKernel=0, *InPointer=0, *TwidPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;

	switch (DataType){
		case FIX16:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_Fix16";
			else           FFTKernel = "Radix2FFT_DIF_Par_Fix16";
			SwapKernel  = "SwapSamples_Par";
			InPointer   = "int16_t * __restrict__";
			TwidPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		case FIX32:
			FFTKernel   = IntScal?"Radix2FFT_DIF_INT_Scal_Par":"Radix2FFT_DIF_Par_Fix32";
			SwapKernel  = IntScal?"SwapSamples_scal":"SwapSamples_Par_Fix32";
			InPointer   = "int * __restrict__";
			TwidPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT16:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f16";
			else  	       FFTKernel = "Radix2FFT_DIF_Par_f16";
			SwapKernel  = "SwapSamples_Par_f16";
			InPointer   = "F16_DSP * __restrict__";
			TwidPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE, OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FLOAT32:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f32";
			else  	       FFTKernel = "Radix2FFT_DIF_Par_f32";
			SwapKernel  = "SwapSamples_Par_f32";
			InPointer   = "float * __restrict__";
			TwidPointer = "float * __restrict__";
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		default:
			return GenTilingError("Data Type %d not known", DataType);
	}
	int ncalls = 10;
	Kernel_T *Kernel = UserKernel(Name,
                KernelIterSpace(1, IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(4,
                	TCArg(InPointer, "In"),
			TCArg(TwidPointer, "Twiddles_fft"),
			TCArg("short int *",   "SwapTable_fft"),
			(DataType==FIX32 && IntScal)?TCArg("char *", "FFT_Shift_Buff"):AT_NO_C_ARG
                     ),
                Calls(2,
			Call(FFTKernel,LOC_LOOP,
			      Bindings((DataType==FIX32 && IntScal)?5:4,
			       K_Arg("In",       KER_ARG_TILE),
			       K_Arg("Twiddles_fft", KER_ARG_TILE ),
			       Imm(Nfft),
			       Imm(0),
			       (DataType==FIX32 && IntScal)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ),
			Call(SwapKernel,LOC_LOOP,
			      Bindings((DataType==FIX32 && IntScal)?4:3,
			       K_Arg("In",       KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       (DataType==FIX32 && IntScal)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     )
		),
		KerArgs(4,
			KerArg("In",              KerArgSpace(1,T0), OBJ_IN_OUT,	  1, 2*Nfft, 	  	    InItemSize,  0, 0, 0, "In"),
			(DataType==FIX32 && IntScal)?
			KerArg("shift_fft",       KerArgSpace(1,T0), OBJ_IN_OUT,	  1, Nfft,			      1, 0, 0, 0, "FFT_Shift_Buff"):AT_NO_KER_ARG,
			UseRadix4?KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(3*Nfft/4),   LUTItemSize, 0, 0, 0, "Twiddles_fft"):
				  KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(Nfft),       LUTItemSize, 0, 0, 0, "Twiddles_fft"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,             		      2, 0, 0, 0, "SwapTable_fft")
		)
	);
}

void IFFT_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int Nfft,
	int ForceRadix2,
	int DataType,
	int IntScal
	)
{
	int UseRadix4 = (!ForceRadix2 && ((__builtin_ctz(Nfft)%2)==0) && (Nfft>64));
	char *FFTKernel=0, *SwapKernel=0, *UserKernType=0, *UserKernPointer=0, *ConjugateKernel=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;

	switch (DataType){
		case FIX16:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_Fix16";
			else  	       FFTKernel = "Radix2FFT_DIF_Par";
			SwapKernel = "SwapSamples_Par";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		case FIX32:
			if (UseRadix4) return GenTilingError("Radix4 FFT Not implemented for FIX32");
			else  	       FFTKernel = "Radix2FFT_DIF_INT_Scal_Par";
			SwapKernel = "SwapSamples_scal";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT16:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f16";
			else	       FFTKernel = "Radix2FFT_DIF_Par_f16";
			SwapKernel = "SwapSamples_Par_f16";
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE, OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FLOAT32:
			if (UseRadix4) FFTKernel = "Radix4FFT_DIF_Par_f32";
			else  	       FFTKernel = "Radix2FFT_DIF_Par_f32";
			SwapKernel = "SwapSamples_Par_f32";
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			InItemSize=4, OutItemSize=4, LUTItemSize=4;
			break;
		default:
			return GenTilingError("Data Type %d not known", DataType);
	}

	Kernel_T *Kernel = UserKernel(Name,
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(DataType==FIX32?4:3,
                	TCArg(UserKernPointer, "In"),
			TCArg(UserKernPointer, "Twiddles_fft"),
			TCArg("short int *", "SwapTable_fft"),
			TCArg("signed char *", "FFT_Shift_Buff")
                ),
                Calls(2,
                	Call(FFTKernel,LOC_LOOP,
			     Bindings(DataType==FIX32?5:4,
			       K_Arg("In",       KER_ARG_TILE),
			       K_Arg("Twiddles_fft", KER_ARG_TILE ),
			       Imm(Nfft),
			       Imm(1),
			       (DataType==FIX32 && IntScal)?K_Arg("shift_fft", KER_ARG_TILE):AT_IGNORE_ARG_BINDING)
			     ),
			Call(SwapKernel,LOC_LOOP,
			     Bindings(DataType==FIX32?4:3,
			       K_Arg("In",       KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       K_Arg("shift_fft", KER_ARG_TILE)) //shift_fft buffer
			     )
		),
		KerArgs(4,	
			KerArg("In",              KerArgSpace(1,D0), OBJ_IN_DB,	  	  1, 2*Nfft,			 InItemSize, 0, 0, 0, "In"),
			(DataType==FIX32 && IntScal)?
			KerArg("shift_fft",       KerArgSpace(1,T0), OBJ_IN_OUT,	  1, Nfft,			  	  1, 0, 0, 0, "FFT_Shift_Buff"):AT_NO_KER_ARG,
			UseRadix4?KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(3*Nfft/4),   	LUTItemSize, 0, 0, 0, "Twiddles_fft"):
				  KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(Nfft),       	LUTItemSize, 0, 0, 0, "Twiddles_fft"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,                sizeof(short int),     0, 0, 0, "SwapTable_fft")
		)
	);
}

void STFT_Generator(
	char *Name,
	CNN_GenControl_T *Ctrl,
	int NFrames,
	int FrameSize,
	int FrameStride,
	int Nfft,
	float PreempFactor,
	int use_radix_4_fft,
	int out_fft,
	int use_power,
	int DataType
	)
{
	int fft_bits = 31 - __builtin_clz(Nfft);
	char *PreEmpKernel=0, *WinKernel=0, *FFTKernel=0, *SwapKernel=0, *MfccKernel=0, *SpectKernel=0, *LogKernel=0, *DCTKernel=0, *UserKernType=0, *UserKernPointer=0, InItemSize=2, OutItemSize=2, LUTItemSize=2;

	switch (DataType){
		case FIX16:
			PreEmpKernel = "MFCC_PreEmphasis";
			WinKernel = "MFCC_WindowedFrame";
			if (use_radix_4_fft) FFTKernel = "Radix4FFT_DIF_Par_Fix16";
			else  		     FFTKernel = "Radix2FFT_DIF_Par";
			SwapKernel = "SwapSamples_Par";
			SpectKernel = use_power?"MFCC_PowerV2S":"MFCC_Abs";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=2, LUTItemSize=2;
			break;
		case FIX32:
			PreEmpKernel = "MFCC_PreEmphasis";
			WinKernel = "MFCC_WindowedFrame_int";
			if (use_radix_4_fft) return GenTilingError("Radix4 FFT Not implemented for FIX32");
			else  		     FFTKernel = "Radix2FFT_DIF_INT_Scal_Par";
			SwapKernel = "SwapSamples_scal";
			SpectKernel = use_power?"MFCC_Power":"MFCC_Abs_BFF";
			UserKernType = "int16_t";
			UserKernPointer = "int16_t * __restrict__";
			InItemSize=2, OutItemSize=4, LUTItemSize=2;
			break;
		case FLOAT16:
			PreEmpKernel = "MFCC_PreEmphasis_f16";
			WinKernel = "MFCC_WindowedFrame_f16";
			if (use_radix_4_fft) FFTKernel = "Radix4FFT_DIF_Par_f16";
			else  		     FFTKernel = "Radix2FFT_DIF_Par_f16";
			SwapKernel = "SwapSamples_Par_f16";
			SpectKernel = use_power?"MFCC_Power_f16":"MFCC_Abs_f16";
			UserKernType = "F16_DSP";
			UserKernPointer = "F16_DSP * __restrict__";
			InItemSize=F16_SIZE, OutItemSize=F16_SIZE, LUTItemSize=F16_SIZE;
			break;
		case FLOAT32:
			PreEmpKernel = "MFCC_PreEmphasis_f32";
			WinKernel = "MFCC_WindowedFrame_f32";
			if (use_radix_4_fft) FFTKernel = "Radix4FFT_DIF_Par_f32";
			else  		     FFTKernel = "Radix2FFT_DIF_Par_f32";
			SwapKernel = "SwapSamples_Par_f32";
			SpectKernel = use_power?"MFCC_Power_f32":"MFCC_Abs_f32";
			UserKernType = "float";
			UserKernPointer = "float * __restrict__";
			InItemSize=4, OutItemSize=4, LUTItemSize=4;
			break;
		default:
			return GenTilingError("Data Type %d not known", DataType);
	}

	Kernel_T *Kernel = UserKernel(Name,
                KernelIterSpace(2, IterFixedSpace(D0, NFrames), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(6,
                	TCArg(UserKernPointer, "In"),
			TCArg(DataType==FIX16||DataType==FIX32?"int * __restrict__":UserKernPointer, "Out"),
			TCArg(UserKernType,    "Prev"),
			TCArg(UserKernPointer, "Twiddles_fft"),
			TCArg("short int *", "SwapTable_fft"),
			TCArg(UserKernPointer, "WinTable")
                ),
                Calls(6,
			Call(PreEmpKernel,LOC_LOOP,
			     Bindings(6,
			       K_Arg("In"   , KER_ARG_TILE),
			       K_Arg("InOut1"   , KER_ARG_TILE),
			       C_Arg("Prev"),
			       (DataType==FIX16 || DataType==FIX32)?Imm(FP2FIX(PreempFactor, 15)):Imm(PreempFactor),
			       K_Arg("Shift"   , KER_ARG_TILE),
			       Imm(FrameSize)
			       )
			     ),
			Call(WinKernel,LOC_LOOP,
			     Bindings(5,
			       K_Arg("InOut1"  , KER_ARG_TILE),
			       out_fft?K_Arg("Out", KER_ARG_TILE):K_Arg("Out_fft", KER_ARG_TILE),
			       K_Arg("WinTable" , KER_ARG_TILE ),
			       Imm(FrameSize),
			       Imm(Nfft))
			     ),
			Call(FFTKernel,LOC_LOOP,
			      Bindings(DataType==FIX32?4:5,
			       out_fft?K_Arg("Out", KER_ARG_TILE):K_Arg("Out_fft", KER_ARG_TILE),
			       K_Arg("Twiddles_fft", KER_ARG_TILE ),
			       K_Arg("shift_fft", KER_ARG_TILE), //shift_fft buffer
			       Imm(Nfft),
			       Imm(0))
			     ),
			Call(SwapKernel,LOC_LOOP,
			      Bindings(DataType==FIX32?4:3,
			       out_fft?K_Arg("Out", KER_ARG_TILE):K_Arg("Out_fft", KER_ARG_TILE),
			       K_Arg("SwapTable_fft", KER_ARG_TILE),
			       Imm(Nfft),
			       K_Arg("shift_fft", KER_ARG_TILE)) //shift_fft buffer
			     ),
			out_fft==0?Call(SpectKernel,LOC_LOOP,
			     Bindings(5,
			       K_Arg("Out_fft"   , KER_ARG_TILE),
			       (DataType==FLOAT32||DataType==FLOAT16)?K_Arg("Out", KER_ARG_TILE):K_Arg("InOut1", KER_ARG_TILE),
			       K_Arg("shift_fft"   , KER_ARG_TILE),
		       	       K_Arg("Shift"   , KER_ARG_TILE),
			       Imm(Nfft))
			     ):AT_NO_CALL,
			out_fft==0&&(DataType==FIX16||DataType==FIX32)?
				Call("norm_clip_32_spect", LOC_LOOP,
				     Bindings(6,
				        K_Arg("InOut1",    KER_ARG_TILE),
				 	K_Arg("Out",    KER_ARG_TILE),
					K_Arg("Shift",  KER_ARG_TILE), // PreEmphasis Shift
					Imm(fft_bits), // fft_bits
				       	K_Arg("shift_fft", KER_ARG_TILE), //shift_bf buffer in case of high_prec_fft
					Imm(Nfft)
					)
				):AT_NO_CALL
		),
		KerArgs(9,
			KerArg("In",              KerArgSpace(1,D0), OBJ_IN_DB,	  	  1, FrameSize, InItemSize,    FrameSize-FrameStride, 0, 0, "In"),
			out_fft==0?
				KerArg("Out",     KerArgSpace(1,D0), OBJ_OUT_DB,   	  1, DataType==FIX32?2*(Nfft/2+1):(Nfft/2+1),		DataType==FLOAT16?F16_SIZE:4, 0, 0, 0, "Out"):
				KerArg("Out",     KerArgSpace(1,D0), OBJ_OUT_DB,	  1, (DataType==FLOAT32||DataType==FIX32)?2*Nfft:Nfft,  DataType==FLOAT16?F16_SIZE:4, 0, 0, 0, "Out"),
			KerArg("InOut1",          KerArgSpace(1,T0), O_BUFF,   		  1, (DataType==FLOAT32||DataType==FIX32)?2*Nfft:Nfft,  DataType==FLOAT16?F16_SIZE:4, 0, 0, 0, ""),
			out_fft==0?
				KerArg("Out_fft", KerArgSpace(1,T0), O_BUFF,	   	  1, (DataType==FLOAT32||DataType==FIX32)?2*Nfft:Nfft,  DataType==FLOAT16?F16_SIZE:4, 0, 0, 0, ""):AT_NO_KER_ARG,
			KerArg("shift_fft",       KerArgSpace(1,T0), O_BUFF,       	  1, Nfft,						sizeof(signed char),   0, 0, 0, ""),
			KerArg("Shift",		  KerArgSpace(1,T0), O_BUFF,	          1, 1, 						sizeof(short int),     0, 0, 0, ""),
			KerArg("WinTable",        KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,						LUTItemSize,    0, 0, 0, "WinTable"),
			use_radix_4_fft?KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(3*Nfft/4),   LUTItemSize,		0, 0, 0, "Twiddles_fft"):
					KerArg("Twiddles_fft", KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, 2*(Nfft),       LUTItemSize,		0, 0, 0, "Twiddles_fft"),
			KerArg("SwapTable_fft",   KerArgSpace(1,T0), O_IN|O_BUFF|O_CONST, 1, Nfft,              	  sizeof(short int),    0, 0, 0, "SwapTable_fft")
		)
	);
}

int DSP_MatMul_Generator(
	char *Name,

	CNN_GenControl_T *Ctrl,

	int ColM1,
	int LineM1,
	int ColM2,
	int LineM2,

	int TransposedIn2,
	int DataType
)

{
	int Log = 1;
	Tile_Orientation_T TileOrientation = TILE_HOR;
	int F = 0;
	unsigned long long int LayerOp = 0;
	unsigned long long int LayerBandwidth = 0;
        int LineO = LineM1, ColO = ColM2;
	int Nbuff, ItemSize;

	if (ColM1 != LineM2) GenTilingError("DSP_MatMul_Generator: %s, Incorrect input matrices dimensions for a matrix multiplication: [%d x %d]*[%d x %d]", Name, LineM1, ColM1, LineM2, ColM2);

	char *MatMulKerName=0, *UserKernType=0, *UserKernPointer=0;
	switch (DataType){
		case FIX16:
			MatMulKerName = TransposedIn2?"KerParMatMulDSPT_Fix16":"KerParMatMulDSP_Fix16";
			UserKernType = "short int"; UserKernPointer = "short int * __restrict__";
			ItemSize=2;
			break;
		case FIX32:
			GenTilingError("DSP_MatMul_Generator Not yet implemented in FIX32");
			UserKernType = "int"; UserKernPointer = "int * __restrict__";
			ItemSize=2;
			break;
		case FLOAT16:
			MatMulKerName = TransposedIn2?"KerParMatMulDSPT_fp16":"KerParMatMulDSP_fp16";
			UserKernType = "F16_DSP"; UserKernPointer = "F16_DSP * __restrict__";
			ItemSize=F16_SIZE; F = O_FLOAT;
			break;
		case FLOAT32:
			MatMulKerName = TransposedIn2?"KerParMatMulDSPT_fp32":"KerParMatMulDSP_fp32";
			UserKernType = "float"; UserKernPointer = "float * __restrict__";
			ItemSize=4; F = O_FLOAT;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
	}


	int ColFirst = ((LineM1*ColM1)<(LineM2*ColM2));
	Nbuff = 4;
	LayerOp += ColM1*ColM2*LineM1;
	LayerBandwidth += LineM1*(ColM1*ColM2*(2+2));
	LayerBandwidth += LineM1*ColM2*2;
	LayerBandwidth += LineM1*2;
	
	if (Log) {
		GenTilingDebug("CNN_MatMulAct_fp16: %s\n", Name);
		GenTilingDebug("In1  => W: %4d, H: %4d\n", ColM1, LineM1);
		GenTilingDebug("In2  => W: %4d, H: %4d\n", ColM2, LineM2);
		GenTilingDebug("Out  => W: %4d, H: %4d => %s\n", ColO, LineO, ColFirst?"Column first":"Line First");
		GenTilingDebug("Total Op: %lld\n", LayerOp);
		if (MatMulKerName) GenTilingDebug("%20s: %s\n", "MatMulKerName", MatMulKerName);
	}

	int ObjCons = (!TransposedIn2)?OBJ_CONSTRAINTS_TILE_VER:0;
	if (TransposedIn2) {
		LineM2 = ColM2; ColM2 = ColM1;
	}
	Kernel_T *Kernel = UserKernel(Name,
		KernelIterSpace(2, IterTiledSpace(T1), IterTiledSpace(T0)),
                TILE_HOR,
                CArgs(4,
                      TCArg(UserKernPointer, "In1"),
                      TCArg(UserKernPointer, "In2"),
                      TCArg(UserKernPointer, "Out"),
                      (DataType==FIX16)?TCArg("short int", "Norm"):AT_NO_C_ARG
                ),
		Calls(1,
			Call(MatMulKerName, LOC_LOOP,
				Bindings(11,
					K_Arg("In1",  KER_ARG_TILE),
					K_Arg("In2",  KER_ARG_TILE),
					K_Arg("Out",  KER_ARG_TILE),
					(!TransposedIn2)?K_Arg("KerBuff", KER_ARG_TILE):AT_IGNORE_ARG_BINDING,
					K_Arg("In1",  KER_ARG_TILE_W),
					K_Arg("In1",  KER_ARG_TILE_H),
					TransposedIn2?K_Arg("In2",  KER_ARG_TILE_H):K_Arg("In2",  KER_ARG_TILE_W),
					K_Arg("Out", KER_ARG_TILE_W),
					K_Arg(ColFirst?"In1":"In2",  KER_ARG_TILE_BASE),
					Imm(ColFirst),
					DataType==FIX16?C_Arg("Norm"):AT_IGNORE_ARG_BINDING
				)
			)
		),
		ColFirst?
		KerArgs(4,
	    		(!TransposedIn2)?
	    		KerArg("KerBuff",KerArgSpace(1,T1), F|O_BUFF|O_NTILED, Nbuff*ColM1,      1, ItemSize, 0, 0,                                                0, 0):AT_NO_KER_ARG,
			KerArg("In1",    KerArgSpace(1,T0), F|O_IN|O_DB|O_CONST,     ColM1, LineM1, ItemSize, 0, OBJ_CONSTRAINTS_PAD_REM,                          8, "In1"),
			KerArg("In2",    KerArgSpace(1,T1), F|O_IN|O_DB,             ColM2, LineM2, ItemSize, 0, ObjCons|OBJ_CONSTRAINTS_PAD_REM,		   2, "In2"),
			KerArg("Out",    KerArgSpace(1,T1), F|O_OUT|O_DB,             ColO,  LineO, ItemSize, 0, OBJ_CONSTRAINTS_TILE_VER|OBJ_CONSTRAINTS_PAD_REM, 0, "Out")
		):
		KerArgs(4,
	 		(!TransposedIn2)?
	 		KerArg("KerBuff",KerArgSpace(1,T0), F|O_BUFF|O_NTILED, Nbuff*ColM1,      1, ItemSize, 0, 0,                                                0, 0):AT_NO_KER_ARG,
			KerArg("In1",    KerArgSpace(1,T1), F|O_IN|O_DB|O_CONST,     ColM1, LineM1, ItemSize, 0, OBJ_CONSTRAINTS_PAD_REM,                          8, "In1"),
			KerArg("In2",    KerArgSpace(1,T0), F|O_IN|O_DB,             ColM2, LineM2, ItemSize, 0, ObjCons|OBJ_CONSTRAINTS_PAD_REM, 	           2, "In2"),
			KerArg("Out",    KerArgSpace(1,T1), F|O_OUT|O_DB,             ColO,  LineO, ItemSize, 0, OBJ_CONSTRAINTS_PAD_REM,                          0, "Out")
		)
	);
	if (Kernel) {
		AddKernelInfos(Name, AT_KERINFO_OPER, LayerOp, 0);
		AddKernelInfos(Name, AT_KERINFO_BANDWIDTH, LayerBandwidth, 0);

		AddKernelFloatArgDim(Name, "In1", 3, LineM1, ColM1, ItemSize);
		AddKernelFloatArgDim(Name, "In2", 3, LineM2, ColM2, ItemSize);
		AddKernelFloatArgDim(Name, "Out", 3, LineO, ColO, ItemSize);
	}
	return (Kernel!=0);
}

int DSP_MatAdd_Generator(
	char *Name,

	CNN_GenControl_T *Ctrl,

	int W,
	int H,
	int DataType
)
{
	int Log = 1;
	Tile_Orientation_T TileOrientation = TILE_HOR;
	int F = 0;
	unsigned long long int LayerOp = (int64_t) W*H;
	unsigned long long int LayerBandwidth = (int64_t) W*H*3;

	int ItemSize;
	char *MatAddKerName=0, *UserKernType=0, *UserKernPointer=0;
	switch (DataType){
		case FIX16:
			MatAddKerName = "KerParMatAddDSP_Fix16";
			UserKernType = "short int"; UserKernPointer = "short int * __restrict__";
			ItemSize=2;
			break;
		case FIX32:
			MatAddKerName = "KerParMatAddDSP_Fix32";
			UserKernType = "int"; UserKernPointer = "int * __restrict__";
			ItemSize=4;
			break;
		case FLOAT16:
			MatAddKerName = "KerParMatAddDSP_fp16";
			UserKernType = "F16_DSP"; UserKernPointer = "F16_DSP * __restrict__";
			ItemSize=F16_SIZE; F = O_FLOAT;
			break;
		case FLOAT32:
			MatAddKerName = "KerParMatAddDSP_fp32";
			UserKernType = "float"; UserKernPointer = "float * __restrict__";
			ItemSize=4; F = O_FLOAT;
			break;
		default:
			GenTilingError("Data Type %d not known", DataType);
	}

	Kernel_T *Kernel = UserKernel(
		Name,
		// 2D kernel
		KernelIterSpace(1, IterTiledSpace(T0)),
		// Tile horizontally
		TILE_HOR,
		// C template for this user kernel
		CArgs(4,
                      TCArg(UserKernPointer, "In1"),
                      TCArg(UserKernPointer, "In2"),
                      TCArg(UserKernPointer, "Out"),
                      (DataType==FIX16 || DataType==FIX32)?TCArg("short int", "Norm"):AT_NO_C_ARG
		),
		// Basic kernel we want to call on tiles, in this case MatSumPar
		Calls(1,
			Call(MatAddKerName, LOC_LOOP,
				Bindings(6,
					K_Arg("In1", KER_ARG_TILE),   // A tile from In1
					K_Arg("In2", KER_ARG_TILE),   // A tile from In2
					K_Arg("Out", KER_ARG_TILE),   // A tile from Out

					K_Arg("In1", KER_ARG_TILE_W), // Tile width
					K_Arg("In1", KER_ARG_TILE_H), // Tile height

					(DataType==FIX16 || DataType==FIX32)?
					C_Arg("Norm"):AT_IGNORE_ARG_BINDING
				)
			)
		),
		// All 3 user kernel arguments are of dimension WxH with
		// basic type = plain int
		KerArgs(3,
			// First kernel argument, bound to In1. In1 in L2, double buffered access
			KerArg("In1", KerArgSpace(1,T0), O_IN|O_DB, W, H,  ItemSize, 0, 0, 0, "In1"),
			// Second kernel argument, bound to In2. In2 in L2, double buffered access
			KerArg("In2", KerArgSpace(1,T0), O_IN|O_DB, W, H,  ItemSize, 0, 0, 0, "In2"),
			// Third kernel argument, bound to Out. Out in L2, double buffered access
			KerArg("Out", KerArgSpace(1,T0), O_OUT|O_DB, W, H, ItemSize, 0, 0, 0, "Out")
		)
	);
	if (Kernel) {
		AddKernelInfos(Name, AT_KERINFO_OPER, LayerOp, 0);
		AddKernelInfos(Name, AT_KERINFO_BANDWIDTH, LayerBandwidth, 0);

		if (DataType==FIX16 || DataType==FIX32) {
			AddKernelArgDim(Name, "In1", 3, H, W, ItemSize);
			AddKernelArgDim(Name, "In2", 3, H, W, ItemSize);
			AddKernelArgDim(Name, "Out", 3, H, W, ItemSize);
		} else {
			AddKernelFloatArgDim(Name, "In1", 3, H, W, ItemSize);
			AddKernelFloatArgDim(Name, "In2", 3, H, W, ItemSize);
			AddKernelFloatArgDim(Name, "Out", 3, H, W, ItemSize);
		}
	}
	return (Kernel!=0);
}
