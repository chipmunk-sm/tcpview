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

#include "connectionstatehelper.h"
#include <QCoreApplication>
#include <QLabel>
#include <QObject>
#include <QSettings>

ConnectionStateHelper::ConnectionStateHelper()
{
    updatetColor(false);
}

QString ConnectionStateHelper::getStateName(eConnectionTcpState id)
{
    auto ptr = m_state.find(id);
    if (ptr != m_state.end())
        return ptr->second.name;
    return QString::number(id);
}

QColor ConnectionStateHelper::getStateColor(eConnectionTcpState id, bool foreground)
{
    auto ptr = m_state.find(id);
    if (ptr != m_state.end())
        return foreground ? ptr->second.foreground : ptr->second.background;
    return foreground ? QColor(0x00, 0x00, 0x00) : QColor(0xFF,0xFF,0xFF);
}

void ConnectionStateHelper::saveColor(eConnectionTcpState id, const QColor &color, bool foreground)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(getKeyName(id, foreground), color);
    if (foreground)
        m_state[id].foreground = color;
    else
        m_state[id].background = color;
}

const std::map<eConnectionTcpState, ConnectionStateItem> ConnectionStateHelper::getArray() const
{
    return m_state;
}

QString ConnectionStateHelper::getKeyName(eConnectionTcpState id, bool foreground)
{
    return QString("connectionState/colorid_%1_%2").arg(id).arg(foreground ? "F" : "B");
}

void ConnectionStateHelper::updatetColor(bool reset)
{
    initParameters(CONNECTION_TCP_EMPTY      , QColor(0x7F,0x7F,0x7F), QColor(0xFF,0xFF,0xFF), QObject::tr(""), reset);
    initParameters(CONNECTION_TCP_ESTABLISHED, QColor(0x00,0x7F,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("established"), reset);
    initParameters(CONNECTION_TCP_SYN_SENT   , QColor(0x00,0xb4,0x57), QColor(0xFF,0xFF,0xFF), QObject::tr("syn_sent"), reset);
    initParameters(CONNECTION_TCP_SYN_RECV   , QColor(0x00,0x00,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("syn_recv"), reset);
    initParameters(CONNECTION_TCP_FIN_WAIT1  , QColor(0x00,0x00,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("fin_wait1"), reset);
    initParameters(CONNECTION_TCP_FIN_WAIT2  , QColor(0x00,0x00,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("fin_wait2"), reset);
    initParameters(CONNECTION_TCP_TIME_WAIT  , QColor(0x8b,0x00,0x46), QColor(0xFF,0xFF,0xFF), QObject::tr("time_wait"), reset);
    initParameters(CONNECTION_TCP_CLOSE      , QColor(0x00,0x00,0xff), QColor(0xFF,0xFF,0xFF), QObject::tr("unconnected"), reset);
    initParameters(CONNECTION_TCP_CLOSE_WAIT , QColor(0xb1,0x00,0xb1), QColor(0xFF,0xFF,0xFF), QObject::tr("close_wait"), reset);
    initParameters(CONNECTION_TCP_LAST_ACK   , QColor(0x7e,0x7e,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("last_ack"), reset);
    initParameters(CONNECTION_TCP_LISTEN     , QColor(0x00,0xaa,0x7f), QColor(0xFF,0xFF,0xFF), QObject::tr("listen"), reset);
    initParameters(CONNECTION_TCP_CLOSING    , QColor(0xaf,0x00,0x58), QColor(0xFF,0xFF,0xFF), QObject::tr("closing"), reset);
    initParameters(CONNECTION_TCP_CLOSED     , QColor(0xc8,0x00,0x64), QColor(0xFF,0xFF,0xFF), QObject::tr("closed"), reset);
    initParameters(CONNECTION_REMOVED        , QColor(0xFF,0x00,0x00), QColor(0xFF,0xFF,0xFF), QObject::tr("removed"), reset);
}

void ConnectionStateHelper::setBwColor() {
     QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    for (const auto & item : m_state) {
        settings.setValue(getKeyName(item.first, true), QColor(0xFF,0xFF,0xFF));
        settings.setValue(getKeyName(item.first, false), QColor(0x00,0x00,0x00));
        m_state[item.first].foreground = settings.value(getKeyName(item.first, true)).value<QColor>();
        m_state[item.first].background = settings.value(getKeyName(item.first, false)).value<QColor>();
    }
}

void ConnectionStateHelper::setWbColor()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    for (const auto & item : m_state) {
        settings.setValue(getKeyName(item.first, true), QColor(0x00,0x00,0x00));
        settings.setValue(getKeyName(item.first, false), QColor(0xFF,0xFF,0xFF));
        m_state[item.first].foreground = settings.value(getKeyName(item.first, true)).value<QColor>();
        m_state[item.first].background = settings.value(getKeyName(item.first, false)).value<QColor>();
    }
}

void ConnectionStateHelper::initParameters(eConnectionTcpState id, QColor defColorForeground, QColor defColorBackground, const QString & name, bool reset)
{

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if(reset)
    {
        settings.setValue(getKeyName(id, true), defColorForeground);
        settings.setValue(getKeyName(id, false), defColorBackground);
    }

    defColorForeground = settings.value(getKeyName(id, true), defColorForeground).value<QColor>();
    defColorBackground = settings.value(getKeyName(id, false), defColorBackground).value<QColor>();

    m_state[id].name = name;
    m_state[id].foreground = defColorForeground;
    m_state[id].background = defColorBackground;

    settings.setValue(getKeyName(id, true), defColorForeground);
    settings.setValue(getKeyName(id, false), defColorBackground);

}
