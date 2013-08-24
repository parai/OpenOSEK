# Makefile for the OpenRTOS part of OpenOSEK

#config here
## 1.platform switch (mingw, 9s12, 9s12x)
PLATFORM = mingw

all:
ifeq ($(PLATFORM),mingw)
	(cd mingw;make all)
endif

clean:
ifeq ($(PLATFORM),mingw)
	(cd mingw;make clean)
endif

run:
	@(../out/OpenOSEK.exe)
