/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2019 chipmunk-sm <dannico@linuxmail.org>
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

#ifndef CCFONTSIZE_H
#define CCFONTSIZE_H

#include <QSlider>
#include <QWidget>

class CCFontSize
{
public:
    CCFontSize();
    bool Init();
    void SetFontSize(int fontIndex);
    void ConfigureSlider(QSlider* slider, QWidget *pObj);
    void SaveConfig();

private:
    int                 m_fontSize = -1;
    QWidget             *m_qwidget;
    QSlider             *m_slider;
    QList<int>          m_fontSizeList;

    int GetPointSize();
    QString GetConfigName();

};

#endif // CCFONTSIZE_H
