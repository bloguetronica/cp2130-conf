/* CP2130 Configurator - Version 1.3.2 for Debian Linux
   Copyright (c) 2021-2025 Samuel Lourenço

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
#include <cstring>
#include <QDir>
#include <QFileDialog>
#include <QIODevice>
#include <QMessageBox>
#include <QMetaObject>
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include "common.h"
#include "configurationreader.h"
#include "configurationwriter.h"
#include "cp2130limits.h"
#include "nonblocking.h"
#include "serialgeneratordialog.h"
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

// Definitions
const int ENUM_RETRIES = 10;  // Number of enumeration retries

// The following values are applicable to displayConfiguration() (implemented in version 1.3.2)
const bool FULL_UPDATE = true;
const bool PARTIAL_UPDATE = false;

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

// Checks if the device window is currently fully enabled (implemented in version 2.0)
bool ConfiguratorWindow::isViewEnabled()
{
    return viewEnabled_;
}

// Opens the device and prepares the corresponding window
void ConfiguratorWindow::openDevice(quint16 vid, quint16 pid, const QString &serialstr)
{
    int err = cp2130_.open(vid, pid, serialstr);
    if (err == CP2130::SUCCESS) {  // Device was successfully opened
        vid_ = vid;  // Pass VID
        pid_ = pid;  // and PID
        serialString_ = serialstr;  // and the serial number as well
        readDeviceConfiguration();
        this->setWindowTitle(tr("CP2130 Device (S/N: %1)").arg(serialString_));
        displayConfiguration(deviceConfiguration_, FULL_UPDATE);  // Modified in versions 3.0 and 1.3.2
        viewEnabled_ = true;
    } else if (err == CP2130::ERROR_INIT) {  // Failed to initialize libusb
        QMessageBox::critical(this, tr("Critical Error"), tr("Could not initialize libusb.\n\nThis is a critical error and execution will be aborted."));
        exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
    } else {
        if (err == CP2130::ERROR_NOT_FOUND) {  // Failed to find device
            QMessageBox::critical(this, tr("Error"), tr("Could not find device."));
        } else if (err == CP2130::ERROR_BUSY) {  // Failed to claim interface
            QMessageBox::critical(this, tr("Error"), tr("Device is currently unavailable.\n\nPlease confirm that the device is not in use."));
        }
        this->deleteLater();  // Close window after the subsequent show() call
    }
}

// Locks the CP2130 OTP ROM, preventing further changes
void ConfiguratorWindow::lockOTP()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.lockOTP(errcnt, errstr);
    validateOperation(tr("lock OTP ROM"), errcnt, errstr);
    requiresReset_ = true;
}

void ConfiguratorWindow::on_actionAbout_triggered()
{
    showAboutDialog();  // Implemented in "common.h" and "common.cpp" since version 2.0
}

void ConfiguratorWindow::on_actionInformation_triggered()
{
    if (informationDialog_.isNull()) {  // If the dialog is not open (implemented in version 2.0, because the device information dialog is now modeless)
        err_ = false;  // Bug fix implemented in version 3.0
        int errcnt = 0;
        QString errstr;
        CP2130::SiliconVersion siversion = cp2130_.getSiliconVersion(errcnt, errstr);
        validateOperation(tr("retrieve device information"), errcnt, errstr);
        if (err_) {  // Fix implemented in version 1.2
            handleError();
        } else {  // If error check passes
            informationDialog_ = new InformationDialog(this);  // The dialog is no longer modal (version 2.0 feature);
            informationDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve information about the device if the window is opened again
            informationDialog_->setWindowTitle(tr("Device Information (S/N: %1)").arg(serialString_));
            informationDialog_->setSiliconVersionValueLabelText(siversion.maj, siversion.min);
            informationDialog_->show();
        }
    } else {
        informationDialog_->showNormal();  // Required if the dialog is minimized
        informationDialog_->activateWindow();  // Set focus on the previous dialog (dialog is raised and selected)
    }
}

// Implemented in version 3.0
void ConfiguratorWindow::on_actionLoadConfiguration_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Configuration from File"), xmlFilePath, tr("XML files (*.xml);;All files (*)"));  // Modified in version 1.3.2
    if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(fileName)));
        } else {
            loadConfigurationFromFile(file);
            file.close();
            xmlFilePath = fileName;
        }
    }
}

// Implemented in version 3.0
void ConfiguratorWindow::on_actionOTPROMViewer_triggered()
{
    if (otpromViewerDialog_.isNull()) {  // If the dialog is not open
        err_ = false;
        int errcnt = 0;
        QString errstr;
        CP2130::PROMConfig promConfig = cp2130_.getPROMConfig(errcnt, errstr);
        validateOperation(tr("retrieve PROM configuration"), errcnt, errstr);
        if (err_) {
            handleError();
        } else {  // If error check passes
            otpromViewerDialog_ = new OTPROMViewerDialog(this);
            otpromViewerDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve the PROM configuration if the window is opened again
            otpromViewerDialog_->setWindowTitle(tr("OTP ROM Viewer (S/N: %1)").arg(serialString_));
            otpromViewerDialog_->setOTPROMViewPlainText(promConfig);
            otpromViewerDialog_->show();
        }
        CP2130::listDevices(vid_, pid_, errcnt, errstr);  // This is a workaround to prevent an issue where the device is mislisted after retrieving its PROM configuration
    } else {
        otpromViewerDialog_->showNormal();  // Required if the dialog is minimized
        otpromViewerDialog_->activateWindow();  // Set focus on the previous dialog (dialog is raised and selected)
    }
}

// Implemented in version 3.0
void ConfiguratorWindow::on_actionSaveConfiguration_triggered()
{
    if(showInvalidInput()) {
        QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
    } else {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Configuration to File"), xmlFilePath, tr("XML files (*.xml);;All files (*)"));  // Modified in version 1.3.2
        if (!fileName.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(fileName)));
            } else {
                saveConfigurationToFile(file);
                file.close();
                xmlFilePath = fileName;
            }
        }
    }
}

// Implemented in version 3.0
void ConfiguratorWindow::on_actionSerialGeneratorEnable_toggled(bool checked)
{
    ui->pushButtonGenerateSerial->setEnabled(checked);
}

// Implemented in version 3.0
void ConfiguratorWindow::on_actionSerialGeneratorSettings_triggered()
{
    SerialGeneratorDialog serialGeneratorDialog(this);
    serialGeneratorDialog.setPrototypeSerialLineEditText(serialGeneratorSettings_.serialgen.prototypeSerial());
    serialGeneratorDialog.setDigitsCheckBox(serialGeneratorSettings_.serialgen.replaceWithDigits());
    serialGeneratorDialog.setUppercaseCheckBox(serialGeneratorSettings_.serialgen.replaceWithUppercaseLetters());
    serialGeneratorDialog.setLowercaseCheckBox(serialGeneratorSettings_.serialgen.replaceWithLowercaseLetters());
    serialGeneratorDialog.setExportToFileCheckBox(serialGeneratorSettings_.doexport);
    serialGeneratorDialog.setEnableCheckBox(serialGeneratorSettings_.genenable);
    serialGeneratorDialog.setAutoGenerateCheckBox(serialGeneratorSettings_.autogen);
    if (serialGeneratorDialog.exec() == QDialog::Accepted) {  // If the user clicks "OK"
        QString prototype = serialGeneratorDialog.prototypeSerialLineEditText();
        bool digit = serialGeneratorDialog.digitsCheckBoxIsChecked();
        bool upper = serialGeneratorDialog.uppercaseCheckBoxIsChecked();
        bool lower = serialGeneratorDialog.lowercaseCheckBoxIsChecked();
        if (!SerialGenerator::isValidPrototypeSerial(prototype) || !SerialGenerator::isValidReplaceMode(digit, upper, lower)) {  // If the user entered invalid settings (i.e. the prototype serial number does not contain a wildcard character or no replacement option was selected)
            QMessageBox::critical(this, tr("Error"), tr("The serial number generator settings are not valid and will not be applied.\n\nPlease verify that the prototype serial number contains at least one wildcard character (?) and that at least one replacement option is selected."));
        } else {  // Valid settings
            serialGeneratorSettings_.serialgen.setPrototypeSerial(prototype);
            serialGeneratorSettings_.serialgen.setReplaceMode(digit, upper, lower);
            serialGeneratorSettings_.doexport = serialGeneratorDialog.exportToFileCheckBoxIsChecked();
            serialGeneratorSettings_.genenable = serialGeneratorDialog.enableCheckBoxIsChecked();  // No further verification required, because "checkBoxEnable" is automatically unchecked if "checkBoxExportToFile" gets unchecked
            serialGeneratorSettings_.autogen = serialGeneratorDialog.autoGenerateCheckBoxIsChecked();  // Same as above, because "checkBoxAutoGenerate" is automatically unchecked if "checkBoxExportToFile" gets unchecked
        }
    }
}

// Implemented in version 1.6
void ConfiguratorWindow::on_lineEditManufacturer_textEdited()
{
    int curPosition = ui->lineEditManufacturer->cursorPosition();
    ui->lineEditManufacturer->setText(ui->lineEditManufacturer->text().replace('\n', ' '));
    ui->lineEditManufacturer->setCursorPosition(curPosition);
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
    if (maxPower > 2 * CP2130Limits::MAXPOW_MAX) {  // Modified in version 3.1
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
    int curPosition = ui->lineEditMaxPowerHex->cursorPosition();
    ui->lineEditMaxPowerHex->setText(ui->lineEditMaxPowerHex->text().toLower());
    int maxPowerHex = ui->lineEditMaxPowerHex->text().toInt(nullptr, 16);  // Modified in version 3.1
    if (maxPowerHex > CP2130Limits::MAXPOW_MAX) {  // Modified in version 3.1
        maxPowerHex = CP2130Limits::MAXPOW_MAX;
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(CP2130Limits::MAXPOW_MAX, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
    ui->lineEditMaxPowerHex->setCursorPosition(curPosition);
    ui->lineEditMaxPower->setText(QString::number(2 * maxPowerHex));
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
    int curPosition = ui->lineEditPID->cursorPosition();
    ui->lineEditPID->setText(ui->lineEditPID->text().toLower());
    ui->lineEditPID->setCursorPosition(curPosition);
}

// Implemented in version 1.6
void ConfiguratorWindow::on_lineEditProduct_textEdited()
{
    int curPosition = ui->lineEditProduct->cursorPosition();
    ui->lineEditProduct->setText(ui->lineEditProduct->text().replace('\n', ' '));
    ui->lineEditProduct->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditResumeMask_textChanged()
{
    if (ui->lineEditResumeMask->text().size() < 4 || ui->lineEditResumeMask->text().toInt(nullptr, 16) > CP2130Limits::WKUPMASK) {  // Extra condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditResumeMask->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditResumeMask->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditResumeMask_textEdited()
{
    int curPosition = ui->lineEditResumeMask->cursorPosition();
    ui->lineEditResumeMask->setText(ui->lineEditResumeMask->text().toLower());
    ui->lineEditResumeMask->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditResumeMatch_textChanged()
{
    if (ui->lineEditResumeMatch->text().size() < 4 || ui->lineEditResumeMatch->text().toInt(nullptr, 16) > CP2130Limits::WKUPMATCH) {  // Extra condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditResumeMatch->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditResumeMatch->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditResumeMatch_textEdited()
{
    int curPosition = ui->lineEditResumeMatch->cursorPosition();
    ui->lineEditResumeMatch->setText(ui->lineEditResumeMatch->text().toLower());
    ui->lineEditResumeMatch->setCursorPosition(curPosition);
}

// Implemented in version 3.0
void ConfiguratorWindow::on_lineEditSerial_textChanged()
{
    if (ui->lineEditSerial->text().isEmpty()) {
        ui->lineEditSerial->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditSerial->setStyleSheet("");
    }
}

// Implemented in version 1.6
void ConfiguratorWindow::on_lineEditSerial_textEdited()
{
    int curPosition = ui->lineEditSerial->cursorPosition();
    ui->lineEditSerial->setText(ui->lineEditSerial->text().replace('\n', ' '));
    ui->lineEditSerial->setCursorPosition(curPosition);
}

void ConfiguratorWindow::on_lineEditSuspendLevel_textChanged()
{
    if (ui->lineEditSuspendLevel->text().size() < 4 || ui->lineEditSuspendLevel->text().toInt(nullptr, 16) > CP2130Limits::SSPNDLVL_MAX) {  // Extra condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditSuspendLevel->setStyleSheet("background: rgb(255, 204, 0);");
    } else {
        ui->lineEditSuspendLevel->setStyleSheet("");
    }
}

void ConfiguratorWindow::on_lineEditSuspendLevel_textEdited()
{
    int curPosition = ui->lineEditSuspendLevel->cursorPosition();
    ui->lineEditSuspendLevel->setText(ui->lineEditSuspendLevel->text().toLower());
    ui->lineEditSuspendLevel->setCursorPosition(curPosition);
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
    int curPosition = ui->lineEditSuspendMode->cursorPosition();
    ui->lineEditSuspendMode->setText(ui->lineEditSuspendMode->text().toLower());
    ui->lineEditSuspendMode->setCursorPosition(curPosition);
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
    int curPosition = ui->lineEditVID->cursorPosition();
    ui->lineEditVID->setText(ui->lineEditVID->text().toLower());
    ui->lineEditVID->setCursorPosition(curPosition);
}

// Implemented in version 3.0
void ConfiguratorWindow::on_pushButtonGenerateSerial_clicked()
{
    ui->lineEditSerial->setText(serialGeneratorSettings_.serialgen.generateSerial());
}

void ConfiguratorWindow::on_pushButtonRevert_clicked()
{
    displayConfiguration(deviceConfiguration_, PARTIAL_UPDATE);  // Since version 3.0 and for efficiency purposes, this action will only revert unlocked fields
}

void ConfiguratorWindow::on_pushButtonWrite_clicked()
{
    if (cp2130_.isOpen()) {  // It is important to check if the device is open, since resetDevice() is non-blocking (a device reset could still be underway)
        if(showInvalidInput()) {
            QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
        } else {
            getEditedConfiguration();
            if (editedConfiguration_ == deviceConfiguration_ && !ui->checkBoxLock->isChecked()) {
                QMessageBox::information(this, tr("No Changes Done"), tr("No changes were effected, because no values were modified."));
            } else {
                int qmret = QMessageBox::question(this, tr("Write Configuration?"), tr("This will write the changes to the OTP ROM of your device. These changes will be permanent.\n\nDo you wish to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (qmret == QMessageBox::Yes) {
                    configureDevice();
                }
            }
        }
    }
}

// Verifies the CP2130 configuration against the input configuration
void ConfiguratorWindow::verifyConfiguration()
{
    resetDevice();  // Since version 1.2, resetDevice() uses err_ and errmsg_ to signal out non-critical errors and pass the corresponding messages (see function implementation below)
    if (!err_ && deviceConfiguration_ != editedConfiguration_) {  // Condition added to prevent the message to be overwritten, which is applicable to a situation when resetDevice() fails
        err_ = true;
        errmsg_ = tr("Failed verification.");
    }
    requiresReset_ = false;
}

// Writes the manufacturer descriptor to the CP2130 OTP ROM
void ConfiguratorWindow::writeManufacturerDesc()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeManufacturerDesc(editedConfiguration_.manufacturer, errcnt, errstr);
    validateOperation(tr("write manufacturer descriptor"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the maximum power consumption configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writeMaxPower()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWMAXPOW), errcnt, errstr);
    validateOperation(tr("write maximum power"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the PID to the CP2130 OTP ROM
void ConfiguratorWindow::writePID()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWPID), errcnt, errstr);
    validateOperation(tr("write PID"), errcnt, errstr);
    if (!err_) {
        pid_ = editedConfiguration_.usbConfig.pid;  // If the previous operation was successful, it is safe to assume that the PID changed to the new value
    }
    requiresReset_ = true;
}

// Writes the pin configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writePinConfig()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writePinConfig(editedConfiguration_.pinConfig, errcnt, errstr);
    validateOperation(tr("write pin configuration"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the power mode configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writePowerMode()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWPOWMODE), errcnt, errstr);
    validateOperation(tr("write power mode"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the product descriptor to the CP2130 OTP ROM
void ConfiguratorWindow::writeProductDesc()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeProductDesc(editedConfiguration_.product, errcnt, errstr);
    validateOperation(tr("write product descriptor"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the release version to the CP2130 OTP ROM
void ConfiguratorWindow::writeReleaseVersion()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWREL), errcnt, errstr);
    validateOperation(tr("write release version"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the serial descriptor to the CP2130 OTP ROM
void ConfiguratorWindow::writeSerialDesc()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeSerialDesc(editedConfiguration_.serial, errcnt, errstr);
    validateOperation(tr("write serial descriptor"), errcnt, errstr);
    if (!err_) {
        serialString_ = editedConfiguration_.serial.toLatin1();  // If the previous operation was successful, it is safe to assume that the serial string changed to the new value (the conversion to ASCII was implemented in version 1.1 as a patch)
    }
    requiresReset_ = true;
}

// Writes the transfer priority configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writeTransferPrio()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWTRFPRIO), errcnt, errstr);
    validateOperation(tr("write transfer priority"), errcnt, errstr);
    requiresReset_ = true;
}

// Writes the VID to the CP2130 OTP ROM
void ConfiguratorWindow::writeVID()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfiguration_.usbConfig, static_cast<quint8>(CP2130::LWVID), errcnt, errstr);
    validateOperation(tr("write VID"), errcnt, errstr);
    if (!err_) {
        vid_ = editedConfiguration_.usbConfig.vid;  // If the previous operation was successful, it is safe to assume that the VID changed to the new value
    }
    requiresReset_ = true;
}

// This is the main configuration routine, used to configure the CP2130 OTP ROM according to the tasks in the task list
void ConfiguratorWindow::configureDevice()
{
    err_ = false;
    requiresReset_ = false;
    QStringList tasks = prepareTaskList();  // Create a new task list
    int nTasks = tasks.size();
    QProgressDialog configProgress(tr("Configuring device..."), tr("Abort"), 0, nTasks, this);
    configProgress.setWindowTitle(tr("Device Configuration"));  // Added in version 1.5
    configProgress.setWindowModality(Qt::WindowModal);
    configProgress.setMinimumDuration(0);  // The progress dialog needs to be displayed right away, since resetDevice() is non-blocking
    configProgress.setValue(0);  // This, along with setMinimumDuration(), will cause the progress dialog to display immediately
    for (int i = 0; i < nTasks; ++i) {  // Iterate through the newly created task list (refactored in version 1.5)
        if (configProgress.wasCanceled()) {  // If the user clicks "Abort"
            break;  // Abort the configuration
        }
        QMetaObject::invokeMethod(this, tasks[i].toStdString().c_str());  // The task list entry is converted to a C string
        if (err_) {  // If an error has occured
            configProgress.cancel();  // This hides the progress dialog (fix implemented in version 1.1)
            break;  // Abort the configuration
        }
        configProgress.setValue(i + 1);  // Update the progress bar for each task done
    }
    if (err_) {  // If an error occured (refactored in version 1.5)
        handleError();
        QMessageBox::critical(this, tr("Error"), tr("The device configuration could not be completed."));
    } else if (configProgress.wasCanceled()) {  // If the device configuration was aborted by the user
        QMessageBox::information(this, tr("Configuration Aborted"), tr("The device configuration was aborted."));
    } else if (tasks.contains("verifyConfiguration")) {  // Successul configuration with verification (condition modified in version 3.1)
        QMessageBox::information(this, tr("Device Configured"), tr("Device was successfully configured and verified."));
    } else {  // Successul configuration without verification
        QMessageBox::information(this, tr("Device Configured"), tr("Device was successfully configured."));
    }
    if (!cp2130_.disconnected() && requiresReset_) {
        QProgressDialog resetProgress(tr("Resetting device..."), tr("Cancel"), 0, 1, this);
        resetProgress.setWindowTitle(tr("Device Reset"));  // Added in version 1.5
        resetProgress.setWindowModality(Qt::WindowModal);
        resetProgress.setMinimumDuration(0);
        resetProgress.setValue(0);  // As before, the progress dialog should appear immediately
        if (!resetProgress.wasCanceled()) {
            resetDevice();
            if (err_) {
                resetProgress.cancel();  // Hide the progress dialog
                handleError();  // Since version 1.2, resetDevice() requires non-critical errors to be handled externally (see function implementation below)
            } else {
                resetProgress.setValue(1);
            }
        }
    }
}

// Partially disables configurator window (fixed in version 2.1 and expanded in version 3.0)
void ConfiguratorWindow::disableView()
{
    ui->actionInformation->setEnabled(false);
    ui->actionLoadConfiguration->setEnabled(false);  // Added in version 3.0
    ui->actionClose->setText(tr("&Close Window"));  // Implemented in version 2.0, to hint the user that the device is effectively closed and only its window remains open
    ui->actionOTPROMViewer->setEnabled(false);  // Added in version 3.0
    ui->actionSerialGeneratorEnable->setChecked(false);  // Added in version 3.0 (this also disables pushButtonGenerateSerial)
    ui->actionSerialGeneratorEnable->setEnabled(false);  // Added in version 3.0
    ui->centralWidget->setEnabled(false);
    viewEnabled_ = false;
}

// This is the main display routine, used to display the given configuration, updating some or all fields accordingly (expanded in version 3.0, in order to implement partial or full updates)
void ConfiguratorWindow::displayConfiguration(const Configuration &configuration, bool fullUpdate)
{
    if (fullUpdate || (CP2130::LWMANUF & lockWord_) == CP2130::LWMANUF) {
        displayManufacturer(configuration.manufacturer);
    }
    if (fullUpdate || (CP2130::LWPROD & lockWord_) == CP2130::LWPROD) {
        displayProduct(configuration.product);
    }
    if (fullUpdate || (CP2130::LWSER & lockWord_) == CP2130::LWSER) {
        displaySerial(configuration.serial);
    }
    if (fullUpdate || (CP2130::LWVID & lockWord_) == CP2130::LWVID) {
        displayVID(configuration.usbConfig.vid);
    }
    if (fullUpdate || (CP2130::LWPID & lockWord_) == CP2130::LWPID) {
        displayPID(configuration.usbConfig.pid);
    }
    if (fullUpdate || (CP2130::LWREL & lockWord_) == CP2130::LWREL) {
        displayReleaseVersion(configuration.usbConfig.majrel, configuration.usbConfig.minrel);
    }
    if (fullUpdate || (CP2130::LWMAXPOW & lockWord_) == CP2130::LWMAXPOW) {
        displayMaxPower(configuration.usbConfig.maxpow);
    }
    if (fullUpdate || (CP2130::LWPOWMODE & lockWord_) == CP2130::LWPOWMODE) {
        displayPowerMode(configuration.usbConfig.powmode);
    }
    if (fullUpdate || (CP2130::LWTRFPRIO & lockWord_) == CP2130::LWTRFPRIO) {
        displayTransferPrio(configuration.usbConfig.trfprio);
    }
    if (fullUpdate || (CP2130::LWPINCFG & lockWord_) == CP2130::LWPINCFG) {
        displayPinConfig(configuration.pinConfig);
    }
    if (fullUpdate) {
        setManufacturerEnabled((CP2130::LWMANUF & lockWord_) == CP2130::LWMANUF);
        setProductEnabled((CP2130::LWPROD & lockWord_) == CP2130::LWPROD);
        setSerialEnabled((CP2130::LWSER & lockWord_) == CP2130::LWSER);  // Since version 3.0, this also enables or disables the related "Tools > Serial Number Generator > Enable" menu action
        setVIDEnabled((CP2130::LWVID & lockWord_) == CP2130::LWVID);
        setPIDEnabled((CP2130::LWPID & lockWord_) == CP2130::LWPID);
        setReleaseEnabled((CP2130::LWREL & lockWord_) == CP2130::LWREL);
        setMaxPowerEnabled((CP2130::LWMAXPOW & lockWord_) == CP2130::LWMAXPOW);
        setPowerModeEnabled((CP2130::LWPOWMODE & lockWord_) == CP2130::LWPOWMODE);
        setTransferPrioEnabled((CP2130::LWTRFPRIO & lockWord_) == CP2130::LWTRFPRIO);
        setPinConfigEnabled((CP2130::LWPINCFG & lockWord_) == CP2130::LWPINCFG);
        setWriteEnabled((CP2130::LWALL & lockWord_) != 0x0000);  // Since version 3.0, this also enables or disables the "Device > Load Configuration..." menu action
    }
}

// Updates the manufacturer descriptor field
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

// Updates the maximum power consumption field (implemented in version 3.0)
void ConfiguratorWindow::displayMaxPower(quint8 maxpow)
{
    ui->lineEditMaxPower->setText(QString::number(2 * maxpow));
    ui->lineEditMaxPowerHex->setText(QString("%1").arg(maxpow, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
}

// Updates the product ID field (implemented in version 3.0)
void ConfiguratorWindow::displayPID(quint16 pid)
{
    ui->lineEditPID->setText(QString("%1").arg(pid, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
}

// Updates all fields pertaining to the CP2130 pin configuration
void ConfiguratorWindow::displayPinConfig(const CP2130::PinConfig &pinConfig)
{
    ui->comboBoxGPIO0->setCurrentIndex(pinConfig.gpio0);
    ui->comboBoxGPIO1->setCurrentIndex(pinConfig.gpio1);
    ui->comboBoxGPIO2->setCurrentIndex(pinConfig.gpio2);
    ui->comboBoxGPIO3->setCurrentIndex(pinConfig.gpio3);
    ui->comboBoxGPIO4->setCurrentIndex(pinConfig.gpio4);
    ui->comboBoxGPIO5->setCurrentIndex(pinConfig.gpio5);
    ui->comboBoxGPIO6->setCurrentIndex(pinConfig.gpio6);
    ui->comboBoxGPIO7->setCurrentIndex(pinConfig.gpio7);
    ui->comboBoxGPIO8->setCurrentIndex(pinConfig.gpio8);
    ui->comboBoxGPIO9->setCurrentIndex(pinConfig.gpio9);
    ui->comboBoxGPIO10->setCurrentIndex(pinConfig.gpio10);
    ui->spinBoxDivider->setValue(pinConfig.divider);
    ui->lineEditSuspendLevel->setText(QString("%1").arg(pinConfig.sspndlvl, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
    ui->lineEditSuspendMode->setText(QString("%1").arg(pinConfig.sspndmode, 4, 16, QChar('0')));  // Same as above
    ui->lineEditResumeMask->setText(QString("%1").arg(pinConfig.wkupmask, 4, 16, QChar('0')));  // Same as above
    ui->lineEditResumeMatch->setText(QString("%1").arg(pinConfig.wkupmatch, 4, 16, QChar('0')));  // Same as above
}

// Updates the power mode combo box (implemented in version 3.0)
void ConfiguratorWindow::displayPowerMode(quint8 powmode)
{
    ui->comboBoxPowerMode->setCurrentIndex(powmode);
}

// Updates the product descriptor field
void ConfiguratorWindow::displayProduct(const QString &product)
{
    ui->lineEditProduct->setText(product);
}

// Updates the release version fields (implemented in version 3.0)
void ConfiguratorWindow::displayReleaseVersion(quint8 majrel, quint8 minrel)
{
    ui->spinBoxMajVersion->setValue(majrel);
    ui->spinBoxMinVersion->setValue(minrel);
}

// Updates the serial descriptor field
void ConfiguratorWindow::displaySerial(const QString &serial)
{
    ui->lineEditSerial->setText(serial);
}

// Updates the transfer priority combo box (implemented in version 3.0)
void ConfiguratorWindow::displayTransferPrio(quint8 trfprio)
{
    ui->comboBoxTransferPrio->setCurrentIndex(trfprio);
}

// Updates the vendor ID field (implemented in version 3.0)
void ConfiguratorWindow::displayVID(quint16 vid)
{
    ui->lineEditVID->setText(QString("%1").arg(vid, 4, 16, QChar('0')));  // This will autofill with up to four leading zeros
}

// Retrieves the user set configuration from the fields
void ConfiguratorWindow::getEditedConfiguration()
{
    editedConfiguration_.manufacturer = ui->lineEditManufacturer->text();
    editedConfiguration_.product = ui->lineEditProduct->text();
    editedConfiguration_.serial = ui->lineEditSerial->text();
    editedConfiguration_.usbConfig.vid = static_cast<quint16>(ui->lineEditVID->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.usbConfig.pid = static_cast<quint16>(ui->lineEditPID->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.usbConfig.majrel = static_cast<quint8>(ui->spinBoxMajVersion->value());
    editedConfiguration_.usbConfig.minrel = static_cast<quint8>(ui->spinBoxMinVersion->value());
    editedConfiguration_.usbConfig.maxpow = static_cast<quint8>(ui->lineEditMaxPowerHex->text().toUShort(nullptr, 16));  // Optimized in version 3.1
    editedConfiguration_.usbConfig.powmode = static_cast<quint8>(ui->comboBoxPowerMode->currentIndex());
    editedConfiguration_.usbConfig.trfprio = static_cast<quint8>(ui->comboBoxTransferPrio->currentIndex());
    editedConfiguration_.pinConfig.gpio0 = static_cast<quint8>(ui->comboBoxGPIO0->currentIndex());
    editedConfiguration_.pinConfig.gpio1 = static_cast<quint8>(ui->comboBoxGPIO1->currentIndex());
    editedConfiguration_.pinConfig.gpio2 = static_cast<quint8>(ui->comboBoxGPIO2->currentIndex());
    editedConfiguration_.pinConfig.gpio3 = static_cast<quint8>(ui->comboBoxGPIO3->currentIndex());
    editedConfiguration_.pinConfig.gpio4 = static_cast<quint8>(ui->comboBoxGPIO4->currentIndex());
    editedConfiguration_.pinConfig.gpio5 = static_cast<quint8>(ui->comboBoxGPIO5->currentIndex());
    editedConfiguration_.pinConfig.gpio6 = static_cast<quint8>(ui->comboBoxGPIO6->currentIndex());
    editedConfiguration_.pinConfig.gpio7 = static_cast<quint8>(ui->comboBoxGPIO7->currentIndex());
    editedConfiguration_.pinConfig.gpio8 = static_cast<quint8>(ui->comboBoxGPIO8->currentIndex());
    editedConfiguration_.pinConfig.gpio9 = static_cast<quint8>(ui->comboBoxGPIO9->currentIndex());
    editedConfiguration_.pinConfig.gpio10 = static_cast<quint8>(ui->comboBoxGPIO10->currentIndex());
    editedConfiguration_.pinConfig.sspndlvl = static_cast<quint16>(ui->lineEditSuspendLevel->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.pinConfig.sspndmode = static_cast<quint16>(ui->lineEditSuspendMode->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.pinConfig.wkupmask = static_cast<quint16>(ui->lineEditResumeMask->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.pinConfig.wkupmatch = static_cast<quint16>(ui->lineEditResumeMatch->text().toUShort(nullptr, 16));  // Conversion done for sanity purposes since version 3.1
    editedConfiguration_.pinConfig.divider = static_cast<quint8>(ui->spinBoxDivider->value());
}

// Determines the type of error and acts accordingly, always showing a message
void ConfiguratorWindow::handleError()
{
    if (cp2130_.disconnected() || !cp2130_.isOpen()) {
        disableView();  // Disable configurator window (since version 1.4, this is done before showing the error message)
        cp2130_.close();  // If the device is already closed, this will have no effect
    }
    QMessageBox::critical(this, tr("Error"), errmsg_);
}

// Loads the configuration from a given file (implemented in version 3.0)
void ConfiguratorWindow::loadConfigurationFromFile(QFile &file)
{
    getEditedConfiguration();
    SerialGeneratorSettings serialGenSettings = serialGeneratorSettings_;  // Local variable required to hold serial generator settings that may or may not be applied
    ConfigurationReader configReader(editedConfiguration_, serialGenSettings);  // It is essential to work on the local variable set above!
    if (!configReader.readFrom(&file)) {
        QMessageBox::critical(this, tr("Error"), configReader.errorString());
    } else {
        displayConfiguration(editedConfiguration_, PARTIAL_UPDATE);  // This partial update will not modify any fields that are locked
        serialGeneratorSettings_ = serialGenSettings;  // Apply serial generator settings
        if ((CP2130::LWSER & lockWord_) == CP2130::LWSER) {
            ui->actionSerialGeneratorEnable->setChecked(serialGeneratorSettings_.genenable);  // This also enables or disables pushButtonGenerateSerial
            if (serialGeneratorSettings_.autogen) {
                ui->lineEditSerial->setText(serialGeneratorSettings_.serialgen.generateSerial());
            }
        }
    }
}

// Prepares the task list, by checking which fields changed, while also setting optional tasks according to the user's requirements
QStringList ConfiguratorWindow::prepareTaskList()
{
    QStringList tasks;
    if (editedConfiguration_.manufacturer != deviceConfiguration_.manufacturer) {
        tasks += "writeManufacturerDesc";
    }
    if (editedConfiguration_.product != deviceConfiguration_.product) {
        tasks += "writeProductDesc";
    }
    if (editedConfiguration_.serial != deviceConfiguration_.serial) {
        tasks += "writeSerialDesc";
    }
    if (editedConfiguration_.usbConfig.vid != deviceConfiguration_.usbConfig.vid) {
        tasks += "writeVID";
    }
    if (editedConfiguration_.usbConfig.pid != deviceConfiguration_.usbConfig.pid) {
        tasks += "writePID";
    }
    if (editedConfiguration_.usbConfig.majrel != deviceConfiguration_.usbConfig.majrel || editedConfiguration_.usbConfig.minrel != deviceConfiguration_.usbConfig.minrel) {
        tasks += "writeReleaseVersion";
    }
    if (editedConfiguration_.usbConfig.maxpow != deviceConfiguration_.usbConfig.maxpow) {
        tasks += "writeMaxPower";
    }
    if (editedConfiguration_.usbConfig.powmode != deviceConfiguration_.usbConfig.powmode) {
        tasks += "writePowerMode";
    }
    if (editedConfiguration_.usbConfig.trfprio != deviceConfiguration_.usbConfig.trfprio) {
        tasks += "writeTransferPrio";
    }
    if (editedConfiguration_.pinConfig != deviceConfiguration_.pinConfig) {
        tasks += "writePinConfig";
    }
    if (ui->checkBoxVerify->isChecked()) {
        tasks += "verifyConfiguration";
    }
    if (ui->checkBoxLock->isChecked()) {
        tasks += "lockOTP";
    }
    return tasks;
}

// This is the routine that reads the configuration from the CP2130 OTP ROM
void ConfiguratorWindow::readDeviceConfiguration()
{
    int errcnt = 0;
    QString errstr;
    deviceConfiguration_.manufacturer = cp2130_.getManufacturerDesc(errcnt, errstr);
    deviceConfiguration_.product = cp2130_.getProductDesc(errcnt, errstr);
    deviceConfiguration_.serial = cp2130_.getSerialDesc(errcnt, errstr);
    deviceConfiguration_.usbConfig = cp2130_.getUSBConfig(errcnt, errstr);
    deviceConfiguration_.pinConfig = cp2130_.getPinConfig(errcnt, errstr);
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

// Resets the device
void ConfiguratorWindow::resetDevice()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.reset(errcnt, errstr);
    cp2130_.close();  // Important! - This should be done always, even if the previous reset operation shows an error, because an error doesn't mean that a device reset was not effected
    int err;
    for (int i = 0; i < ENUM_RETRIES; ++i) {  // Verify enumeration according to the number of times set by "ENUM_RETRIES" [10]
        NonBlocking::msleep(500);  // Wait 500ms each time
        err = cp2130_.open(vid_, pid_, serialString_);
        if (err != CP2130::ERROR_NOT_FOUND) {  // Retry only if the device was not found yet (as it may take some time to enumerate)
            break;
        }
    }
    if (err == CP2130::SUCCESS) {  // Device was successfully reopened
        readDeviceConfiguration();
        this->setWindowTitle(tr("CP2130 Configurator (S/N: %1)").arg(serialString_));
        displayConfiguration(deviceConfiguration_, FULL_UPDATE);  // Modified in version 3.0 and 1.3.2
    } else if (err == CP2130::ERROR_INIT) {  // Failed to initialize libusb
        QMessageBox::critical(this, tr("Critical Error"), tr("Could not reinitialize libusb.\n\nThis is a critical error and execution will be aborted."));
        exit(EXIT_FAILURE);  // This error is critical because libusb failed to initialize
    } else if (err == CP2130::ERROR_NOT_FOUND) {  // Failed to find device
        err_ = true;
        errmsg_ = tr("Device disconnected.\n\nPlease reconnect it and try again.");  // Since version 1.2, the error message is not shown here
    } else if (err == CP2130::ERROR_BUSY) {  // Failed to claim interface
        err_ = true;
        errmsg_ = tr("Device ceased to be available. It could be in use by another application.");  // Same as above
    }
}

// Saves the current configuration to a given file (implemented in version 3.0)
void ConfiguratorWindow::saveConfigurationToFile(QFile &file)
{
    getEditedConfiguration();
    ConfigurationWriter configWriter(editedConfiguration_, serialGeneratorSettings_);
    configWriter.writeTo(&file);
}

// Enables or disables the manufacturer descriptor field
void ConfiguratorWindow::setManufacturerEnabled(bool value)
{
    ui->lineEditManufacturer->setReadOnly(!value);
}

// Enables or disables the maximum power consuption configuration fields
void ConfiguratorWindow::setMaxPowerEnabled(bool value)
{
    ui->lineEditMaxPower->setReadOnly(!value);
    ui->lineEditMaxPowerHex->setReadOnly(!value);
}

// Enables or disables all fields pertaining to pin configuration
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

// Enables or disables the PID field
void ConfiguratorWindow::setPIDEnabled(bool value)
{
    ui->lineEditPID->setReadOnly(!value);
}

// Enables or disables the power mode configuration field
void ConfiguratorWindow::setPowerModeEnabled(bool value)
{
    ui->comboBoxPowerMode->setEnabled(value);
}

// Enables or disables the product descriptor field
void ConfiguratorWindow::setProductEnabled(bool value)
{
    ui->lineEditProduct->setReadOnly(!value);
}

// Enables or disables the release version fields
void ConfiguratorWindow::setReleaseEnabled(bool value)
{
    ui->spinBoxMajVersion->setReadOnly(!value);
    ui->spinBoxMinVersion->setReadOnly(!value);
}

// Enables or disables the serial descriptor field and the related "Tools > Serial Number Generator > Enable" menu action (expanded in version 3.0)
void ConfiguratorWindow::setSerialEnabled(bool value)
{
    if (!value) {  // Implemented in version 3.0
        ui->actionSerialGeneratorEnable->setChecked(false);  // This also disables pushButtonGenerateSerial
    }
    ui->actionSerialGeneratorEnable->setEnabled(value);  // Added in version 3.0
    ui->lineEditSerial->setReadOnly(!value);
}

// Enables or disables the transfer priority configuration field
void ConfiguratorWindow::setTransferPrioEnabled(bool value)
{
    ui->comboBoxTransferPrio->setEnabled(value);
}

// Enables or disables the VID field
void ConfiguratorWindow::setVIDEnabled(bool value)
{
    ui->lineEditVID->setReadOnly(!value);
}

// Enables or disables editing related actions, buttons and checkboxes (expanded in version 3.0)
void ConfiguratorWindow::setWriteEnabled(bool value)
{
    ui->actionLoadConfiguration->setEnabled(value);  // Added in version 3.0
    ui->pushButtonRevert->setEnabled(value);
    ui->checkBoxVerify->setEnabled(value);
    ui->checkBoxLock->setEnabled(value);
    ui->pushButtonWrite->setEnabled(value);
}

// Checks user input, returning false if it is valid, or true otherwise, while also highlighting invalid fields (modified in version 3.1)
bool ConfiguratorWindow::showInvalidInput()
{
    bool retval = false;
    if (ui->lineEditSerial->text().isEmpty()) {  // Condition added in version 3.0
        ui->lineEditSerial->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditMaxPower->text().isEmpty()) {
        ui->lineEditMaxPower->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditMaxPowerHex->text().isEmpty()) {
        ui->lineEditMaxPowerHex->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendLevel->text().size() < 4 || ui->lineEditSuspendLevel->text().toInt(nullptr, 16) > CP2130Limits::SSPNDLVL_MAX) {  // Extra check condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditSuspendLevel->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendMode->text().size() < 4) {
        ui->lineEditSuspendMode->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMask->text().size() < 4 || ui->lineEditResumeMask->text().toInt(nullptr, 16) > CP2130Limits::WKUPMASK) {  // Extra check condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditResumeMask->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMatch->text().size() < 4 || ui->lineEditResumeMatch->text().toInt(nullptr, 16) > CP2130Limits::WKUPMATCH) {  // Extra check condition added in version 1.1 (and modified in version 3.1)
        ui->lineEditResumeMatch->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    return retval;
}

// Checks for errors and validates device operations
// Renamed and modified in version 1.3.2 (previous name was opCheck())
void ConfiguratorWindow::validateOperation(const QString &operation, int errcnt, QString errstr)  // The variable "errstr" is passed by value here, because it needs to be modified locally!
{
    if (errcnt > 0) {
        err_ = true;
        if (cp2130_.disconnected()) {
            errmsg_ = tr("Device disconnected.\n\nPlease reconnect it and try again.");
        } else {
            errstr.chop(1);  // Remove the last character, which is always a newline
            errmsg_ = tr("Failed to %1. The operation returned the following error(s):\n– %2", "", errcnt).arg(operation, errstr.replace("\n", "\n– "));
        }
    }
}
