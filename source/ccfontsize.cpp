/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2021 chipmunk-sm <dannico@linuxmail.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ccfontsize.h"

#include <QApplication>
#include <QSettings>
#include <QFont>

double CCFontSize::changeFontSize(double change)
{

    auto fontX = QApplication::font();

    const double pixSize = fontX.pixelSize();
    const double pointSize = fontX.pointSizeF();

    const double fontSize = pointSize > 0 ? pointSize : pixSize;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    auto tmpSz = settings.value(GetConfigName(), fontSize).toDouble();

    tmpSz += change;

    if (tmpSz < 5.0)
        tmpSz = 5.0;

    if (tmpSz > 100.0)
        tmpSz = 100.0;

    settings.setValue(GetConfigName(), tmpSz);

    if(pointSize > 0)
        fontX.setPointSizeF(tmpSz);
    else
        fontX.setPixelSize(static_cast<int>(tmpSz));

    QApplication::setFont(fontX);

    return tmpSz;
}

const char *CCFontSize::GetConfigName()
{
    return "base/xfontSize";
}

