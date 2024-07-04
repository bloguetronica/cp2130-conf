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

ConfigurationReader::ConfigurationReader(Configuration &configuration, SerialGeneratorSettings &serialGeneratorSetting) :
    configuration_(configuration),
    serialGeneratorSettings_(serialGeneratorSetting)
{
}

// Writes the current configuration to a given file
int ConfigurationReader::readFrom(QIODevice *device)
{
    int retval = SUCCESS;
    serialGeneratorSettings_.doexport = false;  // Default settings if no "generator" element is found
    serialGeneratorSettings_.genenable = false;
    serialGeneratorSettings_.autogen = false;
    xmlReader_.setDevice(device);
    if (xmlReader_.readNextStartElement() && xmlReader_.name() == "cp2130config") {  // If the selected file is a CP2130 configuration file (the XML header is ignored)
        while (xmlReader_.readNextStartElement()) {
            if (xmlReader_.name() == "manufacturer") {  // Get manufacturer string
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString manufacturer = attr.value().toString();
                        if (static_cast<size_t>(manufacturer.size()) > CP2130::DESCMXL_MANUFACTURER) {
                            err_ = true;
                        } else {
                            configuration_.manufacturer = manufacturer;
                        }
                    }
                }
            } else if (xmlReader_.name() == "product") {  // Get product string
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString product = attr.value().toString();
                        if (static_cast<size_t>(product.size()) > CP2130::DESCMXL_PRODUCT) {
                            err_ = true;
                        } else {
                            configuration_.product = product;
                        }
                    }
                }
            } else if (xmlReader_.name() == "serial") {  // Get serial string
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString serial = attr.value().toString();
                        if (serial.isEmpty() || static_cast<size_t>(serial.size()) > CP2130::DESCMXL_SERIAL) {
                            err_ = true;
                        } else {
                            configuration_.serial = serial;
                        }
                    }
                }
          /*} else if (xmlReader_.readNextStartElement() && xmlReader_.name() == "generator") {  // Get serial generator settings
                serialGeneratorSettings_.doexport = true;
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "prototype") {
                        QString prototype = attr.value().toString();
                        if (!SerialGenerator::prototypeSerialIsValid(prototype)) {
                            err_ = true;
                        } else {
                            serialGeneratorSettings_.serialgen.setPrototypeSerial(prototype);
                        }
                    } else if (attr.name().toString() == "mode") {
                        bool ok;
                        quint8 mode = static_cast<quint8>(attr.value().toUShort(&ok));
                        if (!ok || !SerialGenerator::replaceModeIsValid(mode)) {
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
          */} else if (xmlReader_.name() == "vid") {  // Get VID
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "value") {
                        bool ok;
                        quint16 vid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
                        if (!ok || vid == 0x0000) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.vid = vid;
                        }
                    }
                }
            } else if (xmlReader_.name() == "pid") {  // Get PID
                foreach (const QXmlStreamAttribute &attr, xmlReader_.attributes()) {
                    if (attr.name().toString() == "value") {
                        bool ok;
                        quint16 pid = static_cast<quint16>(attr.value().toUShort(&ok, 16));  // Conversion done for sanity purposes
                        if (!ok || pid == 0x0000) {
                            err_ = true;
                        } else {
                            configuration_.usbconfig.pid = pid;
                        }
                    }
                }
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
    } else {  // The selected file is not a CP2130 configuration file (no further reading is done, and no subsequent errors are checked)
        retval = ERROR_NOT_VALID;
    }
    if (xmlReader_.hasError() || err_) {
        retval = ERROR_SYNTAX;
    }
    return retval;
}
