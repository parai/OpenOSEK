# Makefile for the OpenRTOS part of OpenOSEK

#output
TARGET = OpenOSEK

#Base Directory
BD = ../../..

#common compilers
AS = gcc
CC = gcc
LD = gcc
AR = ar
RM = rm

#common flags
ASFLGS = 
INCDIR = -I$(BD)/OpenRTOS/inc -I$(BD)/OpenRTOS/src -I$(BD)/OpenRTOS/portable/mingw
CCFLGS = -c -g -O2 -Wall $(INCDIR)	
LDFLGS = -g -O2 -Wall $(INCDIR) -lpthread
ARFLGS = rv
RMFLGS = -f

#common objectives
OBJS =  $(BD)/OpenRTOS/src/task.o	\
		$(BD)/OpenRTOS/src/osctrl.o	\
		$(BD)/OpenRTOS/portable/mingw/portable.o

#common rules
%.o:%.c
	$(CC) $(CCFLGS) -o $@ $<

.PHONY:all clean generator

all:$(TARGET)
	echo ">>>>>>>>>>>>>>>>>  BUILD DONE   <<<<<<<<<<<<<<<<<<<<<<"

$(TARGET):generator $(OBJS)
	$(LD) $(LDFLGS) $(OBJS) -o $(BD)/OpenRTOS/out/$@.exe $(BD)/OpenRTOS/src/main.c

generator:
	(python $(BD)/OpenTOOL/startosek.py  --oil $(BD)/OpenRTOS/config/oscfg.oil)
	echo ">>>>>>>>>> Convert oscfg.oil DONE <<<<<<<<<<<<<<<<<<<<"

#$(LIB-OSEK):$(OBJS)
#	$(AR) $(ARFLGS) $(BD)/OpenRTOS/out/$@ $(OBJS)

clean:
	$(RM) $(RMFLGS) $(OBJS) $(OUTPUT)
	echo ">>>>>>>>>>>>>>>>>  CLEAN DONE   <<<<<<<<<<<<<<<<<<<<<<"

