/* CP2130 Configurator - Version 3.0 for Debian Linux
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


// Includes
#include "serialgenerator.h"

SerialGenerator::SerialGenerator() :
    prototypeSerial_("????????"),  // Default constructor initializes an 8-digit generator
    replaceMode_(0x01)
{
}

bool SerialGenerator::replaceWithDigits() const
{
    return (0x01 & replaceMode_) != 0x00;
}

bool SerialGenerator::replaceWithLowercaseLetters() const
{
    return (0x04 & replaceMode_) != 0x00;
}

bool SerialGenerator::replaceWithUppercaseLetters() const
{
    return (0x02 & replaceMode_) != 0x00;
}

void SerialGenerator::setReplaceMode(quint8 replaceMode)
{
    replaceMode_ = replaceMode;
}

void SerialGenerator::setReplaceMode(bool replaceWithDigits, bool replaceWithUppercaseLetters, bool replaceWithLowercaseLetters)
{
    replaceMode_ = static_cast<quint8>(replaceWithLowercaseLetters << 2 | replaceWithUppercaseLetters << 1 | replaceWithDigits << 0);
}

void SerialGenerator::setSerialString(const QString &prototypeSerial)
{
    prototypeSerial_ = prototypeSerial;
}
