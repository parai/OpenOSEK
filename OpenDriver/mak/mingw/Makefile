# Makefile for the OpenRTOS part of OpenOSEK

#output Library
TARGET = OpenDriver

#Base Directory
BD = ../../..

OUT = $(BD)/OpenDriver/out

#common compilers
AS = gcc
CC = gcc
LD = gcc
AR = ar
RM = rm

#common flags
ASFLGS = 
INCDIR = -I$(BD)/OpenRTOS/inc -I$(BD)/OpenDriver/inc -I$(BD)/OpenCom/inc -I$(BD)/OpenAPP
MACROS = 
CCFLGS = -c -g $(MACROS) -O2 -Wall $(INCDIR)
LDFLGS = -g -O2 -Wall $(INCDIR) -L$(OUT) -l$(TARGET)  -lwsock32
ARFLGS = rv
RMFLGS = -f

#common objectives		
dir-y =	$(BD)/OpenDriver/src/mingw
src-y = $(foreach x,$(dir-y),$(wildcard $(addprefix $(x)/*,.c)))
obj-y = $(subst .c,.o,$(src-y))

#common rules
%.o:%.c
	@echo compile $<  to $@
	@$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean generator

all:$(TARGET)
	$(AR) $(ARFLGS) $(OUT)/lib$(TARGET).a $(obj-y)
	@echo ">>>>>>>>>>>>>>>>>  BUILD $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"

$(TARGET): $(obj-y)

clean:
	@echo remove $(obj-y) $(OUT)/*
	@$(RM) $(RMFLGS) $(obj-y) $(OUT)/*
	@echo ">>>>>>>>>>>>>>>>>  CLEAN $(TARGET) DONE   <<<<<<<<<<<<<<<<<<<<<<"

# ====================  for test purpose ==============
test:all
	$(LD) $(BD)/OpenDriver/src/main.c -o $(OUT)/test  $(OBJS) $(LDFLGS) 

test-run:
	@echo ">>>>>>>>>>>>>>>  Start server firstly <<<<<<<<<<<<<<<<<<<<<<<<<<<"
	@(start python $(BD)/OpenTOOL/osek/CanBusServer.py --server 8000)
	@echo ">>>>>>>>>>>>>>>  Start app secondly   <<<<<<<<<<<<<<<<<<<<<<<<<<<"
	@(start $(OUT)/test)	

