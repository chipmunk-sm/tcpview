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

#ifndef PROCESSLOADER_H
#define PROCESSLOADER_H

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <locale>
#include <codecvt>
#include <string>

#include <atomic>
#include <cctype>
#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <uuid/uuid.h>

#include "cportservicenames.h"
#include "rootmodule.h"

#define USECONSTFUNCTION

typedef enum eConnectionTcpState{
    CONNECTION_TCP_EMPTY,
    CONNECTION_TCP_ESTABLISHED,
    CONNECTION_TCP_SYN_SENT,
    CONNECTION_TCP_SYN_RECV,
    CONNECTION_TCP_FIN_WAIT1,
    CONNECTION_TCP_FIN_WAIT2,
    CONNECTION_TCP_TIME_WAIT,
    CONNECTION_TCP_CLOSE,
    CONNECTION_TCP_CLOSE_WAIT,
    CONNECTION_TCP_LAST_ACK,
    CONNECTION_TCP_LISTEN,
    CONNECTION_TCP_CLOSING,
    CONNECTION_TCP_CLOSED,
    CONNECTION_REMOVED = 0xfc
}eConnectionTcpState;

typedef enum eColumnData{
    COLUMN_DATA_PROTOCOL,
    COLUMN_DATA_LOCALHOST,
    COLUMN_DATA_LOCALADDRESS,
    COLUMN_DATA_LOCALPORT,
    COLUMN_DATA_STATE,
    COLUMN_DATA_REMOTEHOST,
    COLUMN_DATA_REMOTEADDRESS,
    COLUMN_DATA_REMOTEPORT,
    COLUMN_DATA_UID,
    COLUMN_DATA_INODE,
    COLUMN_DATA_TIME,
    COLUMN_DATA_COMMAND,

    COLUMN_DATA_COUNT
}eColumnData;

typedef enum eNetType{
    conn_tcp,
    conn_udp,
    conn_tcp6,
    conn_udp6,
    conn_raw,
    conn_raw6
}eNetType;

typedef enum eProcPath{
    proc_net_tcp = 1,            // Contains detailed TCP socket information.
    proc_net_tcp6,               //
    proc_net_udp,                // Contains detailed UDP socket information.
    proc_net_udp6,               //
    proc_net_raw,                // Lists raw device statistics.
    proc_net_raw6,               //
}eProcPath;

typedef struct SocketInfo{
    char                localAddr[INET6_ADDRSTRLEN + 1];
    char                localHost[NI_MAXHOST + 1];
    char                localPort[NI_MAXSERV + 1];
    char                remoteAddr[INET6_ADDRSTRLEN + 1];
    char                remoteHost[NI_MAXHOST + 1];
    char                remotePort[NI_MAXSERV + 1];
    char                command[_POSIX_PATH_MAX + 2];
    bool                commandUpdate;
    bool                stateUpdate;
    unsigned int        uid;        // user id
    eNetType            netType;
    unsigned long long  inode;      // connecton id
    uuid_t              uuid;       // unique ID for item
    time_t              last_time;
    uint                state;
    unsigned int        deleteItem;
    struct in6_addr     loc6;
    struct in6_addr     rem6;
    struct in_addr      loc4;
    struct in_addr      rem4;
}SocketInfo;

class CDataSource
{
public:
    static CDataSource & Instance()
    {
        static CDataSource thisInstance;
        return thisInstance;
    }
    CDataSource(const CDataSource &)           = delete;
    CDataSource(const CDataSource&&)           = delete;
    CDataSource& operator=(const CDataSource&) = delete;
    CDataSource& operator=(const CDataSource&&)= delete;

private:
    explicit CDataSource();
    ~CDataSource();

public:

    void Init(const std::function<void(void)> &callbackUpdate);

    void setAbort();

    std::unordered_map<std::string, SocketInfo> *GetConnectionsList();
    void FreeConnectionsList();

    bool InitRootLoader(std::string name);
    bool rootLoaderActive();
    void shutdownRootModule();

    int timeoutSec() const;
    void setTimeoutSec(int timeoutSec);

#ifdef USECONSTFUNCTION
    bool pauseUpdate() const;
#else
    bool pauseUpdate();
#endif
    void setPauseUpdate(bool pauseUpdate);

private:
    void ThreadMain();
    void UpdateTable();

    static void LoadConnections(eNetType netType,
                                const char *commandLine,
                                std::unordered_map<std::string, SocketInfo> *pSocketList,
                                CPortServiceNames *pCPortServiceNames,
                                const time_t nowtime);

    static bool LoadCmd(unsigned long long inode,
                        std::map<unsigned int, std::string> *procCommand,
                        std::map<unsigned long long, unsigned int> *procInodeList,
                        char *pBuff,
                        size_t bufferLen);

    void DomainNamesResolver(SocketInfo &socket_info);

    std::map<eNetType, std::string>             m_eNetTypeList;
    std::unordered_map<std::string, SocketInfo> m_socketList;
    std::shared_ptr<CRootModule>                m_rootModule;
    CPortServiceNames                           m_portServiceNames;
    std::function<void()>                       m_callbackUpdate;
    bool                                        m_threadExit = true;
    bool                                        m_threadStop = true;
    bool                                        m_pauseUpdate = true;
    std::atomic_bool                            m_updateInProgress = ATOMIC_FLAG_INIT;
    bool                                        m_connectionsListInUse = false;
    int                                         m_removeRecordTimeoutSec = 10;

#ifdef USECONSTFUNCTION

#else
    /*mutable*/ std::mutex m_run_update;
#endif
};

#endif // PROCESSLOADER_H
