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

#include <QtWidgets>

#include "window.h"

#include <chrono>
#include <thread>
#include <unistd.h>

Window::Window()
    : m_captureEnable(false)
    , m_fontSize(-1)
{

    InitStringMap();

    QState *offCapture = new QState();
    offCapture->setObjectName("offCapture");
    offCapture->assignProperty(&m_buttonCaptureMode, "text", "REC");
    offCapture->assignProperty(&m_buttonCaptureMode, "toolTip", QObject::tr("Stop delete old items"));

    QState *onCapture = new QState();
    onCapture->setObjectName("onCapture");
    onCapture->assignProperty(&m_buttonCaptureMode, "text", "REC*");
    onCapture->assignProperty(&m_buttonCaptureMode, "toolTip", QObject::tr("Continue delete old items"));

    offCapture->addTransition(&m_buttonCaptureMode, SIGNAL(clicked()), onCapture);
    onCapture->addTransition(&m_buttonCaptureMode, SIGNAL(clicked()), offCapture);

    connect(&m_buttonCaptureMode, SIGNAL(clicked()), this, SLOT(captureMode()));

    m_buttonCaptureMode.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_buttonCaptureMode.setCheckable(true);

    m_qstateCaptureMode.addState(offCapture);
    m_qstateCaptureMode.addState(onCapture);
    m_qstateCaptureMode.setInitialState(offCapture);
    m_qstateCaptureMode.start();

    QState *offPause = new QState();
    offPause->setObjectName("offPause");
    offPause->assignProperty(&m_buttonPause, "text", "||");
    offPause->assignProperty(&m_buttonPause, "toolTip", QObject::tr("Continue load new records"));

    QState *onPause = new QState();
    onPause->setObjectName("onPause");
    onPause->assignProperty(&m_buttonPause, "text", ">");
    onPause->assignProperty(&m_buttonPause, "toolTip", QObject::tr("Stop load new records"));

    offPause->addTransition(&m_buttonPause, SIGNAL(clicked()), onPause);
    onPause->addTransition(&m_buttonPause, SIGNAL(clicked()), offPause);

    connect(&m_buttonPause, SIGNAL(clicked()), this, SLOT(pauseUpdateThread()));

    m_buttonPause.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_buttonPause.setCheckable(true);

    m_qstatePause.addState(offPause);
    m_qstatePause.addState(onPause);
    m_qstatePause.setInitialState(onPause);
    m_qstatePause.start();

    m_TreeView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_TreeView.setRootIsDecorated(false);
    m_TreeView.setAlternatingRowColors(true);
    m_TreeView.setModel(&m_ProxyModel);
    m_TreeView.setSortingEnabled(true);
    m_TreeView.sortByColumn(0, Qt::AscendingOrder);
    m_TreeView.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&m_TreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    m_filterPatternLabel.setBuddy(&m_filterPatternLineEdit);
    m_filterPatternLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_filterSyntaxComboBox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_filterSyntaxLabel.setBuddy(&m_filterSyntaxComboBox);
    m_filterSyntaxLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_filterColumnComboBox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_filterColumnLabel.setBuddy(&m_filterColumnComboBox);
    m_filterColumnLabel.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    connect(&m_filterPatternLineEdit, &QLineEdit::textChanged, this, &Window::filterRegExpChanged);

    typedef void (QComboBox::*QComboIntSignal)(int);
    connect(&m_filterSyntaxComboBox, static_cast<QComboIntSignal>(&QComboBox::currentIndexChanged),  this, &Window::filterRegExpChanged);
    connect(&m_filterColumnComboBox, static_cast<QComboIntSignal>(&QComboBox::currentIndexChanged),  this, &Window::filterColumnChanged);
    connect(this, SIGNAL(callUpdateGui()),   this, SLOT(updateGui()));

    m_buttonRunNetstat.setText("netstat");
    m_buttonRunNetstat.setToolTip(QObject::tr("sudo netstat -natuwpe"));
    connect(&m_buttonRunNetstat, SIGNAL (released()),  this, SLOT(RunNetstat()));

    m_buttonRunRootDatasource.setText(QObject::tr("resolver"));
    m_buttonRunRootDatasource.setToolTip(QObject::tr("Display info in \"Command\" for all process"));
    connect(&m_buttonRunRootDatasource, SIGNAL (released()),  this, SLOT(RunRootDatasource()));

    m_zoomSlider.setOrientation(Qt::Orientation::Horizontal);
    m_zoomSlider.setSingleStep(1);
    m_zoomSlider.setPageStep(1);
    m_zoomSlider.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    m_fontSizeList = QFontDatabase::standardSizes();
    m_zoomSlider.setRange(0, m_fontSizeList.length() - 1);
    m_zoomSlider.setToolTip(QObject::tr("Zoom"));

    connect(&m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(SetFontSize(int)));

    QGridLayout *proxyLayout = new QGridLayout;

    int row = 0;
    proxyLayout->addWidget(&m_TreeView,              row, 0, 1, 11);

    row++;
    int col = 0;
    proxyLayout->addWidget(&m_buttonRunRootDatasource, row, col++);
    proxyLayout->addWidget(&m_buttonPause,           row, col++);

    proxyLayout->addWidget(&m_buttonCaptureMode,     row, col++);

    proxyLayout->addWidget(&m_filterPatternLabel,    row, col++);
    proxyLayout->addWidget(&m_filterPatternLineEdit, row, col++);

    proxyLayout->addWidget(&m_filterSyntaxLabel,     row, col++);
    proxyLayout->addWidget(&m_filterSyntaxComboBox,  row, col++);

    proxyLayout->addWidget(&m_filterColumnLabel,     row, col++);
    proxyLayout->addWidget(&m_filterColumnComboBox,  row, col++);

    proxyLayout->addWidget(&m_zoomSlider,            row, col++);
    proxyLayout->addWidget(&m_buttonRunNetstat,        row, col++);


    m_GroupBox.setLayout(proxyLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(&m_GroupBox);

    setLayout(mainLayout);
    setWindowTitle(tr("TcpView"));
    resize(500, 450);

    connect(&m_rootResolver, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcDataStd()));
    connect(&m_rootResolver, SIGNAL(readyReadStandardError()), this, SLOT(readProcDataErr()));

}

