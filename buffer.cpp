
#include <stdlib.h>
#include <string.h>
#include <string>

#include "buffer.h"

CBuffer::CBuffer()
    : m_pBuffer(nullptr)
    , m_bufferSize(0)
{
}

CBuffer::~CBuffer()
{
    Clear();
}

void CBuffer::Clear()
{
    try
    {

        if(m_pBuffer != nullptr)
            free(m_pBuffer);

        m_pBuffer = nullptr;

        m_bufferSize = 0;

    }
    catch(...)
    {
        m_error = "Failed free a block allocated by malloc!";
    }
}

uint8_t *CBuffer::GetBufferPtr(size_t lBufferSize)
{

    if(m_bufferSize >= lBufferSize)
        return m_pBuffer;

    ldiv_t roundVal = ldiv(lBufferSize, CSTREAMBUFFER_MIN_INC_SIZE_BYTES);
    if(roundVal.rem > 0)
    {
        lBufferSize = ++roundVal.quot * CSTREAMBUFFER_MIN_INC_SIZE_BYTES;
    }

    if(CSTREAMBUFFER_MAX < lBufferSize)
    {
        m_error = "Maximum buffer size exceeded!";
        return nullptr;
    }

    try
    {

        auto pNew = (uint8_t*)malloc(lBufferSize);
        if(pNew == nullptr)
        {
            m_error = "malloc failed";
            return nullptr;
        }

        if(m_bufferSize > 0)
            memcpy(pNew, m_pBuffer, m_bufferSize);

        auto lRem = lBufferSize - m_bufferSize;
        if(lRem > 0)
            memset(pNew + m_bufferSize, 0, lRem);

        Clear();

        m_pBuffer        = pNew;
        m_bufferSize     = lBufferSize;

        return m_pBuffer;
    }
    catch(...)
    {
        m_error = "Unexpected error!";
        return nullptr;
    }
}

size_t CBuffer::GetBufferSize()
{
    return m_bufferSize;
}
