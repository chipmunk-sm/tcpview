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

#include "cportservicenames.h"

#include <netdb.h>
#include <cstring>

CPortServiceNames::CPortServiceNames()
{
    // *Thread safety MT-Unsafe
    //
    struct servent *sentry;
    setservent(0);
    while ( (sentry = getservent()) != nullptr )
    {
        if(std::strcmp(sentry->s_proto,"tcp") == 0)
            m_PortServiceNamesTcp.insert(std::pair<int, std::string>(ntohs(sentry->s_port), std::string(sentry->s_name)));
        else if(std::strcmp(sentry->s_proto,"udp") == 0)
            m_PortServiceNamesUdp.insert(std::pair<int, std::string>(ntohs(sentry->s_port), std::string(sentry->s_name)));
    }
    endservent();

}

void CPortServiceNames::GetServiceName(int port, char *buff, size_t buffLength, bool IsTcp)
{

    auto pNames = IsTcp ? &m_PortServiceNamesTcp : &m_PortServiceNamesUdp;

    auto it = pNames->find(port);
    if (it == pNames->end())
        std::snprintf(buff, buffLength, "%d", port);
    else
        std::snprintf(buff, buffLength, "%d (%s)", port, it->second.c_str());
}