void Window::InitStringMap()
{
    //                                                              caption                                             captionSample                   captionToolTip
    m_TableCaption.insert(CDataSource::COLUMN_DATA_PROTOCOL,        TableHeaderCaption(QObject::tr("Protocol"),         QObject::tr("tcp6"),            QObject::tr("Protocol")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_LOCALHOST,       TableHeaderCaption(QObject::tr("Local Host"),       QObject::tr("Local Host"),      QObject::tr("Local Host")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_LOCALADDRESS,    TableHeaderCaption(QObject::tr("Local Address"),    QObject::tr("255.255.255.255"), QObject::tr("Local Address")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_LOCALPORT,       TableHeaderCaption(QObject::tr("Local Port"),       QObject::tr("65535"),           QObject::tr("Local Port")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_STATE,           TableHeaderCaption(QObject::tr("State"),            QObject::tr("ESTABLISHED"),     QObject::tr("State")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_REMOTEHOST,      TableHeaderCaption(QObject::tr("Remote Host"),      QObject::tr("Remote Host"),     QObject::tr("Remote Host")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_REMOTEADDRESS,   TableHeaderCaption(QObject::tr("Remote Address"),   QObject::tr("255.255.255.255"), QObject::tr("Remote Address")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_REMOTEPORT,      TableHeaderCaption(QObject::tr("Remote Port"),      QObject::tr("65535"),           QObject::tr("Remote Port")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_UID,             TableHeaderCaption(QObject::tr("User"),             QObject::tr("0000000"),         QObject::tr("User")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_INODE,           TableHeaderCaption(QObject::tr("Inode"),            QObject::tr("0000000"),         QObject::tr("Pid")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_TIME,            TableHeaderCaption(QObject::tr("Time"),             QObject::tr("2017-01-01 00:00:00"), QObject::tr("Time")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_COMMAND,         TableHeaderCaption(QObject::tr("Command"),          QObject::tr("Command"),         QObject::tr("Command")));
    m_TableCaption.insert(CDataSource::COLUMN_DATA_DATA,            TableHeaderCaption(QObject::tr("RowId"),            QObject::tr("RowId"),           QObject::tr("RowId")));

    m_TableProtocolName.insert(CDataSource::conn_tcp,    QObject::tr("tcp"));
    m_TableProtocolName.insert(CDataSource::conn_udp,    QObject::tr("udp"));
    m_TableProtocolName.insert(CDataSource::conn_tcp6,   QObject::tr("tcp6"));
    m_TableProtocolName.insert(CDataSource::conn_udp6,   QObject::tr("udp6"));
    m_TableProtocolName.insert(CDataSource::conn_raw,    QObject::tr("raw"));
    m_TableProtocolName.insert(CDataSource::conn_raw6,   QObject::tr("raw6"));

    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_EMPTY,         QObject::tr(""));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_ESTABLISHED,   QObject::tr("established"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_SYN_SENT,      QObject::tr("syn_sent"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_SYN_RECV,      QObject::tr("syn_recv"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_FIN_WAIT1,     QObject::tr("fin_wait1"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_FIN_WAIT2,     QObject::tr("fin_wait2"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_TIME_WAIT,     QObject::tr("time_wait"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_CLOSE,         QObject::tr("unconnected"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_CLOSE_WAIT,    QObject::tr("close_wait"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_LAST_ACK,      QObject::tr("last_ack"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_LISTEN,        QObject::tr("listen"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_CLOSING,       QObject::tr("closing"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_TCP_CLOSED,        QObject::tr("closed"));
    m_TableConnectionTcpState.insert(CDataSource::CONNECTION_REMOVED,           QObject::tr("removed"));

}

void Window::InitControlsText()
{

    m_filterPatternLabel.setText(tr(" Filter pattern:"));
    m_filterSyntaxLabel.setText(tr(" Filter &syntax:"));
    m_filterColumnLabel.setText(tr(" Filter &column:"));
    //m_GroupBox.setTitle(tr("Connections list"));

    m_filterPatternLineEdit.setText("");

    m_filterSyntaxComboBox.addItem(tr("Regular expression"), QRegExp::RegExp);
    m_filterSyntaxComboBox.addItem(tr("Wildcard"), QRegExp::Wildcard);
    m_filterSyntaxComboBox.addItem(tr("Fixed string"), QRegExp::FixedString);
    m_filterSyntaxComboBox.setCurrentIndex(2);

    auto model = new QStandardItemModel(0, m_TableCaption.size(), this);

    QMapIterator<CDataSource::ColumnData, TableHeaderCaption> tmpCaptionName(m_TableCaption);
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        model->setHeaderData(tmpCaptionName.key(), Qt::Horizontal, tmpCaptionName.value().captionSample());
        m_filterColumnComboBox.addItem(tmpCaptionName.value().caption(), tmpCaptionName.key());
    }

    m_filterColumnComboBox.setCurrentIndex(0);

    m_ProxyModel.setSourceModel(model);

    tmpCaptionName.toFront();
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        m_TreeView.resizeColumnToContents(tmpCaptionName.key());
    }
    auto index = 0;
    tmpCaptionName.toFront();
    while (tmpCaptionName.hasNext())
    {
        tmpCaptionName.next();
        model->setHeaderData(tmpCaptionName.key(), Qt::Horizontal, tmpCaptionName.value().caption());
        model->horizontalHeaderItem(index++)->setToolTip(tmpCaptionName.value().captionToolTip());
    }

    m_NetData.setUpdateCallback([&](void)->void{ emit callUpdateGui(); });

}

void Window::filterRegExpChanged()
{

    QRegExp::PatternSyntax syntax =  QRegExp::PatternSyntax(m_filterSyntaxComboBox.itemData(m_filterSyntaxComboBox.currentIndex()).toInt());

    QRegExp regExp(m_filterPatternLineEdit.text(), Qt::CaseInsensitive, syntax);

    m_ProxyModel.setFilterRegExp(regExp);

}

void Window::filterColumnChanged()
{
    m_ProxyModel.setFilterKeyColumn(m_filterColumnComboBox.currentIndex());
}

void Window::closeEvent(QCloseEvent *event)
{
    m_NetData.DeleteRootLoader();
    SaveAppState();
    event->accept();
    //event->ignore();
}

void Window::RestoreAppState()
{

    setWindowIcon(QPixmap(":/data/tcpviewb.svg"));

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    auto testv = settings.value("baseWindow/geometry");

    if(!testv.isValid() || testv.isNull())
    {
        HideDataColumn();
        return;
    }

    restoreGeometry(settings.value("baseWindow/geometry").toByteArray());
    m_TreeView.header()->restoreState(settings.value("baseTable/headerState", "").toByteArray());

    HideDataColumn();

}

void Window::SetFontSize(int fontSizeIndex)
{
    if(m_fontSize < 1)
        return;

    if(fontSizeIndex >= m_fontSizeList.length())
        fontSizeIndex = m_fontSizeList.length() - 1;

    auto font = m_TreeView.font();
    font.setPointSize(m_fontSizeList[fontSizeIndex]);
    m_TreeView.setFont(font);

}

void Window::HideDataColumn()
{

    for(auto idx = 0; idx < CDataSource::COLUMN_DATA_DATA; idx++)
        m_TreeView.setColumnHidden(idx, false);

    m_TreeView.setColumnHidden(CDataSource::COLUMN_DATA_DATA, true);

}

void Window::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );

    try
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        auto initialFontSize = settings.value("baseWindow/zoomSlider", m_TreeView.font().pointSize());
        auto fontSize = initialFontSize.toInt();

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
        {
            m_zoomSlider.setValue(index);
            m_fontSize = fontSize;
            SetFontSize(index);
        }
    }
    catch(...)
    {

    }
}

void Window::SaveAppState()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("baseWindow/geometry", saveGeometry());
    settings.setValue("baseTable/headerState", m_TreeView.header()->saveState());
    settings.setValue("baseWindow/zoomSlider", m_TreeView.font().pointSize());
}

void Window::updateTreeView(CDataSource::SocketInfo * dataSource, bool disableCleanup)
{

    char guid[64];
    uuid_unparse(dataSource->uuid, guid);
    QUuid uuid(guid);

    auto idx = m_ProxyModel.index(0, CDataSource::COLUMN_DATA_DATA);
    auto Items = m_ProxyModel.match(
                idx,
                Qt::EditRole,
                uuid,
                1,
                Qt::MatchRecursive /*| Qt::MatchFixedString*/);

    if(!disableCleanup && !Items.isEmpty() && dataSource->deleteItem > 0)
    {
       m_ProxyModel.removeRow(Items.constFirst().row());
       return;
    }

    if(!Items.isEmpty())
    {
        auto iRow = Items.constFirst().row();
        if(dataSource->stateUpdate)
        {
            dataSource->stateUpdate = false;
            auto it= m_TableConnectionTcpState.find((CDataSource::ConnectionTcpState)dataSource->state);
            QString State = it == m_TableConnectionTcpState.end() ? QString::number(dataSource->state) : it.value();
            m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_STATE), State);
        }

        if(dataSource->commandUpdate)
        {
            dataSource->commandUpdate = false;
            m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_COMMAND), dataSource->Command);
        }

    }
    else
    {

        auto it= m_TableConnectionTcpState.find((CDataSource::ConnectionTcpState)dataSource->state);
        QString State = it == m_TableConnectionTcpState.end() ? QString::number(dataSource->state) : it.value();

        auto iRow = m_ProxyModel.rowCount();
        m_ProxyModel.insertRows(iRow, 1);

        auto tmpTime = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");

        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_PROTOCOL),        m_TableProtocolName[dataSource->netType]);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_LOCALHOST),       dataSource->localHost);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_LOCALADDRESS),    dataSource->localAddr);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_LOCALPORT),       dataSource->localPort);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_STATE),           State);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_REMOTEHOST),      dataSource->remoteHost);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_REMOTEADDRESS),   dataSource->remoteAddr);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_REMOTEPORT),      dataSource->remotePort);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_UID),             QString::fromStdString(CDataSource::makeUserNameStr(dataSource->uid)));
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_INODE),           QString::number(dataSource->inode));
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_TIME),            tmpTime);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_COMMAND),         dataSource->Command);
        m_ProxyModel.setData(m_ProxyModel.index(iRow, CDataSource::COLUMN_DATA_DATA),            uuid);

    }

}

