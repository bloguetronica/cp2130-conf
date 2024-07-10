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


#ifndef CONFIGURATIONREADER_H
#define CONFIGURATIONREADER_H

// Includes
#include <QIODevice>
#include <QString>
#include <QXmlStreamReader>
#include "configuration.h"
#include "serialgeneratorsettings.h"

class ConfigurationReader
{
private:
    Configuration &configuration_;
    SerialGeneratorSettings &serialGeneratorSettings_;
    QXmlStreamReader xmlReader_;

    void readConfiguration();
    void readGenerator();
    void readManufacturer();
    void readPID();
    void readPower();
    void readProduct();
    void readRelease();
    void readSerial();
    void readSerialSubElements();
    void readVID();

public:
    // Class definitions
    static const int SUCCESS = 0;          // Returned by readFrom() if successful
    static const int ERROR_NOT_VALID = 1;  // Returned by readFrom() if the file is not a valid CP2130 configuration file
    static const int ERROR_SYNTAX = 2;     // Returned by readFrom() if the file contains syntax errors

    ConfigurationReader(Configuration &configuration, SerialGeneratorSettings &serialGeneratorSettings);

    QString errorString() const;

    bool readFrom(QIODevice *device);
};

#endif  // CONFIGURATIONREADER_H
