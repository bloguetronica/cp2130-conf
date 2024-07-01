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
#include <QFile>
#include "configuration.h"
#include "serialgeneratorsetting.h"

class ConfigurationWriter
{
private:
    const Configuration &configuration_;
    const SerialGeneratorSetting &serialGeneratorSetting_;

public:
    ConfigurationWriter(const Configuration &configuration, const SerialGeneratorSetting &serialGeneratorSetting);

    void writeToFile(QFile &file);
};

#endif  // CONFIGURATIONWRITER_H
