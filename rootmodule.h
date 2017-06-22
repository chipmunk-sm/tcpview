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

#ifndef CROOTMODULE_H
#define CROOTMODULE_H

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <locale>
#include <codecvt>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <uuid/uuid.h>
#include <semaphore.h>
#include <buffer.h>

#define DEF_STARTCODE 0x010101010

class CRootModule
{
public:
    CRootModule(__pid_t processId);
    ~CRootModule();

private:
    typedef enum{
        ItemType_ERROR = 0x02,
        ItemType_none,
        ItemType_inode,
        ItemType_cmd,
        ItemType_end
    }ItemType;

    typedef struct{
        unsigned int startcode1;
        unsigned int startcode2;
        ItemType     command;
        unsigned int procId;
        unsigned int dataCount;
    }ItemInfo;

public:
    void RunServer();
    bool RunClient(std::map<unsigned long long, unsigned int> *pProcInodeList,
                   std::map<unsigned int, std::string> *procCommand);

    std::string     m_error;
    unsigned int    m_syncCounter;
    __pid_t         m_processId;

private:
    std::string     m_fifoNameSrv;
    int             m_fifoSrv;
    CBuffer         m_buffer;

    static bool LoadProcessInodeList(unsigned int pid, int fifoSrv);
    static unsigned int GetSocketFromNameTypeA(const char *buf, size_t strLen);
    static unsigned int GetSocketFromNameTypeB(const char *buf, size_t strLen);

    static void makeCmdStr(unsigned int pid, int fifoSrv);

    static bool writeFifo(int fifo, const char *pBuffer, size_t size);
    static int readFifo(int fifo, CBuffer *pBuffer);

};




#endif // CROOTMODULE_H
