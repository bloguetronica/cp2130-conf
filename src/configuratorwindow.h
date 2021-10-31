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


#ifndef CONFIGURATORWINDOW_H
#define CONFIGURATORWINDOW_H

// Includes
#include <QMainWindow>
#include "configuration.h"
#include "cp2130.h"

namespace Ui {
class ConfiguratorWindow;
}

class ConfiguratorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConfiguratorWindow(QWidget *parent = nullptr);
    ~ConfiguratorWindow();

    void openDevice(quint16 vid, quint16 pid, const QString &serialstr);

private slots:
    void on_lineEditMaxPower_editingFinished();
    void on_lineEditMaxPower_textChanged();
    void on_lineEditMaxPower_textEdited();
    void on_lineEditMaxPowerHex_editingFinished();
    void on_lineEditMaxPowerHex_textChanged();
    void on_lineEditMaxPowerHex_textEdited();
    void on_lineEditPID_textChanged();
    void on_lineEditPID_textEdited();
    void on_lineEditSuspendLevel_textChanged();
    void on_lineEditSuspendLevel_textEdited();
    void on_lineEditSuspendMode_textChanged();
    void on_lineEditSuspendMode_textEdited();
    void on_lineEditVID_textChanged();
    void on_lineEditVID_textEdited();
    void on_lineEditResumeMatch_textChanged();
    void on_lineEditResumeMatch_textEdited();
    void on_lineEditResumeMask_textChanged();
    void on_lineEditResumeMask_textEdited();
    void on_pushButtonRevert_clicked();
    void on_pushButtonWrite_clicked();

private:
    Ui::ConfiguratorWindow *ui;
    CP2130 cp2130_;
    Configuration editedConfig_, initConfig_;
    QString serialstr_;
    quint16 lockWord_, pid_, vid_;

    void displayConfiguration(const Configuration &config);
    void displayManufacturer(const QString &manufacturer);
    void displayPinConfig(const CP2130::PinConfig &pinconfig);
    void displayProduct(const QString &product);
    void displaySerial(const QString &serial);
    void displayUSBConfig(const CP2130::USBConfig &usbconfig);
    void getEditedConfiguration();
    void readInitialConfiguration();
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
};

#endif // CONFIGURATORWINDOW_H
