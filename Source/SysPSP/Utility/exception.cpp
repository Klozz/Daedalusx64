#include "stdafx.h"

#include <pspkernel.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ConfigOptions.h"
#include "Core/ROM.h"
#include "Core/CPU.h"

PspDebugRegBlock *exception_regs;

static const char *codeTxt[32] =
{
    "Interrupt", "TLB modification", "TLB load/inst fetch", "TLB store",
    "Address load/inst fetch", "Address store", "Bus error (instr)",
    "Bus error (data)", "Syscall", "Breakpoint", "Reserved instruction",
    "Coprocessor unusable", "Arithmetic overflow", "Unknown 14",
    "Unknown 15", "Unknown 16", "Unknown 17", "Unknown 18", "Unknown 19",
    "Unknown 20", "Unknown 21", "Unknown 22", "Unknown 23", "Unknown 24",
    "Unknown 25", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29",
    "Unknown 31"
};

static const unsigned char regName[32][5] =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

static void DumpInformation(PspDebugRegBlock * regs)
{
	FILE *fp = fopen("exception.txt", "wt");
	if (fp == NULL)
		return;

	fprintf(fp, "Exception details:\n");
	{
		fprintf(fp, "\tException - %s\n", codeTxt[(regs->cause >> 2) & 31]);
		fprintf(fp, "\tEPC       - %08X\n", (int)regs->epc);
		fprintf(fp, "\tCause     - %08X\n", (int)regs->cause);
		fprintf(fp, "\tStatus    - %08X\n", (int)regs->status);
		fprintf(fp, "\tBadVAddr  - %08X\n", (int)regs->badvaddr);
	}
	// output Registers Info
	fprintf(fp, "\nRegister File:\n");
	{
		for(int i=0; i<32; i+=4)
			fprintf(fp, "\t%s:%08X %s:%08X %s:%08X %s:%08X\n", regName[i], (int)regs->r[i], regName[i+1], (int)regs->r[i+1], regName[i+2], (int)regs->r[i+2], regName[i+3], (int)regs->r[i+3]);
	}

	fprintf(fp, "\nRom Infomation:\n");
	{
		fprintf(fp, "\tClockrate:       0x%08x\n", g_ROM.rh.ClockRate);
		fprintf(fp, "\tBootAddr:		0x%08x\n", SwapEndian(g_ROM.rh.BootAddress));
		fprintf(fp, "\tRelease:         0x%08x\n", g_ROM.rh.Release);
		fprintf(fp, "\tCRC1:            0x%08x\n", g_ROM.rh.CRC1);
		fprintf(fp, "\tCRC2:            0x%08x\n", g_ROM.rh.CRC2);
		fprintf(fp, "\tImageName:       '%s'\n",   g_ROM.rh.Name);
		fprintf(fp, "\tManufacturer:    0x%02x\n", g_ROM.rh.Manufacturer);
		fprintf(fp, "\tCartID:          0x%04x\n", g_ROM.rh.CartID);
		fprintf(fp, "\tCountryID:       0x%02x - '%c'\n", g_ROM.rh.CountryID, (char)g_ROM.rh.CountryID);
	}

	fprintf(fp, "\nSettings:\n");
	{
		fprintf(fp, "\tDoubleDisplayEnabled:          %01d\n", gDoubleDisplayEnabled);
		fprintf(fp, "\tDynarecEnabled:                %01d\n", gDynarecEnabled);
		fprintf(fp, "\tDynarecStackOptimisation:      %01d\n", gDynarecStackOptimisation);
		fprintf(fp, "\tDynarecLoopOptimisation:       %01d\n", gDynarecLoopOptimisation);	
		fprintf(fp, "\tOSHooksEnabled:                %01d\n", gOSHooksEnabled);
		fprintf(fp, "\tCheckN64FPUsageDisable:        %01d\n", gCheckN64FPUsageDisable);
	}

	fprintf(fp, "\nEmulation CPU State:\n");
	{
		for(int i=0; i<32; i+=4)
			fprintf(fp, "\t%s:%08X %s:%08X %s:%08X %s:%08X\n", 
			regName[i], gCPUState.CPU[i]._u32_0, regName[i+1], gCPUState.CPU[i+1]._u32_0, 
			regName[i+2], gCPUState.CPU[i+2]._u32_0, regName[i+3], gCPUState.CPU[i+3]._u32_0);

		fprintf(fp, "PC: %08x", gCPUState.CurrentPC);
	}

	fprintf(fp, "Disassembly:\n");
	{

	}
	fclose(fp);
}

void ExceptionHandler(PspDebugRegBlock * regs)
{
    int i;
    SceCtrlData pad;

    pspDebugScreenInit();
    pspDebugScreenSetBackColor(0x00FF0000);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenClear();
    pspDebugScreenPrintf("Your PSP has just crashed!\n");
    pspDebugScreenPrintf("Exception details:\n\n");
    pspDebugScreenPrintf("Exception - %s\n", codeTxt[(regs->cause >> 2) & 31]);
    pspDebugScreenPrintf("EPC       - %08X\n", (int)regs->epc);
    pspDebugScreenPrintf("Cause     - %08X\n", (int)regs->cause);
    pspDebugScreenPrintf("Status    - %08X\n", (int)regs->status);
    pspDebugScreenPrintf("BadVAddr  - %08X\n", (int)regs->badvaddr);
    for(i=0; i<32; i+=4) pspDebugScreenPrintf("%s:%08X %s:%08X %s:%08X %s:%08X\n", regName[i], (int)regs->r[i], regName[i+1], (int)regs->r[i+1], regName[i+2], (int)regs->r[i+2], regName[i+3], (int)regs->r[i+3]);

    sceKernelDelayThread(1000000);
    pspDebugScreenPrintf("\n\nPress X to dump information on file exception.log and quit");
    pspDebugScreenPrintf("\nPress O to quit");

    for (;;){
        sceCtrlPeekBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_CROSS){
			DumpInformation(regs);
            break;
        }else if (pad.Buttons & PSP_CTRL_CIRCLE){
            break;
        }
    }    
    sceKernelExitGame();
}

void initExceptionHandler()
{
   SceKernelLMOption option;
   int args[2], fd, modid;

   memset(&option, 0, sizeof(option));
   option.size = sizeof(option);
   option.mpidtext = PSP_MEMORY_PARTITION_KERNEL;
   option.mpiddata = PSP_MEMORY_PARTITION_KERNEL;
   option.position = 0;
   option.access = 1;

   if((modid = sceKernelLoadModule("exception.prx", 0, &option)) >= 0)
   {
      args[0] = (int)ExceptionHandler;
      args[1] = (int)&exception_regs;
      sceKernelStartModule(modid, 8, args, &fd, NULL);
   }
}
