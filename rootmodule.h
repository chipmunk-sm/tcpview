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
