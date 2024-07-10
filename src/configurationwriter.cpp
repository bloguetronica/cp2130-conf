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
#include <QVector>
#include "configurationwriter.h"

// Writes "bitmaps" element
void ConfigurationWriter::writeBitmaps()
{
    xmlWriter_.writeStartElement("bitmaps");
    writeWordGeneric("suspendlevel", configuration_.pinconfig.sspndlvl);
    writeWordGeneric("suspendmode", configuration_.pinconfig.sspndmode);
    writeWordGeneric("wakeupmask", configuration_.pinconfig.wkupmask);
    writeWordGeneric("wakeupmatch", configuration_.pinconfig.wkupmatch);
    xmlWriter_.writeEndElement();
}

// Writes descriptor element (used for manufacturer, product and serial descriptors)
void ConfigurationWriter::writeDescriptor(QString name, QString value)
{
    xmlWriter_.writeStartElement(name);
    xmlWriter_.writeAttribute("string", value);
    if (name == "serial" && serialGeneratorSettings_.doexport) {
        writeGenerator();
    }
    xmlWriter_.writeEndElement();
}

// Writes "divider" element
void ConfigurationWriter::writeDivider()
{
    xmlWriter_.writeStartElement("divider");
    xmlWriter_.writeAttribute("value", QString::number(configuration_.pinconfig.divider));
    xmlWriter_.writeEndElement();
}

// Writes "generator" element regarding serial generator
void ConfigurationWriter::writeGenerator()
{
    xmlWriter_.writeStartElement("generator");
    xmlWriter_.writeAttribute("prototype", serialGeneratorSettings_.serialgen.prototypeSerial());
    xmlWriter_.writeAttribute("mode", QString::number(serialGeneratorSettings_.serialgen.replaceMode()));
    xmlWriter_.writeAttribute("enable", (serialGeneratorSettings_.genenable ? "true" : "false"));
    xmlWriter_.writeAttribute("auto-generate", (serialGeneratorSettings_.autogen ? "true" : "false"));
    xmlWriter_.writeEndElement();
}

// Writes GPIO element
void ConfigurationWriter::writeGPIO(int number, quint8 mode)
{
    xmlWriter_.writeStartElement(QString("gpio%1").arg(number));
    xmlWriter_.writeAttribute("mode", QString::number(mode));
    xmlWriter_.writeEndElement();
}

// Writes "pins" element
void ConfigurationWriter::writePins()
{
    xmlWriter_.writeStartElement("pins");
    QVector<quint8> pins{
        configuration_.pinconfig.gpio0,
        configuration_.pinconfig.gpio1,
        configuration_.pinconfig.gpio2,
        configuration_.pinconfig.gpio3,
        configuration_.pinconfig.gpio4,
        configuration_.pinconfig.gpio5,
        configuration_.pinconfig.gpio6,
        configuration_.pinconfig.gpio7,
        configuration_.pinconfig.gpio8,
        configuration_.pinconfig.gpio9,
        configuration_.pinconfig.gpio10
    };
    int numberOfPins = pins.size();
    for (int i = 0; i < numberOfPins; ++i) {
        writeGPIO(i, pins.at(i));
    }
    xmlWriter_.writeEndElement();
}

// Writes "power" element
void ConfigurationWriter::writePower()
{
    xmlWriter_.writeStartElement("power");
    xmlWriter_.writeAttribute("maximum", QString::number(configuration_.usbconfig.maxpow, 16));
    xmlWriter_.writeAttribute("mode", QString::number(configuration_.usbconfig.powmode));
    xmlWriter_.writeEndElement();
}

// Writes "transfer" element
void ConfigurationWriter::writeTransfer()
{
    xmlWriter_.writeStartElement("transfer");
    xmlWriter_.writeAttribute("priority", QString::number(configuration_.usbconfig.trfprio));
    xmlWriter_.writeEndElement();
}

// Writes "release" element
void ConfigurationWriter::writeRelease()
{
    xmlWriter_.writeStartElement("release");
    xmlWriter_.writeAttribute("major", QString::number(configuration_.usbconfig.majrel));
    xmlWriter_.writeAttribute("minor", QString::number(configuration_.usbconfig.minrel));
    xmlWriter_.writeEndElement();
}

// Generic procedure to write a named element with a word value in hexadecimal as its attribute (used for VID, PID and bitmaps in general)
void ConfigurationWriter::writeWordGeneric(QString name, quint16 value)
{
    xmlWriter_.writeStartElement(name);
    xmlWriter_.writeAttribute("value", QString::number(value, 16));
    xmlWriter_.writeEndElement();
}

ConfigurationWriter::ConfigurationWriter(const Configuration &configuration, const SerialGeneratorSettings &serialGeneratorSettings) :
    configuration_(configuration),
    serialGeneratorSettings_(serialGeneratorSettings)
{
}

// Writes the current configuration to a given file
void ConfigurationWriter::writeTo(QIODevice *device)
{
    xmlWriter_.setDevice(device);
    xmlWriter_.setAutoFormatting(true);
    xmlWriter_.writeStartDocument();
    xmlWriter_.writeStartElement("cp2130config");  // Root element
    xmlWriter_.writeAttribute("version", "1.0");
    writeDescriptor("manufacturer", configuration_.manufacturer);
    writeDescriptor("product", configuration_.product);
    writeDescriptor("serial", configuration_.serial);
    writeWordGeneric("vid", configuration_.usbconfig.vid);
    writeWordGeneric("pid", configuration_.usbconfig.pid);
    writeRelease();
    writePower();
    writeTransfer();
    writePins();
    writeDivider();
    writeBitmaps();
    xmlWriter_.writeEndElement();
}
