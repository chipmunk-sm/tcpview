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

#ifndef CPORTSERVICENAMES_H
#define CPORTSERVICENAMES_H

#include <map>
#include <string>

class CPortServiceNames
{
public:
    CPortServiceNames();
    void getServiceName(int port, char *buff, size_t buffLength, bool IsTcp);
private:
    std::map<int, std::string> m_portTcp;
    std::map<int, std::string> m_portUdp;
};

#endif // CPORTSERVICENAMES_H
