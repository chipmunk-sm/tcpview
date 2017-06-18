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
