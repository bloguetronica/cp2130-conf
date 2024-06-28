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
#include <cstring>
#include <QDir>
#include <QFileDialog>
#include <QIODevice>
#include <QMessageBox>
#include <QMetaObject>
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "common.h"
#include "nonblocking.h"
#include "serialgeneratordialog.h"
#include "configuratorwindow.h"
#include "ui_configuratorwindow.h"

// Definitions
const int ENUM_RETRIES = 10;  // Number of enumeration retries
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
        serialstr_ = serialstr;  // and the serial number as well
        readDeviceConfiguration();
        this->setWindowTitle(tr("CP2130 Device (S/N: %1)").arg(serialstr_));
        displayConfiguration(deviceConfig_);
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
    opCheck(tr("lock-otp-op"), errcnt, errstr);  // The string "lock-otp-op" should be translated to "Lock OTP ROM"
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
        opCheck(tr("device-information-retrieval-op"), errcnt, errstr);  // The string "device-information-retrieval-op" should be translated to "Device information retrieval"
        if (err_) {  // Fix implemented in version 1.2
            handleError();
        } else {  // If error check passes
            informationDialog_ = new InformationDialog(this);  // The dialog is no longer modal (version 2.0 feature);
            informationDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve information about the device if the window is opened again
            informationDialog_->setWindowTitle(tr("Device Information (S/N: %1)").arg(serialstr_));
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
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Configuration from File"), filepath, tr("XML files (*.xml);;All files (*)"));
    if (!filename.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not read from %1.\n\nPlease verify that you have read access to this file.").arg(QDir::toNativeSeparators(filename)));
        } else {
            loadConfigurationFromFile(file);
            file.close();
            filepath = filename;
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
        opCheck(tr("prom-configuration-retrieval-op"), errcnt, errstr);  // The string "prom-configuration-retrieval-op" should be translated to "PROM configuration retrieval"
        if (err_) {
            handleError();
        } else {  // If error check passes
            otpromViewerDialog_ = new OTPROMViewerDialog(this);
            otpromViewerDialog_->setAttribute(Qt::WA_DeleteOnClose);  // It is important to delete the dialog in memory once closed, in order to force the application to retrieve the PROM configuration if the window is opened again
            otpromViewerDialog_->setWindowTitle(tr("OTP ROM Viewer (S/N: %1)").arg(serialstr_));
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
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Configuration to File"), filepath, tr("XML files (*.xml);;All files (*)"));
        if (!filename.isEmpty()) {  // Note that the previous dialog will return an empty string if the user cancels it
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, tr("Error"), tr("Could not write to %1.\n\nPlease verify that you have write access to this file.").arg(QDir::toNativeSeparators(filename)));
            } else {
                saveConfigurationToFile(file);
                file.close();
                filepath = filename;
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
    serialGeneratorDialog.setPrototypeSerialLineEditText(serialgensetting_.serialgen.prototypeSerial());
    serialGeneratorDialog.setDigitsCheckBox(serialgensetting_.serialgen.replaceWithDigits());
    serialGeneratorDialog.setUppercaseCheckBox(serialgensetting_.serialgen.replaceWithUppercaseLetters());
    serialGeneratorDialog.setLowercaseCheckBox(serialgensetting_.serialgen.replaceWithLowercaseLetters());
    serialGeneratorDialog.setExportToFileCheckBox(serialgensetting_.doexport);
    serialGeneratorDialog.setEnableCheckBox(serialgensetting_.genenable);
    serialGeneratorDialog.setAutoGenerateCheckBox(serialgensetting_.autogen);
    if (serialGeneratorDialog.exec() == QDialog::Accepted) {  // If the user clicks "OK"
        QString prototype = serialGeneratorDialog.prototypeSerialLineEditText();
        bool digit = serialGeneratorDialog.digitsCheckBoxIsChecked();
        bool upper = serialGeneratorDialog.uppercaseCheckBoxIsChecked();
        bool lower = serialGeneratorDialog.lowercaseCheckBoxIsChecked();
        if (!SerialGenerator::prototypeSerialIsValid(prototype) || !SerialGenerator::replaceModeIsValid(digit, upper, lower)) {  // If the user entered invalid settings (i.e. the prototype serial number does not contain a wildcard character or no replacement option was selected)
            QMessageBox::critical(this, tr("Error"), tr("The serial generator settings are not valid and will not be applied.\n\nPlease verify that the prototype serial number contains at least one wildcard character (?) and that at least one replacement option is selected."));
        } else {  // Valid settings
            serialgensetting_.serialgen.setPrototypeSerial(prototype);
            serialgensetting_.serialgen.setReplaceMode(digit, upper, lower);
            serialgensetting_.doexport = serialGeneratorDialog.exportToFileCheckBoxIsChecked();
            serialgensetting_.genenable = serialGeneratorDialog.enableCheckBoxIsChecked();  // No further verification required, because "checkBoxEnable" is automatically unchecked if "checkBoxExportToFile" gets unchecked
            serialgensetting_.autogen = serialGeneratorDialog.autoGenerateCheckBoxIsChecked();  // Same as above, because "checkBoxAutoGenerate" is automatically unchecked if "checkBoxExportToFile" gets unchecked
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
    int curPosition = ui->lineEditMaxPowerHex->cursorPosition();
    ui->lineEditMaxPowerHex->setText(ui->lineEditMaxPowerHex->text().toLower());
    int maxPower = 2 * ui->lineEditMaxPowerHex->text().toInt(nullptr, 16);
    if (maxPower > POWER_LIMIT) {
        maxPower = POWER_LIMIT;
        ui->lineEditMaxPowerHex->setText(QString("%1").arg(POWER_LIMIT / 2, 2, 16, QChar('0')));  // This will autofill with up to two leading zeros
    }
    ui->lineEditMaxPowerHex->setCursorPosition(curPosition);
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
    if (ui->lineEditResumeMask->text().size() < 4 || ui->lineEditResumeMask->text().toInt(nullptr, 16) > 0x7fff) {  // Extra condition added in version 1.1
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
    if (ui->lineEditResumeMatch->text().size() < 4 || ui->lineEditResumeMatch->text().toInt(nullptr, 16) > 0x7fff) {  // Extra condition added in version 1.1
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
    if (ui->lineEditSuspendLevel->text().size() < 4 || ui->lineEditSuspendLevel->text().toInt(nullptr, 16) > 0x7fff) {  // Extra condition added in version 1.1
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
    ui->lineEditSerial->setText(serialgensetting_.serialgen.generateSerial());
}

void ConfiguratorWindow::on_pushButtonRevert_clicked()
{
    displayConfiguration(deviceConfig_);
}

void ConfiguratorWindow::on_pushButtonWrite_clicked()
{
    if (cp2130_.isOpen()) {  // It is important to check if the device is open, since resetDevice() is non-blocking (a device reset could still be underway)
        if(showInvalidInput()) {
            QMessageBox::critical(this, tr("Error"), tr("One or more fields have invalid information.\n\nPlease correct the information in the fields highlighted in red."));
        } else {
            getEditedConfiguration();
            if (editedConfig_ == deviceConfig_ && !ui->checkBoxLock->isChecked()) {
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
    if (!err_ && deviceConfig_ != editedConfig_) {  // Condition added to prevent the message to be overwritten, which is applicable to a situation when resetDevice() fails
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
    cp2130_.writeManufacturerDesc(editedConfig_.manufacturer, errcnt, errstr);
    opCheck(tr("write-manufacturer-desc-op"), errcnt, errstr);  // The string "write-manufacturer-desc-op" should be translated to "Write manufacturer descriptor"
    requiresReset_ = true;
}

// Writes the maximum power consumption configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writeMaxPower()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWMAXPOW), errcnt, errstr);
    opCheck(tr("write-max-power-op"), errcnt, errstr);  // The string "write-max-power-op" should be translated to "Write maximum power"
    requiresReset_ = true;
}

// Writes the PID to the CP2130 OTP ROM
void ConfiguratorWindow::writePID()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWPID), errcnt, errstr);
    opCheck(tr("write-pid-op"), errcnt, errstr);  // The string "write-pid-op" should be translated to "Write PID"
    if (!err_) {
        pid_ = editedConfig_.usbconfig.pid;  // If the previous operation was successful, it is safe to assume that the PID changed to the new value
    }
    requiresReset_ = true;
}

// Writes the pin configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writePinConfig()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writePinConfig(editedConfig_.pinconfig, errcnt, errstr);
    opCheck(tr("write-pin-config-op"), errcnt, errstr);  // The string "write-pin-config-op" should be translated to "Write pin configuration"
    requiresReset_ = true;
}

// Writes the power mode configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writePowerMode()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWPOWMODE), errcnt, errstr);
    opCheck(tr("write-power-mode-op"), errcnt, errstr);  // The string "write-power-mode-op" should be translated to "Write power mode"
    requiresReset_ = true;
}

// Writes the product descriptor to the CP2130 OTP ROM
void ConfiguratorWindow::writeProductDesc()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeProductDesc(editedConfig_.product, errcnt, errstr);
    opCheck(tr("write-product-desc-op"), errcnt, errstr);  // The string "write-product-desc-op" should be translated to "Write product descriptor"
    requiresReset_ = true;
}

// Writes the release version to the CP2130 OTP ROM
void ConfiguratorWindow::writeReleaseVersion()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWREL), errcnt, errstr);
    opCheck(tr("write-release-version-op"), errcnt, errstr);  // The string "write-release-version-op" should be translated to "Write release version"
    requiresReset_ = true;
}

// Writes the serial descriptor to the CP2130 OTP ROM
void ConfiguratorWindow::writeSerialDesc()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeSerialDesc(editedConfig_.serial, errcnt, errstr);
    opCheck(tr("write-serial-desc-op"), errcnt, errstr);  // The string "write-serial-desc-op" should be translated to "Write serial descriptor"
    if (!err_) {
        serialstr_ = editedConfig_.serial.toLatin1();  // If the previous operation was successful, it is safe to assume that the serial string changed to the new value (the conversion to ASCII was implemented in version 1.1 as a patch)
    }
    requiresReset_ = true;
}

