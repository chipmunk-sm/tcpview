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

#include "ccustomproxymodel.h"

CCustomProxyModel::CCustomProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    m_QRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    m_QRegExp.setPatternSyntax(QRegExp::RegExp);
}

void CCustomProxyModel::setFilterRegExpEx(const QString &val)
{
    m_QRegExp.setPattern(val);
    setFilterRegExp(m_QRegExp);
}

bool CCustomProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{

    if (filterRegExp().isEmpty())
        return true;

    for(int ind = 0; ind < sourceModel()->columnCount() - 1; ind++)
    {
        QModelIndex index = sourceModel()->index(sourceRow, ind, sourceParent);
        if(sourceModel()->data(index).toString().contains(filterRegExp()))
            return true;
    }

    return false;
}


