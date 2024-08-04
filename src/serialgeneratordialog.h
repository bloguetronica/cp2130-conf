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


#ifndef SERIALGENERATORDIALOG_H
#define SERIALGENERATORDIALOG_H

// Includes
#include <QDialog>
#include <QString>

namespace Ui {
class SerialGeneratorDialog;
}

class SerialGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SerialGeneratorDialog(QWidget *parent = nullptr);
    ~SerialGeneratorDialog();

    bool autoGenerateCheckBoxIsChecked();
    bool digitsCheckBoxIsChecked();
    bool enableCheckBoxIsChecked();
    bool exportToFileCheckBoxIsChecked();
    bool lowercaseCheckBoxIsChecked();
    QString prototypeSerialLineEditText();
    void setAutoGenerateCheckBox(bool autogen);
    void setDigitsCheckBox(bool digit);
    void setEnableCheckBox(bool enable);
    void setExportToFileCheckBox(bool doexport);
    void setLowercaseCheckBox(bool lower);
    void setPrototypeSerialLineEditText(QString prototype);
    void setUppercaseCheckBox(bool upper);
    bool uppercaseCheckBoxIsChecked();

private slots:
    void on_checkBoxExportToFile_stateChanged(int state);

private:
    Ui::SerialGeneratorDialog *ui;
};

#endif  // SERIALGENERATORDIALOG_H
