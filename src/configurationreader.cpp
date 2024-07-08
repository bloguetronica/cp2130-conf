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
#include "cp2130.h"
#include "configurationreader.h"

void ConfigurationReader::readDescriptor(QString name, QString &toValue)
{
    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "string") {
            QString descriptor = attr.value().toString();
            size_t limit;
            if (name == "manufacturer") {
                limit = CP2130::DESCMXL_MANUFACTURER;
            } else if (name == "product") {
                limit = CP2130::DESCMXL_PRODUCT;
            } else {
                limit = CP2130::DESCMXL_SERIAL;
            }
            if ((name == "serial" && descriptor.isEmpty()) || static_cast<size_t>(descriptor.size()) > limit) {
                err_ = true;
            } else {
                toValue = descriptor;
            }
            /*if (name == "serial") {

            }*/
        }
    }
}

void ConfigurationReader::readWordGeneric(quint16 &toValue)
{
    foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
        if (attr.name().toString() == "value") {
            bool ok;
            quint16 word = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
            if (!ok || word == 0x0000) {
                err_ = true;
            } else {
                toValue = word;
            }
        }
    }
}

ConfigurationReader::ConfigurationReader(Configuration &configuration, SerialGeneratorSettings &serialGeneratorSettings) :
    configuration_(configuration),
    serialGeneratorSettings_(serialGeneratorSettings)
{
}

// Writes the current configuration to a given file
int ConfigurationReader::readFrom(QIODevice *device)
{
    int retval;
    serialGeneratorSettings_.doexport = false;  // Default settings if no "generator" element is found
    serialGeneratorSettings_.genenable = false;
    serialGeneratorSettings_.autogen = false;
    xmlReader_.setDevice(device);
    if (xmlReader_.readNextStartElement() && xmlReader_.name() == "cp2130config") {  // If the selected file is a CP2130 configuration file (the XML header is ignored)
        while (xmlReader_.readNextStartElement()) {
            if (xmlReader_.name() == "manufacturer") {
                readDescriptor("manufacturer", configuration_.manufacturer);
            } else if (xmlReader_.name() == "product") {
                readDescriptor("product", configuration_.product);
            } else if (xmlReader_.name() == "serial") {
                readDescriptor("serial", configuration_.serial);
          /*} else if (xmlReader_.readNextStartElement() && xmlReader_.name() == "generator") {  // Get serial generator settings
                serialGeneratorSettings_.doexport = true;
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "prototype") {
                        QString prototype = attr.value().toString();
                        if (!SerialGenerator::isValidPrototypeSerial(prototype)) {
                            err_ = true;
                        } else {
                            serialGeneratorSettings_.serialgen.setPrototypeSerial(prototype);
                        }
                    } else if (attr.name().toString() == "mode") {
                        bool ok;
                        quint8 mode = static_cast<quint8>(attr.value().toUShort(&ok));
                        if (!ok || !SerialGenerator::isValidReplaceMode(mode)) {
                            err_ = true;
                        } else {
                            serialGeneratorSettings_.serialgen.setReplaceMode(mode);
                        }
                    } else if (attr.name().toString() == "enable") {
                        QString genenable = attr.value().toString();
                        if (genenable != "true" || genenable != "false") {
                            err_ = true;
                        } else {
                            serialGeneratorSettings_.genenable = genenable == "true";
                        }
                    } else if (attr.name().toString() == "auto-generate") {
                        QString autogen = attr.value().toString();
                        if (autogen != "true" || autogen != "false") {
                            err_ = true;
                        } else {
                            serialGeneratorSettings_.autogen = autogen == "true";
                        }
                    }
                }
                xmlReader_.skipCurrentElement();
          */} else if (xmlReader_.name() == "vid") {
                readWordGeneric(configuration_.usbconfig.vid);
            } else if (xmlReader_.name() == "pid") {
                readWordGeneric(configuration_.usbconfig.pid);
            } else if (xmlReader_.name() == "release") {  // Get release version
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "major") {  // Major release number
                        bool ok;
                        ushort major = attr.value().toUShort(&ok);
                        if (!ok || major > 255) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.majrel = static_cast<quint8>(major);
                        }
                    } else if (attr.name().toString() == "minor") {  // Minor release number
                        bool ok;
                        ushort minor = attr.value().toUShort(&ok);
                        if (!ok || minor > 255) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.minrel = static_cast<quint8>(minor);
                        }
                    }
                }
            } else if (xmlReader_.name() == "power") {  // Get power related parameters
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "maximum") {  // Maximum power (hexadecimal)
                        bool ok;
                        ushort maxpow = attr.value().toUShort(&ok, 16);
                        if (!ok || maxpow > 0xff) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.maxpow = static_cast<quint8>(maxpow);
                        }
                    } else if (attr.name().toString() == "mode") {  // Power mode (index)
                        bool ok;
                        ushort powmode = attr.value().toUShort(&ok);
                        if (!ok || powmode > 2) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.powmode = static_cast<quint8>(powmode);
                        }
                    }
                }
            }
          /*} else if ((CP2130::LWTRFPRIO & lockWord_) == CP2130::LWTRFPRIO) {
                // To implement
            } else if ((CP2130::LWPINCFG & lockWord_) == CP2130::LWPINCFG) {
                // To implement
            }*/
            xmlReader_.skipCurrentElement();
        }
        if (xmlReader_.hasError() || err_) {
            retval = ERROR_SYNTAX;
        } else {
            retval = SUCCESS;
        }
    } else {  // The selected file is not a CP2130 configuration file (no further reading is done, and no subsequent errors are checked)
        retval = ERROR_NOT_VALID;
    }
    return retval;
}
