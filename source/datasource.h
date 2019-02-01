/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2019 chipmunk-sm <dannico@linuxmail.org>
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
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <uuid/uuid.h>
#include <atomic>
#include <cctype>
#include <algorithm>


#include "cportservicenames.h"
#include "rootmodule.h"

class CDataSource
{
public:
    CDataSource();
    ~CDataSource();
    typedef enum{
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
    }ConnectionTcpState;

    typedef enum{
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
        COLUMN_DATA_DATA,

        COLUMN_DATA_COUNT
    }ColumnData;

    typedef enum{
        conn_tcp,
        conn_udp,
        conn_tcp6,
        conn_udp6,
        conn_raw,
        conn_raw6
    }eNetType;

    typedef enum{
        proc_net_tcp = 1,            // Contains detailed TCP socket information.
        proc_net_tcp6,               //
        proc_net_udp,                // Contains detailed UDP socket information.
        proc_net_udp6,               //
        proc_net_raw,                // Lists raw device statistics.
        proc_net_raw6,               //
    }eProcPath;

    typedef struct{

        char                localAddr[INET6_ADDRSTRLEN];
        char                localHost[NI_MAXHOST];
        char                localPort[NI_MAXSERV];

        char                remoteAddr[INET6_ADDRSTRLEN];
        char                remoteHost[NI_MAXHOST];
        char                remotePort[NI_MAXSERV];

        char                Command[_POSIX_PATH_MAX];
        bool                commandUpdate;

        eNetType            netType;

        uint                state;
        bool                stateUpdate;

        unsigned long long  inode;      // connecton id

        unsigned int        uid;        // user id
        uuid_t              uuid;       // unique ID for item

        unsigned int       loadCycles;
        unsigned int       deleteItem;

        struct in6_addr loc6;
        struct in6_addr rem6;
        struct in_addr loc4;
        struct in_addr rem4;

    }SocketInfo;

    void UpdateTable();
    bool IsRootLoaderValid();
    std::unordered_map<std::string, SocketInfo> *GetConnectionsList();

    bool InitRootLoader();
    void DeleteRootLoader();
    
private:
    unsigned int m_loadCycles;
    std::map<eNetType, std::string>             m_eNetTypeList;
    std::unordered_map<std::string, SocketInfo> m_socketList;
    CRootModule                                *m_pRootModule;
    std::atomic_bool                            m_enableRootMod;
    CPortServiceNames                           m_CPortServiceNames;
    bool                                        m_RootModuleInvalid;
    int                                         m_errors;

    static void LoadConnections(eNetType netType,
                               const char *commandLine,
                               std::unordered_map<std::string, SocketInfo> *pSocketList,
                               CPortServiceNames *pCPortServiceNames,
                               unsigned int loadCycles);

    static bool LoadCmd(unsigned long long inode,
                             std::map<unsigned int, std::string> *procCommand,
                             std::map<unsigned long long, unsigned int> *procInodeList,
                             char *pBuff,
                            size_t bufferLen);

    void DomainNamesResolver(SocketInfo &socket_info);
};

#endif // PROCESSLOADER_H
