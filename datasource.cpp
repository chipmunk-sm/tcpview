/*
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

CDataSource::CDataSource()
    : m_loadCycles(0)
    , m_rootModule(nullptr)
{
    m_enableRootMod = false;

    m_eNetTypeList.insert(std::make_pair(conn_tcp,  std::string("/proc/net/tcp")));
    m_eNetTypeList.insert(std::make_pair(conn_udp,  std::string("/proc/net/udp")));
    m_eNetTypeList.insert(std::make_pair(conn_tcp6, std::string("/proc/net/tcp6")));
    m_eNetTypeList.insert(std::make_pair(conn_udp6, std::string("/proc/net/udp6")));
    m_eNetTypeList.insert(std::make_pair(conn_raw,  std::string("/proc/net/raw")));
    m_eNetTypeList.insert(std::make_pair(conn_raw6, std::string("/proc/net/raw6")));

    // *Thread safety MT-Unsafe
    //
    struct servent *sentry;
    setservent(0);
    while ( (sentry = getservent()) != nullptr )
    {
        if(strcmp(sentry->s_proto,"tcp") == 0)
            m_PortServiceNamesTcp.insert(std::pair<int, std::string>(ntohs(sentry->s_port), std::string(sentry->s_name)));
        else if(strcmp(sentry->s_proto,"udp") == 0)
            m_PortServiceNamesUdp.insert(std::pair<int, std::string>(ntohs(sentry->s_port), std::string(sentry->s_name)));
    }
    endservent();

}

CDataSource::~CDataSource()
{
   DeleteRootLoader();
}

void CDataSource::DeleteRootLoader()
{
    if(!m_rootModule)
        return;
    m_enableRootMod = false;
    auto tmp = m_rootModule;
    m_rootModule = nullptr;
    delete tmp;
}

bool CDataSource::InitRootLoader()
{

   if(m_rootModule)
       return true;

    auto tmpptr = new CRootModule(-1);
    if(tmpptr->m_error.length() > 1)
    {
        std::cout << "CRootModule failed " << tmpptr->m_processId << " error " << tmpptr->m_error << std::endl;
        delete tmpptr;
        return false;
    }

    m_rootModule = tmpptr;
    m_enableRootMod = true;

    return true;
}

void CDataSource::UpdateTable()
{

    m_loadCycles++;

    std::map<unsigned long long, unsigned int> procInodeList;
    std::map<unsigned int, std::string> procCommand;

    if(m_enableRootMod && m_rootModule)
        m_rootModule->RunClient(&procInodeList, &procCommand);

    /* load socket connections */
    for (const auto& it : m_eNetTypeList)
         getConnections(it.first,
                        it.second.c_str(),
                        &m_socketList,
                        &procInodeList,
                        &procCommand,
                        &m_PortServiceNamesTcp,
                        &m_PortServiceNamesUdp,
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

void CDataSource::getConnections(eNetType netType,
                                 const char * commandLine,
                                 std::unordered_map<std::string, SocketInfo> *pSocketList,
                                 std::map<unsigned long long, unsigned int> *procInodeList,
                                 std::map<unsigned int, std::string> *procCommand,
                                 std::map<int, std::string> *pPortServiceNamesTcp,
                                 std::map<int, std::string> *pPortServiceNamesUdp,
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

        serviceName(loc_port, socket_info.localPort, sizeof(socket_info.localPort),
                    (netType == conn_tcp || netType == conn_tcp6) ? pPortServiceNamesTcp : pPortServiceNamesUdp);

        serviceName(rem_port, socket_info.remotePort, sizeof(socket_info.remotePort),
                    (netType == conn_tcp || netType == conn_tcp6) ? pPortServiceNamesTcp : pPortServiceNamesUdp);

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

std::string CDataSource::makeUserNameStr(__uid_t euid)
{

    char buf[32] = {0};

    struct passwd *p = getpwuid(euid);
    if(p)
        snprintf(buf, sizeof(buf), "%s", p->pw_name);
    else
        snprintf(buf, sizeof(buf), "%5u   ", euid);

    return std::string(buf);

}

void CDataSource::serviceName(int port, char *buff, size_t buffLength, std::map<int, std::string> *pNames)
{

    if(pNames != nullptr)
    {
        auto it = pNames->find(port);
        if (it != pNames->end())
        {
            std::snprintf(buff, buffLength, "%d (%s)", port, it->second.c_str());
            return;
        }
    }

    std::snprintf(buff, buffLength, "%d", port);

}

std::unordered_map<std::string, CDataSource::SocketInfo> *CDataSource::GetConnectionsList()
{
    return &m_socketList;
}

bool CDataSource::run_posix_spawn(char *pPath, char *pArg)
{

    //TODO: test

    //"pkexec " + QCoreApplication::applicationFilePath() + "--rootmodule"

    char *argv[] = { pPath, pArg, (char *)0 };

    pid_t pid;
    auto status = posix_spawn(&pid, pPath, nullptr, nullptr, argv, environ);
    if ( status != 0 )
    {
        std::cout << "Failed posix_spawn: " << strerror(status) <<  std::endl;
        return false;
    }

    std::cout << "posix_spawn: " << pPath << " pid: " << pid <<  std::endl;
    fflush(nullptr);

    if (waitpid(pid, &status, 0) == -1)
    {
        std::cout << "Failed waitpid: " << strerror(status) <<  std::endl;
        return false;
    }

    std::cout << "waitpid: " << pPath << " pid: " << pid <<  std::endl;
    fflush(nullptr);

    return true;

}
