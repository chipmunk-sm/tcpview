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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QApplication>

#include <stdio.h>
#include <cstdint>
#include <iostream>

#include "source/cconnectionstree.h"
#include "source/ccfontsize.h"
#include "source/clanguage.h"

#define MAIN_WINDOW_LAYOUT  "base/MAIN_WINDOW_LAYOUT"
#define DEFCFG_CONNECTIONTABLE "base/TableHeaderState"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void on_pushButton_displayNames_clicked();
    void on_pushButton_Pause_toggled(bool checked);
    void on_pushButton_Record_toggled(bool checked);
    void on_pushButton_copyToClipboard_clicked();
    void on_pushButton_SaveToFile_clicked();
    void on_pushButton_whois_clicked();
    void on_pushButton_Settings_clicked();
    void on_lineEdit_include_textChanged(const QString &arg1);
    void on_treeView_connection_customContextMenuRequested(const QPoint &pos);
    void updateGui();
    void onCurrentSelectionChanged(const QModelIndex current, const QModelIndex previous);

signals:
    void callUpdateGui();

private:
    Ui::MainWindow  *ui;
    CConnectionsTree m_cconnectionstree;
    CCFontSize       m_ccfontsize;
    bool             m_captureEnable;
    QString          m_whoisText;
    QString          m_RowText;
    QString          m_ClipBoardString;
    int              m_visibleItems;
    int              m_totalItems;
    QFont            m_deffont = QApplication::font();

    void ShowInfoDialog(QString title, QString dialogText, bool readonly);
    bool GetAuthGuiName(QString &command, QStringList &cmdArgs);
    QString CutLongText(const QString &sourceText, int maxLength);
    void UpdateStatusText();

    void tooltipText(const QString & text);

    void UpdateConfig();
    void CloseConfig();
};

#endif // MAINWINDOW_H
