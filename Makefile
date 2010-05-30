#
#	Specify:
#		DEBUG=y					- Debug build
#		CONFIG=<configname>		- Build using Source/Configs/<configname>
#

TARGET = Daedalus

DAED_MAIN_SRCS =	Source/SysPSP/main.cpp \
					Source/ConfigOptions.cpp \
					Source/Test/BatchTest.cpp

DAED_DEBUG_SRCS =	Source/SysPSP/Debug/DBGConsolePSP.cpp \
					Source/Debug/DebugLog.cpp \
					Source/Debug/Dump.cpp
					

DAED_CORE_SRCS =	Source/System.cpp \
					Source/Core/CPU.cpp \
					Source/Core/DMA.cpp \
					Source/Core/Interrupts.cpp \
					Source/Core/Memory.cpp \
					Source/Core/PIF.cpp \
					Source/Core/R4300.cpp \
					Source/Core/Registers.cpp \
					Source/Core/ROM.cpp \
					Source/Core/RomSettings.cpp \
					Source/Core/ROMBuffer.cpp \
					Source/Core/ROMImage.cpp \
					Source/Core/RSP.cpp \
					Source/Core/RSP_HLE.cpp \
					Source/Core/Savestate.cpp \
					Source/Core/TLB.cpp \
					Source/Core/Dynamo.cpp \
					Source/Core/Interpret.cpp \
					Source/Core/Save.cpp \
					Source/Core/JpegTask.cpp \
					Source/Core/FlashMem.cpp

DAED_INTERFACE_SRCS =	Source/Interface/RomDB.cpp

DAED_INPUT_SRCS =		Source/SysPSP/Input/InputManagerPSP.cpp

DAED_DYNREC_SRCS =		Source/SysPSP/DynaRec/AssemblyUtilsPSP.cpp \
						Source/SysPSP/DynaRec/AssemblyWriterPSP.cpp \
						Source/SysPSP/DynaRec/CodeBufferManagerPSP.cpp \
						Source/SysPSP/DynaRec/CodeGeneratorPSP.cpp \
						Source/SysPSP/DynaRec/DynarecTargetPSP.cpp \
						Source/SysPSP/DynaRec/N64RegisterCachePSP.cpp \
						Source/SysPSP/DynaRec/DynaRecStubs.S \
						Source/DynaRec/BranchType.cpp \
						Source/DynaRec/DynaRecProfile.cpp \
						Source/DynaRec/Fragment.cpp \
						Source/DynaRec/FragmentCache.cpp \
						Source/DynaRec/IndirectExitMap.cpp \
						Source/DynaRec/StaticAnalysis.cpp \
						Source/DynaRec/TraceRecorder.cpp

DAED_UTILITY_SRCS =		Source/Utility/CRC.cpp \
						Source/Utility/FramerateLimiter.cpp \
						Source/Utility/IniFile.cpp \
						Source/Utility/Hash.cpp \
						Source/Utility/MemoryHeap.cpp \
						Source/Utility/Preferences.cpp \
						Source/Utility/Profiler.cpp \
						Source/Utility/ROMFile.cpp \
						Source/Utility/ROMFileCache.cpp \
						Source/Utility/ROMFileCompressed.cpp \
						Source/Utility/ROMFileUncompressed.cpp \
						Source/Utility/Stream.cpp \
						Source/Utility/Timer.cpp \
						Source/Utility/unzip.cpp \
						Source/Utility/ZLibWrapper.cpp \
						Source/Utility/PrintOpCode.cpp \
						Source/Math/Matrix4x4.cpp
						
