/* CP2130 Configurator - Version 1.3.3 for Debian Linux
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


#ifndef CONFIGURATORWINDOW_H
#define CONFIGURATORWINDOW_H

// Includes
#include <QFile>
#include <QMainWindow>
#include <QPointer>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include "configuration.h"
#include "cp2130.h"
#include "informationdialog.h"
#include "otpromviewerdialog.h"
#include "serialgeneratorsettings.h"

namespace Ui {
class ConfiguratorWindow;
}

class ConfiguratorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfiguratorWindow(QWidget *parent = nullptr);
    ~ConfiguratorWindow();

    bool isViewEnabled();
    void openDevice(quint16 vid, quint16 pid, const QString &serialstr);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void lockOTP();
    void on_actionAbout_triggered();
    void on_actionInformation_triggered();
    void on_actionLoadConfiguration_triggered();
    void on_actionOTPROMViewer_triggered();
    void on_actionSaveConfiguration_triggered();
    void on_actionSerialGeneratorEnable_toggled(bool checked);
    void on_actionSerialGeneratorSettings_triggered();
    void on_lineEditManufacturer_textEdited(QString text);
    void on_lineEditMaxPower_editingFinished();
    void on_lineEditMaxPower_textChanged();
    void on_lineEditMaxPower_textEdited(QString text);
    void on_lineEditMaxPowerHex_editingFinished();
    void on_lineEditMaxPowerHex_textChanged();
    void on_lineEditMaxPowerHex_textEdited();
    void on_lineEditPID_textChanged(const QString &text);
    void on_lineEditPID_textEdited(const QString &text);
    void on_lineEditProduct_textEdited(QString text);
    void on_lineEditResumeMask_textChanged();
    void on_lineEditResumeMask_textEdited();
    void on_lineEditResumeMatch_textChanged();
    void on_lineEditResumeMatch_textEdited();
    void on_lineEditSerial_textChanged();
    void on_lineEditSerial_textEdited(QString text);
    void on_lineEditSuspendLevel_textChanged();
    void on_lineEditSuspendLevel_textEdited();
    void on_lineEditSuspendMode_textChanged();
    void on_lineEditSuspendMode_textEdited();
    void on_lineEditVID_textChanged(const QString &text);
    void on_lineEditVID_textEdited(const QString &text);
    void on_pushButtonGenerateSerial_clicked();
    void on_pushButtonRevert_clicked();
    void on_pushButtonWrite_clicked();
    void verifyConfiguration();
    void writeManufacturerDesc();
    void writeMaxPower();
    void writePID();
    void writePinConfig();
    void writePowerMode();
    void writeProductDesc();
    void writeReleaseVersion();
    void writeSerialDesc();
    void writeTransferPrio();
    void writeVID();

private:
    Ui::ConfiguratorWindow *ui;
    Configuration deviceConfiguration_, editedConfiguration_;
    CP2130 cp2130_;
    QPointer<InformationDialog> informationDialog_;
    QPointer<OTPROMViewerDialog> otpromViewerDialog_;
    QString errmsg_, serialString_;
    SerialGeneratorSettings serialGeneratorSettings_;
    quint16 lockWord_, pid_, vid_;
    bool err_, requiresReset_, viewEnabled_ = false;

    void configureDevice();
    void disableView();
    void displayConfiguration(const Configuration &configuration, bool fullUpdate);
    void displayManufacturer(const QString &manufacturer);
    void displayMaxPower(quint8 maxpow);
    void displayPID(quint16 pid);
    void displayPinConfig(const CP2130::PinConfig &pinConfig);
    void displayPowerMode(quint8 powmode);
    void displayProduct(const QString &product);
    void displayReleaseVersion(quint8 majrel, quint8 minrel);
    void displaySerial(const QString &serial);
    void displayTransferPrio(quint8 trfprio);
    void displayVID(quint16 vid);
    void getEditedConfiguration();
    void handleError();
    void loadConfigurationFromFile(QFile &file);
    QStringList prepareTaskList();
    void readDeviceConfiguration();
    void resetDevice();
    void saveConfigurationToFile(QFile &file);
    void setManufacturerEnabled(bool value);
    void setMaxPowerEnabled(bool value);
    void setPinConfigEnabled(bool value);
    void setPIDEnabled(bool value);
    void setPowerModeEnabled(bool value);
    void setProductEnabled(bool value);
    void setReleaseEnabled(bool value);
    void setSerialEnabled(bool value);
    void setTransferPrioEnabled(bool value);
    void setVIDEnabled(bool value);
    void setWriteEnabled(bool value);
    bool showInvalidInput();
    void validateOperation(const QString &operation, int errcnt, QString errstr);
};

#endif  // CONFIGURATORWINDOW_H
