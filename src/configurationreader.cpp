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


// Include
#include <QObject>
#include "cp2130.h"
#include "configurationreader.h"

// Reads the sub-elements of "bitmap" element
void ConfigurationReader::readBitmaps()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("bitmaps"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("suspendlevel")) {
            readWordGeneric("suspendlevel", configuration_.pinconfig.sspndlvl, 0x0000, 0x7fff);
        } else if (xmlReader_.name() == QLatin1String("suspendmode")) {
            readWordGeneric("suspendmode", configuration_.pinconfig.sspndlvl, 0x0000, 0xffff);
        } else if (xmlReader_.name() == QLatin1String("resumemask")) {
            readWordGeneric("resumemask", configuration_.pinconfig.sspndlvl, 0x0000, 0x7fff);
        } else if (xmlReader_.name() == QLatin1String("resumematch")) {
            readWordGeneric("resumematch", configuration_.pinconfig.sspndlvl, 0x0000, 0x7fff);
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

// Reads the sub-elements of "cp2130config" element, which is the root element
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
            readWordGeneric("vid", configuration_.pinconfig.sspndlvl, 0x0001, 0xffff);
        } else if (xmlReader_.name() == QLatin1String("pid")) {
            readWordGeneric("pid", configuration_.pinconfig.sspndlvl, 0x0001, 0xffff);
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

// Reads "divider" element
void ConfigurationReader::readDivider()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("divider"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "value") {
            bool ok;
            ushort divider = attr.value().toUShort(&ok);
            if (!ok || divider > 255) {
                xmlReader_.raiseError(QObject::tr("In \"divider\" element, the \"value\" attribute contains an invalid value. It should be an integer between 0 and 255."));
            } else {
                configuration_.pinconfig.divider = static_cast<quint8>(divider);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "generator" element
void ConfigurationReader::readGenerator()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("generator"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "prototype") {
            QString prototype = attr.value().toString();
            if (!SerialGenerator::isValidPrototypeSerial(prototype)) {
                xmlReader_.raiseError(QObject::tr("In \"generator\" element, the \"prototype\" attribute contains an invalid value. It should contain a valid prototype serial string, having at least one least one wildcard character (?) and no more than % characters.").arg(CP2130::DESCMXL_SERIAL));
            } else {
                serialGeneratorSettings_.serialgen.setPrototypeSerial(prototype);
            }
        } else if (attr.name().toString() == "mode") {
            bool ok;
            quint8 mode = static_cast<quint8>(attr.value().toUShort(&ok));
            if (!ok || !SerialGenerator::isValidReplaceMode(mode)) {
                xmlReader_.raiseError(QObject::tr("In \"generator\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 1 and 7."));  // Corrected in version 3.1
            } else {
                serialGeneratorSettings_.serialgen.setReplaceMode(mode);
            }
        } else if (attr.name().toString() == "enable") {
            QString genenable = attr.value().toString();
            if (genenable != "true" && genenable != "false" && genenable != "1" && genenable != "0") {
                xmlReader_.raiseError(QObject::tr("In \"generator\" element, the \"enable\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                serialGeneratorSettings_.genenable = genenable == "true" || genenable == "1";
            }
        } else if (attr.name().toString() == "auto-generate") {
            QString autogen = attr.value().toString();
            if (autogen != "true" && autogen != "false" && autogen != "1" && autogen != "0") {
                xmlReader_.raiseError(QObject::tr("In \"generator\" element, the \"auto-generate\" attribute contains an invalid value. It should be \"true\", \"false\", \"1\" or \"0\"."));
            } else {
                serialGeneratorSettings_.autogen = autogen == "true" || autogen == "1";
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads GPIO element (implemented in version 3.1 to replace readGPIO0(), readGPIO1(), etc)
void ConfigurationReader::readGPIO(int number, quint8 &toVariable, quint8 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QString("gpio%1").arg(number));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio = attr.value().toUShort(&ok);
            if (!ok || gpio > max) {
                xmlReader_.raiseError(QObject::tr("In \"gpio%1\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and %2.").arg(number).arg(max));
            } else {
                toVariable = static_cast<quint8>(gpio);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "manufacturer" element
void ConfigurationReader::readManufacturer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("manufacturer"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "string") {
            QString manufacturer = attr.value().toString();
            if (static_cast<size_t>(manufacturer.size()) > CP2130::DESCMXL_MANUFACTURER) {
                xmlReader_.raiseError(QObject::tr("In \"manufacturer\" element, the \"string\" attribute contains an invalid value. It should contain a valid manufacturer string, having no more than %1 characters.").arg(CP2130::DESCMXL_MANUFACTURER));
            } else {
                configuration_.manufacturer = manufacturer;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads the sub-elements of "pins" element
void ConfigurationReader::readPins()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("pins"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("gpio0")) {
            readGPIO(0, configuration_.pinconfig.gpio0, 3);
        } else if (xmlReader_.name() == QLatin1String("gpio1")) {
            readGPIO(1, configuration_.pinconfig.gpio1, 3);
        } else if (xmlReader_.name() == QLatin1String("gpio2")) {
            readGPIO(2, configuration_.pinconfig.gpio2, 3);
        } else if (xmlReader_.name() == QLatin1String("gpio3")) {
            readGPIO(3, configuration_.pinconfig.gpio3, 5);
        } else if (xmlReader_.name() == QLatin1String("gpio4")) {
            readGPIO(4, configuration_.pinconfig.gpio4, 7);
        } else if (xmlReader_.name() == QLatin1String("gpio5")) {
            readGPIO(5, configuration_.pinconfig.gpio5, 4);
        } else if (xmlReader_.name() == QLatin1String("gpio6")) {
            readGPIO(6, configuration_.pinconfig.gpio6, 3);
        } else if (xmlReader_.name() == QLatin1String("gpio7")) {
            readGPIO(7, configuration_.pinconfig.gpio7, 3);
        } else if (xmlReader_.name() == QLatin1String("gpio8")) {
            readGPIO(8, configuration_.pinconfig.gpio8, 4);
        } else if (xmlReader_.name() == QLatin1String("gpio9")) {
            readGPIO(9, configuration_.pinconfig.gpio9, 4);
        } else if (xmlReader_.name() == QLatin1String("gpio10")) {
            readGPIO(10, configuration_.pinconfig.gpio10, 4);
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

// Reads "power" element
void ConfigurationReader::readPower()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("power"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "maximum") {
            bool ok;
            ushort maxpow = attr.value().toUShort(&ok, 16);
            if (!ok || maxpow > 0xfa) {
                xmlReader_.raiseError(QObject::tr("In \"power\" element, the \"maximum\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and fa."));
            } else {
                configuration_.usbconfig.maxpow = static_cast<quint8>(maxpow);
            }
        } else if (attr.name().toString() == "mode") {
            bool ok;
            ushort powmode = attr.value().toUShort(&ok);
            if (!ok || powmode > 2) {
                xmlReader_.raiseError(QObject::tr("In \"power\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 2."));
            } else {
                configuration_.usbconfig.powmode = static_cast<quint8>(powmode);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "product" element
void ConfigurationReader::readProduct()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("product"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "string") {
            QString product = attr.value().toString();
            if (static_cast<size_t>(product.size()) > CP2130::DESCMXL_PRODUCT) {
                xmlReader_.raiseError(QObject::tr("In \"product\" element, the \"string\" attribute contains an invalid value. It should contain a valid product string, having no more than %1 characters.").arg(CP2130::DESCMXL_PRODUCT));
            } else {
                configuration_.product = product;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "release" element
void ConfigurationReader::readRelease()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("release"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "major") {
            bool ok;
            ushort major = attr.value().toUShort(&ok);
            if (!ok || major > 255) {
                xmlReader_.raiseError(QObject::tr("In \"release\" element, the \"major\" attribute contains an invalid value. It should be an integer between 0 and 255."));
            } else {
                configuration_.usbconfig.majrel = static_cast<quint8>(major);
            }
        } else if (attr.name().toString() == "minor") {
            bool ok;
            ushort minor = attr.value().toUShort(&ok);
            if (!ok || minor > 255) {
                xmlReader_.raiseError(QObject::tr("In \"release\" element, the \"minor\" attribute contains an invalid value. It should be an integer between 0 and 255."));
            } else {
                configuration_.usbconfig.minrel = static_cast<quint8>(minor);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "serial" element
void ConfigurationReader::readSerial()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("serial"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "string") {
            QString serial = attr.value().toString();
            if (serial.isEmpty() || static_cast<size_t>(serial.size()) > CP2130::DESCMXL_SERIAL) {
                xmlReader_.raiseError(QObject::tr("In \"serial\" element, the \"string\" attribute contains an invalid value. It should contain a valid serial string, which cannot be empty and cannot be longer than %1 characters.").arg(CP2130::DESCMXL_SERIAL));
            } else {
                configuration_.serial = serial;
            }
        }
    }
    readSerialSubElements();  // This follows the same pattern as xmlReader_.skipCurrentElement()
}

// Reads the sub-elements of "serial" element
void ConfigurationReader::readSerialSubElements()
{
    // Call to Q_ASSERT() removed from here since version 3.1 (bug fix)

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("generator")) {
            serialGeneratorSettings_.doexport = true;
            readGenerator();
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

// Reads "transfer" element
void ConfigurationReader::readTransfer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("transfer"));

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "priority") {
            bool ok;
            ushort trfprio = attr.value().toUShort(&ok);
            if (!ok || trfprio > 1) {
                xmlReader_.raiseError(QObject::tr("In \"transfer\" element, the \"priority\" attribute contains an invalid value. It should be an integer between 0 and 1."));
            } else {
                configuration_.usbconfig.trfprio = static_cast<quint8>(trfprio);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Generic procedure to read a named element with a word value in hexadecimal as it's attribute (implemented in version 3.1 to replace readPID(), readResumeMask(), readResumeMatch(), readSuspendLevel(), readSuspendMode() and readVID())
void ConfigurationReader::readWordGeneric(QString name, quint16 &toVariable, quint16 min, quint16 max)
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == name);

    const QXmlStreamAttributes attrs = xmlReader_.attributes();
    for (const QXmlStreamAttribute &attr : attrs) {  // Refactored in version 3.1
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 word = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || word > max || word < min) {
                xmlReader_.raiseError(QObject::tr("In \"%1\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between %2 and %3.").arg(name).arg(min, 0, 16).arg(max, 0, 16));
            } else {
                toVariable = word;
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

// Returns an error string
QString ConfigurationReader::errorString() const
{
    return QObject::tr("Line %1, column %2: %3").arg(xmlReader_.lineNumber()).arg(xmlReader_.columnNumber()).arg(xmlReader_.errorString());
}

// Reads the configuration from a given file, returning false in case of error or true if it succeeds
bool ConfigurationReader::readFrom(QIODevice *device)
{
    serialGeneratorSettings_.serialgen = SerialGenerator();  // Default settings if no "generator" element is found (this line was added in version 3.1, so the serial generator parameters are also set to their default values)
    serialGeneratorSettings_.doexport = false;
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
