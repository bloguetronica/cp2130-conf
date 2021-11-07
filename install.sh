#!/bin/sh

echo Obtaining required packages...
apt-get -qq update
apt-get -qq install build-essential
apt-get -qq install libusb-1.0-0-dev
apt-get -qq install qt5-default
echo Copying source code files...
mkdir -p /usr/local/src/cp2130-conf/icons
mkdir -p /usr/local/src/cp2130-conf/misc
mkdir -p /usr/local/src/cp2130-conf/translations
cp -f src/aboutdialog.cpp /usr/local/src/cp2130-conf/.
cp -f src/aboutdialog.h /usr/local/src/cp2130-conf/.
cp -f src/aboutdialog.ui /usr/local/src/cp2130-conf/.
cp -f src/configuration.cpp /usr/local/src/cp2130-conf/.
cp -f src/configuration.h /usr/local/src/cp2130-conf/.
cp -f src/configuratorwindow.cpp /usr/local/src/cp2130-conf/.
cp -f src/configuratorwindow.h /usr/local/src/cp2130-conf/.
cp -f src/configuratorwindow.ui /usr/local/src/cp2130-conf/.
cp -f src/cp2130-conf.pro /usr/local/src/cp2130-conf/.
cp -f src/cp2130.cpp /usr/local/src/cp2130-conf/.
cp -f src/cp2130.h /usr/local/src/cp2130-conf/.
cp -f src/GPL.txt /usr/local/src/cp2130-conf/.
cp -f src/icons/active64.png /usr/local/src/cp2130-conf/icons/.
cp -f src/icons/cp2130-conf.png /usr/local/src/cp2130-conf/icons/.
cp -f src/icons/greyed64.png /usr/local/src/cp2130-conf/icons/.
cp -f src/icons/icon.svg /usr/local/src/cp2130-conf/icons/.
cp -f src/icons/selected64.png /usr/local/src/cp2130-conf/icons/.
cp -f src/informationdialog.cpp /usr/local/src/cp2130-conf/.
cp -f src/informationdialog.h /usr/local/src/cp2130-conf/.
cp -f src/informationdialog.ui /usr/local/src/cp2130-conf/.
cp -f src/LGPL.txt /usr/local/src/cp2130-conf/.
cp -f src/libusb-extra.c /usr/local/src/cp2130-conf/.
cp -f src/libusb-extra.h /usr/local/src/cp2130-conf/.
cp -f src/main.cpp /usr/local/src/cp2130-conf/.
cp -f src/mainwindow.cpp /usr/local/src/cp2130-conf/.
cp -f src/mainwindow.h /usr/local/src/cp2130-conf/.
cp -f src/mainwindow.ui /usr/local/src/cp2130-conf/.
cp -f src/misc/cp2130-conf.desktop /usr/local/src/cp2130-conf/misc/.
cp -f src/nonblocking.cpp /usr/local/src/cp2130-conf/.
cp -f src/nonblocking.h /usr/local/src/cp2130-conf/.
cp -f src/README.txt /usr/local/src/cp2130-conf/.
cp -f src/resources.qrc /usr/local/src/cp2130-conf/.
cp -f src/translations/cp2130-conf_en.qm /usr/local/src/cp2130-conf/translations/.
cp -f src/translations/cp2130-conf_en.ts /usr/local/src/cp2130-conf/translations/.
cp -f src/translations/cp2130-conf_en_US.qm /usr/local/src/cp2130-conf/translations/.
cp -f src/translations/cp2130-conf_en_US.ts /usr/local/src/cp2130-conf/translations/.
cp -f src/translations/cp2130-conf_pt_PT.qm /usr/local/src/cp2130-conf/translations/.
cp -f src/translations/cp2130-conf_pt_PT.ts /usr/local/src/cp2130-conf/translations/.
echo Building and installing application...
cd /usr/local/src/cp2130-conf
qmake
make install clean
rm -f cp2130-conf
echo Done!
