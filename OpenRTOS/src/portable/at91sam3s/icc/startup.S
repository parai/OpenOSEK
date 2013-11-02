/* Copyright(C) 2013, OpenOSEK by Fan Wang(parai). All rights reserved.
 *
 * This file is part of OpenOSEK.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Email: parai@foxmail.com
 * Sourrce Open At: https://github.com/parai/OpenOSEK/
 */
    ;; Forward declaration of sections.
    SECTION CSTACK:DATA:NOROOT(3)

    SECTION .intvec:CODE:NOROOT(2)
    EXTERN __iar_program_start
    EXTERN  knl_dispatch_entry,knl_force_dispatch_impl 
    EXTERN  knl_system_timer
    PUBLIC __vector_table
    DATA
__vector_table:
     /*    Internal Exceptions Vector Define                                        */
    DCD     sfe(CSTACK)                          /* 00: Top of Main Stack           */
    DCD     reset_handler                        /* 01: Reset Handler               */
    DCD     default_handler                      /* 02: NMI Handler                 */
    DCD     default_handler                      /* 03: Hard Fault Handler          */
    DCD     default_handler                      /* 04: MPU Fault Handler           */
    DCD     default_handler                      /* 05: Bus Fault Handler           */
    DCD     default_handler                      /* 06: Usage Fault Handler         */
    DCD     0                                    /* 07: Reserved                    */
    DCD     0                                    /* 08: Reserved                    */
    DCD     0                                    /* 09: Reserved                    */
    DCD     0                                    /* 10: Reserved                    */
    DCD     knl_force_dispatch_impl              /* 11: SVCall Handler              */
    DCD     default_handler                      /* 12: Debug Monitor Handler       */
    DCD     0                                    /* 13: Reserved                    */
    DCD     knl_dispatch_entry                   /* 14: PendSV Handler              */
    DCD     knl_system_timer                     /* 15: Systick handler             */

    /*    External Interrupts Vector Define                                         */
    DCD     default_handler                      /* 16: */
    DCD     default_handler                      /* 17: */
    DCD     default_handler                      /* 18: */
    DCD     default_handler                      /* 19: */
    DCD     default_handler                      /* 20: */
    DCD     default_handler                      /* 21: */
    DCD     default_handler                      /* 22: */
    DCD     default_handler                      /* 23: */
    DCD     default_handler                      /* 24: */
    DCD     default_handler                      /* 25: */
    DCD     default_handler                      /* 26: */
    DCD     default_handler                      /* 27: */
    DCD     default_handler                      /* 28: */
    DCD     default_handler                      /* 29: */
    DCD     default_handler                      /* 30: */
    DCD     default_handler                      /* 31: */
    DCD     default_handler                      /* 32: */
    DCD     default_handler                      /* 33: */
    DCD     default_handler                      /* 34: */
    DCD     default_handler                      /* 35: */
    DCD     default_handler                      /* 36: */
    DCD     default_handler                      /* 37: */
    DCD     default_handler                      /* 38: */
    DCD     default_handler                      /* 39: */
    DCD     default_handler                      /* 40: */
    DCD     default_handler                      /* 41: */
    DCD     default_handler                      /* 42: */
    DCD     default_handler                      /* 43: */
    DCD     default_handler                      /* 44: */
    DCD     default_handler                      /* 45: */
    DCD     default_handler                      /* 46: */
    DCD     default_handler                      /* 47: */
    DCD     default_handler                      /* 48: */
    DCD     default_handler                      /* 49: */
    DCD     default_handler                      /* 50: */
    DCD     default_handler                      /* 51: */
    DCD     default_handler                      /* 52: */
    DCD     default_handler                      /* 53: */
    DCD     default_handler                      /* 54: */
    DCD     default_handler                      /* 55: */
    DCD     default_handler                      /* 56: */
    DCD     default_handler                      /* 57: */
    DCD     default_handler                      /* 58: */
    DCD     default_handler                      /* 59: */
    DCD     default_handler                      /* 60: */
    DCD     default_handler                      /* 61: */
    DCD     default_handler                      /* 62: */
    DCD     default_handler                      /* 63: */
    DCD     default_handler                      /* 64: */
    DCD     default_handler                      /* 65: */
    DCD     default_handler                      /* 66: */
    DCD     default_handler                      /* 67: */
    DCD     default_handler                      /* 68: */
    DCD     default_handler                      /* 69: */
    DCD     default_handler                      /* 70: */
    DCD     default_handler                      /* 71: */
    DCD     default_handler                      /* 72: */
    DCD     default_handler                      /* 73: */
    DCD     default_handler                      /* 74: */
    DCD     default_handler                      /* 75: */
    DCD     default_handler                      /* 76: */
    DCD     default_handler                      /* 77: */
    DCD     default_handler                      /* 78: */
    DCD     default_handler                      /* 79: */
    DCD     default_handler                      /* 80: */
    DCD     default_handler                      /* 81: */
    DCD     default_handler                      /* 82: */
    DCD     default_handler                      /* 83: */
    DCD     default_handler                      /* 84: */
    DCD     default_handler                      /* 85: */
    DCD     default_handler                      /* 86: */
    DCD     default_handler                      /* 87: */
    DCD     default_handler                      /* 88: */
    DCD     default_handler                      /* 89: */
    DCD     default_handler                      /* 90: */
    DCD     default_handler                      /* 91: */
    DCD     default_handler                      /* 92: */
    DCD     default_handler                      /* 93: */
    DCD     default_handler                      /* 94: */
    DCD     default_handler                      /* 95: */
    DCD     default_handler                      /* 96: */
    DCD     default_handler                      /* 97: */
    DCD     default_handler                      /* 98: */
    DCD     default_handler                      /* 99: */
    DCD     default_handler                      /* 100: */
    DCD     default_handler                      /* 101: */
    DCD     default_handler                      /* 102: */
    DCD     default_handler                      /* 103: */
    DCD     default_handler                      /* 104: */
    DCD     default_handler                      /* 105: */
    DCD     default_handler                      /* 106: */
    DCD     default_handler                      /* 107: */
    DCD     default_handler                      /* 108: */
    DCD     default_handler                      /* 109: */
    DCD     default_handler                      /* 110: */
    DCD     default_handler                      /* 111: */
    DCD     default_handler                      /* 112: */
    DCD     default_handler                      /* 113: */
    DCD     default_handler                      /* 114: */
    DCD     default_handler                      /* 115: */
    DCD     default_handler                      /* 116: */
    DCD     default_handler                      /* 117: */
    DCD     default_handler                      /* 118: */
    DCD     default_handler                      /* 119: */
    DCD     default_handler                      /* 120: */
    DCD     default_handler                      /* 121: */
    DCD     default_handler                      /* 122: */
    DCD     default_handler                      /* 123: */
    DCD     default_handler                      /* 124: */
    DCD     default_handler                      /* 125: */
    DCD     default_handler                      /* 126: */
    DCD     default_handler                      /* 127: */
    DCD     default_handler                      /* 128: */
    DCD     default_handler                      /* 129: */
    DCD     default_handler                      /* 130: */
    DCD     default_handler                      /* 131: */
    DCD     default_handler                      /* 132: */
    DCD     default_handler                      /* 133: */
    DCD     default_handler                      /* 134: */
    DCD     default_handler                      /* 135: */
    DCD     default_handler                      /* 136: */
    DCD     default_handler                      /* 137: */
    DCD     default_handler                      /* 138: */
    DCD     default_handler                      /* 139: */
    DCD     default_handler                      /* 140: */
    DCD     default_handler                      /* 141: */
    DCD     default_handler                      /* 142: */
    DCD     default_handler                      /* 143: */
    DCD     default_handler                      /* 144: */
    DCD     default_handler                      /* 145: */
    DCD     default_handler                      /* 146: */
    DCD     default_handler                      /* 147: */
    DCD     default_handler                      /* 148: */
    DCD     default_handler                      /* 149: */
    DCD     default_handler                      /* 150: */
    DCD     default_handler                      /* 151: */
    DCD     default_handler                      /* 152: */
    DCD     default_handler                      /* 153: */
    DCD     default_handler                      /* 154: */
    DCD     default_handler                      /* 155: */
    DCD     default_handler                      /* 156: */
    DCD     default_handler                      /* 157: */
    DCD     default_handler                      /* 158: */
    DCD     default_handler                      /* 159: */
    DCD     default_handler                      /* 160: */
    DCD     default_handler                      /* 161: */
    DCD     default_handler                      /* 162: */
    DCD     default_handler                      /* 163: */
    DCD     default_handler                      /* 164: */
    DCD     default_handler                      /* 165: */
    DCD     default_handler                      /* 166: */
    DCD     default_handler                      /* 167: */
    DCD     default_handler                      /* 168: */
    DCD     default_handler                      /* 169: */
    DCD     default_handler                      /* 170: */
    DCD     default_handler                      /* 171: */
    DCD     default_handler                      /* 172: */
    DCD     default_handler                      /* 173: */
    DCD     default_handler                      /* 174: */
    DCD     default_handler                      /* 175: */
    DCD     default_handler                      /* 176: */
    DCD     default_handler                      /* 177: */
    DCD     default_handler                      /* 178: */
    DCD     default_handler                      /* 179: */
    DCD     default_handler                      /* 180: */
    DCD     default_handler                      /* 181: */
    DCD     default_handler                      /* 182: */
    DCD     default_handler                      /* 183: */
    DCD     default_handler                      /* 184: */
    DCD     default_handler                      /* 185: */
    DCD     default_handler                      /* 186: */
    DCD     default_handler                      /* 187: */
    DCD     default_handler                      /* 188: */
    DCD     default_handler                      /* 189: */
    DCD     default_handler                      /* 190: */
    DCD     default_handler                      /* 191: */
    DCD     default_handler                      /* 192: */
    DCD     default_handler                      /* 193: */
    DCD     default_handler                      /* 194: */
    DCD     default_handler                      /* 195: */
    DCD     default_handler                      /* 196: */
    DCD     default_handler                      /* 197: */
    DCD     default_handler                      /* 198: */
    DCD     default_handler                      /* 199: */
    DCD     default_handler                      /* 200: */
    DCD     default_handler                      /* 201: */
    DCD     default_handler                      /* 202: */
    DCD     default_handler                      /* 203: */
    DCD     default_handler                      /* 204: */
    DCD     default_handler                      /* 205: */
    DCD     default_handler                      /* 206: */
    DCD     default_handler                      /* 207: */
    DCD     default_handler                      /* 208: */
    DCD     default_handler                      /* 209: */
    DCD     default_handler                      /* 210: */
    DCD     default_handler                      /* 211: */
    DCD     default_handler                      /* 212: */
    DCD     default_handler                      /* 213: */
    DCD     default_handler                      /* 214: */
    DCD     default_handler                      /* 215: */
    DCD     default_handler                      /* 216: */
    DCD     default_handler                      /* 217: */
    DCD     default_handler                      /* 218: */
    DCD     default_handler                      /* 219: */
    DCD     default_handler                      /* 220: */
    DCD     default_handler                      /* 221: */
    DCD     default_handler                      /* 222: */
    DCD     default_handler                      /* 223: */
    DCD     default_handler                      /* 224: */
    DCD     default_handler                      /* 225: */
    DCD     default_handler                      /* 226: */
    DCD     default_handler                      /* 227: */
    DCD     default_handler                      /* 228: */
    DCD     default_handler                      /* 229: */
    DCD     default_handler                      /* 230: */
    DCD     default_handler                      /* 231: */
    DCD     default_handler                      /* 232: */
    DCD     default_handler                      /* 233: */
    DCD     default_handler                      /* 234: */
    DCD     default_handler                      /* 235: */
    DCD     default_handler                      /* 236: */
    DCD     default_handler                      /* 237: */
    DCD     default_handler                      /* 238: */
    DCD     default_handler                      /* 239: */
    DCD     default_handler                      /* 240: */
    DCD     default_handler                      /* 241: */
    DCD     default_handler                      /* 242: */
    DCD     default_handler                      /* 243: */
    DCD     default_handler                      /* 244: */
    DCD     default_handler                      /* 245: */
    DCD     default_handler                      /* 246: */
    DCD     default_handler                      /* 247: */
    DCD     default_handler                      /* 248: */
    DCD     default_handler                      /* 249: */
    DCD     default_handler                      /* 250: */
    DCD     default_handler                      /* 251: */
    DCD     default_handler                      /* 252: */
    DCD     default_handler                      /* 253: */
    DCD     default_handler                      /* 254: */
    DCD     default_handler                      /* 255: */

/*
 * Reset handle routine
 */
    SECTION .text:CODE
    THUMB
    PUBLIC reset_handler
reset_handler:
        LDR     R0, =__iar_program_start
        BX      R0
       
/*
 * Default exeception and interrupt process handler define
 */
	PUBLIC default_handler
default_handler:
    b       default_handler

    END

