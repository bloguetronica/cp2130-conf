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

// Reads the sub-elements of "bitmap" element
void ConfigurationReader::readBitmaps()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("bitmaps"));

    while (xmlReader_.readNextStartElement()) {
        if (xmlReader_.name() == QLatin1String("suspendlevel")) {
            readSuspendLevel();
        } else if (xmlReader_.name() == QLatin1String("suspendmode")) {
            readSuspendMode();
        } else if (xmlReader_.name() == QLatin1String("resumemask")) {
            readResumeMask();
        } else if (xmlReader_.name() == QLatin1String("resumematch")) {
            readResumeMatch();
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

// Reads "divider" element
void ConfigurationReader::readDivider()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("divider"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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
                xmlReader_.raiseError(QObject::tr("In \"generator\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 1 and 7"));
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

// Reads "gpio0" element
void ConfigurationReader::readGPIO0()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio0"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio0 = attr.value().toUShort(&ok);
            if (!ok || gpio0 > 3) {
                xmlReader_.raiseError(QObject::tr("In \"gpio0\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 3."));
            } else {
                configuration_.pinconfig.gpio0 = static_cast<quint8>(gpio0);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio1" element
void ConfigurationReader::readGPIO1()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio1"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio1 = attr.value().toUShort(&ok);
            if (!ok || gpio1 > 3) {
                xmlReader_.raiseError(QObject::tr("In \"gpio1\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 3."));
            } else {
                configuration_.pinconfig.gpio1 = static_cast<quint8>(gpio1);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio2" element
void ConfigurationReader::readGPIO2()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio2"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio2 = attr.value().toUShort(&ok);
            if (!ok || gpio2 > 3) {
                xmlReader_.raiseError(QObject::tr("In \"gpio2\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 3."));
            } else {
                configuration_.pinconfig.gpio2 = static_cast<quint8>(gpio2);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio3" element
void ConfigurationReader::readGPIO3()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio3"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio3 = attr.value().toUShort(&ok);
            if (!ok || gpio3 > 5) {
                xmlReader_.raiseError(QObject::tr("In \"gpio3\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 5."));
            } else {
                configuration_.pinconfig.gpio3 = static_cast<quint8>(gpio3);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio4" element
void ConfigurationReader::readGPIO4()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio4"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio4 = attr.value().toUShort(&ok);
            if (!ok || gpio4 > 7) {
                xmlReader_.raiseError(QObject::tr("In \"gpio4\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 7."));
            } else {
                configuration_.pinconfig.gpio4 = static_cast<quint8>(gpio4);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio5" element
void ConfigurationReader::readGPIO5()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio5"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio5 = attr.value().toUShort(&ok);
            if (!ok || gpio5 > 4) {
                xmlReader_.raiseError(QObject::tr("In \"gpio5\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 4."));
            } else {
                configuration_.pinconfig.gpio5 = static_cast<quint8>(gpio5);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio6" element
void ConfigurationReader::readGPIO6()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio6"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio6 = attr.value().toUShort(&ok);
            if (!ok || gpio6 > 3) {
                xmlReader_.raiseError(QObject::tr("In \"gpio6\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 3."));
            } else {
                configuration_.pinconfig.gpio6 = static_cast<quint8>(gpio6);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio7" element
void ConfigurationReader::readGPIO7()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio7"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio7 = attr.value().toUShort(&ok);
            if (!ok || gpio7 > 3) {
                xmlReader_.raiseError(QObject::tr("In \"gpio7\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 3."));
            } else {
                configuration_.pinconfig.gpio7 = static_cast<quint8>(gpio7);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio8" element
void ConfigurationReader::readGPIO8()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio8"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio8 = attr.value().toUShort(&ok);
            if (!ok || gpio8 > 4) {
                xmlReader_.raiseError(QObject::tr("In \"gpio8\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 4."));
            } else {
                configuration_.pinconfig.gpio8 = static_cast<quint8>(gpio8);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio9" element
void ConfigurationReader::readGPIO9()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio9"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio9 = attr.value().toUShort(&ok);
            if (!ok || gpio9 > 4) {
                xmlReader_.raiseError(QObject::tr("In \"gpio9\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 4."));
            } else {
                configuration_.pinconfig.gpio9 = static_cast<quint8>(gpio9);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "gpio10" element
void ConfigurationReader::readGPIO10()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("gpio10"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "mode") {
            bool ok;
            ushort gpio10 = attr.value().toUShort(&ok);
            if (!ok || gpio10 > 4) {
                xmlReader_.raiseError(QObject::tr("In \"gpio10\" element, the \"mode\" attribute contains an invalid value. It should be an integer between 0 and 4."));
            } else {
                configuration_.pinconfig.gpio10 = static_cast<quint8>(gpio10);
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "manufacturer" element
void ConfigurationReader::readManufacturer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("manufacturer"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

// Reads "pid" element
void ConfigurationReader::readPID()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("pid"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 pid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || pid == 0x0000) {
                xmlReader_.raiseError(QObject::tr("In \"pid\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 1 and ffff."));
            } else {
                configuration_.usbconfig.pid = pid;
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
            readGPIO0();
        } else if (xmlReader_.name() == QLatin1String("gpio1")) {
            readGPIO1();
        } else if (xmlReader_.name() == QLatin1String("gpio2")) {
            readGPIO2();
        } else if (xmlReader_.name() == QLatin1String("gpio3")) {
            readGPIO3();
        } else if (xmlReader_.name() == QLatin1String("gpio4")) {
            readGPIO4();
        } else if (xmlReader_.name() == QLatin1String("gpio5")) {
            readGPIO5();
        } else if (xmlReader_.name() == QLatin1String("gpio6")) {
            readGPIO6();
        } else if (xmlReader_.name() == QLatin1String("gpio7")) {
            readGPIO7();
        } else if (xmlReader_.name() == QLatin1String("gpio8")) {
            readGPIO8();
        } else if (xmlReader_.name() == QLatin1String("gpio9")) {
            readGPIO9();
        } else if (xmlReader_.name() == QLatin1String("gpio10")) {
            readGPIO10();
        } else {
            xmlReader_.skipCurrentElement();
        }
    }
}

// Reads "power" element
void ConfigurationReader::readPower()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("power"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

// Reads "resumemask" element
void ConfigurationReader::readResumeMask()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("resumemask"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort wkupmask = attr.value().toUShort(&ok, 16);
            if (!ok || wkupmask > 0x7fff) {
                xmlReader_.raiseError(QObject::tr("In \"resumemask\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and 7fff."));
            } else {
                configuration_.pinconfig.wkupmask = wkupmask;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "resumematch" element
void ConfigurationReader::readResumeMatch()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("resumematch"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort wkupmatch = attr.value().toUShort(&ok, 16);
            if (!ok || wkupmatch > 0x7fff) {
                xmlReader_.raiseError(QObject::tr("In \"resumematch\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and 7fff."));
            } else {
                configuration_.pinconfig.wkupmatch = wkupmatch;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "serial" element
void ConfigurationReader::readSerial()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("serial"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "string") {
            QString serial = attr.value().toString();
            if (serial.isEmpty() || static_cast<size_t>(serial.size()) > CP2130::DESCMXL_SERIAL) {
                xmlReader_.raiseError(QObject::tr("In \"serial\" element, the \"string\" attribute contains an invalid value. It should contain a valid serial string, which cannot be empty and cannot be longer than %1 characters.").arg(CP2130::DESCMXL_SERIAL));
            } else {
                configuration_.serial = serial;
            }
        }
    }
    readSerialSubElements();
}

// Reads the sub-elements of "serial" element
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

// Reads "suspendlevel" element
void ConfigurationReader::readSuspendLevel()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("suspendlevel"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort sspndlvl = attr.value().toUShort(&ok, 16);
            if (!ok || sspndlvl > 0x7fff) {
                xmlReader_.raiseError(QObject::tr("In \"suspendlevel\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and 7fff."));
            } else {
                configuration_.pinconfig.sspndlvl = sspndlvl;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "suspendmode" element
void ConfigurationReader::readSuspendMode()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("suspendmode"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            ushort sspndmode = attr.value().toUShort(&ok, 16);
            if (!ok) {
                xmlReader_.raiseError(QObject::tr("In \"suspendmode\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 0 and ffff."));
            } else {
                configuration_.pinconfig.sspndmode = sspndmode;
            }
        }
    }
    xmlReader_.skipCurrentElement();
}

// Reads "transfer" element
void ConfigurationReader::readTransfer()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("transfer"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
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

// Reads "vid" element
void ConfigurationReader::readVID()
{
    Q_ASSERT(xmlReader_.isStartElement() && xmlReader_.name() == QLatin1String("vid"));

    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 vid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || vid == 0x0000) {
                xmlReader_.raiseError(QObject::tr("In \"vid\" element, the \"value\" attribute contains an invalid value. It should be an hexadecimal integer between 1 and ffff."));
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

// Returns an error string
QString ConfigurationReader::errorString() const
{
    return QObject::tr("Line %1, column %2: %3").arg(xmlReader_.lineNumber()).arg(xmlReader_.columnNumber()).arg(xmlReader_.errorString());
}

// Reads the configuration from a given file, returning false in case of error or true if it succeeds
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
