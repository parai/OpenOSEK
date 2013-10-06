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
#ifndef COM_H_H_H_H
#define COM_H_H_H_H
/* ================================ INCLUDEs  =============================== */
#include "Os.h"
#include "ComStack_Types.h"
#include "Nm.h"
#include "Can.h"
#include "Dll.h"
#include "comcfg.h"
#include "CanTp.h"
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */
typedef struct
{
	PduInfoType pdu;
	Can_ControllerIdType controller;
	Can_IdType id;
}Com_IPDUConfigType;
/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */

#endif