void Window::updateGui()
{

    auto dataHelper = m_NetData.GetData();
    if(dataHelper != nullptr)
    {

        /* start update code here */
        auto captureEnable = m_captureEnable;

        m_ProxyModel.setDynamicSortFilter(false);

        auto connList = dataHelper->GetConnectionsList();
        for (auto it = connList->begin(); it != connList->end(); it++ )
        {
            updateTreeView(&it->second, captureEnable);
        }

        if(!captureEnable)
        {
            for (auto it = connList->begin(); it != connList->end(); )
            {
                if (it->second.deleteItem > 0)
                    it = connList->erase(it);
                else
                    ++it;
            }
        }

        m_NetData.EnableUpdateData();

        m_ProxyModel.setDynamicSortFilter(true);

        /* end update code here */

    }

}

void Window::pauseUpdateThread()
{
    m_NetData.pauseUpdate(m_buttonPause.isChecked());
}

void Window::captureMode()
{
    m_captureEnable = m_buttonCaptureMode.isChecked();
}

void Window::onCustomContextMenu(const QPoint &point)
{

    auto selCell = m_TreeView.indexAt(point);
    if (!selCell.isValid())
    {
        return;
    }

    m_ProxyModel.setDynamicSortFilter(false);
    auto cellText = selCell.data().toString();

    auto whoisText = QString();

    auto rowText = QString();
    for(auto index = 0; index < CDataSource::COLUMN_DATA_DATA; index++)
    {
        rowText += m_ProxyModel.data(m_ProxyModel.index(selCell.row(), index)).toString() + "\t";
        if(index == CDataSource::COLUMN_DATA_REMOTEADDRESS)
        {
            whoisText = m_ProxyModel.data(m_ProxyModel.index(selCell.row(), index)).toString();
        }
    }

    m_ProxyModel.setDynamicSortFilter(true);

    QMenu menu;

    auto action1 = new QAction(QObject::tr("Copy whole row"), &menu);
    action1->setData(1);
    menu.addAction(action1);

    if(!cellText.isNull() && !cellText.isEmpty())
    {
        auto action2 = new QAction(QObject::tr("Copy \"") + cellText + QObject::tr("\""), &menu);
        action2->setData(2);
        menu.addAction(action2);
    }

    auto action3 = new QAction(QObject::tr("Whois \"") + whoisText + QObject::tr("\""), &menu);
    action3->setData(3);
    menu.addAction(action3);

    if(!whoisText.isNull() && !whoisText.isEmpty() &&
            QString::compare(whoisText, "0.0.0.0", Qt::CaseInsensitive) != 0 &&
            QString::compare(whoisText, "::", Qt::CaseInsensitive) != 0)
    {
        action3->setEnabled(true);
    }
    else
    {
        action3->setEnabled(false);
    }

    auto ret = menu.exec(QCursor::pos());
    if(ret == nullptr)
        return;

    if(ret->data() == 1)
    {
        auto pClp = QApplication::clipboard( );
        if( pClp != nullptr )
            pClp->setText(rowText);
    }
    else if(ret->data() == 2)
    {
        auto pClp = QApplication::clipboard();
        if( pClp != nullptr )
            pClp->setText(cellText);
    }
    else if(ret->data() == 3)
    {
        ShowWhois(rowText, whoisText);
    }
}

