# Makefile for the OpenRTOS part of OpenOSEK

#output Library
TARGET = OpenRTOS

#Base Directory
BD = ../../..

OUT = $(BD)/OpenRTOS/out

#common compilers
AS = gcc
CC = gcc
LD = gcc
AR = ar
RM = rm

#common flags
ASFLGS = 
INCDIR = 	-I$(BD)/OpenRTOS/inc \
			-I$(BD)/OpenRTOS/src \
			-I$(BD)/OpenRTOS/portable/mingw	\
			-I$(BD)/OpenAPP/out	\
			-I$(BD)/OpenAPP
MACROS = -DSIMULATE_ON_WIN
CCFLGS = -c -g $(MACROS) -O2 -Wall $(INCDIR)
LDFLGS = -g -O2 -Wall $(INCDIR)
ARFLGS = rv
RMFLGS = -f
		
#common objectives		
dir-y =	$(BD)/OpenRTOS/src	\
		$(BD)/OpenRTOS/portable/mingw
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))	\
		$(BD)/OpenAPP/out/oscfg.c
obj-y = $(subst .c,.o,$(src-y))

#common rules
%.o:%.c
	@echo compile $<  to $@
	@$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean

all:$(TARGET)
	$(AR) $(ARFLGS) $(OUT)/lib$(TARGET).a $(obj-y)
	@echo ">>>>>>>>>>>>>>>>>  BUILD $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"

$(TARGET): $(obj-y)

clean:
	@echo remove $(obj-y) $(OUT)/*
	@$(RM) $(RMFLGS) $(obj-y) $(OUT)/*
	@echo ">>>>>>>>>>>>>>>>>  CLEAN $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"	
