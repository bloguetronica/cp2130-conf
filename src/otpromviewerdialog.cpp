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


// Includes
#include <QMetaObject>
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
    QString otpromViewContents;
    for (size_t i = 0; i < CP2130::PROM_BLOCKS; ++i) {
        otpromViewContents += tr("Block %1:\n").arg(i);
        for (size_t j = 0; j < CP2130::PROM_BLOCK_SIZE; ++j) {
            otpromViewContents += QString("%1").arg(promConfig.blocks[i][j], 2, 16, QChar('0'));
            if (j % 16 == 15) {
                otpromViewContents += "\n";
            } else {
                otpromViewContents += " ";
            }
        }
        otpromViewContents += "\n";
    }
    otpromViewContents += tr("%1 blocks of %2 bytes. %3 bytes in total.").arg(CP2130::PROM_BLOCKS).arg(CP2130::PROM_BLOCK_SIZE).arg(CP2130::PROM_SIZE);
    ui->plainTextEditOTPROMView->setPlainText(otpromViewContents);
}
