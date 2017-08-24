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

#include "cconnectionstree.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QUuid>
#include <QDateTime>
#include <QStandardPaths>
#include <source/cfilesavewrapper.h>
#include <source/cusername.h>


CConnectionsTree::CConnectionsTree()
    : m_pProxyModel(nullptr)
{
    PrepareTableCaptionString();
    PrepareTableProtocolString();
    PrepareTableStateString();
}

CConnectionsTree::~CConnectionsTree()
{
    delete m_pProxyModel;
}

void CConnectionsTree::PrepareTableCaptionString()
{
    //                                                              caption                                             captionSample                   captionToolTip
    m_caption.insert(CDataSource::COLUMN_DATA_PROTOCOL,        TableHeaderCaption(QObject::tr("Protocol"),         QObject::tr("tcp6"),            QObject::tr("Protocol")));
    m_caption.insert(CDataSource::COLUMN_DATA_LOCALHOST,       TableHeaderCaption(QObject::tr("Local Host"),       QObject::tr("Local Host"),      QObject::tr("Local Host")));
    m_caption.insert(CDataSource::COLUMN_DATA_LOCALADDRESS,    TableHeaderCaption(QObject::tr("Local Address"),    QObject::tr("255.255.255.255"), QObject::tr("Local Address")));
    m_caption.insert(CDataSource::COLUMN_DATA_LOCALPORT,       TableHeaderCaption(QObject::tr("Local Port"),       QObject::tr("65535"),           QObject::tr("Local Port")));
    m_caption.insert(CDataSource::COLUMN_DATA_STATE,           TableHeaderCaption(QObject::tr("State"),            QObject::tr("ESTABLISHED"),     QObject::tr("State")));
    m_caption.insert(CDataSource::COLUMN_DATA_REMOTEHOST,      TableHeaderCaption(QObject::tr("Remote Host"),      QObject::tr("Remote Host"),     QObject::tr("Remote Host")));
    m_caption.insert(CDataSource::COLUMN_DATA_REMOTEADDRESS,   TableHeaderCaption(QObject::tr("Remote Address"),   QObject::tr("255.255.255.255"), QObject::tr("Remote Address")));
    m_caption.insert(CDataSource::COLUMN_DATA_REMOTEPORT,      TableHeaderCaption(QObject::tr("Remote Port"),      QObject::tr("65535"),           QObject::tr("Remote Port")));
    m_caption.insert(CDataSource::COLUMN_DATA_UID,             TableHeaderCaption(QObject::tr("User"),             QObject::tr("0000000"),         QObject::tr("User")));
    m_caption.insert(CDataSource::COLUMN_DATA_INODE,           TableHeaderCaption(QObject::tr("Inode"),            QObject::tr("0000000"),         QObject::tr("Inode")));
    m_caption.insert(CDataSource::COLUMN_DATA_TIME,            TableHeaderCaption(QObject::tr("Time"),             QObject::tr("2017-01-01 00:00:00"), QObject::tr("Time")));
    m_caption.insert(CDataSource::COLUMN_DATA_COMMAND,         TableHeaderCaption(QObject::tr("Command"),          QObject::tr("Command"),         QObject::tr("Command")));
    m_caption.insert(CDataSource::COLUMN_DATA_DATA,            TableHeaderCaption(QObject::tr("RowId"),            QObject::tr("RowId"),           QObject::tr("RowId")));
}

void CConnectionsTree::PrepareTableProtocolString()
{
    m_protocol.insert(CDataSource::conn_tcp,    QObject::tr("tcp"));
    m_protocol.insert(CDataSource::conn_udp,    QObject::tr("udp"));
    m_protocol.insert(CDataSource::conn_tcp6,   QObject::tr("tcp6"));
    m_protocol.insert(CDataSource::conn_udp6,   QObject::tr("udp6"));
    m_protocol.insert(CDataSource::conn_raw,    QObject::tr("raw"));
    m_protocol.insert(CDataSource::conn_raw6,   QObject::tr("raw6"));
}

