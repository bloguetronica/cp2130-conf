/* CP2130 Configurator - Version 1.0 for Debian Linux
   Copyright (c) 2021 Samuel Lourenço

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
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

// Definitions
const int POWER_LIMIT = 500;  // Maximum current consumption limit, as per the USB 2.0 specification

ConfiguratorWindow::ConfiguratorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfiguratorWindow)
{
    ui->setupUi(this);
    ui->lineEditVID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditPID->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditMaxPower->setValidator(new QRegExpValidator(QRegExp("[\\d]+"), this));
    ui->lineEditMaxPowerHex->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditSuspendLevel->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditSuspendMode->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditResumeMask->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
    ui->lineEditResumeMatch->setValidator(new QRegExpValidator(QRegExp("[A-Fa-f\\d]+"), this));
}

ConfiguratorWindow::~ConfiguratorWindow()
{
    delete ui;
}

void ConfiguratorWindow::on_lineEditMaxPower_editingFinished()
{
    ui->lineEditMaxPower->setText(QString::number(2 * (ui->lineEditMaxPower->text().toInt() / 2)));  // This removes any leading zeros and also rounds to the previous even number, if the value is odd
}

void ConfiguratorWindow::on_lineEditMaxPower_textChanged()
{
    if (ui->lineEditMaxPower->text().isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPower->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPower_textEdited()
{
    QString maxPowerStr = ui->lineEditMaxPower->text();
    int maxPower = maxPowerStr.toInt();
    if (maxPower > POWER_LIMIT) {
        maxPowerStr.chop(1);
        ui->lineEditMaxPower->setText(maxPowerStr);
        maxPower /= 10;
    }
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(maxPower / 2, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_editingFinished()
{
    if (ui->lineEditMaxPowerHex->text().size() < 2) {
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(ui->lineEditMaxPowerHex->text().toInt(nullptr, 16), 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_textChanged()
{
    if (ui->lineEditMaxPowerHex->text().isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditMaxPowerHex->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditMaxPowerHex_textEdited()
{
    ui->lineEditMaxPowerHex->setText(ui->lineEditMaxPowerHex->text().toLower());
    int maxPower = 2 * ui->lineEditMaxPowerHex->text().toInt(nullptr, 16);
    if (maxPower > POWER_LIMIT) {
        maxPower = POWER_LIMIT;
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(POWER_LIMIT / 2, 2, 16, QChar('0')));
    }
    ui->lineEditMaxPower->setText(QString::number(maxPower));
}

void ConfiguratorWindow::on_lineEditPID_textChanged()
{
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditPID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditPID_textEdited()
{
    ui->lineEditPID->setText(ui->lineEditPID->text().toLower());
}

void ConfiguratorWindow::on_lineEditSuspendLevel_textChanged()
{
    if (ui->lineEditSuspendLevel->text().size() < 4) {
        ui->lineEditSuspendLevel->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditSuspendLevel->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditSuspendLevel_textEdited()
{
    ui->lineEditSuspendLevel->setText(ui->lineEditSuspendLevel->text().toLower());
}

void ConfiguratorWindow::on_lineEditSuspendMode_textChanged()
{
    if (ui->lineEditSuspendMode->text().size() < 4) {
        ui->lineEditSuspendMode->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditSuspendMode->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditSuspendMode_textEdited()
{
    ui->lineEditSuspendMode->setText(ui->lineEditSuspendMode->text().toLower());
}

void ConfiguratorWindow::on_lineEditVID_textChanged()
{
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditVID->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditVID_textEdited()
{
    ui->lineEditVID->setText(ui->lineEditVID->text().toLower());
}

void ConfiguratorWindow::on_lineEditResumeMatch_textChanged()
{
    if (ui->lineEditResumeMatch->text().size() < 4) {
        ui->lineEditResumeMatch->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditResumeMatch->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditResumeMatch_textEdited()
{
    ui->lineEditResumeMatch->setText(ui->lineEditResumeMatch->text().toLower());
}

void ConfiguratorWindow::on_lineEditResumeMask_textChanged()
{
    if (ui->lineEditResumeMask->text().size() < 4) {
        ui->lineEditResumeMask->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditResumeMask->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditResumeMask_textEdited()
{
    ui->lineEditResumeMask->setText(ui->lineEditResumeMask->text().toLower());
}

void ConfiguratorWindow::on_pushButtonRevert_clicked()
{
    displayConfiguration(initConfig_);
}

void ConfiguratorWindow::on_pushButtonWrite_clicked()
{
    if(showInvalidInput()) {
        QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
    } else {
        getEditedConfiguration();
        if (editedConfig_ == initConfig_ && !ui->checkBoxLock->isChecked()) {
            QMessageBox::information(this, tr("No changes done"), tr("No changes were effected, because no values were modified."));
        } else {
            int qmret = QMessageBox::question(this, tr("Write configuration?"), tr("This will write the changes to the OTP ROM of your device. These changes will be permanent.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (qmret == QMessageBox::Yes)
            {
                QProgressDialog progress(tr("Configuring device..."), tr("Abort"), 0, 1700000, this);
                progress.setWindowModality(Qt::WindowModal);
                for (int i = 0; i < 1700000; i++) {
                    progress.setValue(i);

                    if (progress.wasCanceled())
                        break;
                    //... copy one file
                }
                progress.setValue(1700000);
            }
        }
    }
}

//
void ConfiguratorWindow::openDevice(quint16 vid, quint16 pid, const QString &serialstr)
{
    int err = cp2130_.open(vid, pid, serialstr);
    if (err == 1) {  // Failed to initialize libusb
        QMessageBox::critical(this, tr("Critical Error"), tr("Could not initialize libusb.\n\nThis is a critical error and execution will be aborted."));
        exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
    } else if (err == 2) {  // Failed to find device
        QMessageBox::critical(this, tr("Error"), tr("Could not find device."));
        this->deleteLater();  // Close window after the subsequent show() call
    } else if (err == 3) {  // Failed to claim interface
        QMessageBox::critical(this, tr("Error"), tr("Device is currently unavailable.\n\nPlease confirm that the device is not in use."));
        this->deleteLater();  // Close window after the subsequent show() call
    } else {
        vid_ = vid;
        pid_ = pid;
        serialstr_ = serialstr;  // Valid serial number
        readInitialConfiguration();
        this->setWindowTitle(tr("CP2130 Configurator (S/N: %1)").arg(serialstr_));
        displayConfiguration(initConfig_);
    }
}

//
void ConfiguratorWindow::displayConfiguration(const Configuration &config)
{
    displayManufacturer(config.manufacturer);
    setManufacturerEnabled((lockWord_ & CP2130::LWMANUF) == CP2130::LWMANUF);
    displayProduct(config.product);
    setProductEnabled((lockWord_ & CP2130::LWPROD) == CP2130::LWPROD);
    displaySerial(config.serial);
    setSerialEnabled((lockWord_ & CP2130::LWSER) != 0x0000);
    displayUSBConfig(config.usbconfig);
    setVIDEnabled((lockWord_ & CP2130::LWVID) != 0x0000);
    setPIDEnabled((lockWord_ & CP2130::LWPID) != 0x0000);
    setReleaseEnabled((lockWord_ & CP2130::LWREL) != 0x0000);
    setMaxPowerEnabled((lockWord_ & CP2130::LWMAXPOW) != 0x0000);
    setPowerModeEnabled((lockWord_ & CP2130::LWPOWMODE) != 0x0000);
    setTransferPrioEnabled((lockWord_ & CP2130::LWTRFPRIO) != 0x0000);
    displayPinConfig(config.pinconfig);
    setPinConfigEnabled((lockWord_ & CP2130::LWPINCFG) != 0x0000);
    setWriteEnabled((lockWord_ & CP2130::LWALL) != 0x0000);
}

//
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

//
void ConfiguratorWindow::displayPinConfig(const CP2130::PinConfig &pinconfig)
{
    ui->comboBoxGPIO0->setCurrentIndex(pinconfig.gpio0);
    ui->comboBoxGPIO1->setCurrentIndex(pinconfig.gpio1);
    ui->comboBoxGPIO2->setCurrentIndex(pinconfig.gpio2);
    ui->comboBoxGPIO3->setCurrentIndex(pinconfig.gpio3);
    ui->comboBoxGPIO4->setCurrentIndex(pinconfig.gpio4);
    ui->comboBoxGPIO5->setCurrentIndex(pinconfig.gpio5);
    ui->comboBoxGPIO6->setCurrentIndex(pinconfig.gpio6);
    ui->comboBoxGPIO7->setCurrentIndex(pinconfig.gpio7);
    ui->comboBoxGPIO8->setCurrentIndex(pinconfig.gpio8);
    ui->comboBoxGPIO9->setCurrentIndex(pinconfig.gpio9);
    ui->comboBoxGPIO10->setCurrentIndex(pinconfig.gpio10);
    ui->spinBoxDivider->setValue(pinconfig.divider);
    ui->lineEditSuspendLevel->setText(QString("%1").arg(pinconfig.sspndlvl, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
    ui->lineEditSuspendMode->setText(QString("%1").arg(pinconfig.sspndmode, 4, 16, QChar('0')));  // Same as above
    ui->lineEditResumeMask->setText(QString("%1").arg(pinconfig.wkupmask, 4, 16, QChar('0')));  // Same as above
    ui->lineEditResumeMatch->setText(QString("%1").arg(pinconfig.wkupmatch, 4, 16, QChar('0')));  // Same as above
}

//
void ConfiguratorWindow::displayProduct(const QString &product)
{
    ui->lineEditProduct->setText(product);
}

//
void ConfiguratorWindow::displaySerial(const QString &serial)
{
    ui->lineEditSerial->setText(serial);
}

//
void ConfiguratorWindow::displayUSBConfig(const CP2130::USBConfig &usbconfig)
{
    ui->lineEditVID->setText(QString("%1").arg(usbconfig.vid, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
    ui->lineEditPID->setText(QString("%1").arg(usbconfig.pid, 4, 16, QChar('0')));  // Same as before
    ui->spinBoxMajVersion->setValue(usbconfig.majrel);
    ui->spinBoxMinVersion->setValue(usbconfig.minrel);
    ui->lineEditMaxPower->setText(QString::number(2 * usbconfig.maxpow));
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(usbconfig.maxpow, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    ui->comboBoxPowerMode->setCurrentIndex(usbconfig.powmode);
    ui->comboBoxTransferPrio->setCurrentIndex(usbconfig.trfprio);
}

//
void ConfiguratorWindow::getEditedConfiguration()
{
    editedConfig_.manufacturer = ui->lineEditManufacturer->text();
    editedConfig_.product = ui->lineEditProduct->text();
    editedConfig_.serial = ui->lineEditSerial->text();
    editedConfig_.usbconfig.vid = static_cast<quint16>(ui->lineEditVID->text().toInt(nullptr, 16));
    editedConfig_.usbconfig.pid = static_cast<quint16>(ui->lineEditPID->text().toInt(nullptr, 16));
    editedConfig_.usbconfig.majrel = static_cast<quint8>(ui->spinBoxMajVersion->value());
    editedConfig_.usbconfig.minrel = static_cast<quint8>(ui->spinBoxMinVersion->value());
    editedConfig_.usbconfig.maxpow = static_cast<quint8>(ui->lineEditMaxPower->text().toInt() / 2);
    editedConfig_.usbconfig.powmode = static_cast<quint8>(ui->comboBoxPowerMode->currentIndex());
    editedConfig_.usbconfig.trfprio = static_cast<quint8>(ui->comboBoxTransferPrio->currentIndex());
    editedConfig_.pinconfig.gpio0 = static_cast<quint8>(ui->comboBoxGPIO0->currentIndex());
    editedConfig_.pinconfig.gpio1 = static_cast<quint8>(ui->comboBoxGPIO1->currentIndex());
    editedConfig_.pinconfig.gpio2 = static_cast<quint8>(ui->comboBoxGPIO2->currentIndex());
    editedConfig_.pinconfig.gpio3 = static_cast<quint8>(ui->comboBoxGPIO3->currentIndex());
    editedConfig_.pinconfig.gpio4 = static_cast<quint8>(ui->comboBoxGPIO4->currentIndex());
    editedConfig_.pinconfig.gpio5 = static_cast<quint8>(ui->comboBoxGPIO5->currentIndex());
    editedConfig_.pinconfig.gpio6 = static_cast<quint8>(ui->comboBoxGPIO6->currentIndex());
    editedConfig_.pinconfig.gpio7 = static_cast<quint8>(ui->comboBoxGPIO7->currentIndex());
    editedConfig_.pinconfig.gpio8 = static_cast<quint8>(ui->comboBoxGPIO8->currentIndex());
    editedConfig_.pinconfig.gpio9 = static_cast<quint8>(ui->comboBoxGPIO9->currentIndex());
    editedConfig_.pinconfig.gpio10 = static_cast<quint8>(ui->comboBoxGPIO10->currentIndex());
    editedConfig_.pinconfig.sspndlvl = static_cast<quint16>(ui->lineEditSuspendLevel->text().toInt());
    editedConfig_.pinconfig.sspndmode = static_cast<quint16>(ui->lineEditSuspendMode->text().toInt());
    editedConfig_.pinconfig.wkupmask = static_cast<quint16>(ui->lineEditResumeMask->text().toInt());
    editedConfig_.pinconfig.wkupmatch = static_cast<quint16>(ui->lineEditResumeMatch->text().toInt());
    editedConfig_.pinconfig.divider = static_cast<quint8>(ui->spinBoxDivider->value());
}

//
void ConfiguratorWindow::readInitialConfiguration()
{
    int errcnt = 0;
    QString errstr;
    initConfig_.manufacturer = cp2130_.getManufacturerDesc(errcnt, errstr);
    initConfig_.product = cp2130_.getProductDesc(errcnt, errstr);
    initConfig_.serial = cp2130_.getSerialDesc(errcnt, errstr);
    initConfig_.usbconfig = cp2130_.getUSBConfig(errcnt, errstr);
    initConfig_.pinconfig = cp2130_.getPinConfig(errcnt, errstr);
    lockWord_ = cp2130_.getLockWord(errcnt, errstr);
    if (errcnt > 0) {
        if (cp2130_.disconnected()) {
            cp2130_.close();
            QMessageBox::critical(this, tr("Error"), tr("Device disconnected.\n\nPlease reconnect it and try again."));
        } else {
            cp2130_.reset(errcnt, errstr);  // Try to reset the device for sanity purposes, but don't check if it was successful
            cp2130_.close();
            errstr.chop(1);  // Remove the last character, which is always a newline
            QMessageBox::critical(this, tr("Error"), tr("Read operation returned the following error(s):\n– %1\n\nPlease try accessing the device again.", "", errcnt).arg(errstr.replace("\n", "\n– ")));
        }
        this->deleteLater();  // In a context where the window is already visible, it has the same effect as this->close()
    }
}

//
void ConfiguratorWindow::setManufacturerEnabled(bool value)
{
    ui->lineEditManufacturer->setReadOnly(!value);
}

//
void ConfiguratorWindow::setMaxPowerEnabled(bool value)
{
    ui->lineEditMaxPower->setReadOnly(!value);
    ui->lineEditMaxPowerHex->setReadOnly(!value);
}

//
void ConfiguratorWindow::setPinConfigEnabled(bool value)
{
    ui->comboBoxGPIO0->setEnabled(value);
    ui->comboBoxGPIO1->setEnabled(value);
    ui->comboBoxGPIO2->setEnabled(value);
    ui->comboBoxGPIO3->setEnabled(value);
    ui->comboBoxGPIO4->setEnabled(value);
    ui->comboBoxGPIO5->setEnabled(value);
    ui->comboBoxGPIO6->setEnabled(value);
    ui->comboBoxGPIO7->setEnabled(value);
    ui->comboBoxGPIO8->setEnabled(value);
    ui->comboBoxGPIO9->setEnabled(value);
    ui->comboBoxGPIO10->setEnabled(value);
    ui->spinBoxDivider->setReadOnly(!value);
    ui->lineEditSuspendLevel->setReadOnly(!value);
    ui->lineEditSuspendMode->setReadOnly(!value);
    ui->lineEditResumeMask->setReadOnly(!value);
    ui->lineEditResumeMatch->setReadOnly(!value);
}

//
void ConfiguratorWindow::setPIDEnabled(bool value)
{
    ui->lineEditPID->setReadOnly(!value);
}

//
void ConfiguratorWindow::setPowerModeEnabled(bool value)
{
    ui->comboBoxPowerMode->setEnabled(value);
}

//
void ConfiguratorWindow::setProductEnabled(bool value)
{
    ui->lineEditProduct->setReadOnly(!value);
}

//
void ConfiguratorWindow::setReleaseEnabled(bool value)
{
    ui->spinBoxMajVersion->setReadOnly(!value);
    ui->spinBoxMinVersion->setReadOnly(!value);
}

//
void ConfiguratorWindow::setSerialEnabled(bool value)
{
    ui->lineEditSerial->setReadOnly(!value);
}

//
void ConfiguratorWindow::setTransferPrioEnabled(bool value)
{
    ui->comboBoxTransferPrio->setEnabled(value);
}

//
void ConfiguratorWindow::setVIDEnabled(bool value)
{
    ui->lineEditVID->setReadOnly(!value);
}

//
void ConfiguratorWindow::setWriteEnabled(bool value)
{
    ui->pushButtonRevert->setEnabled(value);
    ui->checkBoxVerify->setEnabled(value);
    ui->checkBoxLock->setEnabled(value);
    ui->pushButtonWrite->setEnabled(value);
}

// Checks user input, returning true if it is valid, or false otherwise, while also highlighting invalid fields
bool ConfiguratorWindow::showInvalidInput()
{
    bool retval = false;
    if (ui->lineEditMaxPower->text().isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditMaxPowerHex->text().isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendLevel->text().size() < 4) {
        ui->lineEditSuspendLevel->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendMode->text().size() < 4) {
        ui->lineEditSuspendMode->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMask->text().size() < 4) {
        ui->lineEditResumeMask->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMatch->text().size() < 4) {
        ui->lineEditResumeMatch->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    return retval;
}
