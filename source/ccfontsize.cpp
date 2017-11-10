/* This file is part of tcpview - network connections viewer for Linux
 * Copyright (C) 2017 chipmunk-sm <dannico@linuxmail.org>
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

#include <QtGui>


CCFontSize::CCFontSize()
    : m_fontSize(-1)
    , m_qwidget(nullptr)
    , m_slider(nullptr)
{

}

void CCFontSize::ConfigureSlider(QSlider* slider, QWidget *pObj)
{
    m_qwidget = pObj;
    m_slider = slider;

    slider->setSingleStep(1);
    slider->setPageStep(1);

}

bool CCFontSize::Init()
{

    if(m_fontSize != -1)
        return false;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    auto initialFontSize = settings.value(GetConfigName(), m_qwidget->font().pointSize());
    auto fontSize = initialFontSize.toInt();

    m_fontSizeList = QFontDatabase::standardSizes();
    m_slider->setRange(0, m_fontSizeList.length() - 1);

    auto index = m_fontSizeList.indexOf(fontSize);
    if(index < 0)
    {
        for(QList<int>::iterator it(m_fontSizeList.begin()); it != m_fontSizeList.end(); ++it)
        {
            if(*it < fontSize)
                continue;
            m_fontSizeList.insert(it,fontSize);
            break;
        }
        index = m_fontSizeList.indexOf(fontSize);
    }

    if(index >= 0)
       m_slider->setValue(index);

    if(index >= 0)
        m_fontSize = fontSize;

    if(index >= 0)
        SetFontSize(index);

    return true;
}

void CCFontSize::SetFontSize(int fontIndex)
{
    if(m_fontSize < 0 || m_qwidget == nullptr)
        return;

    if( fontIndex >= m_fontSizeList.length() )
        fontIndex  = m_fontSizeList.length() - 1;

    m_fontSize = m_fontSizeList[fontIndex];

    auto font = m_qwidget->font();
    font.setPointSize(m_fontSize);
    m_qwidget->setFont(font);

}

int CCFontSize::GetPointSize()
{
    return m_fontSize;
}

QString CCFontSize::GetConfigName()
{
    return QString("base/fontSize");
}

void CCFontSize::SaveConfig()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(GetConfigName(), GetPointSize());
}
