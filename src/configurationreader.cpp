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
#include <QObject>
#include "cp2130.h"
#include "configurationreader.h"

void ConfigurationReader::readBitmaps()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("bitmaps"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("bitmaps")) {
            //
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

void ConfigurationReader::readConfiguration()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("cp2130config"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("manufacturer")) {
            readManufacturer();
        } else if (xmlReader_.name() == QLatin1String("product")) {
            readProduct();
        } else if (xmlReader_.name() == QLatin1String("serial")) {
            readSerial();
        } else if (xmlReader_.name() == QLatin1String("vid")) {
            readVID();
        } else if (xmlReader_.name() == QLatin1String("pid")) {
            readPID();
        } else if (xmlReader_.name() == QLatin1String("release")) {
            readRelease();
        } else if (xmlReader_.name() == QLatin1String("power")) {
            readPower();
        } else if (xmlReader_.name() == QLatin1String("transfer")) {
            readTransfer();
        } else if (xmlReader_.name() == QLatin1String("pins")) {
            readPins();
        } else if (xmlReader_.name() == QLatin1String("divider")) {
            readDivider();
        } else if (xmlReader_.name() == QLatin1String("bitmaps")) {
            readBitmaps();
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

void ConfigurationReader::readDivider()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("divider"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort divider = attr.value().toUShort(&ok);
            if (!ok || divider > 255) {
                xmlReader_.raiseError(QObject::tr("Invalid divider value."));
            } else {
                configuration_.pinconfig.divider = static_cast<quint8>(divider);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readGenerator()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("generator"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "prototype") {
            QString prototype = attr.value().toString();
            if (!SerialGenerator::isValidPrototypeSerial(prototype)) {
                xmlReader_.raiseError(QObject::tr("Invalid prototype string."));
            } else {
                serialGeneratorSettings_.serialgen.setPrototypeSerial(prototype);
            }
        } else if (attr.name().toString() == "mode") {
            bool ok;
            quint8 mode = static_cast<quint8>(attr.value().toUShort(&ok));
            if (!ok || !SerialGenerator::isValidReplaceMode(mode)) {
                xmlReader_.raiseError(QObject::tr("Invalid mode."));
            } else {
                serialGeneratorSettings_.serialgen.setReplaceMode(mode);
            }
        } else if (attr.name().toString() == "enable") {
            QString genenable = attr.value().toString();
            if (genenable != "true" && genenable != "false" && genenable != "1" && genenable != "0") {
                xmlReader_.raiseError(QObject::tr("Invalid enable flag."));
            } else {
                serialGeneratorSettings_.genenable = genenable == "true" || genenable == "1";
            }
        } else if (attr.name().toString() == "auto-generate") {
            QString autogen = attr.value().toString();
            if (autogen != "true" && autogen != "false" && autogen != "1" && autogen != "0") {
                xmlReader_.raiseError(QObject::tr("Invalid auto-generate flag."));
            } else {
                serialGeneratorSettings_.autogen = autogen == "true" || autogen == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readManufacturer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("manufacturer"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "string") {
            QString manufacturer = attr.value().toString();
            if (static_cast<size_t>(manufacturer.size()) > CP2130::DESCMXL_MANUFACTURER) {
                xmlReader_.raiseError(QObject::tr("Manufacturer string cannot be longer than %1 characters.").arg(CP2130::DESCMXL_MANUFACTURER));
            } else {
                configuration_.manufacturer = manufacturer;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readPID()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("pid"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 pid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || pid == 0x0000) {
                xmlReader_.raiseError(QObject::tr("Invalid PID value."));
            } else {
                configuration_.usbconfig.pid = pid;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readPins()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("pins"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("pins")) {
            //
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

void ConfigurationReader::readPower()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("power"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "maximum") {
            bool ok;
            ushort maxpow = attr.value().toUShort(&ok, 16);
            if (!ok || maxpow > 0xff) {
                xmlReader_.raiseError(QObject::tr("Invalid maximum power value."));
            } else {
                configuration_.usbconfig.maxpow = static_cast<quint8>(maxpow);
            }
        } else if (attr.name().toString() == "mode") {
            bool ok;
            ushort powmode = attr.value().toUShort(&ok);
            if (!ok || powmode > 2) {
                xmlReader_.raiseError(QObject::tr("Invalid power mode."));
            } else {
                configuration_.usbconfig.powmode = static_cast<quint8>(powmode);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readTransfer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("transfer"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "priority") {
            bool ok;
            ushort trfprio = attr.value().toUShort(&ok);
            if (!ok || trfprio > 1) {
                xmlReader_.raiseError(QObject::tr("Invalid transfer priority."));
            } else {
                configuration_.usbconfig.trfprio = static_cast<quint8>(trfprio);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readProduct()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("product"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "string") {
            QString product = attr.value().toString();
            if (static_cast<size_t>(product.size()) > CP2130::DESCMXL_PRODUCT) {
                xmlReader_.raiseError(QObject::tr("Product string cannot be longer than %1 characters.").arg(CP2130::DESCMXL_PRODUCT));
            } else {
                configuration_.product = product;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readRelease()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("release"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "major") {
            bool ok;
            ushort major = attr.value().toUShort(&ok);
            if (!ok || major > 255) {
                xmlReader_.raiseError(QObject::tr("Invalid major release number."));
            } else {
                configuration_.usbconfig.majrel = static_cast<quint8>(major);
            }
        } else if (attr.name().toString() == "minor") {
            bool ok;
            ushort minor = attr.value().toUShort(&ok);
            if (!ok || minor > 255) {
                xmlReader_.raiseError(QObject::tr("Invalid minor release number."));
            } else {
                configuration_.usbconfig.minrel = static_cast<quint8>(minor);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

void ConfigurationReader::readSerial()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("serial"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "string") {
            QString serial = attr.value().toString();
            if (serial.isEmpty()) {
                xmlReader_.raiseError(QObject::tr("Serial string cannot be empty."));
            } else if (static_cast<size_t>(serial.size()) > CP2130::DESCMXL_SERIAL) {
                xmlReader_.raiseError(QObject::tr("Serial string cannot be longer than %1 characters.").arg(CP2130::DESCMXL_SERIAL));
            } else {
                configuration_.serial = serial;
            }
        }
    }
    readSerialSubElements();
}

void ConfigurationReader::readSerialSubElements()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("serial"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("generator")) {
            serialGeneratorSettings_.doexport = true;
            readGenerator();
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

void ConfigurationReader::readVID()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("vid"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 vid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || vid == 0x0000) {
                xmlReader_.raiseError(QObject::tr("Invalid VID value."));
            } else {
                configuration_.usbconfig.vid = vid;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

ConfigurationReader::ConfigurationReader(Configuration &configuration, SerialGeneratorSettings &serialGeneratorSettings) :
    configuration_(configuration),
    serialGeneratorSettings_(serialGeneratorSettings)
{
}

QString ConfigurationReader::errorString() const
{
    return QObject::tr("Line %1, column %2: %3").arg(xmlReader_.lineNumber()).arg(xmlReader_.columnNumber()).arg(xmlReader_.errorString());
}

// Reads the configuration from a given file
bool ConfigurationReader::readFrom(QIODevice *device)
{
    serialGeneratorSettings_.doexport = false;  // Default settings if no "generator" element is found
    serialGeneratorSettings_.genenable = false;
    serialGeneratorSettings_.autogen = false;
    xmlReader_.setDevice(device);
    if (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("cp2130config")) {
            readConfiguration();
        } else {
            xmlReader_.raiseError(QObject::tr("Unknown root element.\n\nThe selected file is not a valid CP2130 configuration file."));
        }
    }
    return xmlReader_.error() == QXmlStreamReader::NoError;
}
