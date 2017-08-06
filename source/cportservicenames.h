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

#ifndef CPORTSERVICENAMES_H
#define CPORTSERVICENAMES_H

#include <map>

class CPortServiceNames
{
public:
    CPortServiceNames();
    void GetServiceName(int port, char *buff, size_t buffLength, bool IsTcp);

    std::map<int, std::string>                  m_PortServiceNamesTcp;
    std::map<int, std::string>                  m_PortServiceNamesUdp;

};

#endif // CPORTSERVICENAMES_H
