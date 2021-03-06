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
#include <QSortFilterProxyModel>
#include <atomic>

#ifndef CCUSTOMPROXYMODEL_H
#define CCUSTOMPROXYMODEL_H

typedef struct ColorItem
{
    QColor Foreground;
    QColor Background;
}ColorItem;

typedef enum DataTyp{
    DataTyp_TypId = Qt::UserRole + 50,
    DataTyp_TypState = Qt::UserRole + 51
}DataTyp;

class CCustomProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CCustomProxyModel(QObject *parent = nullptr);
    void setFilterRegExpEx(const QString &val);
    void updateColorMap();
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
    QRegExp m_QRegExp;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    std::atomic<bool> m_updateColor = ATOMIC_FLAG_INIT;
    mutable std::map<eConnectionTcpState, ColorItem> m_color;
};

#endif // CCUSTOMPROXYMODEL_H