// Writes the transfer priority configuration to the CP2130 OTP ROM
void ConfiguratorWindow::writeTransferPrio()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWTRFPRIO), errcnt, errstr);
    opCheck(tr("write-transfer-prio-op"), errcnt, errstr);  // The string "write-transfer-prio-op" should be translated to "Write transfer priority"
    requiresReset_ = true;
}

// Writes the VID to the CP2130 OTP ROM
void ConfiguratorWindow::writeVID()
{
    int errcnt = 0;
    QString errstr;
    cp2130_.writeUSBConfig(editedConfig_.usbconfig, static_cast<quint8>(CP2130::LWVID), errcnt, errstr);
    opCheck(tr("write-vid-op"), errcnt, errstr);  // The string "write-vid-op" should be translated to "Write VID"
    if (!err_) {
        vid_ = editedConfig_.usbconfig.vid;  // If the previous operation was successful, it is safe to assume that the VID changed to the new value
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
    } else if (ui->checkBoxVerify->isChecked()) {  // Successul configuration with verification
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

// Partially disables configurator window (fixed in version 2.1)
void ConfiguratorWindow::disableView()
{
    ui->actionInformation->setEnabled(false);
    ui->actionLoadConfiguration->setEnabled(false);  // Added in version 3.0
    ui->actionClose->setText(tr("&Close Window"));  // Implemented in version 2.0, to hint the user that the device is effectively closed and only its window remains open
    ui->actionOTPROMViewer->setEnabled(false);  // Added in version 3.0
    ui->actionSerialGeneratorEnable->setEnabled(false);  // Added in version 3.0
    ui->actionSerialGeneratorEnable->setChecked(false);  // Added in version 3.0
    ui->centralWidget->setEnabled(false);
    viewEnabled_ = false;
}

// This is the main display routine, used to display the given configuration, updating all fields accordingly
void ConfiguratorWindow::displayConfiguration(const Configuration &config)
{
    displayManufacturer(config.manufacturer);
    setManufacturerEnabled((CP2130::LWMANUF & lockWord_) == CP2130::LWMANUF);
    displayProduct(config.product);
    setProductEnabled((CP2130::LWPROD & lockWord_) == CP2130::LWPROD);
    displaySerial(config.serial);
    setSerialEnabled((CP2130::LWSER & lockWord_) == CP2130::LWSER);
    displayUSBConfig(config.usbconfig);
    setVIDEnabled((CP2130::LWVID & lockWord_) == CP2130::LWVID);
    setPIDEnabled((CP2130::LWPID & lockWord_) == CP2130::LWPID);
    setReleaseEnabled((CP2130::LWREL & lockWord_) == CP2130::LWREL);
    setMaxPowerEnabled((CP2130::LWMAXPOW & lockWord_) == CP2130::LWMAXPOW);
    setPowerModeEnabled((CP2130::LWPOWMODE & lockWord_) == CP2130::LWPOWMODE);
    setTransferPrioEnabled((CP2130::LWTRFPRIO & lockWord_) == CP2130::LWTRFPRIO);
    displayPinConfig(config.pinconfig);
    setPinConfigEnabled((CP2130::LWPINCFG & lockWord_) == CP2130::LWPINCFG);
    setWriteEnabled((CP2130::LWALL & lockWord_) != 0x0000);  // Since version 3.0, this also enables the "Load Configuration..." menu action
}

// Updates the manufacturer descriptor field
void ConfiguratorWindow::displayManufacturer(const QString &manufacturer)
{
    ui->lineEditManufacturer->setText(manufacturer);
}

// Updates all fields pertaining to the CP2130 pin configuration
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

// Updates the product descriptor field
void ConfiguratorWindow::displayProduct(const QString &product)
{
    ui->lineEditProduct->setText(product);
}

// Updates the serial descriptor field
void ConfiguratorWindow::displaySerial(const QString &serial)
{
    ui->lineEditSerial->setText(serial);
}

// Updates all fields pertaining to USB configuration
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

// Retrieves the user set configuration from the fields
void ConfiguratorWindow::getEditedConfiguration()
{
    editedConfig_.manufacturer = ui->lineEditManufacturer->text();
    editedConfig_.product = ui->lineEditProduct->text();
    editedConfig_.serial = ui->lineEditSerial->text();
    editedConfig_.usbconfig.vid = static_cast<quint16>(ui->lineEditVID->text().toUInt(nullptr, 16));
    editedConfig_.usbconfig.pid = static_cast<quint16>(ui->lineEditPID->text().toUInt(nullptr, 16));
    editedConfig_.usbconfig.majrel = static_cast<quint8>(ui->spinBoxMajVersion->value());
    editedConfig_.usbconfig.minrel = static_cast<quint8>(ui->spinBoxMinVersion->value());
    editedConfig_.usbconfig.maxpow = static_cast<quint8>(ui->lineEditMaxPower->text().toUInt() / 2);
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
    editedConfig_.pinconfig.sspndlvl = static_cast<quint16>(ui->lineEditSuspendLevel->text().toUInt());
    editedConfig_.pinconfig.sspndmode = static_cast<quint16>(ui->lineEditSuspendMode->text().toUInt());
    editedConfig_.pinconfig.wkupmask = static_cast<quint16>(ui->lineEditResumeMask->text().toUInt());
    editedConfig_.pinconfig.wkupmatch = static_cast<quint16>(ui->lineEditResumeMatch->text().toUInt());
    editedConfig_.pinconfig.divider = static_cast<quint8>(ui->spinBoxDivider->value());
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
    bool err = false;
    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);
    if (xmlReader.readNextStartElement() && xmlReader.name() == "cp2130config") {  // If the selected file is a CP2130 configuration file (the XML header is ignored)
        while (xmlReader.readNextStartElement()) {
            if (xmlReader.name() == "manufacturer") {  // Get manufacturer string
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString manufacturer = attr.value().toString();
                        if (static_cast<size_t>(manufacturer.size()) > CP2130::DESCMXL_MANUFACTURER) {
                            err = true;
                        } else if ((CP2130::LWMANUF & lockWord_) == CP2130::LWMANUF) {
                            ui->lineEditManufacturer->setText(manufacturer);
                        }
                    }
                }
            } else if (xmlReader.name() == "product") {  // Get product string
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString product = attr.value().toString();
                        if (static_cast<size_t>(product.size()) > CP2130::DESCMXL_PRODUCT) {
                            err = true;
                        } else if ((CP2130::LWPROD & lockWord_) == CP2130::LWPROD) {
                            ui->lineEditProduct->setText(product);
                        }
                    }
                }
            } else if (xmlReader.name() == "serial") {  // Get serial string
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "string") {
                        QString serial = attr.value().toString();
                        if (serial.isEmpty() || static_cast<size_t>(serial.size()) > CP2130::DESCMXL_SERIAL) {
                            err = true;
                        } else if ((CP2130::LWSER & lockWord_) == CP2130::LWSER) {
                            ui->lineEditSerial->setText(serial);
                        }
                    }
                }
                /*while (xmlReader.readNextStartElement()) {
                    if (xmlReader.name() == "generator") {  // Get serial generator settings
                        foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                            if (attr.name().toString() == "prototype") {
                                QString prototype = attr.value().toString();
                                if (!SerialGenerator::prototypeSerialIsValid(prototype)) {
                                    err = true;
                                } else {
                                    serialgensetting_.serialgen.setPrototypeSerial(prototype);
                                }
                            } else if (attr.name().toString() == "mode") {
                                bool ok;
                                int mode = attr.value().toInt(&ok);
                                if (!ok || mode > 0 || mode < 0) {
                                    err = true;
                                } else {
                                    serialgensetting_.serialgen.setReplaceMode(static_cast<quint8>(mode));
                                }
                            } else if (attr.name().toString() == "enable") {
                                QString genenable = attr.value().toString();
                                if (genenable != "true" || genenable != "false") {
                                    err = true;
                                } else {
                                    serialgensetting_.genenable = genenable == "true";
                                }
                            } else if (attr.name().toString() == "auto-generate") {
                                QString autogen = attr.value().toString();
                                if (autogen != "true" || autogen != "false") {
                                    err = true;
                                } else {
                                    serialgensetting_.autogen = autogen == "true";
                                }
                            }
                        }
                    }
                    xmlReader.skipCurrentElement();
                }*/
            } else if (xmlReader.name() == "vid") {  // Get VID
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "value") {
                        QString vid = attr.value().toString().toLower();
                        int pos = 0;
                        QRegExpValidator validator(QRegExp("[a-f\\d]{4}"), 0);
                        if (validator.validate(vid, pos) != QRegExpValidator::Acceptable) {
                            err = true;
                        } else if ((CP2130::LWVID & lockWord_) == CP2130::LWVID) {
                            ui->lineEditVID->setText(vid);
                        }
                    }
                }
            } else if (xmlReader.name() == "pid") {  // Get PID
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "value") {
                        QString pid = attr.value().toString().toLower();
                        int pos = 0;
                        QRegExpValidator validator(QRegExp("[a-f\\d]{4}"), 0);
                        if (validator.validate(pid, pos) != QRegExpValidator::Acceptable) {
                            err = true;
                        } else if ((CP2130::LWPID & lockWord_) == CP2130::LWPID) {
                            ui->lineEditPID->setText(pid);
                        }
                    }
                }
            } else if (xmlReader.name() == "release") {  // Get release version
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "major") {  // Major release number
                        bool ok;
                        int major = attr.value().toInt(&ok);
                        if (!ok || major > 255 || major < 0) {
                            err = true;
                        } else if ((CP2130::LWREL & lockWord_) == CP2130::LWREL) {
                            ui->spinBoxMajVersion->setValue(major);
                        }
                    } else if (attr.name().toString() == "minor") {  // Minor release number
                        bool ok;
                        int minor = attr.value().toInt(&ok);
                        if (!ok || minor > 255 || minor < 0) {
                            err = true;
                        } else if ((CP2130::LWREL & lockWord_) == CP2130::LWREL) {
                            ui->spinBoxMinVersion->setValue(minor);
                        }
                    }
                }
            } else if (xmlReader.name() == "power") {  // Get power related parameters
                foreach (const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                    if (attr.name().toString() == "maximum") {  // Maximum power (hexadecimal)
                        QString maxpow = attr.value().toString().toLower();
                        int pos = 0;
                        QRegExpValidator validator(QRegExp("[a-f\\d]{2}"), 0);
                        if (validator.validate(maxpow, pos) != QRegExpValidator::Acceptable) {
                            err = true;
                        } else if ((CP2130::LWMAXPOW & lockWord_) == CP2130::LWMAXPOW) {
                            ui->lineEditMaxPower->setText(QString::number(2 * maxpow.toInt(nullptr, 16)));
                            ui->lineEditMaxPowerHex->setText(maxpow);
                        }
                    } else if (attr.name().toString() == "mode") {  // Power mode (index)
                        bool ok;
                        int powmode = attr.value().toInt(&ok);
                        if (!ok || powmode > 2 || powmode < 0) {
                            err =true;
                        } else if ((CP2130::LWPOWMODE & lockWord_) == CP2130::LWPOWMODE) {
                            ui->comboBoxPowerMode->setCurrentIndex(powmode);
                        }
                    }
                }
            }
          /*} else if ((CP2130::LWTRFPRIO & lockWord_) == CP2130::LWTRFPRIO) {
                // To implement
            } else if ((CP2130::LWPINCFG & lockWord_) == CP2130::LWPINCFG) {
                // To implement
            }*/
            xmlReader.skipCurrentElement();
        }
    } else {  // The selected file is not a CP2130 configuration file (no further reading is done, and no subsequent errors are checked)
        QMessageBox::critical(this, tr("Error"), tr("The selected file is not a valid CP2130 configuration file."));
    }
    if (xmlReader.hasError() || err) {
        QMessageBox::critical(this, tr("Error"), tr("The file contains one or more errors. As a result, some parameters may have unintended values."));
    }
}

