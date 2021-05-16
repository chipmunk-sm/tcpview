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
#include <source/buffer.h>

#define DEF_STARTCODE 0x010101010
#define TIMEOUT_SERVER_START 1000

class CRootModule
{
public:
    CRootModule(__pid_t processId, std::string fifoName);
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
    void RunClient(std::map<unsigned long long, unsigned int> *pProcInodeList,
                   std::map<unsigned int, std::string> *procCommand);

    bool isAbort() const;
    void setAbort();

private:
    unsigned int    m_syncCounter;
    __pid_t         m_processId;
    std::string     m_fifoNameSrv;
    std::string     m_fifoNameSrvRun;
    int             m_fifoSrv;
    CBuffer         m_buffer;
    bool            m_abort;

    bool LoadProcessInodeList(unsigned int pid, int fifoSrv);
    unsigned int GetSocketFromNameTypeA(const char *buf, size_t strLen);
    unsigned int GetSocketFromNameTypeB(const char *buf, size_t strLen);

    void GetCommandString(unsigned int pid, int fifoSrv);

    bool WriteFifo(int fifo, const char *pBuffer, size_t size);
    int ReadFifo(int fifo, CBuffer *pBuffer);

};




#endif // CROOTMODULE_H