DAED_PSP_SRCS =	Source/SysPSP/Graphics/DrawText.cpp \
				Source/SysPSP/Graphics/GraphicsContext.cpp \
				Source/SysPSP/Graphics/NativeTexturePSP.cpp \
				Source/SysPSP/Graphics/VideoMemoryManager.cpp \
				Source/SysPSP/Graphics/PngUtilPSP.cpp \
				Source/SysPSP/Graphics/intraFont/intraFont.c \
				Source/SysPSP/Graphics/intraFont/libccc.c \
				Source/SysPSP/Debug/DaedalusAssertPSP.cpp \
				Source/Graphics/ColourValue.cpp \
				Source/SysPSP/UI/UIContext.cpp \
				Source/SysPSP/UI/UIElement.cpp \
				Source/SysPSP/UI/UICommand.cpp \
				Source/SysPSP/UI/UIComponent.cpp \
				Source/SysPSP/UI/UISetting.cpp \
				Source/SysPSP/UI/UIScreen.cpp \
				Source/SysPSP/UI/AboutComponent.cpp \
				Source/SysPSP/UI/ColourPulser.cpp \
				Source/SysPSP/UI/GlobalSettingsComponent.cpp \
				Source/SysPSP/UI/RomPreferencesScreen.cpp \
				Source/SysPSP/UI/AdvancedOptionsScreen.cpp \
				Source/SysPSP/UI/SavestateSelectorComponent.cpp \
				Source/SysPSP/UI/PauseOptionsComponent.cpp \
				Source/SysPSP/UI/SelectedRomComponent.cpp \
				Source/SysPSP/UI/AdjustDeadzoneScreen.cpp \
				Source/SysPSP/UI/MainMenuScreen.cpp \
				Source/SysPSP/UI/PauseScreen.cpp \
				Source/SysPSP/UI/SplashScreen.cpp \
				Source/SysPSP/Utility/AtomicPrimitives.S \
				Source/SysPSP/Utility/JobManager.cpp \
				Source/SysPSP/Utility/DebugMemory.cpp \
				Source/SysPSP/Utility/DisableFPUExceptions.S \
				Source/SysPSP/Utility/IOPSP.cpp \
				Source/SysPSP/Utility/ThreadPSP.cpp \
				Source/SysPSP/Utility/TimingPSP.cpp \
				Source/SysPSP/MediaEnginePRX/MediaEngine.S \
				Source/SysPSP/MediaEnginePRX/me.c \
				Source/SysPSP/DveMgr/pspDveManager.S \
				Source/SysPSP/KernelButtonsPrx/KernelButtons.S \
				Source/SysPSP/Utility/exception.cpp

DAED_HLEGFX_SRCS =	Source/SysPSP/Plugins/GraphicsPluginPSP.cpp \
					Source/Plugins/GraphicsPlugin.cpp \
					Source/HLEGraphics/Blender.cpp \
					Source/HLEGraphics/BlendModes.cpp \
					Source/HLEGraphics/ColourAdjuster.cpp \
					Source/HLEGraphics/ConvertImage.cpp \
					Source/HLEGraphics/DisplayListDebugger.cpp \
					Source/HLEGraphics/DLParser.cpp \
					Source/HLEGraphics/Microcode.cpp \
					Source/HLEGraphics/RDP.cpp \
					Source/HLEGraphics/RDPStateManager.cpp \
					Source/HLEGraphics/PSPRenderer.cpp \
					Source/HLEGraphics/Texture.cpp \
					Source/HLEGraphics/TextureCache.cpp \
					Source/HLEGraphics/TextureDescriptor.cpp \
					Source/HLEGraphics/gsp/gspMacros.cpp \
					Source/HLEGraphics/gsp/gspSprite2D.cpp \
					Source/HLEGraphics/gsp/gspS2DEX.cpp \
					Source/HLEGraphics/gsp/gspCustom.cpp \
					Source/SysPSP/HLEGraphics/ConvertVertices.S \
					Source/SysPSP/HLEGraphics/TransformWithColour.S\
					Source/SysPSP/HLEGraphics/TransformWithLighting.S\
					Source/SysPSP/HLEGraphics/VectorClipping.S \
					Source/HLEGraphics/Combiner/CombinerExpression.cpp \
					Source/HLEGraphics/Combiner/CombinerTree.cpp \
					Source/HLEGraphics/Combiner/RenderSettings.cpp

DAED_AUDIO_SRCS =   Source/HLEAudio/ABI1.cpp \
					Source/HLEAudio/ABI2.cpp \
					Source/HLEAudio/ABI3.cpp \
					Source/HLEAudio/ABI3mp3.cpp \
					Source/HLEAudio/AudioBuffer.cpp \
					Source/HLEAudio/AudioHLEProcessor.cpp \
					Source/HLEAudio/HLEMain.cpp \
					Source/HLEAudio/SafeABI.cpp \
					Source/SysPSP/HLEAudio/AudioCodePSP.cpp \
					Source/SysPSP/HLEAudio/AudioPluginPSP.cpp

DAED_OSHLE_SRCS = Source/OSHLE/OS.cpp \
					Source/OSHLE/patch.cpp

DAED_RELEASE_SRCS = Source/SysPSP/UI/RomSelectorComponent.cpp 

ADDITIONAL_DEBUG_SRCS = Source/SysPSP/UI/RomSelectorComponentdebug.cpp
ADDITIONAL_SYNC_SRCS  = Source/Utility/Synchroniser.cpp Source/Utility/ZLibWrapper.cpp

CORE_SRCS = $(DAED_MAIN_SRCS) $(DAED_DEBUG_SRCS) $(DAED_CORE_SRCS) $(DAED_INTERFACE_SRCS) $(DAED_INPUT_SRCS) $(DAED_DYNREC_SRCS) $(DAED_UTILITY_SRCS) $(DAED_PSP_SRCS) $(DAED_HLEGFX_SRCS) $(DAED_AUDIO_SRCS) $(DAED_OSHLE_SRCS)