// Checks for errors and validates device operations
// Void since version 1.2, since the return value was found to be redundant
void ConfiguratorWindow::opCheck(const QString &op, int errcnt, QString errstr)
{
    if (errcnt > 0) {
        err_ = true;
        if (cp2130_.disconnected()) {
            errmsg_ = tr("Device disconnected.\n\nPlease reconnect it and try again.");
        } else {
            errstr.chop(1);  // Remove the last character, which is always a newline
            errmsg_ = tr("%1 operation returned the following error(s):\n– %2", "", errcnt).arg(op, errstr.replace("\n", "\n– "));
        }
    }
}

// Prepares the task list, by checking which fields changed, while also setting optional tasks according to the user's requirements
QStringList ConfiguratorWindow::prepareTaskList()
{
    QStringList tasks;
    if (editedConfig_.manufacturer != deviceConfig_.manufacturer) {
        tasks += "writeManufacturerDesc";
    }
    if (editedConfig_.product != deviceConfig_.product) {
        tasks += "writeProductDesc";
    }
    if (editedConfig_.serial != deviceConfig_.serial) {
        tasks += "writeSerialDesc";
    }
    if (editedConfig_.usbconfig.vid != deviceConfig_.usbconfig.vid) {
        tasks += "writeVID";
    }
    if (editedConfig_.usbconfig.pid != deviceConfig_.usbconfig.pid) {
        tasks += "writePID";
    }
    if (editedConfig_.usbconfig.majrel != deviceConfig_.usbconfig.majrel || editedConfig_.usbconfig.minrel != deviceConfig_.usbconfig.minrel) {
        tasks += "writeReleaseVersion";
    }
    if (editedConfig_.usbconfig.maxpow != deviceConfig_.usbconfig.maxpow) {
        tasks += "writeMaxPower";
    }
    if (editedConfig_.usbconfig.powmode != deviceConfig_.usbconfig.powmode) {
        tasks += "writePowerMode";
    }
    if (editedConfig_.usbconfig.trfprio != deviceConfig_.usbconfig.trfprio) {
        tasks += "writeTransferPrio";
    }
    if (editedConfig_.pinconfig != deviceConfig_.pinconfig) {
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
    deviceConfig_.manufacturer = cp2130_.getManufacturerDesc(errcnt, errstr);
    deviceConfig_.product = cp2130_.getProductDesc(errcnt, errstr);
    deviceConfig_.serial = cp2130_.getSerialDesc(errcnt, errstr);
    deviceConfig_.usbconfig = cp2130_.getUSBConfig(errcnt, errstr);
    deviceConfig_.pinconfig = cp2130_.getPinConfig(errcnt, errstr);
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
        err = cp2130_.open(vid_, pid_, serialstr_);
        if (err != CP2130::ERROR_NOT_FOUND) {  // Retry only if the device was not found yet (as it may take some time to enumerate)
            break;
        }
    }
    if (err == CP2130::SUCCESS) {  // Device was successfully reopened
        readDeviceConfiguration();
        this->setWindowTitle(tr("CP2130 Configurator (S/N: %1)").arg(serialstr_));
        displayConfiguration(deviceConfig_);
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
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("cp2130config");  // Write root element
    xmlWriter.writeAttribute("version", "1.0");
    xmlWriter.writeStartElement("manufacturer");  // Write manufacturer element
    xmlWriter.writeAttribute("string", ui->lineEditManufacturer->text());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("product");  // Write product element
    xmlWriter.writeAttribute("string", ui->lineEditProduct->text());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("serial");  // Write serial element
    xmlWriter.writeAttribute("string", ui->lineEditSerial->text());
    if (serialgensetting_.doexport) {
        xmlWriter.writeStartElement("generator");  // Write generator element
        xmlWriter.writeAttribute("prototype", serialgensetting_.serialgen.prototypeSerial());
        xmlWriter.writeAttribute("mode", QString::number(serialgensetting_.serialgen.replaceMode()));
        xmlWriter.writeAttribute("enable", (serialgensetting_.genenable ? "true" : "false"));
        xmlWriter.writeAttribute("auto-generate", (serialgensetting_.autogen ? "true" : "false"));
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("vid");  // Write VID element
    xmlWriter.writeAttribute("value", ui->lineEditVID->text());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("pid");  // Write PID element
    xmlWriter.writeAttribute("value", ui->lineEditPID->text());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("release");  // Write release element
    xmlWriter.writeAttribute("major", ui->spinBoxMajVersion->text());
    xmlWriter.writeAttribute("minor", ui->spinBoxMinVersion->text());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("power");  // Write maximum power element
    xmlWriter.writeAttribute("maximum", ui->lineEditMaxPowerHex->text());
    xmlWriter.writeAttribute("mode", QString::number(ui->comboBoxPowerMode->currentIndex()));
    xmlWriter.writeEndElement();
    // To do
    xmlWriter.writeEndElement();
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

// Enables or disables the serial descriptor field and the related "Serial Number Generator > Enable" action
void ConfiguratorWindow::setSerialEnabled(bool value)
{
    ui->actionSerialGeneratorEnable->setEnabled(value);  // Added in version 3.0
    if (!value) {  // Implemented in version 3.0
        ui->actionSerialGeneratorEnable->setChecked(false);  // This also disables pushButtonGenerateSerial
    }
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

// Enables or disables editing related actions, buttons and checkboxes
void ConfiguratorWindow::setWriteEnabled(bool value)
{
    ui->actionLoadConfiguration->setEnabled(value);  // Added in version 3.0
    ui->pushButtonRevert->setEnabled(value);
    ui->checkBoxVerify->setEnabled(value);
    ui->checkBoxLock->setEnabled(value);
    ui->pushButtonWrite->setEnabled(value);
}

// Checks user input, returning false if it is valid, or true otherwise, while also highlighting invalid fields
bool ConfiguratorWindow::showInvalidInput()
{
    bool retval = false;
    if (ui->lineEditSerial->text().isEmpty()) {  // Condition added in version 3.0
        ui->lineEditSerial->setStyleSheet("background: rgb(255, 102, 102);");
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
    if (ui->lineEditPID->text().size() < 4 || ui->lineEditPID->text() == "0000") {
        ui->lineEditPID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditVID->text().size() < 4 || ui->lineEditVID->text() == "0000") {
        ui->lineEditVID->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendLevel->text().size() < 4 || ui->lineEditSuspendLevel->text().toInt(nullptr, 16) > 0x7fff) {  // Extra check condition added in version 1.1
        ui->lineEditSuspendLevel->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditSuspendMode->text().size() < 4) {
        ui->lineEditSuspendMode->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMask->text().size() < 4 || ui->lineEditResumeMask->text().toInt(nullptr, 16) > 0x7fff) {  // Extra check condition added in version 1.1
        ui->lineEditResumeMask->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    if (ui->lineEditResumeMatch->text().size() < 4 || ui->lineEditResumeMatch->text().toInt(nullptr, 16) > 0x7fff) {  // Extra check condition added in version 1.1
        ui->lineEditResumeMatch->setStyleSheet("background: rgb(255, 102, 102);");
        retval = true;
    }
    return retval;
}
