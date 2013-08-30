# Makefile for the OpenRTOS part of OpenOSEK

# the install dir of codewarrior 5.0
COMPILER_DIR = "D:\Program Files\Freescale\CodeWarrior for S12(X) V5.0"

# mklink : the link of the codewarrior 5.0
CW = OpenRTOS/mak/9s12xep100/CW
OUT = OpenRTOS/out

.PHONY:all sim clean compiler
all:compiler
	@(cd ../../../;$(CW)/Prog/piper.exe $(CW)/Prog/maker.exe OpenRTOS/mak/9s12xep100/Makefile.mk)

sim:
	@(cd ../../../;$(CW)/Prog/hiwave.exe -W -Prod=$(OUT)/OpenOSEK.ini -instance=sim OpenOSEK.abs)

clean:
	@(cd ../../../;make clean -f OpenRTOS/mak/9s12xep100/Makefile.mk)

compiler:CW/Prog/piper.exe
	@echo ">>>   Compiler Ok!  <<<"

CW/Prog/piper.exe:
	@echo "Please do the command below:"
	@echo "    cd path/to/OpenOSEK/OpenRTOS/mak/9s12xep100"
	@echo "    mklink /d CW5.0 path/to/codewarrior5.0/install/dir"
	@echo "such as:"
	@echo "    mklink /d CW5.0 \$(COMPILER_DIR)\"
	@exit -1
