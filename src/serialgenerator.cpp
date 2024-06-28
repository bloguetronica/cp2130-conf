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
#include <QRandomGenerator>
#include "serialgenerator.h"

SerialGenerator::SerialGenerator() :
    prototypeSerial_("????????"),  // Default constructor initializes an 8-digit generator
    replaceMode_(RMDIGIT)
{
}

// Generates a serial number
QString SerialGenerator::generateSerial() const
{
    QString replaceWith;
    if (replaceWithDigits()) {
        replaceWith += "0123456789";
    }
    if (replaceWithUppercaseLetters()) {
        replaceWith += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }
    if (replaceWithLowercaseLetters()) {
        replaceWith += "abcdefghijklmnopqrstuvwxyz";
    }
    int bounds = replaceWith.size();
    QString serial(prototypeSerial_);
    while (serial.contains('?')) {
        serial[serial.indexOf('?')] = replaceWith[QRandomGenerator::global()->bounded(bounds)];
    }
    return serial;
}

// Returns the prototype serial string
QString SerialGenerator::prototypeSerial() const
{
    return prototypeSerial_;
}

// Returns the replace mode
quint8 SerialGenerator::replaceMode() const
{
    return replaceMode_;
}

// Returns true if the "replace with digits" mode is enabled
bool SerialGenerator::replaceWithDigits() const
{
    return (RMDIGIT & replaceMode_) != 0x00;
}

// Returns true if the "replace with lowercase letters" mode is enabled
bool SerialGenerator::replaceWithLowercaseLetters() const
{
    return (RMLOWER & replaceMode_) != 0x00;
}

// Returns true if the "replace with uppercase letters" mode is enabled
bool SerialGenerator::replaceWithUppercaseLetters() const
{
    return (RMUPPER & replaceMode_) != 0x00;
}

// Sets the prototype serial string
void SerialGenerator::setPrototypeSerial(const QString &prototypeSerial)
{
    if (prototypeSerialIsValid(prototypeSerial)) {
        prototypeSerial_ = prototypeSerial;
    }
}

// Sets the replace mode
void SerialGenerator::setReplaceMode(quint8 replaceMode)
{
    if (replaceModeIsValid(replaceMode)) {
        replaceMode_ = replaceMode;
    }
}

// Sets the replace mode via separate flags
void SerialGenerator::setReplaceMode(bool replaceWDigit, bool replaceWUpper, bool replaceWLower)
{
    if (replaceModeIsValid(replaceWDigit, replaceWUpper, replaceWLower)) {
        replaceMode_ = static_cast<quint8>(replaceWLower << 2 | replaceWUpper << 1 | replaceWDigit << 0);
    }
}

// Helper function to check if a given prototype serial string is valid
bool SerialGenerator::prototypeSerialIsValid(const QString &prototypeSerial)
{
    return prototypeSerial.size() <= 30 && prototypeSerial.contains('?');
}

// Helper function to check if a given replace mode is valid
bool SerialGenerator::replaceModeIsValid(quint8 replaceMode)
{
    return (0x07 & replaceMode) != 0x00;
}

// Helper function to check if a given bitwise replace mode is valid
bool SerialGenerator::replaceModeIsValid(bool replaceWDigit, bool replaceWUpper, bool replaceWLower)
{
    return replaceWDigit == true || replaceWUpper == true || replaceWLower == true;
}
