#ifndef PROCESSLOADER_H
#define PROCESSLOADER_H

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
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <uuid/uuid.h>
#include <atomic>

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

    }SocketInfo;

    void UpdateTable();

    std::unordered_map<std::string, SocketInfo> *GetConnectionsList();
    static std::string makeUserNameStr(__uid_t euid);

    bool InitRootLoader();
    void DeleteRootLoader();

private:
    unsigned int m_loadCycles;
    std::map<eNetType, std::string>             m_eNetTypeList;
    std::map<int, std::string>                  m_PortServiceNamesTcp;
    std::map<int, std::string>                  m_PortServiceNamesUdp;
    std::unordered_map<std::string, SocketInfo> m_socketList;
    CRootModule                                *m_rootModule;
    std::atomic_bool                            m_enableRootMod;

    static void getConnections(eNetType netType,
                               const char *commandLine,
                               std::unordered_map<std::string, SocketInfo> *pSocketList,
                               std::map<unsigned long long, unsigned int> *procInodeList,
                               std::map<unsigned int, std::string> *procCommand,
                               std::map<int, std::string> *pPortServiceNamesTcp,
                               std::map<int, std::string> *pPortServiceNamesUdp,
                               unsigned int loadCycles);

    static bool FillCommand(unsigned long long inode,
                             std::map<unsigned int, std::string> *procCommand,
                             std::map<unsigned long long, unsigned int> *procInodeList,
                             char *pBuff,
                             size_t bufferLen);

    static void serviceName(int port, char *buff, size_t buffLength, std::map<int, std::string> *pNames);
    static bool run_posix_spawn(char *pPath, char *pArg);
};

#endif // PROCESSLOADER_H
