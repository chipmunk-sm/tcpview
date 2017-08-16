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

#include "datasource.h"
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <QMessageBox>

CDataSource::CDataSource()
    : m_loadCycles(0)
    , m_pRootModule(nullptr)
    , m_RootModuleInvalid(false)
{
    m_enableRootMod = false;

    m_eNetTypeList.insert(std::make_pair(conn_tcp,  std::string("/proc/net/tcp")));
    m_eNetTypeList.insert(std::make_pair(conn_udp,  std::string("/proc/net/udp")));
    m_eNetTypeList.insert(std::make_pair(conn_tcp6, std::string("/proc/net/tcp6")));
    m_eNetTypeList.insert(std::make_pair(conn_udp6, std::string("/proc/net/udp6")));
    m_eNetTypeList.insert(std::make_pair(conn_raw,  std::string("/proc/net/raw")));
    m_eNetTypeList.insert(std::make_pair(conn_raw6, std::string("/proc/net/raw6")));

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

    std::map<unsigned long long, unsigned int> procInodeList;
    std::map<unsigned int, std::string> procCommand;

    if(m_enableRootMod && m_pRootModule)
    {
      auto retv = m_pRootModule->RunClient(&procInodeList, &procCommand);
      if(!retv)
      {
          QMessageBox::critical(nullptr, "Datasource", QObject::tr("Failed to load data from RootModule"), QMessageBox::Ok);
          m_RootModuleInvalid = true;
      }
    }

    /* load socket connections */
    for (const auto& it : m_eNetTypeList)
         LoadConnections(it.first,
                        it.second.c_str(),
                        &m_socketList,
                        &procInodeList,
                        &procCommand,
                        &m_CPortServiceNames,
                        m_loadCycles);

    /* set cleanup flag for old entries in socketList */
    for (auto it = m_socketList.begin(); it != m_socketList.end(); )
    {
        if (it->second.loadCycles != m_loadCycles)
        {
            it->second.state = CONNECTION_REMOVED;
            it->second.stateUpdate = true;
            it->second.deleteItem++;
        }
        ++it;
    }

}

bool CDataSource::IsRootLoaderValid()
{
    return !m_RootModuleInvalid;
}

bool CDataSource::FillCommand(unsigned long long inode,
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

void CDataSource::LoadConnections(eNetType netType,
                                 const char * commandLine,
                                 std::unordered_map<std::string, SocketInfo> *pSocketList,
                                 std::map<unsigned long long, unsigned int> *procInodeList,
                                 std::map<unsigned int, std::string> *procCommand,
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

        std::snprintf(buffer, sizeof(buffer),"%s%d%s%d%d",loc_addr, loc_port, rem_addr, rem_port, netType);
        std::string keystring(buffer);

        SocketInfo socket_info;

        auto search = pSocketList->find(keystring);
        if(search != pSocketList->end())
        {

            search->second.loadCycles = loadCycles;

            if(search->second.state != state)
            {
                search->second.state        = state;
                search->second.stateUpdate  = true;
            }

            if(strlen(search->second.Command) < 1)
            {
                char* ptr = search->second.Command;
                size_t len = sizeof(socket_info.Command);
                unsigned int tminod = search->second.inode;
                if(FillCommand(tminod, procCommand, procInodeList, ptr, len))
                    search->second.commandUpdate = true;
            }

            continue; // next connection

        }

        memset(&socket_info, 0, sizeof(socket_info));

        socket_info.loadCycles  = loadCycles;
        socket_info.netType     = netType;
        socket_info.uid         = uid;
        socket_info.inode       = inode;
        socket_info.state       = state;

        FillCommand(inode, procCommand, procInodeList, socket_info.Command, sizeof(socket_info.Command));

        pCPortServiceNames->GetServiceName(loc_port, socket_info.localPort, sizeof(socket_info.localPort), (netType == conn_tcp || netType == conn_tcp6));
        pCPortServiceNames->GetServiceName(rem_port, socket_info.remotePort, sizeof(socket_info.remotePort), (netType == conn_tcp || netType == conn_tcp6));


        if (netType == conn_tcp6 || netType == conn_udp6 || netType == conn_raw6)
        {
            struct in6_addr loc;
            sscanf(loc_addr, "%08X%08X%08X%08X", &loc.s6_addr32[0], &loc.s6_addr32[1], &loc.s6_addr32[2], &loc.s6_addr32[3]);
            inet_ntop(AF_INET6, &loc, socket_info.localAddr,  INET6_ADDRSTRLEN);
            auto locval = gethostbyaddr(&loc, sizeof(loc), AF_INET6);
            strcpy(socket_info.localHost,  locval == nullptr ? "*" : locval->h_name);

            struct in6_addr rem;
            sscanf(rem_addr, "%08X%08X%08X%08X", &rem.s6_addr32[0], &rem.s6_addr32[1], &rem.s6_addr32[2], &rem.s6_addr32[3]);
            inet_ntop(AF_INET6, &rem, socket_info.remoteAddr, INET6_ADDRSTRLEN);
            auto remval = gethostbyaddr(&rem, sizeof(rem), AF_INET6);
            strcpy(socket_info.remoteHost, remval == nullptr ? "*" : remval->h_name);
        }
        else
        {
            struct in_addr loc;
            sscanf(loc_addr, "%X", &(loc.s_addr));
            strcpy(socket_info.localAddr,  inet_ntoa(loc));
            auto locval = gethostbyaddr(&loc, sizeof(loc), AF_INET);
            strcpy(socket_info.localHost,  locval == nullptr ? "*" : locval->h_name);

            struct in_addr rem;
            sscanf(rem_addr, "%X", &(rem.s_addr));
            strcpy(socket_info.remoteAddr, inet_ntoa(rem));
            auto remval = gethostbyaddr(&rem, sizeof(rem), AF_INET);
            strcpy(socket_info.remoteHost, remval == nullptr ? "*" : remval->h_name);
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

