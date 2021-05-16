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

#include "rootmodule.h"

#include "defined.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <thread>
#include <signal.h>

//Read-only, without O_NONBLOCK: open blocks until another process opens the FIFO for writing.
//Write-only, without O_NONBLOCK: open blocks until another process opens the FIFO for reading.
//Read-only, with O_NONBLOCK: open returns immediately.
//Write-only, with O_NONBLOCK: open returns an error with errno set to ENXIO
//unless another process has the FIFO open for reading.

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

CRootModule::CRootModule(__pid_t processId, std::string)
    : m_syncCounter(0)
    , m_processId(processId)
    , m_fifoNameSrv("/tmp/TcpView_2021")
    , m_fifoNameSrvRun(m_fifoNameSrv + "_run")
    , m_fifoSrv(-1)
    , m_abort(false)
{

    int timeout = TIMEOUT_SERVER_START;

    auto serverMode = m_processId > 0;//server - child  process with root access
    if (!serverMode)
    {

        unlink(m_fifoNameSrv.c_str());

        if (mkfifo(m_fifoNameSrv.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
        {
            setAbort();
            return;
        }

        if ((m_fifoSrv = open(m_fifoNameSrv.c_str(), O_RDONLY|O_TRUNC|O_NONBLOCK)) <= 0)
        {
            unlink(m_fifoNameSrv.c_str());
            setAbort();
        }
    }
    else
    {

        unlink(m_fifoNameSrvRun.c_str());
        if (mkfifo(m_fifoNameSrvRun.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
        {
            setAbort();
            return;
        }
        chmod(m_fifoNameSrvRun.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);


        //        chmod(m_fifoNameSrv.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

        while ((m_fifoSrv = open(m_fifoNameSrv.c_str(), O_WRONLY|O_TRUNC)) <= 0)
        {
            if(!timeout--)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (m_fifoSrv <= 0)
            setAbort();
    }
}

CRootModule::~CRootModule()
{
    setAbort();
}

void CRootModule::RunClient(
        std::map<unsigned long long, unsigned int> *pProcInodeList,
        std::map<unsigned int, std::string> *procCommand)
{

    if (m_fifoSrv <= 0 || isAbort())
    {
        setAbort();
        return;
    }

    while(true && !isAbort())
    {
        auto retv = ReadFifo(m_fifoSrv, &m_buffer);
        if (retv <= 0)
            return;

        auto const itemInf = reinterpret_cast<ItemInfo*>(m_buffer.GetBufferPtr(sizeof(ItemInfo) + _POSIX_PATH_MAX + 1));
        if (itemInf->startcode1 != DEF_STARTCODE || itemInf->startcode1 != DEF_STARTCODE)
        {
            setAbort();
            return;
        }

        switch (itemInf->command)
        {
        case ItemType_ERROR:
        default:
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        case ItemType_none:
            continue;
        case ItemType_end:
            return;
        case ItemType_inode:
            {
                auto inodePtr = reinterpret_cast<unsigned int*>(itemInf + 1);

                for (unsigned int inodeInd = 0; inodeInd < itemInf->dataCount/sizeof(unsigned int); inodeInd++)
                    pProcInodeList->insert(std::pair<unsigned int, unsigned int>(*inodePtr++, itemInf->procId));

                continue;
            }
        case ItemType_cmd:
            {
                if (itemInf->dataCount > 0 )
                {
                    auto dataPtr = reinterpret_cast<char*>(itemInf + 1);

                    if(itemInf->dataCount > _POSIX_PATH_MAX)
                        itemInf->dataCount = _POSIX_PATH_MAX;

                    dataPtr[itemInf->dataCount - 1] = 0;
                    procCommand->insert(std::pair<unsigned int, std::string>(itemInf->procId, std::string(dataPtr)));
                }
                continue;
            }
        }
    }
}

bool CRootModule::isAbort() const
{
    return m_abort;
}

void CRootModule::setAbort()
{
    if (m_fifoSrv)
    {
        close(m_fifoSrv);
        m_fifoSrv = -1;
    }

//    auto serverMode = m_processId > 0;//server - child  process with root access
//    if (!serverMode)
    unlink(m_fifoNameSrv.c_str());
    unlink(m_fifoNameSrvRun.c_str());

    m_abort = true;
}

void CRootModule::RunServer()
{
    char bufferPtr[sizeof(ItemInfo)];
    auto const inf  = reinterpret_cast<ItemInfo*>(bufferPtr);
    inf->dataCount  = 0;
    inf->startcode1 = DEF_STARTCODE;
    inf->startcode2 = DEF_STARTCODE;

    inf->procId     = 0;
    auto bActive = true;
    while (bActive)
    {
        inf->command = ItemType_none;
        if (!WriteFifo(m_fifoSrv, bufferPtr, sizeof(ItemInfo)))
            break;

        struct dirent *ent;
        auto dir = opendir(PROC_PATH);
        while (( ent = readdir(dir) ))
        {
            if ((*ent->d_name < '0') || (*ent->d_name> '9'))
                continue;
            auto procId = strtoul(ent->d_name, nullptr, 0);
            if (!LoadProcessInodeList(static_cast<unsigned int>(procId), m_fifoSrv))
            {
                bActive = false;
                break;
            }
        }

        closedir(dir);

        if (!bActive)
            break;

        inf->command = ItemType_end;
        if (!WriteFifo(m_fifoSrv, bufferPtr, sizeof(ItemInfo)))
            break;

    } // while(bActive)

    setAbort();

}

bool CRootModule::LoadProcessInodeList(unsigned int pid, int fifoSrv)
{

    int retCount = 0;
    char bufferPtr[_POSIX_PATH_MAX + sizeof(ItemInfo) + 1];
    auto bufferSize = sizeof(bufferPtr);
    snprintf(bufferPtr, bufferSize, PROC_PATH_FD, pid);

    auto fddir = opendir (bufferPtr);
    if (fddir == nullptr)
        return true;

    auto const inf  = reinterpret_cast<ItemInfo*>(bufferPtr);
    inf->procId     = pid;
    inf->command    = ItemType_inode;
    inf->dataCount  = 0;
    inf->startcode1 = DEF_STARTCODE;
    inf->startcode2 = DEF_STARTCODE;

    auto dataptr = reinterpret_cast<unsigned int*>(inf + 1);

    struct dirent *result;
    while ((result = readdir(fddir)) != nullptr)
    {
        if (result->d_type != DT_LNK)
            continue;

        char pathBuf[_POSIX_PATH_MAX];
        auto retl = snprintf(pathBuf, sizeof(pathBuf), PROC_PATH_FD2, pid, result->d_name);
        if (retl < 1)
            continue;

        char tmpBuf[_POSIX_PATH_MAX];
        memset(tmpBuf, 0, sizeof(tmpBuf));
        auto retlen = readlink(pathBuf, tmpBuf, sizeof(tmpBuf) - 1);
        if ( (retlen < 4) || retlen > ssize_t(sizeof(tmpBuf) - 1) )
            continue;

        tmpBuf[retlen] = 0x0;

        auto inode = GetSocketFromNameTypeA(tmpBuf, static_cast<size_t>(retlen));
        if ( inode <= 0 )
            inode = GetSocketFromNameTypeB(tmpBuf,  static_cast<size_t>(retlen));

        if ( inode > 0 )
        {
            retCount++;
            dataptr[inf->dataCount++] = inode;
            if(inf->dataCount >= (_POSIX_PATH_MAX / sizeof(dataptr[0]) - 1) )
            {
                inf->dataCount *= sizeof(dataptr[0]);
                if(!WriteFifo(fifoSrv, bufferPtr, sizeof(ItemInfo) + inf->dataCount))
                {
                    closedir(fddir);
                    return false;
                }
                inf->dataCount = 0;
            }
        }
    }

    closedir(fddir);

    if (inf->dataCount > 0)
    {
        inf->dataCount *= sizeof(dataptr[0]);
        if (!WriteFifo(fifoSrv, bufferPtr, sizeof(ItemInfo) + inf->dataCount))
            return false;
    }

    if (retCount)
        GetCommandString(pid, fifoSrv);

    return true;

}

unsigned int CRootModule::GetSocketFromNameTypeA(const char *buf, size_t strLen)
{
    const char keyA[] = "socket:[";
    const size_t keyALen = sizeof(keyA) - 1;

    if (strLen < keyALen + 2)
        return 0;

    if (memcmp(buf, keyA, keyALen) != 0)
        return 0;

    if (buf[strLen - 1] != ']')
        return 0;

    const char *inodeStr = buf + keyALen;
    char *endP = nullptr;
    errno = 0;

    auto inodeVal = static_cast<unsigned int>(strtoul(inodeStr, &endP, 0));

    if (endP == (buf+strLen-1) && errno == 0)
        return inodeVal;

    return 0;
}

unsigned int CRootModule::GetSocketFromNameTypeB(const char *buf, size_t strLen)
{
    const char keyB[] = "[0000]:";
    const size_t keyBLen = sizeof(keyB) - 1;

    if (strLen < keyBLen + 1)
        return 0;

    if (memcmp(buf, keyB, keyBLen) != 0)
        return 0;

    const char *inodeStr = buf + keyBLen;
    char *endP = nullptr;
    errno = 0;

    auto inodeVal = static_cast<unsigned int>(strtoul(inodeStr, &endP, 0));

    if (endP == (buf+strLen) && errno == 0)
        return inodeVal;

    return 0;
}

void CRootModule::GetCommandString(unsigned int pid, int fifoSrv)
{

    char bufferPtr[_POSIX_PATH_MAX + sizeof(ItemInfo) + 1];
    auto bufferSize = sizeof(bufferPtr);

    snprintf(bufferPtr, bufferSize, PROC_PATH_CMD, pid);

    auto fd = fopen( bufferPtr, "r" );
    if (fd == nullptr)
        return;

    auto const inf  = reinterpret_cast<ItemInfo*>(bufferPtr);
    inf->procId     = pid;
    inf->command    = ItemType_cmd;
    inf->dataCount  = 0;
    inf->startcode1 = DEF_STARTCODE;
    inf->startcode2 = DEF_STARTCODE;

    auto dataPtr = reinterpret_cast<unsigned char*>(inf + 1);
    int c = EOF;
    while ((c = fgetc(fd)) != EOF && inf->dataCount < (_POSIX_PATH_MAX - 1))
    {
        *dataPtr++ = (static_cast<char>(c) == '\0' ||
                      static_cast<char>(c) == '\t' ||
                      static_cast<char>(c) == '\n' ||
                      static_cast<char>(c) == '\r' ) ? ' ' : static_cast<unsigned char>(c);
        inf->dataCount++;
    }

    fclose( fd );

    if (inf->dataCount < 1)
        return;

    *dataPtr = '\0';
    inf->dataCount++;

    WriteFifo(fifoSrv, bufferPtr, inf->dataCount + sizeof(ItemInfo));

}

bool CRootModule::WriteFifo(int fifo, const char *pBuffer, size_t size)
{
    while(size > 0)
    {
        auto written = write(fifo, pBuffer, size);
        if (written == static_cast<decltype(written)>(-1))
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        pBuffer += written;
        size    -= static_cast<decltype(size)>(written);
    }
    return true;
}

int CRootModule::ReadFifo(int fifo, CBuffer *pBuffer)
{
    auto bufferPtr  = pBuffer->GetBufferPtr(sizeof(ItemInfo) + _POSIX_PATH_MAX + 1);
    auto inf        = reinterpret_cast<ItemInfo*>(bufferPtr);
    inf->command    = ItemType_ERROR;
    inf->startcode1 = 0;
    inf->startcode2 = 0;

    int readFifoAttempt = 25;
    int output = 0;

    while (readFifoAttempt && !isAbort())
    {
        auto readRes = read(fifo, bufferPtr + output, sizeof(ItemInfo) - static_cast<uint>(output));
        if (readRes == static_cast<decltype(readRes)>(-1))
        {
            if (!isAbort() && errno == EINTR)
                continue;
            return -1;
        }

        output += readRes;

        if (output < static_cast<decltype(output)>(sizeof(ItemInfo)))
        {
            if(readRes == 0)
            {
                readFifoAttempt--;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            continue;
        }

        if (inf->startcode1 == DEF_STARTCODE && inf->startcode2 == DEF_STARTCODE)
            break;

        memmove(bufferPtr, bufferPtr + 1, static_cast<uint>(output));
        output--;

    }

    switch (inf->command)
    {
    case ItemType_cmd:
    case ItemType_inode:
        break;
    default:
    case ItemType_none:
    case ItemType_end:
        return output;
    }

    if(inf->dataCount > _POSIX_PATH_MAX)
        inf->dataCount = _POSIX_PATH_MAX;

    auto toread = inf->dataCount;
    while (toread > 0)
    {
        auto readRes = read(fifo, bufferPtr + output, toread);
        if (readRes == static_cast<decltype(readRes)>(-1))
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        output += readRes;
        toread -= readRes;
    }
    return output;
}
