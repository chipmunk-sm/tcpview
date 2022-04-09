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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QTextEdit>
#include <QClipboard>
#include <QMenu>
#include <QStandardItemModel>
#include <QToolTip>
#include <QDebug>
#include <QScroller>
#include <QGestureEvent>
#include <QScrollBar>

#if ((QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 15) || QT_VERSION_MAJOR >= 6)
#   include <QScreen>
#else
#   include <QDesktopWidget>
#   define OLDVERSION
#endif

#include <thread>

#include <source/configdialog.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_captureEnable(false)
    , m_visibleItems(0)
    , m_totalItems(0)

{

    ui->setupUi(this);

    CCFontSize::changeFontSize(0.0, this);

    setWindowIcon(QPixmap(":/data/tcpviewU.svg"));

    m_cconnectionstree.InitConnectonsTree(ui->treeView_connection);

    connect(this, SIGNAL(callUpdateGui()), this, SLOT(updateGui()));

    connect(ui->treeView_connection->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentSelectionChanged(QModelIndex,QModelIndex)));

    ui->pushButton_whois->setEnabled(false);

    CDataSource::Instance().Init([&](void)->void{ emit callUpdateGui(); });

    CDataSource::Instance().setPauseUpdate(false);



    auto pObj1 = this;
    pObj1->grabGesture(Qt::PinchGesture);
    //pObj1->grabGesture(Qt::PanGesture);
    //pObj1->grabGesture(Qt::SwipeGesture);

    auto pObj = ui->treeView_connection->viewport();
    ui->treeView_connection->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->treeView_connection->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScroller::grabGesture(pObj, QScroller::LeftMouseButtonGesture);

    QScrollerProperties properties = QScroller::scroller(pObj)->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                               QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff));
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                               QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff));
    QScroller::scroller(pObj)->setScrollerProperties(properties);

#if ((QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 9) || QT_VERSION_MAJOR >= 6)

#endif


    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    //QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    ui->treeView_connection->header()->restoreState(settings.value(DEFCFG_CONNECTIONTABLE, "").toByteArray());

#if defined (Q_OS_ANDROID)

#else
#ifdef OLDVERSION
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        //QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
        const QByteArray restoredGeometry = settings.value(QLatin1String(MAIN_WINDOW_LAYOUT)).toByteArray();
        if (restoredGeometry.isEmpty() || !restoreGeometry(restoredGeometry)) {
            const QRect availableGeometry = QApplication::desktop()->availableGeometry();
            const QSize size = (availableGeometry.size() * 4) / 5;
            resize(size);
            move(availableGeometry.center() - QPoint(size.width(), size.height()) / 2);
        }
    }
#else
    readGeometry();
#endif
#endif

}

MainWindow::~MainWindow()
{
    CDataSource::Instance().setAbort();
#if defined (Q_OS_ANDROID)

#else
#ifdef OLDVERSION
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    //    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(QLatin1String(MAIN_WINDOW_LAYOUT), saveGeometry());
    settings.setValue(DEFCFG_CONNECTIONTABLE, ui->treeView_connection->header()->saveState());
#endif
#endif
    delete ui;
}

void MainWindow::on_pushButton_Pause_toggled(bool checked)
{
    CDataSource::Instance().setPauseUpdate(checked);
}

void MainWindow::on_pushButton_Record_toggled(bool checked)
{
    m_captureEnable = checked;
}

void MainWindow::on_pushButton_copyToClipboard_clicked()
{
    auto pClp = QApplication::clipboard();
    if( pClp != nullptr )
        pClp->setText(m_ClipBoardString);
    tooltipText("Copied");
}

void MainWindow::on_pushButton_SaveToFile_clicked()
{
    auto pause = CDataSource::Instance().pauseUpdate();
    CDataSource::Instance().setPauseUpdate(true);
    m_cconnectionstree.Save(ui->treeView_connection);
    CDataSource::Instance().setPauseUpdate(pause);
}

