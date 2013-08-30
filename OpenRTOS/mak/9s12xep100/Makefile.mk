# Makefile for the OpenRTOS part of OpenOSEK

#output
TARGET = OpenOSEK.abs

# mklink : the link of the codewarrior 5.0
CW = OpenRTOS/mak/9s12xep100/CW
#default link file
DEF_PRM = $(CW)/lib/hc12c/prm/mc9s12xep100.prm

#Base Directory
# should be executed at " OpenOSEK/ "
BD = 

OUT = $(BD)OpenRTOS/out

#common compilers
CC  = $(CW)/Prog/chc12.exe
AS  = $(CW)/Prog/ahc12.exe
LD  = $(CW)/Prog/linker.exe
AR = ar
RM = rm

#common flags
# / normal flags
NMFLGS = -WErrFileOff -WOutFileOff -EnvOBJPATH=$(OUT)

INCDIR = -I$(BD)OpenRTOS/inc \
		-I$(BD)OpenRTOS/src \
		-I$(BD)OpenRTOS/portable/9s12xep100 \
		 -I$(BD)OpenRTOS/config \
		 -I$(BD)OpenAPP \
		 -I$(CW)/lib/hc12c/include	\
		 -I$(BD)OpenRTOS/
MACROS = -D__NO_FLOAT__
ASFLGS = $(NMFLGS) $(INCDIR) -Mb -CpuHCS12XE
CCFLGS = $(INCDIR) $(NMFLGS) $(MACROS) -Mb -CpuHCS12XE
LDFLGS = -M -WmsgNu=abcet $(NMFLGS)
ARFLGS = rv
RMFLGS = -f

LIBS = $(CW)/lib/hc12c/lib/ansibi.lib

#common objectives	
OBJS=	\
	$(BD)OpenRTOS/src/main.o \
	$(BD)OpenRTOS/src/osctrl.o \
	$(BD)OpenRTOS/src/task.o \
	$(BD)OpenRTOS/config/oscfg.o	\
	$(BD)OpenRTOS/portable/9s12xep100/portable.o	\
	$(BD)OpenAPP/OpenAPP.o	\
	$(CW)/lib/hc12c/src/start12.o	\
	$(CW)/lib/hc12c/src/mc9s12xep100.o \
	$(CW)/lib/hc12c/src/DATAPAGE.o
	
OBJS_LINK= 	\
	$(OUT)/main.o \
	$(OUT)/osctrl.o \
	$(OUT)/task.o \
	$(OUT)/oscfg.o	\
	$(OUT)/portable.o	\
	$(OUT)/OpenAPP.o	\
	$(OUT)/start12.o	\
	$(OUT)/mc9s12xep100.o	\
	$(OUT)/DATAPAGE.o

#common rules	
.asm.o:
	$(AS) $*.s $(ASFLGS)

.c.o:
	$(CC) $*.c $(CCFLGS)

all: $(OBJS) $(TARGET)
	echo >>>>>>  BUILD DONE OK!  <<<<<<<
		
$(TARGET):
	$(LD) $(DEF_PRM) $(LDFLGS) -Add($(LIBS)) -Add($(OBJS_LINK)) -O$(TARGET)
	mv -f OpenOSEK.map $(OUT)
clean:
	@echo remove $(OBJS_LINK) $(TARGET) 
	@$(RM) $(RMFLGS) err.log $(BD)OpenRTOS/out/* $(TARGET)
	@echo ">>>>>>>>>>>>>>>>>  CLEAN DONE   <<<<<<<<<<<<<<<<<<<<<<"

