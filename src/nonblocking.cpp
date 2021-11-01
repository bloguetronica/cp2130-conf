/* Non-blocking utility functions for use in Qt - Version 1.0.0
   Copyright (c) 2021 Samuel Lourenço

   This library is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <https://www.gnu.org/licenses/>.


   Please feel free to contact me via e-mail: samuel.fmlourenco@gmail.com */


// Includes
#include <QEventLoop>
#include <QTimer>
#include "nonblocking.h"

// Non-blocking sleep in milliseconds
void NonBlocking::msleep(int msecs)
{
    if (msecs > 0) {
        QEventLoop loop;
        QTimer::singleShot(msecs, &loop, SLOT(quit()));
        loop.exec();
    }
}

// Non-blocking sleep in seconds
void NonBlocking::sleep(int secs)
{
    if (secs > 0) {
        QEventLoop loop;
        QTimer::singleShot(1000 * secs, &loop, SLOT(quit()));
        loop.exec();
    }
}
