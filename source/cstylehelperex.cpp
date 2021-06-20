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

#include "cstylehelperex.h"

#include <QCoreApplication>
#include <QWidget>
#include <QSettings>

double CStyleHelperEx::SetStyle(double size, QWidget *obj) {


    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    auto valColor = settings.value("tree/color");

    // QFontMetrics fm(fontX);

    auto valSize = QString::number(size);
    auto valSize1 = QString::number(size / 4);

    auto style = "QPushButton { icon-size: " + valSize + "px;  padding-left: " + valSize + "px; padding-right: " + valSize + "px; padding-top: " + valSize1 + "px; padding-bottom: " + valSize1 + "px; }"
                 "QHeaderView::section {   padding-left: " + valSize + "px; padding-right: " + valSize + "px; padding-top: " + valSize1 + "px; padding-bottom: " + valSize1 + "px }";

    if (valColor.isValid()) {
        style += "QTreeView { background: " + valColor.toString() + "; }";
    }

    obj->setStyleSheet(style);

    return size;
}
