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

#ifndef CBUFFER_H
#define CBUFFER_H

#include <stdio.h>
#include <cstdint>

#define CSTREAMBUFFER_MAX_100MB 1024 * 1024 * 100
#define CSTREAMBUFFER_MAX CSTREAMBUFFER_MAX_100MB

#define CSTREAMBUFFER_MIN_INC_SIZE_BYTES 1024

class CBuffer
{
public:
   CBuffer();
   ~CBuffer();

   void Clear();

   uint8_t* GetBufferPtr(size_t lBufferSize);
   size_t GetBufferSize();

private:
   uint8_t* m_pBuffer;
   size_t m_bufferSize;
   std::string m_error;

};

#endif // CBUFFER_H
