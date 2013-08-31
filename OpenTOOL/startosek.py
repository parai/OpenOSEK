"""
Copyright 2012, Fan Wang(Parai). All rights reserved.

This file is part of OpenOSEK.

This version is just for oil25.
"""
import sys, os
from osek.oil25 import Oil

def OsekOilCongigToCcode(oilfile):
    oil = Oil()
    oil.parse(oilfile) #parse it
    oil.gen(os.path.dirname(oilfile))

if __name__ == "__main__":
    if(len(sys.argv) == 3 and sys.argv[1] == '--oil'):
        OsekOilCongigToCcode(sys.argv[2])
