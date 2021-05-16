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
}

CConnectionsTree::~CConnectionsTree()
{
    delete m_pProxyModel;
}

void CConnectionsTree::PrepareTableCaptionString()
{
    //                                                              caption                                     captionSample                   captionToolTip
    m_caption.insert(COLUMN_DATA_PROTOCOL,        TableHeaderCaption(QObject::tr("Protocol"),         QObject::tr("tcp6"),                QObject::tr("Protocol")));
    m_caption.insert(COLUMN_DATA_LOCALHOST,       TableHeaderCaption(QObject::tr("Local Host"),       QObject::tr("Local Host"),          QObject::tr("Local Host")));
    m_caption.insert(COLUMN_DATA_LOCALADDRESS,    TableHeaderCaption(QObject::tr("Local Address"),    QObject::tr("255.255.255.255"),     QObject::tr("Local Address")));
    m_caption.insert(COLUMN_DATA_LOCALPORT,       TableHeaderCaption(QObject::tr("Local Port"),       QObject::tr("65535"),               QObject::tr("Local Port")));
    m_caption.insert(COLUMN_DATA_STATE,           TableHeaderCaption(QObject::tr("State"),            QObject::tr("ESTABLISHED"),         QObject::tr("State")));
    m_caption.insert(COLUMN_DATA_REMOTEHOST,      TableHeaderCaption(QObject::tr("Remote Host"),      QObject::tr("Remote Host"),         QObject::tr("Remote Host")));
    m_caption.insert(COLUMN_DATA_REMOTEADDRESS,   TableHeaderCaption(QObject::tr("Remote Address"),   QObject::tr("255.255.255.255"),     QObject::tr("Remote Address")));
    m_caption.insert(COLUMN_DATA_REMOTEPORT,      TableHeaderCaption(QObject::tr("Remote Port"),      QObject::tr("65535"),               QObject::tr("Remote Port")));
    m_caption.insert(COLUMN_DATA_UID,             TableHeaderCaption(QObject::tr("User"),             QObject::tr("0000000"),             QObject::tr("User")));
    m_caption.insert(COLUMN_DATA_INODE,           TableHeaderCaption(QObject::tr("Inode"),            QObject::tr("0000000"),             QObject::tr("Inode")));
    m_caption.insert(COLUMN_DATA_TIME,            TableHeaderCaption(QObject::tr("Time"),             QObject::tr("2017-01-01 00:00:00"), QObject::tr("Time")));
    m_caption.insert(COLUMN_DATA_COMMAND,         TableHeaderCaption(QObject::tr("Process path"),     QObject::tr("Process path"),        QObject::tr("Process path")));
}

void CConnectionsTree::PrepareTableProtocolString()
{
    m_protocol.insert(conn_tcp,    QObject::tr("tcp"));
    m_protocol.insert(conn_udp,    QObject::tr("udp"));
    m_protocol.insert(conn_tcp6,   QObject::tr("tcp6"));
    m_protocol.insert(conn_udp6,   QObject::tr("udp6"));
    m_protocol.insert(conn_raw,    QObject::tr("raw"));
    m_protocol.insert(conn_raw6,   QObject::tr("raw6"));
}