void CConnectionsTree::PrepareTableStateString()
{
    m_state.insert(CDataSource::CONNECTION_TCP_EMPTY,         QObject::tr(""));
    m_state.insert(CDataSource::CONNECTION_TCP_ESTABLISHED,   QObject::tr("established"));
    m_state.insert(CDataSource::CONNECTION_TCP_SYN_SENT,      QObject::tr("syn_sent"));
    m_state.insert(CDataSource::CONNECTION_TCP_SYN_RECV,      QObject::tr("syn_recv"));
    m_state.insert(CDataSource::CONNECTION_TCP_FIN_WAIT1,     QObject::tr("fin_wait1"));
    m_state.insert(CDataSource::CONNECTION_TCP_FIN_WAIT2,     QObject::tr("fin_wait2"));
    m_state.insert(CDataSource::CONNECTION_TCP_TIME_WAIT,     QObject::tr("time_wait"));
    m_state.insert(CDataSource::CONNECTION_TCP_CLOSE,         QObject::tr("unconnected"));
    m_state.insert(CDataSource::CONNECTION_TCP_CLOSE_WAIT,    QObject::tr("close_wait"));
    m_state.insert(CDataSource::CONNECTION_TCP_LAST_ACK,      QObject::tr("last_ack"));
    m_state.insert(CDataSource::CONNECTION_TCP_LISTEN,        QObject::tr("listen"));
    m_state.insert(CDataSource::CONNECTION_TCP_CLOSING,       QObject::tr("closing"));
    m_state.insert(CDataSource::CONNECTION_TCP_CLOSED,        QObject::tr("closed"));
    m_state.insert(CDataSource::CONNECTION_REMOVED,           QObject::tr("removed"));
}

QString CConnectionsTree::GetStateString(uint state)
{
    auto it= m_state.find((CDataSource::ConnectionTcpState)state);
    return it == m_state.end() ? QString::number(state) : it.value();
}

void CConnectionsTree::UpdateData(CDataSource::SocketInfo * dataSource, bool disableCleanup)
{

    char guid[64];
    uuid_unparse(dataSource->uuid, guid);
    QUuid uuid(guid);

    auto src = qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel());
    if(src == nullptr)
        throw std::runtime_error("Error Update Proxy Data qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel())");

    auto start = src->index(0, CDataSource::COLUMN_DATA_DATA);
    auto pParent = src->parent(start);
    auto totalRow = src->rowCount(pParent);
    auto value = uuid.toString();
    QModelIndex Item;
    for (int rowTmp = 0; rowTmp < totalRow; ++rowTmp)
    {
        auto idx = src->index(rowTmp, CDataSource::COLUMN_DATA_DATA, pParent);
        if (!idx.isValid())
            continue;

        if (src->data(idx).toString().compare(value, Qt::CaseInsensitive) == 0)
        {
            Item = idx;
            break;
        }
    }

    if(!disableCleanup && Item.isValid() && dataSource->deleteItem > 0)
    {
       src->removeRow(Item.row());
       return;
    }

    if(Item.isValid())
    {

        auto iRow = Item.row();
        if(dataSource->stateUpdate)
        {
            dataSource->stateUpdate = false;
            src->setData(src->index(iRow, CDataSource::COLUMN_DATA_STATE), GetStateString(dataSource->state));
        }

        if(dataSource->commandUpdate)
        {
            dataSource->commandUpdate = false;
            src->setData(src->index(iRow, CDataSource::COLUMN_DATA_COMMAND), dataSource->Command);
        }

    }
    else
    {

        auto iRow = src->rowCount();
        src->insertRows(iRow, 1);

        auto tmpTime = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss.zzz");

        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_PROTOCOL),        m_protocol[dataSource->netType]);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_LOCALHOST),       dataSource->localHost);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_LOCALADDRESS),    dataSource->localAddr);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_LOCALPORT),       dataSource->localPort);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_STATE),           GetStateString(dataSource->state));
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_REMOTEHOST),      dataSource->remoteHost);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_REMOTEADDRESS),   dataSource->remoteAddr);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_REMOTEPORT),      dataSource->remotePort);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_UID),             CUserName::GetUserNameString(dataSource->uid));
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_INODE),           QString::number(dataSource->inode));
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_TIME),            tmpTime);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_COMMAND),         dataSource->Command);
        src->setData(src->index(iRow, CDataSource::COLUMN_DATA_DATA),            uuid);

        dataSource->commandUpdate = false;

    }

}