void MainWindow::on_pushButton_whois_clicked()
{
    auto rowText = m_RowText;
    auto whoisText = m_whoisText;

    rowText.replace("\t"," ");
    rowText += "\n\n";

    QString whoisOutput;
    QString err;
    QString commandString = "whois " + whoisText;

    try {
        QProcess exec;
        //exec.start(commandString);
        exec.start("whois", QStringList(whoisText));

        exec.waitForFinished();
        whoisOutput = QString(exec.readAllStandardOutput());
    } catch(const std::exception &e) {
        err = QString(e.what());
    } catch(...) {
        err = QObject::tr("Unexpected exception");
    }

    if(whoisOutput.isEmpty())
    {
        if(err.isEmpty())
        {
            whoisOutput = QObject::tr("Command 'whois' not found,\n"
                                      "but can be installed with:\n"
                                      "sudo apt install whois");
        }
        else
        {
            whoisOutput = QObject::tr("Failed run 'whois':\n") + err;
        }
    }

    rowText += whoisOutput;

    ShowInfoDialog(commandString, rowText, true);

}

void MainWindow::on_pushButton_Settings_clicked()
{
    ui->pushButton_Settings->setEnabled(false);
    auto cfg = new ConfigDialog( [&](void)->void { UpdateConfig(); }, [&](void)->void { CloseConfig(); } );
    cfg->setWindowTitle(windowTitle() + " " + tr("Settings"));
    cfg->setAttribute (Qt::WA_DeleteOnClose);
    auto flags = cfg->windowFlags() & (~Qt::WindowContextHelpButtonHint);
    cfg->setWindowFlags(flags);
    cfg->setWindowIcon(windowIcon());
    cfg->show();
}