void CConnectionsTree::UpdateData(SocketInfo * dataSource, bool disableCleanup)
{

    char guid[64];
    uuid_unparse(dataSource->uuid, guid);
    QUuid uuid(guid);

    auto src = qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel());
    if(src == nullptr)
        throw std::runtime_error("Error Update Proxy Data qobject_cast<QStandardItemModel*>(m_pProxyModel->sourceModel())");

    auto start = src->index(0, COLUMN_DATA_STATE);
    auto pParent = src->parent(start);
    auto totalRow = src->rowCount(pParent);
    QModelIndex Item;
    for (int rowTmp = 0; rowTmp < totalRow; ++rowTmp)
    {
        auto idx = src->index(rowTmp, COLUMN_DATA_STATE, pParent);
        if (!idx.isValid())
            continue;
        auto rowid = src->data(idx, DataTyp::DataTyp_TypId).toUuid();
        if (rowid == uuid)
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
            src->setData(src->index(iRow, COLUMN_DATA_STATE), m_ConnectionStateHelper.getStateName(static_cast<eConnectionTcpState>(dataSource->state)));
            src->setData(src->index(iRow, COLUMN_DATA_STATE), dataSource->state, DataTyp::DataTyp_TypState);
            emit src->dataChanged(src->index(iRow, 0), src->index(iRow, src->columnCount() - 1));
        }

        if(dataSource->commandUpdate)
        {
            dataSource->commandUpdate = false;
            src->setData(src->index(iRow, COLUMN_DATA_COMMAND), dataSource->command);
        }
    }
    else
    {
        auto iRow = src->rowCount();
        src->insertRows(iRow, 1);

        auto tmpTime = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss.zzz");

        src->setData(src->index(iRow, COLUMN_DATA_PROTOCOL),        m_protocol[dataSource->netType]);
        src->setData(src->index(iRow, COLUMN_DATA_LOCALHOST),       dataSource->localHost);
        src->setData(src->index(iRow, COLUMN_DATA_LOCALADDRESS),    dataSource->localAddr);
        src->setData(src->index(iRow, COLUMN_DATA_LOCALPORT),       dataSource->localPort);
        src->setData(src->index(iRow, COLUMN_DATA_STATE),           m_ConnectionStateHelper.getStateName(static_cast<eConnectionTcpState>(dataSource->state)));
        src->setData(src->index(iRow, COLUMN_DATA_STATE),           dataSource->state, DataTyp::DataTyp_TypState);
        src->setData(src->index(iRow, COLUMN_DATA_STATE),           uuid, DataTyp::DataTyp_TypId);
        src->setData(src->index(iRow, COLUMN_DATA_REMOTEHOST),      dataSource->remoteHost);
        src->setData(src->index(iRow, COLUMN_DATA_REMOTEADDRESS),   dataSource->remoteAddr);
        src->setData(src->index(iRow, COLUMN_DATA_REMOTEPORT),      dataSource->remotePort);
        src->setData(src->index(iRow, COLUMN_DATA_UID),             CUserName::GetUserNameString(dataSource->uid, dataSource->inode == 0));
        src->setData(src->index(iRow, COLUMN_DATA_INODE),           QString::number(dataSource->inode));
        src->setData(src->index(iRow, COLUMN_DATA_TIME),            tmpTime);
        src->setData(src->index(iRow, COLUMN_DATA_COMMAND),         dataSource->command);

        dataSource->commandUpdate = false;
        dataSource->stateUpdate = false;
    }

}

void CConnectionsTree::InitConnectonsTree(QTreeView *connView)
{

    connView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto pStandardModel = new QStandardItemModel(0, m_caption.size(), connView);
    auto pProxyModel = new CCustomProxyModel(connView);
    m_pProxyModel = pProxyModel;
    connView->setModel(pProxyModel);
    pProxyModel->setSourceModel(pStandardModel);

    //1) adjust column width
    QMapIterator<eColumnData, TableHeaderCaption> tmpCaptionName(m_caption);
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        pStandardModel->setHeaderData(tmpCaptionName.key(), Qt::Horizontal, tmpCaptionName.value().captionSample());
    }

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

//void CConnectionsTree::SetDataColumnHiden(QTreeView *tree)
//{
//    for(auto idx = 0; idx < COLUMN_DATA_DATA; idx++)
//        tree->setColumnHidden(idx, false);
//    tree->setColumnHidden(COLUMN_DATA_DATA, true);
//}

void CConnectionsTree::Save(QWidget *parent)
{

    auto docsLocation = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    auto docPath = (docsLocation.isEmpty()) ? "~/" : QString("%1").arg(docsLocation.first());

    QStringList filters;
    filters << "XML (*.xml)" << "CSV (*.csv)";

    QFileDialog dialog(parent, QObject::tr("Save as ..."), docPath);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilters(filters);

    if (dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().empty())
    {

        auto selectedFilter = dialog.selectedNameFilter();
        const auto fileName = dialog.selectedFiles().constFirst();

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



