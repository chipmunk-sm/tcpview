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

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QGroupBox>
#include <QTreeView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStateMachine>
#include <QTextEdit>
#include <QProcess>

#include "tableheadercaption.h"
#include "updatethread.h"

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();
    void InitStringMap();
    void InitControlsText();
    void RestoreAppState();
    void SaveAppState();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void filterRegExpChanged();
    void filterColumnChanged();
    void updateGui();
    void pauseUpdateThread();
    void onCustomContextMenu(const QPoint &point);
    void SetFontSize(int fontSizeIndex);
    void captureMode();
    void RunRootDatasource();

    void readProcDataStd();
    void readProcDataErr();
signals:
    void callUpdateGui();

private:
    CUpdateThread                                       m_NetData;

    QMap<CDataSource::ColumnData, TableHeaderCaption>   m_TableCaption;
    QMap<CDataSource::eNetType, QString>                m_TableProtocolName;
    QMap<CDataSource::ConnectionTcpState, QString>      m_TableConnectionTcpState;

    QSortFilterProxyModel                               m_ProxyModel;
    QGroupBox                                           m_GroupBox;
    QTreeView                                           m_TreeView;
    QLabel                                              m_filterPatternLabel;
    QLabel                                              m_filterSyntaxLabel;
    QLabel                                              m_filterColumnLabel;
    QLineEdit                                           m_filterPatternLineEdit;
    QComboBox                                           m_filterSyntaxComboBox;
    QComboBox                                           m_filterColumnComboBox;
    QVBoxLayout                                         m_connLayout;
    QWidget                                             m_connWidg;

    QPushButton         m_buttonPause;
    QStateMachine       m_qstatePause;

    QPushButton         m_buttonCaptureMode;
    QStateMachine       m_qstateCaptureMode;
    bool                m_captureEnable;

    QSlider             m_zoomSlider;

    int                 m_fontSize;
    QList<int>          m_fontSizeList;

    QPushButton         m_buttonRunRootDatasource;
    QProcess            m_rootResolver;

    void updateTreeView(CDataSource::SocketInfo *dataSource, bool disableCleanup);
    void HideDataColumn();
    void showEvent( QShowEvent* event );
    void ShowWhois(QString rowText, QString whoisText);
    void ShowInfoDialog(QString title, QString dialogText, bool readonly);
};

#endif // WINDOW_H
