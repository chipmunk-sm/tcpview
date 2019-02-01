/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2019 chipmunk-sm <dannico@linuxmail.org>
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

#ifndef DEFINED_H
#define DEFINED_H


#define PROC_NET_TCP  "/proc/net/tcp"
#define PROC_NET_UDP  "/proc/net/udp"
#define PROC_NET_TCP6 "/proc/net/tcp6"
#define PROC_NET_UDP6 "/proc/net/udp6"
#define PROC_NET_RAW  "/proc/net/raw"
#define PROC_NET_RAW6 "/proc/net/raw6"

#define PROC_PATH     "/proc"
#define PROC_PATH_FD  "/proc/%d/fd"
#define PROC_PATH_FD2 "/proc/%d/fd/%s"
#define PROC_PATH_CMD "/proc/%d/cmdline"


#endif // DEFINED_H
