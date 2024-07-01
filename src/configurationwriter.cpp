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
#include <QXmlStreamWriter>
#include "configurationwriter.h"

ConfigurationWriter::ConfigurationWriter(const Configuration &configuration, SerialGeneratorSetting &serialGeneratorSetting) :
    configuration_(configuration),
    serialGeneratorSetting_(serialGeneratorSetting)
{
}

// Writes the current configuration to a given file
void ConfigurationWriter::writeToFile(QFile &file)
{
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("cp2130config");  // Write root element
    xmlWriter.writeAttribute("version", "1.0");
    xmlWriter.writeStartElement("manufacturer");  // Write manufacturer element
    xmlWriter.writeAttribute("string", configuration_.manufacturer);
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("product");  // Write product element
    xmlWriter.writeAttribute("string", configuration_.product);
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("serial");  // Write serial element
    xmlWriter.writeAttribute("string", configuration_.serial);
    if (serialGeneratorSetting_.doexport) {
        xmlWriter.writeStartElement("generator");  // Write generator element
        xmlWriter.writeAttribute("prototype", serialGeneratorSetting_.serialgen.prototypeSerial());
        xmlWriter.writeAttribute("mode", QString::number(serialGeneratorSetting_.serialgen.replaceMode()));
        xmlWriter.writeAttribute("enable", (serialGeneratorSetting_.genenable ? "true" : "false"));
        xmlWriter.writeAttribute("auto-generate", (serialGeneratorSetting_.autogen ? "true" : "false"));
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("vid");  // Write VID element
    xmlWriter.writeAttribute("value", QString::number(configuration_.usbconfig.vid, 16));
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("pid");  // Write PID element
    xmlWriter.writeAttribute("value", QString::number(configuration_.usbconfig.pid, 16));
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("release");  // Write release element
    xmlWriter.writeAttribute("major", QString::number(configuration_.usbconfig.majrel));
    xmlWriter.writeAttribute("minor", QString::number(configuration_.usbconfig.minrel));
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("power");  // Write maximum power element
    xmlWriter.writeAttribute("maximum", QString::number(configuration_.usbconfig.maxpow, 16));
    xmlWriter.writeAttribute("mode", QString::number(configuration_.usbconfig.powmode));
    xmlWriter.writeEndElement();
    // To do
    xmlWriter.writeEndElement();
}
