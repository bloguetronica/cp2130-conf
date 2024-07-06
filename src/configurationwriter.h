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


#ifndef CONFIGURATIONWRITER_H
#define CONFIGURATIONWRITER_H

// Includes
#include <QIODevice>
#include <QString>
#include <QXmlStreamWriter>
#include "configuration.h"
#include "serialgeneratorsettings.h"

class ConfigurationWriter
{
private:
    const Configuration &configuration_;
    const SerialGeneratorSettings &serialGeneratorSettings_;
    QXmlStreamWriter xmlWriter_;

    void writeDescriptor(QString name, QString value);
    void writeGenerator();
    void writeID(QString name, quint16 value);
    void writePower();
    void writeRelease();

public:
    ConfigurationWriter(const Configuration &configuration, const SerialGeneratorSettings &serialGeneratorSettings);

    void writeTo(QIODevice *device);
};

#endif  // CONFIGURATIONWRITER_H
