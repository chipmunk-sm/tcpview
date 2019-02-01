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

#ifndef TABLEHEADERCAPTION_H
#define TABLEHEADERCAPTION_H

#include <QWidget>

class TableHeaderCaption
{
public:
    TableHeaderCaption(const QString &caption, const QString &captionSample, const QString &captionToolTip)
    {
        m_caption = caption;
        m_captionSample = captionSample;
        m_captionToolTip = captionToolTip;
    }

    QString caption() const { return m_caption; }
    QString captionSample() const { return m_captionSample; }
    QString captionToolTip() const { return m_captionToolTip; }

  private:
    QString m_caption;
    QString m_captionSample;
    QString m_captionToolTip;

};

#endif // TABLEHEADERCAPTION_H
