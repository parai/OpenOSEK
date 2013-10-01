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

#common flags
ASFLGS = 
INCDIR = 	-I$(BD)/OpenRTOS/inc \
			-I$(BD)/OpenDriver/inc \
			-I$(BD)/OpenCom/inc \
			-I$(BD)/OpenAPP/inc	\
			-I$(BD)/OpenAPP	\
			-I$(BD)/OpenAPP/out
MACROS = 
CCFLGS = -c -g $(MACROS) -O2 -Wall $(INCDIR)
LDFLGS = -g -O2 -Wall $(INCDIR) 
ARFLGS = rv
RMFLGS = -f

#common objectives		
dir-y =	$(BD)/OpenAPP/src
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))
obj-y = $(subst .c,.o,$(src-y))

# common library needed for OpenOSEK
lib-y = OpenDriver OpenRTOS 
#OpenCom

# append you speical library after LIB-Y
LIB-Y = $(foreach x,$(lib-y),$(addprefix -L$(BD)/$(x)/,out))	\
		$(foreach x,$(lib-y),$(addprefix -l,$(x)))

#common rules
%.o:%.c
	@echo compile $<  to $@
	@$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean gen openosek.lib

all:openosek.lib $(TARGET)
	@$(LD) $(BD)/OpenAPP/src/start/main.c -o $(OUT)/$(TARGET) $(obj-y) $(LDFLGS) $(LIB-Y)
	@echo ">>>>>>>>>>>>>>>>>  BUILD $(TARGET).EXE DONE   <<<<<<<<<<<<<<<<<<<<<<"

openosek.lib: $(lib-y)

OpenRTOS:
	(cd $(BD)/OpenRTOS/mak/mingw; make all)
OpenDriver:
	(cd $(BD)/OpenDriver/mak/mingw; make all)
OpenCom:
	(cd $(BD)/OpenCom/mak/mingw; make all)
$(TARGET): $(obj-y)

gen:
	(cd $(BD)/OpenAPP/config; make all)

run:
	@(start $(OUT)/$(TARGET))
clean:
	@echo remove $(obj-y) $(OUT)/*
	@$(RM) $(RMFLGS) $(obj-y) $(OUT)/*
	@(cd $(BD)/OpenRTOS/mak/mingw; make clean)
	@(cd $(BD)/OpenDriver/mak/mingw; make clean)
	@(cd $(BD)/OpenCom/mak/mingw; make clean)
	@echo ">>>>>>>>>>>>>>>>>  CLEAN $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"