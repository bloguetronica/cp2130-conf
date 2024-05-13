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
#include <QString>
#include "otpromviewerdialog.h"
#include "ui_otpromviewerdialog.h"

OTPROMViewerDialog::OTPROMViewerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OTPROMViewerDialog)
{
    ui->setupUi(this);
}

OTPROMViewerDialog::~OTPROMViewerDialog()
{
    delete ui;
}

// Sets the content text of "plainTextEditPROMView"
void OTPROMViewerDialog::setOTPROMViewPlainText(const CP2130::PROMConfig &promConfig)
{
    QString promViewContents;
    for (size_t i = 0; i < CP2130::PROM_BLOCKS; ++i) {
        for (size_t j = 0; j < CP2130::PROM_BLOCK_SIZE; ++j) {
            promViewContents += QString("%1").arg(promConfig.blocks[i][j], 2, 16, QChar('0'));
            if (j % 16 == 15) {
                promViewContents += "\n";
            } else {
                promViewContents += " ";
            }
        }
    }
    ui->plainTextEditOTPROMView->setPlainText(promViewContents);
}