void Window::ShowInfoDialog(QString title, QString dialogText, bool readonly)
{
    auto infoDialog = new QDialog(this);
    infoDialog->setWindowTitle(title);
    infoDialog->setWindowFlags(infoDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto infoEditor = new QTextEdit;
    infoEditor->setLineWrapMode(QTextEdit::NoWrap);
    infoEditor->setPlainText(dialogText);

    if(readonly)
    {
        infoEditor->setReadOnly(true);
        //infoEditor->setTextInteractionFlags(bigEditor->textInteractionFlags() | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    }

    QFont newfont("monospace");
    newfont.setStyleHint(QFont::Monospace);
    newfont.setPointSize((m_TreeView.font()).pointSize());
    infoEditor->setFont(newfont);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoEditor );
    infoDialog->setLayout(mainLayout);

    infoDialog->setAttribute(Qt::WA_DeleteOnClose);// ?
    auto newsize = this->size();

    infoDialog->resize(newsize.width()/2, newsize.height()/2);
    infoDialog->show();
    infoDialog->raise();
    infoDialog->activateWindow();
}

void Window::ShowWhois(QString rowText, QString whoisText)
{
    rowText.replace("\t"," ");
    rowText += "\n";

    QProcess exec;
    QString commandString = "whois " + whoisText;
    exec.start(commandString);
    exec.waitForFinished();
    rowText += exec.readAllStandardOutput();
    ShowInfoDialog(commandString, rowText, true);
}

void Window::RunNetstat()
{
    QProcess exec;
    QString commandString = "pkexec netstat -natuwpe";
    exec.start(commandString);
    exec.waitForFinished();
    auto result = exec.readAllStandardOutput();

    result = "sudo netstat -natuwpe\n" + result;

    ShowInfoDialog("netstat", result, true);
}

void Window::RunRootDatasource()
{

    m_buttonRunRootDatasource.setEnabled(false);

    m_rootResolver.start( "pkexec " + QCoreApplication::applicationFilePath() + " --rootmodule");

    m_rootResolver.waitForStarted();

    if(!m_NetData.InitRootLoader())
    {
        m_buttonRunRootDatasource.setEnabled(true);
        std::cout << "Connect failed" << std::endl;
        return;
    }

}

void Window::readProcDataStd()
{
    std::cout << m_rootResolver.readAllStandardOutput().toStdString() << std::endl;
}
void Window::readProcDataErr()
{
    std::cout << m_rootResolver.readAllStandardError().toStdString() << std::endl;
}
