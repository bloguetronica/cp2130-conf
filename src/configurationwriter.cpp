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


// Include
#include <QString>
#include "configurationwriter.h"

ConfigurationWriter::ConfigurationWriter(const Configuration &configuration, const SerialGeneratorSetting &serialGeneratorSetting) :
    configuration_(configuration),
    serialGeneratorSetting_(serialGeneratorSetting)
{
}

// Writes the current configuration to a given file
void ConfigurationWriter::writeTo(QIODevice *device)
{
    xmlWriter_.setDevice(device);
    xmlWriter_.setAutoFormatting(true);
    xmlWriter_.writeStartDocument();
    xmlWriter_.writeStartElement("cp2130config");  // Write root element
    xmlWriter_.writeAttribute("version", "1.0");
    xmlWriter_.writeStartElement("manufacturer");  // Write manufacturer element
    xmlWriter_.writeAttribute("string", configuration_.manufacturer);
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("product");  // Write product element
    xmlWriter_.writeAttribute("string", configuration_.product);
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("serial");  // Write serial element
    xmlWriter_.writeAttribute("string", configuration_.serial);
    if (serialGeneratorSetting_.doexport) {
        xmlWriter_.writeStartElement("generator");  // Write generator element
        xmlWriter_.writeAttribute("prototype", serialGeneratorSetting_.serialgen.prototypeSerial());
        xmlWriter_.writeAttribute("mode", QString::number(serialGeneratorSetting_.serialgen.replaceMode()));
        xmlWriter_.writeAttribute("enable", (serialGeneratorSetting_.genenable ? "true" : "false"));
        xmlWriter_.writeAttribute("auto-generate", (serialGeneratorSetting_.autogen ? "true" : "false"));
        xmlWriter_.writeEndElement();
    }
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("vid");  // Write VID element
    xmlWriter_.writeAttribute("value", QString::number(configuration_.usbconfig.vid, 16));
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("pid");  // Write PID element
    xmlWriter_.writeAttribute("value", QString::number(configuration_.usbconfig.pid, 16));
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("release");  // Write release element
    xmlWriter_.writeAttribute("major", QString::number(configuration_.usbconfig.majrel));
    xmlWriter_.writeAttribute("minor", QString::number(configuration_.usbconfig.minrel));
    xmlWriter_.writeEndElement();
    xmlWriter_.writeStartElement("power");  // Write power element
    xmlWriter_.writeAttribute("maximum", QString::number(configuration_.usbconfig.maxpow, 16));
    xmlWriter_.writeAttribute("mode", QString::number(configuration_.usbconfig.powmode));
    xmlWriter_.writeEndElement();
    // To do
    xmlWriter_.writeEndElement();
}
