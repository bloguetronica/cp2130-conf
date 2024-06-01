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


// Includes
#include "serialgeneratordialog.h"
#include "ui_serialgeneratordialog.h"

SerialGeneratorDialog::SerialGeneratorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialGeneratorDialog)
{
    ui->setupUi(this);
}

SerialGeneratorDialog::~SerialGeneratorDialog()
{
    delete ui;
}

// Returns the state of "checkBoxDigits"
bool SerialGeneratorDialog::digitsCheckBoxIsChecked()
{
    return ui->checkBoxDigits->isChecked();
}

// Returns the state of "checkBoxEnable"
bool SerialGeneratorDialog::enableCheckBoxIsChecked()
{
    return ui->checkBoxEnable->isChecked();
}

// Returns the state of "checkBoxExportToFile"
bool SerialGeneratorDialog::exportToFileCheckBoxIsChecked()
{
    return ui->checkBoxExportToFile->isChecked();
}

// Returns the state of "checkBoxLowercase"
bool SerialGeneratorDialog::lowercaseCheckBoxIsChecked()
{
    return ui->checkBoxLowercase->isChecked();
}

// Returns the state of "checkBoxOverwrite"
bool SerialGeneratorDialog::overwriteCheckBoxIsChecked()
{
    return ui->checkBoxOverwrite->isChecked();
}

// Returns the value of "lineEditPrototypeSerial"
QString SerialGeneratorDialog::prototypeSerialLineEditText()
{
    return ui->lineEditPrototypeSerial->text();
}

// Sets the state of "checkBoxDigits"
void SerialGeneratorDialog::setDigitsCheckBox(bool digit)
{
    ui->checkBoxDigits->setChecked(digit);
}

// Sets the state of "checkBoxEnabled"
void SerialGeneratorDialog::setEnableCheckBox(bool enable)
{
    ui->checkBoxEnable->setChecked(enable);
}

// Sets the state of "checkBoxExportToFile"
void SerialGeneratorDialog::setExportToFileCheckBox(bool toFile)
{
    ui->checkBoxExportToFile->setChecked(toFile);
}

// Sets the state of "checkBoxLowercase"
void SerialGeneratorDialog::setLowercaseCheckBox(bool lower)
{
    ui->checkBoxLowercase->setChecked(lower);
}

// Sets the state of "checkBoxOverwrite"
void SerialGeneratorDialog::setOverwriteCheckBox(bool overwrite)
{
    ui->checkBoxOverwrite->setChecked(overwrite);
}

// Sets the text of "lineEditPrototypeSerial"
void SerialGeneratorDialog::setPrototypeSerialLineEditText(QString prototypeSerial)
{
    ui->lineEditPrototypeSerial->setText(prototypeSerial);
}

// Sets the state of "checkBoxUppercase"
void SerialGeneratorDialog::setUppercaseCheckBox(bool upper)
{
    ui->checkBoxUppercase->setChecked(upper);
}

// Returns the state of "checkBoxUppercase"
bool SerialGeneratorDialog::uppercaseCheckBoxIsChecked()
{
    return ui->checkBoxUppercase->isChecked();
}

void SerialGeneratorDialog::on_checkBoxExportToFile_stateChanged(int state)
{
    ui->checkBoxEnable->setEnabled(!state == Qt::Unchecked);
    ui->checkBoxOverwrite->setEnabled(!state == Qt::Unchecked);
}
