
CC  = python ../../OpenTOOL/osek/oilcc.py

OUT = ../out
.PHONY:all test

all:
	@($(CC) -S $(OUT)/OpenAPP.csv -o $(OUT) OpenAPP.oil)
	@echo ">>>>>>>>>>  OILCC BUILD DONE <<<<<<<<<<<<<<<< "

test:
	@($(CC) -S $(OUT)/OpenAPP.csv -o $(OUT) test.oil)
	@echo ">>>>>>>>>>  OILCC BUILD DONE <<<<<<<<<<<<<<<< "