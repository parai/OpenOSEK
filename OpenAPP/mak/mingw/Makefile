# Makefile for the OpenRTOS part of OpenOSEK

#output EXE
TARGET = OpenAPP

#Base Directory
BD = ../../..

OUT = $(BD)/OpenAPP/out

#common compilers
AS = gcc
CC = gcc
LD = gcc
AR = ar
RM = rm
OILCC = python $(BD)/OpenTOOL/osek/oilcc.py

OILCC-I = -I$(BD)/OpenCom/config
#common flags
ASFLGS = 
INCDIR = 	-I$(BD)/OpenRTOS/inc \
			-I$(BD)/OpenDriver/inc \
			-I$(BD)/OpenCom/inc \
			-I$(BD)/OpenCom/config \
			-I$(BD)/OpenAPP/inc	\
			-I$(BD)/OpenAPP/out	\
			-I$(BD)/OpenRTOS/src	\
			-I$(BD)/OpenRTOS/portable/mingw

MACROS = -DSIMULATE_ON_WIN
CCFLGS = -c -g $(MACROS) -O2 -Wall $(INCDIR)
LDFLGS = -g $(MACROS) -O2 -Wall $(INCDIR) -lwsock32
ARFLGS = rv
RMFLGS = -f	

#common objectives		
dir-y =	$(BD)/OpenAPP/src
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))
obj-y = $(subst .c,.o,$(src-y))

# common library needed for OpenOSEK
lib-y = OpenDriver OpenRTOS OpenCom

# append you speical library after LIB-Y
LIB-Y = $(foreach x,$(lib-y),$(addprefix -L$(BD)/$(x)/,out))	\
		$(foreach x,$(lib-y),$(addprefix -l,$(x)))

#common rules
%.o:%.c
	@echo compile $<  to $@
	@$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean gen openosek.lib
	
######	
all:openosek.lib $(TARGET) 
	@$(LD) $(BD)/OpenAPP/src/start/main.c -o $(OUT)/$(TARGET) $(obj-y) $(LDFLGS) $(LIB-Y)
	@echo ">>>>>>>>>>>>>>>>>  BUILD $(TARGET).EXE DONE   <<<<<<<<<<<<<<<<<<<<<<"

openosek.lib: $(lib-y)

OpenRTOS:$(BD)/OpenRTOS/out/libOpenRTOS.a
	
$(BD)/OpenRTOS/out/libOpenRTOS.a:
	(cd $(BD)/OpenRTOS/mak/mingw; make all)
OpenDriver:$(BD)/OpenDriver/out/libOpenDriver.a

$(BD)/OpenDriver/out/libOpenDriver.a:
	(cd $(BD)/OpenDriver/mak/mingw; make all)
OpenCom:$(BD)/OpenCom/out/libOpenCom.a

$(BD)/OpenCom/out/libOpenCom.a:
	(cd $(BD)/OpenCom/mak/mingw; make all)

$(TARGET): $(obj-y)

gen:
	@($(OILCC) $(OILCC-I) -S $(OUT)/OpenAPP.csv -o $(OUT) $(BD)/OpenAPP/config/OpenAPP.oil)
	@echo ">>>>>>>>>>  OILCC BUILD DONE <<<<<<<<<<<<<<<< "
	
run:
	@(start python $(BD)/OpenTOOL/osek/CanBusServer.py --server 8000)
	@(start $(OUT)/$(TARGET) 16)
	@(start $(OUT)/$(TARGET) 32)
	@(start $(OUT)/$(TARGET) 48)
	@(start $(OUT)/$(TARGET) 64)
	@(start $(OUT)/$(TARGET) 81)
	
clean:
	@echo remove $(obj-y) $(OUT)/*
	@$(RM) $(RMFLGS) $(obj-y) $(OUT)/*
	@(cd $(BD)/OpenRTOS/mak/mingw; make clean)
	@(cd $(BD)/OpenDriver/mak/mingw; make clean)
	@(cd $(BD)/OpenCom/mak/mingw; make clean)
	@echo ">>>>>>>>>>>>>>>>>  CLEAN $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"
	
##   Fask-Build
#common objectives		
dir-y =	$(BD)/OpenAPP/src	\
		$(BD)/OpenRTOS/src \
		$(BD)/OpenRTOS/portable/mingw \
		$(BD)/OpenDriver/src/mingw \
		$(BD)/OpenCom/src \
		$(BD)/OpenCom/config \
		$(BD)/OpenAPP/out
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))
obj-y = $(subst .c,.o,$(src-y))	

f-all:$(obj-y)
	@$(LD) $(BD)/OpenAPP/src/start/main.c -o $(OUT)/$(TARGET) $(obj-y) $(LDFLGS)
	@echo ">>>>>>>>>>>>>>>>>  BUILD $(TARGET).EXE DONE   <<<<<<<<<<<<<<<<<<<<<<"	