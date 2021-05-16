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

#include <QString>
#include <QColor>
#include <map>

#include "datasource.h"

#ifndef CONNECTIONSTATEHELPER_H
#define CONNECTIONSTATEHELPER_H

class ConnectionStateItem
{
public:
    QString name;
    QColor foreground;
    QColor background;
};

class ConnectionStateHelper
{
public:
    ConnectionStateHelper();

    QString getStateName(eConnectionTcpState id);
    QColor getStateColor(eConnectionTcpState id, bool foreground);
    void saveColor(eConnectionTcpState id, const QColor & defColor, bool foreground);
    const std::map<eConnectionTcpState, ConnectionStateItem> getArray() const;
    void updatetColor(bool reset);
private:
    void initParameters(eConnectionTcpState id, QColor defColorForeground, QColor defColorBackground, const QString &name, bool reset);
    QString getKeyName(eConnectionTcpState id, bool foreground);

    std::map<eConnectionTcpState, ConnectionStateItem> m_state;
};

#endif // CONNECTIONSTATEHELPER_H
