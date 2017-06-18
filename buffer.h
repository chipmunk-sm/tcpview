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