void MainWindow::on_lineEdit_include_textChanged(const QString &arg1)
{
    m_cconnectionstree.UpdateIncludeFilter(arg1);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    auto bControl = (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
    if (bControl)
    {
        const auto numPixels = event->pixelDelta();
        const auto numDegrees = event->angleDelta();

        if (!numPixels.isNull()) {
            auto fontSize = CCFontSize::changeFontSize((numPixels.y() < 0.0) ? -1 : 1, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        } else if (!numDegrees.isNull()) {
            auto fontSize = CCFontSize::changeFontSize((numDegrees.y() < 0.0) ? -1 : 1, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        }

        event->accept();
        return;
    }
    QMainWindow::wheelEvent(event);
}

bool MainWindow::event(QEvent *event) {
    if (event->type() == QEvent::Gesture) {
        auto pGestureEvent = dynamic_cast<QGestureEvent *>(event);
        if (pGestureEvent) {
            auto pPinchGesture = pGestureEvent->gesture(Qt::PinchGesture);
            if (pPinchGesture) {
                return gestureEventPinch(static_cast<QPinchGesture *>(pPinchGesture));
            }
        }
    }
    return QWidget::event(event);
}

bool MainWindow::gestureEventPinch(QPinchGesture *gesture) {

    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if ((changeFlags & QPinchGesture::ScaleFactorChanged) == QPinchGesture::ScaleFactorChanged) {

        double scale = 0.0;
        double scaleFactor = gesture->scaleFactor() - 1.0;

        if (scaleFactor >= 0.01)
            scale = 1;
        else if (scaleFactor <= -0.01)
            scale = -1;

        if (std::abs(scale) > 0.01) {
            auto fontSize = CCFontSize::changeFontSize(scale, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        }
    }
    return true;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (ui->treeView_connection->verticalScrollBar()->isEnabled() && (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
    {
        ui->treeView_connection->verticalScrollBar()->setEnabled(false);
        toolTip();
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(!ui->treeView_connection->verticalScrollBar()->isEnabled() && (event->modifiers() & Qt::ControlModifier) != Qt::ControlModifier)
    {
        ui->treeView_connection->verticalScrollBar()->setEnabled(true);
        QToolTip::hideText();
    }
    QWidget::keyReleaseEvent(event);
}

void MainWindow::updateGui()
{
    try
    {
        auto captureEnable = m_captureEnable;

        auto connList = CDataSource::Instance().GetConnectionsList();
        for (auto it = connList->begin(); it != connList->end(); it++ )
        {
            m_cconnectionstree.UpdateData(&it->second, captureEnable);
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

        UpdateStatusText();
    }
    catch(std::exception &e)
    {
        QMessageBox::critical(this, this->windowTitle(), e.what(), QMessageBox::Ok);
        exit(-1);
    }
    catch(...)
    {
        QMessageBox::critical(this, this->windowTitle(), "Unexpected exception", QMessageBox::Ok);
        exit(-1);
    }
    CDataSource::Instance().FreeConnectionsList();
}

void MainWindow::onCurrentSelectionChanged(const QModelIndex current, const QModelIndex previous)
{

    Q_UNUSED(previous)

    m_whoisText = "";
    m_RowText = "";

    if(!current.isValid())
    {
        ui->pushButton_whois->setEnabled(false);
        ui->pushButton_whois->setText("Whois");
        return;
    }

    auto row = current.row();

    auto src = qobject_cast<QSortFilterProxyModel*>(ui->treeView_connection->model());
    if(src == nullptr)
    {
        ui->pushButton_whois->setEnabled(false);
        ui->pushButton_whois->setText("Whois");
        return;
    }

    QString tmpToolTipStr;

    for(auto index = 0; index < COLUMN_DATA_COUNT; index++)
    {
        auto tmpStr = src->data(src->index(row, index)).toString();

        m_RowText += tmpStr + "\t";
        tmpToolTipStr +=  tmpStr + "\n";

        if(index == COLUMN_DATA_REMOTEADDRESS)
        {
            auto whoisText = src->data(src->index(row, index)).toString();
            if(QString::compare(whoisText, "0.0.0.0", Qt::CaseInsensitive) != 0 &&
               QString::compare(whoisText, "::", Qt::CaseInsensitive) != 0)
            {
                m_whoisText = whoisText;
                ui->pushButton_whois->setText("Whois \"" + whoisText + "\"");
                ui->pushButton_whois->setEnabled(true);
            }
            else
            {
                ui->pushButton_whois->setEnabled(false);
                ui->pushButton_whois->setText("Whois");
            }
        }
    }

    ui->pushButton_copyToClipboard->setToolTip(tmpToolTipStr);

    m_ClipBoardString = m_RowText;

}

void MainWindow::on_pushButton_resize_clicked() {
    for (int i = 0; i < ui->treeView_connection->model()->columnCount(); ++i)
        ui->treeView_connection->resizeColumnToContents(i);
}

void MainWindow::ShowInfoDialog(QString title, QString dialogText, bool readonly)
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
    }

    QFont newfont("monospace");
    newfont.setStyleHint(QFont::Monospace);
    newfont.setPointSize((this->font()).pointSize());
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

void MainWindow::on_treeView_connection_customContextMenuRequested(const QPoint &pos)
{

    typedef enum MenuAction{
        CopyCell = 1,
        CopyRow
    }MenuAction;

    auto selCell = ui->treeView_connection->indexAt(pos);
    if (!selCell.isValid())
        return;

    auto src = qobject_cast<QSortFilterProxyModel*>(ui->treeView_connection->model());
    if(src == nullptr)
        return;

    src->setDynamicSortFilter(false);

    auto cellText = selCell.data().toString();
    auto rowText = QString();

    for(auto index = 0; index < COLUMN_DATA_COUNT; index++)
        rowText += src->data(src->index(selCell.row(), index)).toString() + "\t";

    src->setDynamicSortFilter(true);

    QMenu menu;

    if(!cellText.isEmpty())
    {
        auto actionCell = new QAction(QObject::tr("Copy") + " \"" + CutLongText(cellText, 80) + "\"", &menu);
        actionCell->setData(MenuAction::CopyCell);
        menu.addAction(actionCell);
    }

    auto actionRow = new QAction(QObject::tr("Copy row"), &menu);
    actionRow->setData(MenuAction::CopyRow);
    menu.addAction(actionRow);

    auto ret = menu.exec(QCursor::pos());
    if(ret == nullptr)
        return;

    if(QApplication::clipboard() == nullptr)
        return;

    if(ret->data() == MenuAction::CopyRow)
    {
        QApplication::clipboard()->setText(rowText);
        return;
    }

    if(ret->data() == MenuAction::CopyCell)
    {
        QApplication::clipboard()->setText(cellText);
        return;
    }

}

bool MainWindow::GetAuthGuiName(QString &command, QStringList &cmdArgs)
{
    QStringList passwordReqApp;
    passwordReqApp << "/usr/bin/pkexec";
    passwordReqApp << "/usr/bin/kdesudo";
    passwordReqApp << "/usr/bin/gksudo";
    passwordReqApp << "/usr/bin/kdesu";
    passwordReqApp << "/usr/bin/gksu";

    foreach(auto tmpv, passwordReqApp)
    {
        auto name = tmpv.split(" ").at(0);
        if( access(name.toLatin1(), X_OK ) == 0 )
        {
            command = tmpv;
            cmdArgs << QCoreApplication::applicationFilePath();
            cmdArgs << "--rootmodule";
            return true;
        }
    }

    auto errorString = QString(QObject::tr("Cannot start module, because \n%1\n is missing or execution access is denied.")).arg(passwordReqApp.join("\n"));
    QMessageBox::critical(this, QObject::tr("Run as root"), errorString, QMessageBox::Ok);

    return false;
}

void MainWindow::on_pushButton_displayNames_clicked()
{

    if (CDataSource::Instance().rootLoaderActive()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(windowTitle());
        msgBox.setText(QObject::tr("Restart?"));
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::Yes) {
            //CDataSource::Instance().shutdownRootModule();
            auto tmp = QCoreApplication::applicationFilePath().toStdString();
            char arr[PATH_MAX] = {0};
            strncpy(arr, tmp.c_str(), std::min(sizeof(arr), tmp.size()));
            char *argv[] = {arr, nullptr};
            execv(tmp.c_str(), argv);
            QCoreApplication::quit();
            return;
        } else {
            return;
            }
    }

    QString command;
    QStringList cmdArgs;
    if(!GetAuthGuiName(command, cmdArgs))
        return;


    qInfo("Run root acces module [%s]", command.toStdString().c_str());

    uuid_t uuid;
    uuid_generate(uuid);

    char guid[65];
    memset(guid, 0, sizeof(guid));
    uuid_unparse(uuid, guid);
    cmdArgs << guid;

    auto rootResolver = new QProcess(this);
    rootResolver->start(command, cmdArgs);
    if(!rootResolver->waitForStarted(-1))
    {
        auto retErrString = rootResolver->errorString();
        QMessageBox::critical(this, QObject::tr("Run as root"), "Failed: \n" + command + "\n" +retErrString, QMessageBox::Ok);
        return;
    }

    if(!CDataSource::Instance().InitRootLoader(guid))
    {
        QMessageBox::critical(this, QObject::tr("Run as root"), QObject::tr("Root module not found"), QMessageBox::Ok);
        return;
    }

    ui->pushButton_displayNames->setText(QObject::tr("Restart"));

}

QString MainWindow::CutLongText(const QString &sourceText, int maxLength)
{

    if(sourceText.length() < maxLength)
        return sourceText;

    auto splitText = sourceText.split(" ");

    QString retText;
    foreach(auto varTmp, splitText)
    {
        if(retText.length() + varTmp.length() > maxLength)
        {
            retText += "...";
            break;
        }
        retText += " " + varTmp;
    }
    return retText;
}

void MainWindow::UpdateStatusText()
{
    auto visibleItems = 0;
    auto totalItems = 0;

    auto proxy = qobject_cast<QSortFilterProxyModel*>(ui->treeView_connection->model());
    if(proxy != nullptr)
    {

        visibleItems = proxy->rowCount();

        auto source = qobject_cast<QStandardItemModel*>(proxy->sourceModel());
        if(source != nullptr)
        {
            totalItems = source->rowCount();
        }

    }

    if(visibleItems == m_visibleItems && totalItems == m_totalItems)
        return;

    m_visibleItems = visibleItems;
    m_totalItems = totalItems;

    auto labelText = QString(tr("Visible %1 / total %2")).arg(visibleItems).arg(totalItems);
    ui->label_status->setText(labelText);
    ui->label_status->setToolTip(labelText);

}

void MainWindow::tooltipText(const QString &text)
{
    if(width() < 1 || height() < 1)
        return;

    const auto toolTipText = QString("<center><b>%1</b></center>").arg(text);
    auto fm = fontMetrics();
    auto boundingRect = fm.boundingRect(QRect(0,0, width(), height()), Qt::TextWordWrap, toolTipText);
    boundingRect.setWidth( boundingRect.width());
    boundingRect.setHeight(boundingRect.height());

    auto xPos =  width() - boundingRect.width()/2;
    auto yPos =  height() - boundingRect.height()/2;

    if(xPos < 1 || yPos < 1)
        return;

    xPos /= 2;
    yPos /= 2;

    QToolTip::showText(this->mapToGlobal(QPoint(xPos, yPos)), toolTipText, this);
}

void MainWindow::UpdateConfig()
{
     auto proxy = qobject_cast<CCustomProxyModel*>(ui->treeView_connection->model());
     if (proxy != nullptr)
         proxy->updateColorMap();

     CCFontSize::changeFontSize(0.0, this);

     ui->treeView_connection->viewport()->update();
}

void MainWindow::CloseConfig()
{
    ui->pushButton_Settings->setEnabled(true);
}

void MainWindow::readGeometry()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("MainWindow/geometry", saveGeometry());
}
