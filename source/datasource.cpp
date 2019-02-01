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

#include "datasource.h"
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <QMessageBox>
#include "defined.h"

CDataSource::CDataSource()
    : m_loadCycles(0)
    , m_pRootModule(nullptr)
    , m_RootModuleInvalid(false)
    , m_errors(0)
{
    m_enableRootMod = false;

    m_eNetTypeList.insert(std::make_pair(conn_tcp,  PROC_NET_TCP));
    m_eNetTypeList.insert(std::make_pair(conn_udp,  PROC_NET_UDP));
    m_eNetTypeList.insert(std::make_pair(conn_tcp6, PROC_NET_TCP6));
    m_eNetTypeList.insert(std::make_pair(conn_udp6, PROC_NET_UDP6));
    m_eNetTypeList.insert(std::make_pair(conn_raw,  PROC_NET_RAW));
    m_eNetTypeList.insert(std::make_pair(conn_raw6, PROC_NET_RAW6));

}

CDataSource::~CDataSource()
{
   DeleteRootLoader();
}

void CDataSource::DeleteRootLoader()
{
    if(!m_pRootModule)
        return;

    m_RootModuleInvalid = false;
    m_enableRootMod = false;

    auto tmp = m_pRootModule;
    m_pRootModule = nullptr;
    delete tmp;
}

bool CDataSource::InitRootLoader()
{

   if(m_pRootModule)
       return false;

    auto tmpptr = new CRootModule(-1);
    if(tmpptr->m_error.length() > 1)
    {
        std::cout << "CRootModule failed. " << tmpptr->m_error << std::endl;
        delete tmpptr;
        return false;
    }

    m_pRootModule = tmpptr;
    m_enableRootMod = true;

    return m_enableRootMod;
}

void CDataSource::UpdateTable()
{

    m_loadCycles++;

    /* load socket connections */
    for (const auto& it : m_eNetTypeList)
         LoadConnections(it.first,
                        it.second.c_str(),
                        &m_socketList,
                        &m_CPortServiceNames,
                        m_loadCycles);


    std::map<unsigned long long, unsigned int> procInodeList;
    std::map<unsigned int, std::string> procCommand;
    bool chekCmdUpdate = false;

    if(m_enableRootMod && m_pRootModule)
    {
        auto retv = m_pRootModule->RunClient(&procInodeList, &procCommand);
        if(retv)
        {
            chekCmdUpdate = true;
        }
        else
        {
            m_RootModuleInvalid = true;
            if(m_errors++ == 0)
                QMessageBox::critical(nullptr, "Datasource", "Failed to load data from RootModule", QMessageBox::Ok);
        }
    }

    for (auto it = m_socketList.begin(); it != m_socketList.end(); it++)
    {
        const int loadCommandTimeout = 6;

        DomainNamesResolver(it->second);

        SocketInfo *sinf = &it->second;

        /* set cleanup flag for old entries in socketList */
        if (sinf->loadCycles != m_loadCycles)
        {
            sinf->state = CONNECTION_REMOVED;
            sinf->stateUpdate = true;
            sinf->deleteItem++;
        }

        if(!chekCmdUpdate || sinf->deleteItem > loadCommandTimeout)
            continue;

        if(sinf->Command[0] == 0)
        {
            if(LoadCmd( sinf->inode, &procCommand, &procInodeList,
                            sinf->Command, sizeof(SocketInfo::Command)))
                sinf->commandUpdate = true;
        }
    }

}

bool CDataSource::IsRootLoaderValid()
{
    return !m_RootModuleInvalid;
}

bool CDataSource::LoadCmd(unsigned long long inode,
                              std::map<unsigned int, std::string> *procCommand,
                              std::map<unsigned long long, unsigned int> *procInodeList,
                              char * pBuff,
                              size_t bufferLen)
{
    auto it = procInodeList->find(inode);
    if (it != procInodeList->end())
    {
        auto pid = it->second;
        auto comm = procCommand->find(pid);
        if ( comm != procCommand->end() )
        {
            snprintf(pBuff, bufferLen, "(%u) %s", pid, comm->second.c_str());
            return true;
        }
        else
        {
            snprintf(pBuff, bufferLen, "(%u)", pid);
            return true;
        }
    }
    return false;
}

void CDataSource::DomainNamesResolver(SocketInfo &socket_info)
{

    if(socket_info.remoteHost[0] != 0)
        return;

    if (socket_info.netType == conn_tcp6 || socket_info.netType == conn_udp6 || socket_info.netType == conn_raw6)
    {
        auto locval = gethostbyaddr(&socket_info.loc6, sizeof(socket_info.loc6), AF_INET6);
        strcpy(socket_info.localHost,  locval == nullptr ? "*" : locval->h_name);

        auto remval = gethostbyaddr(&socket_info.rem6, sizeof(socket_info.rem6), AF_INET6);
        strcpy(socket_info.remoteHost, remval == nullptr ? "*" : remval->h_name);
    }
    else
    {
        auto locval = gethostbyaddr(&socket_info.loc4, sizeof(socket_info.loc4), AF_INET);
        strcpy(socket_info.localHost,  locval == nullptr ? "*" : locval->h_name);

        auto remval = gethostbyaddr(&socket_info.rem4, sizeof(socket_info.rem4), AF_INET);
        strcpy(socket_info.remoteHost, remval == nullptr ? "*" : remval->h_name);
    }
}