ifdef DEBUG
CFLAGS			= -g -O3 -G0 -D_DEBUG -MD \
  -W -Wcast-qual -Wchar-subscripts -Wno-unused -Wpointer-arith -Wredundant-decls -Wshadow -Wwrite-strings
#-Winline -Wcast-align 
LDFLAGS = -g
LIBS= -lsupc++ -lstdc++ -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lc -lpng -lz -lg -lm -lpspfpu -lpspvfpu -lpspkubridge

SRCS			= $(CORE_SRCS) $(ADDITIONAL_DEBUG_SRCS) $(ADDITIONAL_SYNC_SRCS)
else
CFLAGS			= -O3 -G0 -DNDEBUG -Wall -MD
SRCS			= $(CORE_SRCS) $(DAED_RELEASE_SRCS)
endif

ifndef CONFIG
CONFIG=Release
SRS = $(CORE_SRCS) $(DAED_RELEASE_SRCS)
endif

CXXFLAGS = -fno-exceptions -fno-rtti -iquote./Source/SysPSP/Include -iquote./Source/Config/$(CONFIG) -iquote./Source -iquote./Source/SysPSP

OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.S=.o)
DEP_FILES := $(SRCS:.cpp=.d)
DEP_FILES := $(DEP_FILES:.c=.d)
DEP_FILES := $(DEP_FILES:.S=.d)

ASFLAGS =

INCDIR = $(PSPDEV)/SDK/include ./SDK/include
LIBDIR = $(PSPDEV)/SDK/lib ./SDK/lib
LDFLAGS= "-Wl,-O1"
LIBS = -lstdc++ -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lpsprtc -lc -lpng -lz -lm -lpspfpu -lpspvfpu -lpspkubridge

EXTRA_TARGETS = EBOOT.PBP $(DEP_FILES) dvemgr.prx exception.prx mediaengine.prx kernelbuttons.prx
PSP_EBOOT_TITLE = DaedalusX64 Alpha
PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_PIC1 = pic1.png
#PSP_EBOOT_ICON1 = ICON1.PMF
#PSP_EBOOT_UNKPNG = PIC0.PNG
#PSP_EBOOT_SND0 = SND0.AT3
#PSP_EBOOT_PSAR =

PSPSDK=$(shell psp-config --pspsdk-path)
#USE_PSPSDK_LIBC=1
PSP_FW_VERSION=500
BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

include $(PSPSDK)/lib/build.mak

BUILDS_DIR = ./Builds
BUILDS_PSP_DIR = $(BUILDS_DIR)/PSP

#svn revision in code
RESULT := $(shell svnversion -n 2> Makefile.cache)
ifeq ($(RESULT),)
#try windows with tortoise svn

svn:
	@echo svnrevision not found, trying SubWCRev
	@SubWCRev . ./Source/svnversion.txt ./Source/svnversion.h
else
#linux
+CFLAGS += -DSVNVERSION=\"$(RESULT)\"
endif

psplink: $(PSP_EBOOT) $(TARGET).elf
	prxtool -y $(TARGET).elf > $(BUILDS_PSP_DIR)/$(TARGET).sym
	
install: $(PSP_EBOOT) $(TARGET).prx dvemgr.prx exception.prx mediaengine.prx kernelbuttons.prx $(TARGET).elf
	svn export --force Data $(BUILDS_DIR)
	cp $(PSP_EBOOT) "$(BUILDS_PSP_DIR)"
	cp $(TARGET).elf "$(BUILDS_PSP_DIR)"
	cp *.prx "$(BUILDS_PSP_DIR)"

Source/SysPSP/MediaEnginePRX/MediaEngine.S:
	make -C Source/SysPSP/MediaEnginePRX all

Source/SysPSP/DveMgr/pspDveManager.S:
	make -C Source/SysPSP/DveMgr all

dvemgr.prx:
	make -C Source/SysPSP/DveMgr all

mediaengine.prx:
	make -C Source/SysPSP/MediaEnginePRX all
	
exception.prx:
	make -C Source/SysPSP/ExceptionHandler/prx all
	
kernelbuttons.prx:
	make -C Source/SysPSP/KernelButtonsPrx all

allclean:
	make -C Source/SysPSP/ExceptionHandler/prx clean
	make -C Source/SysPSP/MediaEnginePRX clean
	make -C Source/SysPSP/DveMgr clean
	make -C Source/SysPSP/KernelButtonsPrx clean
	make clean

CC       = psp-gcc
CXX      = psp-g++

-include $(DEP_FILES)

