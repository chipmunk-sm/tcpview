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

#ifndef NETDATA_H
#define NETDATA_H

#include <QWidget>
#include <vector>
#include <functional>
#include <atomic>

#include "datasource.h"

class CUpdateThread
{

public:
    CUpdateThread();
    ~CUpdateThread();

    typedef enum{
        STATE_THREAD_INIT = 0x7,
        STATE_THREAD_RUN,
        STATE_THREAD_WAIT_FOR_STOP,
        STATE_THREAD_STOP,
        STATE_THREAD_EXEPTION
    }stateThreadEnum;

public:
    CDataSource *GetData();
    void EnableUpdateData();
    void setUpdateCallback(std::function<void(void)> callbackUpdate);
    void pauseUpdate(bool state);
    bool IsPause();

    bool InitRootLoader();
    void DeleteRootLoader();
private:
    bool           m_pauseUpdate;
    unsigned int   m_updateCounter;

    std::function<void(void)> m_callbackUpdate;
    CDataSource m_netdatahelper;

    std::atomic_bool m_updateUi;
    std::atomic_bool m_updateData;

    stateThreadEnum m_stateThread;

    void CoreProc();
    static void threadRun(CUpdateThread *ptr);

};

#endif // NETDATA_H