void CConnectionsTree::InitConnectonsTree(QTreeView *connView)
{

    connView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto pProxyModel = new CCustomProxyModel(connView);
    m_pProxyModel = pProxyModel;

    auto pStandardModel = new QStandardItemModel(0, m_caption.size(), connView);

    //1) adjust column width
    QMapIterator<CDataSource::ColumnData, TableHeaderCaption> tmpCaptionName(m_caption);
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        pStandardModel->setHeaderData(tmpCaptionName.key(), Qt::Horizontal, tmpCaptionName.value().captionSample());
    }

    connView->setModel(pProxyModel);
    pProxyModel->setSourceModel(pStandardModel);

    tmpCaptionName.toFront();
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        connView->resizeColumnToContents(tmpCaptionName.key());
    }

    //2) set name & tooltip
    auto index = 0;
    tmpCaptionName.toFront();
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        pStandardModel->setHeaderData(tmpCaptionName.key(), Qt::Horizontal, tmpCaptionName.value().caption());
        pStandardModel->horizontalHeaderItem(index++)->setToolTip(tmpCaptionName.value().captionToolTip());
    }

    pProxyModel->setDynamicSortFilter(true);


}

void CConnectionsTree::UpdateIncludeFilter(const QString &value)
{
    m_pProxyModel->setFilterRegExpEx(value);
}

void CConnectionsTree::SetDataColumnHiden(QTreeView *tree)
{

    for(auto idx = 0; idx < CDataSource::COLUMN_DATA_DATA; idx++)
        tree->setColumnHidden(idx, false);

    tree->setColumnHidden(CDataSource::COLUMN_DATA_DATA, true);

}

void CConnectionsTree::Save(QWidget *parent)
{

    auto docsLocation = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    auto docPath = (docsLocation.isEmpty()) ? "~/" : QString("%1").arg(docsLocation.first());

    QStringList filters;
    filters << "XML (*.xml)" << "CSV (*.csv)";

    QFileDialog dialog(parent, QObject::tr("Save as ..."), docPath);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilters(filters);

    if (dialog.exec() == QDialog::Accepted)
    {

        auto selectedFilter = dialog.selectedNameFilter();
        auto fileName = dialog.selectedFiles()[0];

        try
        {

            CFileSaveWrapper file;
            file.Open(fileName, selectedFilter, parent);

            auto src = qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel());
            if(src == nullptr)
                throw std::runtime_error("Error qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel())");

            for(auto row = 0; row < src->rowCount(); row++)
            {
                file.ElemStart("r");
                auto colmax = src->columnCount();

                for(int col = 0; col < colmax; col++)
                {
                    file.ElemStart("c");
                    file.ElemVal(src->data(src->index(row, col)).toString());
                    file.ElemEnd("c");
                }
                file.ElemEnd("r");
            }

            file.Close();

        }
        catch(std::exception &e)
        {
            QMessageBox::critical(parent, QObject::tr("Save"), e.what(), QMessageBox::Ok);
        }
        catch(...)
        {
            QMessageBox::critical(parent, QObject::tr("Save"), QObject::tr("Unexpected exception"), QMessageBox::Ok);
        }
    }
}



