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

#ifndef CCONNECTIONSTREE_H
#define CCONNECTIONSTREE_H

#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QTreeWidget>
#include <QXmlStreamWriter>
#include "datasource.h"
#include "tableheadercaption.h"
#include "ccustomproxymodel.h"

class CConnectionsTree
{
public:
    CConnectionsTree();
    ~CConnectionsTree();


    void InitConnectonsTree(QTreeView *connView);
    void UpdateIncludeFilter(const QString &value);
    void UpdateData(CDataSource::SocketInfo * dataSource, bool disableCleanup);


    void SetDataColumnHiden(QTreeView *tree);
    void Save(QWidget *parent);

    QString GetStateString(uint state);

private:
    QMap<CDataSource::ColumnData, TableHeaderCaption>   m_caption;
    QMap<CDataSource::eNetType, QString>                m_protocol;
    QMap<CDataSource::ConnectionTcpState, QString>      m_state;
    CCustomProxyModel                                   *m_pProxyModel;


    void PrepareTableCaptionString();
    void PrepareTableStateString();
    void PrepareTableProtocolString();

};

#endif // CCONNECTIONSTREE_H