void CDataSource::LoadConnections(eNetType netType,
                                 const char * commandLine,
                                 std::unordered_map<std::string, SocketInfo> *pSocketList,
                                 CPortServiceNames *pCPortServiceNames,
                                 unsigned int loadCycles)
{

    FILE *netstat;
    char buffer[1024];

    if ((netstat = fopen(commandLine, "r")) == nullptr)
        return;

    int loadIndex = 0;

    while (fgets(buffer, sizeof(buffer), netstat) != nullptr)
    {

        if(loadIndex++ == 0)
            continue; // skip header

        if (buffer[0] == 0)
            continue; // next connection

        char loc_addr[INET6_ADDRSTRLEN*2] = {0};
        int loc_port = 0;

        char rem_addr[INET6_ADDRSTRLEN*2] = {0};
        int rem_port = 0;

        unsigned int state = -1;
        unsigned int uid = 0;
        unsigned long long inode = 0;
//    sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
//     0: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 374316 1 0000000000000000 100 0 0 10 0
//     1: 00000000:14EB 00000000:0000 0A 00000000:00000000 00:00000000 00000000   102        0 11821  1 0000000000000000 100 0 0 10 0
//     2: 0101007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 14305  1 0000000000000000 100 0 0 10 0
//     3: 3500007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000   102        0 11819  1 0000000000000000 100 0 0 10 0

//kernel 4.8
//tcp  0  1        2    3        4    5  6        7        8  9        10          11  12      13
//     0: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 20167 1 ffff95d359afbc00 100 0 0 10 0
//    sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode

//tcp6 0  1                                2    3                                4    5  6        7        8  9        10         11   12      13
//     0: 00000000000000000000000001000000:0277 00000000000000000000000000000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 151787 1 ffff95d35a92b9c0 100 0 0 10 0
//    sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
        auto num = sscanf(buffer,
           //0    1              2  3              4  5  6   7   8   9   10  11 12  13
            "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %*x:%*x %*x:%*x %*x %u %*d %llu\n",
          /*1         2*/
            loc_addr, &loc_port,
          /*3         4*/
            rem_addr, &rem_port,
          /*5*/
            &state,
          /*11*/
            &uid,
          /*13*/
            &inode);

        if (num != 7)
        {
            std::cout << "Parser error for:\t"  << buffer << std::endl;
            continue; // next connection
        }

        if (inode == 0)
        {
            //std::cout << "           sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode" << std::endl;
            //std::cout << "Zero inode:"  << buffer << std::endl;
        }

        std::snprintf(buffer, sizeof(buffer),"%s%d%s%d%d",loc_addr, loc_port, rem_addr, rem_port, netType);
        std::string keystring(buffer);

        SocketInfo socket_info;

        //check for already exist record
        auto search = pSocketList->find(keystring);
        if(search != pSocketList->end())
        {

            search->second.loadCycles = loadCycles;

            if(search->second.state != state)
            {
                search->second.state        = state;
                search->second.stateUpdate  = true;
            }

            continue; // next connection

        }

        //process new record

        memset(&socket_info, 0, sizeof(socket_info));

        socket_info.loadCycles  = loadCycles;
        socket_info.netType     = netType;
        socket_info.uid         = uid;
        socket_info.inode       = inode;
        socket_info.state       = state;

        pCPortServiceNames->GetServiceName(loc_port, socket_info.localPort, sizeof(socket_info.localPort), (netType == conn_tcp || netType == conn_tcp6));
        pCPortServiceNames->GetServiceName(rem_port, socket_info.remotePort, sizeof(socket_info.remotePort), (netType == conn_tcp || netType == conn_tcp6));

        if (socket_info.netType == conn_tcp6 || socket_info.netType == conn_udp6 || socket_info.netType == conn_raw6)
        {
            sscanf(loc_addr, "%08X%08X%08X%08X", &socket_info.loc6.s6_addr32[0], &socket_info.loc6.s6_addr32[1], &socket_info.loc6.s6_addr32[2], &socket_info.loc6.s6_addr32[3]);
            inet_ntop(AF_INET6, &socket_info.loc6, socket_info.localAddr,  INET6_ADDRSTRLEN);

            sscanf(rem_addr, "%08X%08X%08X%08X", &socket_info.rem6.s6_addr32[0], &socket_info.rem6.s6_addr32[1], &socket_info.rem6.s6_addr32[2], &socket_info.rem6.s6_addr32[3]);
            inet_ntop(AF_INET6, &socket_info.rem6, socket_info.remoteAddr, INET6_ADDRSTRLEN);
        }
        else
        {
            sscanf(loc_addr, "%X", &(socket_info.loc4.s_addr));
            strcpy(socket_info.localAddr,  inet_ntoa(socket_info.loc4));

            sscanf(rem_addr, "%X", &(socket_info.rem4.s_addr));
            strcpy(socket_info.remoteAddr, inet_ntoa(socket_info.rem4));
        }

        uuid_generate(socket_info.uuid);
        pSocketList->emplace(std::make_pair(keystring, socket_info));

    }

    fclose(netstat);

}

std::unordered_map<std::string, CDataSource::SocketInfo> *CDataSource::GetConnectionsList()
{
    return &m_socketList;
}




