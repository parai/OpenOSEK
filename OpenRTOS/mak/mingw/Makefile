# Makefile for the OpenRTOS part of OpenOSEK

#output
TARGET = OpenOSEK

#Base Directory
BD = ../../..

OUT = $(BD)/OpenRTOS/out/

#common compilers
AS = gcc
CC = gcc
LD = gcc
AR = ar
RM = rm

#common flags
ASFLGS = 
INCDIR = -I$(BD)/OpenRTOS/inc -I$(BD)/OpenRTOS/src -I$(BD)/OpenRTOS/portable/mingw	\
		-I$(BD)/OpenRTOS/config	-I$(BD)/OpenAPP
MACROS = 
CCFLGS = -c -g $(MACROS) -O2 -Wall $(INCDIR)
LDFLGS = -g -O2 -Wall $(INCDIR)
ARFLGS = rv
RMFLGS = -f

#common objectives
OBJS =  $(BD)/OpenRTOS/src/task.o	\
		$(BD)/OpenRTOS/src/alarm.o	\
		$(BD)/OpenRTOS/src/osctrl.o	\
		$(BD)/OpenRTOS/portable/mingw/portable.o	\
		$(BD)/OpenRTOS/config/oscfg.o	\
		$(BD)/OpenAPP/OpenAPP.o	

app-obj = $(BD)/OpenAPP/OpenAPP.o

dir-y =	$(BD)/OpenRTOS/src	\
		$(BD)/OpenRTOS/portable/mingw	
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))
obj-y = $(subst .c,.o,$(src-y))
obj-y += $(app-obj)

#common rules
%.o:%.c
	@echo compile $<  to $@
	@$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean generator

all:$(TARGET)
	@echo ">>>>>>>>>>>>>>>>>  BUILD DONE   <<<<<<<<<<<<<<<<<<<<<<"

$(TARGET): $(OBJS)
	@echo link to $(OUT)/$@
	@$(LD) $(BD)/OpenRTOS/src/main.c -o $(OUT)/$@  $(OBJS) $(LDFLGS) 

gen : generator
	
generator:
	@(python $(BD)/OpenTOOL/startosek.py  --oil $(BD)/OpenRTOS/config/oscfg.oil)
	@echo ">>>>>>>>>> Convert oscfg.oil DONE <<<<<<<<<<<<<<<<<<<<"

#$(LIB-OSEK):$(OBJS)
#	$(AR) $(ARFLGS) $(BD)/OpenRTOS/out/$@ $(OBJS)

run:
	@(start $(OUT)/$(TARGET))
clean:
	@echo remove $(OBJS) $(OUT)/*
	@$(RM) $(RMFLGS) $(OBJS) $(OUT)/*
	@echo ">>>>>>>>>>>>>>>>>  CLEAN DONE   <<<<<<<<<<<<<<<<<<<<<<"

