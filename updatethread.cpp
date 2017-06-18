#include "updatethread.h"
#include "datasource.h"
#include <thread>
#include <iostream>

CUpdateThread::CUpdateThread()
    : m_updateCounter(0)
    , m_stateThread(STATE_THREAD_INIT)
{
    m_pauseUpdate = false;
    m_updateUi = false;
    m_updateData = false;

    (std::thread(threadRun, this)).detach();

}

CUpdateThread::~CUpdateThread()
{
    if(m_stateThread == STATE_THREAD_RUN)
    {
        m_stateThread =  STATE_THREAD_WAIT_FOR_STOP;
    }

    int timeout = 10;
    while(m_stateThread == STATE_THREAD_WAIT_FOR_STOP && timeout--)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

}

void CUpdateThread::CoreProc()
{

    do{

        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        if(m_updateUi || m_pauseUpdate)
        {

        }
        else
        {

            m_updateCounter++;

            m_updateData = true;

            m_netdatahelper.UpdateTable();

            m_updateData = false;

            m_callbackUpdate();

        }

    }while(m_stateThread == STATE_THREAD_RUN);

}

void CUpdateThread::threadRun(CUpdateThread *ptr)
{

    try
    {
        ptr->m_stateThread = STATE_THREAD_RUN;

        ptr->CoreProc();

        ptr->m_stateThread = STATE_THREAD_STOP;

        return;
    }
    catch(...)
    {
        // STATE_THREAD_EXEPTION
    }

    ptr->m_stateThread = STATE_THREAD_EXEPTION;

}

CDataSource *CUpdateThread::GetData()
{
    if(m_updateData)
    {
        return nullptr;
    }

    m_updateUi = true;

    return &m_netdatahelper;
}

void CUpdateThread::EnableUpdateData()
{
    m_updateUi = false;
}

void CUpdateThread::setUpdateCallback(std::function<void ()> callbackUpdate)
{
    m_callbackUpdate = callbackUpdate;
}

void CUpdateThread::pauseUpdate(bool state)
{
    m_pauseUpdate = state;
}

bool CUpdateThread::InitRootLoader()
{
    return m_netdatahelper.InitRootLoader();
}

void CUpdateThread::DeleteRootLoader()
{
    m_pauseUpdate = true;
    while(m_updateData)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_netdatahelper.DeleteRootLoader();
}

