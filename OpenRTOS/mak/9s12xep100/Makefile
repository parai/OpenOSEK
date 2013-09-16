# Makefile for the OpenRTOS part of OpenOSEK

# the install dir of codewarrior 5.0
COMPILER_DIR = "D:\Program Files\Freescale\CodeWarrior for S12(X) V5.0"

# mklink : the link of the codewarrior 5.0
CW = OpenRTOS/mak/9s12xep100/CW

#Base Directory
BD = ../../..

.PHONY:all sim clean compiler generator

all:compiler
	@(cd $(BD);$(CW)/Prog/piper.exe $(CW)/Prog/maker.exe OpenRTOS/mak/9s12xep100/Makefile.mk)

sim:
	@(cd $(BD);$(CW)/Prog/hiwave.exe -W -Prod=OpenOSEK.ini -instance=sim OpenOSEK.abs)

clean:
	@(cd $(BD);make clean -f OpenRTOS/mak/9s12xep100/Makefile.mk)

gen:generator
generator:
	@(python $(BD)/OpenTOOL/startosek.py  --oil $(BD)/OpenRTOS/config/oscfg.oil)
	@echo ">>>>>>>>>> Convert oscfg.oil DONE <<<<<<<<<<<<<<<<<<<<"

compiler:CW/Prog/piper.exe
	@echo ">>>   Compiler Ok!  <<<"

CW/Prog/piper.exe:
	@echo "Please do the command below:"
	@echo "    cd path/to/OpenOSEK/OpenRTOS/mak/9s12xep100"
	@echo "    mklink /d CW5.0 path/to/codewarrior5.0/install/dir"
	@echo "such as:"
	@echo "    mklink /d CW5.0 \$(COMPILER_DIR)\"
	@exit -1
