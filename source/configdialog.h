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

#include "connectionstatehelper.h"

#include <QDialog>
#include <functional>

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

class QLabel;

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(const std::function<void ()> &callbackUpdate, const std::function<void ()> &callbackClose, QWidget *parent = nullptr);
    ~ConfigDialog() override;

    void setLabelColor(const QColor &frg, const QColor &bkg, QLabel *pLabel);

public slots:
    void onClick();
    void onReset();
    void onBwColor();
    void onWbColor();
    void onBackgroundColor();
    void onDefaultBackgroundColor();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    Ui::ConfigDialog* m_ui;
    std::function<void(void)> m_callbackUpdate;
    std::function<void(void)> m_callbackClose;
    ConnectionStateHelper     m_ConnectionStateHelper;

    void tooltipText(const QString &text);
    void reloadColor();
};

#endif // CONFIGDIALOG_H
