/* CP2130 Configurator - Version 3.1 for Debian Linux
   Copyright (c) 2021-2024 Samuel Lourenço

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation, either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


#ifndef CP2130LIMITS_H
#define CP2130LIMITS_H

// Includes
#include <QtGlobal>
#include "cp2130.h"

namespace CP2130Limits
{
const quint8 GPIO0_MAX = CP2130::PCCS;
const quint8 GPIO1_MAX = CP2130::PCCS;
const quint8 GPIO2_MAX = CP2130::PCCS;
const quint8 GPIO3_MAX = CP2130::PCRTR;
const quint8 GPIO4_MAX = CP2130::PCEVTCNTRPP;
const quint8 GPIO5_MAX = CP2130::PCCLKOUT;
const quint8 GPIO6_MAX = CP2130::PCCS;
const quint8 GPIO7_MAX = CP2130::PCCS;
const quint8 GPIO8_MAX = CP2130::PCSPIACT;
const quint8 GPIO9_MAX = CP2130::PCSSPND;
const quint8 GPIO10_MAX = CP2130::PCNSSPND;
const quint8 MAJREL_MAX = 0xff;
const quint8 MAXPOW_MAX = 0xfa;
const quint8 MINREL_MAX = 0xff;
const quint16 PID_MAX = 0xffff;
const quint16 PID_MIN = 0x0001;
const quint8 POWMODE_MAX = CP2130::PMSELFREGEN;
const quint8 TRFPRIO_MAX = CP2130::PRIOWRITE;
const quint16 VID_MAX = 0xffff;
const quint16 VID_MIN = 0x0001;
}

#endif  // CP2130LIMITS_H
