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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <QProcess>
#include "source/cconnectionstree.h"
#include "source/ccfontsize.h"
#include "source/updatethread.h"
#include "source/clanguage.h"

#define DEFCFG_MAINWINDOWGEOM  "base/MainWindowGeometry"
#define DEFCFG_CONNECTIONTABLE "base/TableHeaderState"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent( QShowEvent* event );

private slots:
    void on_pushButton_displayNames_clicked();
    void on_pushButton_Pause_toggled(bool checked);
    void on_pushButton_Record_toggled(bool checked);
    void on_pushButton_copyToClipboard_clicked();
    void on_pushButton_SaveToFile_clicked();
    void on_pushButton_whois_clicked();
    void on_lineEdit_include_textChanged(const QString &arg1);
    void on_horizontalSlider_valueChanged(int value);
    void updateGui();
    void currentSelectionChanged(const QModelIndex current, const QModelIndex previous);

signals:
    void callUpdateGui();

private:

    Ui::MainWindow  *ui;
    CConnectionsTree m_cconnectionstree;
    CCFontSize       m_ccfontsize;
    QProcess         m_rootResolver;
    CUpdateThread    m_NetData;
    bool             m_captureEnable;
    QString          m_whoisText;
    QString          m_RowText;
    QString          m_ClipBoardString;

    void SaveAppState();
    void ShowWhois(QString rowText, QString whoisText);
    void ShowInfoDialog(QString title, QString dialogText, bool readonly);
};

#endif // MAINWINDOW_H
