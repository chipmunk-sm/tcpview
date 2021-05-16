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

#ifndef CCONNECTIONSTREE_H
#define CCONNECTIONSTREE_H

#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QTreeWidget>
#include <QXmlStreamWriter>
#include "datasource.h"
#include "tableheadercaption.h"
#include "ccustomproxymodel.h"
#include "connectionstatehelper.h"

class CConnectionsTree
{
public:
    CConnectionsTree();
    ~CConnectionsTree();
    void InitConnectonsTree(QTreeView *connView);
    void UpdateIncludeFilter(const QString &value);
    void UpdateData(SocketInfo * dataSource, bool disableCleanup);
    void Save(QWidget *parent);

private:
    QMap<eColumnData, TableHeaderCaption>   m_caption;
    QMap<eNetType, QString>                 m_protocol;
    CCustomProxyModel                       *m_pProxyModel;
    ConnectionStateHelper                   m_ConnectionStateHelper;
    void PrepareTableCaptionString();
    void PrepareTableProtocolString();

};

#endif // CCONNECTIONSTREE_H
